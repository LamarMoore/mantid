#pylint: disable=invalid-name
"""
    Simple local data catalog for Mantid
    Gets main information from data files in a directory and stores
    that information in a database in the user home area.
"""
from __future__ import (absolute_import, division, print_function)
import os
import sqlite3
import sys
import traceback

from multiprocessing import Process, Queue, Lock

# Only way that I have found to use the logger from both the command line
# and mantiplot
try:
    import mantidplot # noqa
    from mantid.kernel import logger
except ImportError:
    import logging
    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger("data_cat")


class DataType(object):
    TABLE_NAME = "datatype"

    # Data type names
    DATA_TYPES = {"FLOOD_FIELD":"Flood Field",
                  "DARK_CURRENT":"Dark Current",
                  "TRANS_SAMPLE":"Transmission Sample",
                  "TRANS_BCK":"Transmission Background",
                  "TRANS_DIRECT":"Transmission Empty"}

    @classmethod
    def create_table(cls, cursor, data_set_table):
        cursor.execute("""create table if not exists %s (
                            id integer primary key,
                            type_id integer,
                            dataset_id integer,
                            foreign key(dataset_id) references %s(id))"""
                       % (cls.TABLE_NAME, data_set_table))

    @classmethod
    def add(cls, dataset_id, type_id, cursor):
        """
            Add a data type entry to the datatype table
        """
        if type_id not in cls.DATA_TYPES.keys():
            raise RuntimeError("DataType got an unknown type ID: %s" % type_id)

        t = (type_id, dataset_id,)
        cursor.execute("insert into %s(type_id, dataset_id) values (?,?)"
                       % cls.TABLE_NAME, t)

    @classmethod
    def get_likely_type(cls, dataset_id, cursor):
        t = (dataset_id,)
        cursor.execute("select type_id from %s where dataset_id=?" % cls.TABLE_NAME, t)
        rows = cursor.fetchall()
        if len(rows)>1:
            return cls.DATA_TYPES[rows[len(rows)-1][0]]
        return None


class DataSet(object):
    TABLE_NAME = "dataset"
    data_type_cls = DataType

    def __init__(self, run_number, title, run_start, duration, sdd, id=None):
        self.run_number = run_number
        self.title = title
        self.run_start = run_start
        self.duration = duration
        self.sdd = sdd
        self.id = id

    @classmethod
    def header(cls):
        """
            Return the column headers
        """
        return "%-6s %-60s %-16s %-7s %-10s" % ("Run", "Title", "Start", "Time[s]", "SDD[mm]")

    @classmethod
    def load_meta_data(cls, file_path, outputWorkspace):
        """
            Load method that needs to be implemented for each catalog/instrument
        """
        return False

    @classmethod
    def handle(cls, file_path):
        """
            Return a DB handle for the given file, such as a run number
        """
        return file_path

    @classmethod
    def read_properties(cls, ws, run, cursor):
        return None

    def __str__(self):
        """
            Pretty print the current data set attributes
        """
        return "%-6s %-60s %-16s %-7g %-10.0f" % (self.run_number, self.title, self.run_start, self.duration, self.sdd)

    def as_list(self):
        """
            Return a list of data set attributes
        """
        return (self.run_number, self.title, self.run_start, self.duration, self.sdd)

    def as_string_list(self):
        """
            Return a list of data set attributes as strings
        """
        return (str(self.run_number), self.title, self.run_start, "%-g"%self.duration, "%-10.0f"%self.sdd)

    @classmethod
    def get_data_set_id(cls, run, cursor):
        t = (run,)
        cursor.execute('select * from %s where run=?'% cls.TABLE_NAME, t)
        rows = cursor.fetchall()
        if len(rows) == 0:
            return -1
        else:
            return rows[0][0]

    @classmethod
    def find(cls, file_path, conn, process_files=True, queue=None, lock=None):
        """
            Find an entry in the database, or create on as needed
        """
        run = cls.handle(file_path)
        if run is None:
            logger.warning("Could not get run number from file {}.".format(file_path))
            return

        t = (run,)
        
        # First checks if there are entries in the DB for this run
        lock.acquire()
        try:
            with conn:
                cursor = conn.cursor()
                cursor.execute('select * from %s where run=?'% cls.TABLE_NAME, t)
                rows = cursor.fetchall()
                cursor.close()
        except sqlite3.OperationalError as e:
            logger.error("DataCatalog: Error while looking up on the DB:\n{}".format(str(traceback.format_exc())))
            logger.error(e)
        lock.release()
        
        if len(rows) > 0:
            logger.debug("Got run info from the database...")
            row = rows[0]
            queue.put(DataSet(row[1], row[2], row[3], row[4], row[5], id=row[0]))
        elif process_files:
            logger.debug("Processing file: {}.".format(file_path))
            log_ws = os.path.basename(file_path)
            ds =  cls.load_meta_data(file_path, run, out_ws_name=log_ws)
            
            lock.acquire()
            try:
                with conn:
                    cursor = conn.cursor()
                    ds.insert_in_db(cursor)
                    cursor.close()
            except sqlite3.OperationalError as e:
                logger.error("DataCatalog: Error while inserting on the DB:\n{}".format(str(traceback.format_exc())))
            lock.release()
            queue.put(ds)
                
    @classmethod
    def create_table(cls, cursor):
        cursor.execute("""create table if not exists %s (
                            id integer primary key,
                            run text unique,
                            title text,
                            start text,
                            duration real, sdd real)""" % cls.TABLE_NAME)

        cls.data_type_cls.create_table(cursor, cls.TABLE_NAME)

    def insert_in_db(self, cursor):
        t = (self.run_number, self.title, self.run_start, self.duration, self.sdd,)
        cursor.execute('insert into %s(run, title, start, duration,sdd) values (?,?,?,?,?)'%self.TABLE_NAME, t)
        return cursor.lastrowid


class DataCatalog(object):
    """
        Data catalog
    """
    extension = ["nxs"]
    data_set_cls = DataSet

    def __init__(self, replace_db=True):
        ## List of data sets
        self.catalog = []

        # Connect/create to DB
        db_path = os.path.join(os.path.expanduser("~"), ".mantid_data_sets")
        self.db_exists = False
        self.conn = None

        try:
            self._create_db(db_path, replace_db)
        except Exception as msg:
            logger.error("DataCatalog: Could not access local data catalog:\n{}".format(sys.exc_info()[1]))


    def _create_db(self, db_path, replace_db):
        """
            Create the database if we need to
        """
        if os.path.isfile(db_path):
            if replace_db:
                os.remove(db_path)

        self.conn = sqlite3.connect(db_path)
        cursor = self.conn.cursor()
        self.data_set_cls.create_table(cursor)
        self.conn.commit()
        cursor.close()

    def __str__(self):
        """
            Pretty print the whole list of data
        """
        output = "%s\n" % self.data_set_cls.header()
        for r in self.catalog:
            #data_cat
            output += "%s\n" % str(r)
        return output

    def size(self):
        """
            Return size of the catalog
        """
        return len(self.catalog)

    def get_list(self, data_dir=None):
        """
            Get list of catalog entries
        """
        self.list_data_sets(data_dir, process_files=False)
        output = []
        for r in self.catalog:
            output.append(r.as_list())
        return output

    def get_string_list(self, data_dir=None):
        """
            Get list of catalog entries
        """
        self.list_data_sets(data_dir, process_files=False)
        output = []
        for r in self.catalog:
            output.append(r.as_string_list())
        return output

    def add_type(self, run, type):
        if self.conn is None:
            print ("DataCatalog: Could not access local data catalog")
            return

        c = self.conn.cursor()
        id = self.data_set_cls.get_data_set_id(run, c)
        if id>0:
            self.data_set_cls.data_type_cls.add(id, type, c)

        self.conn.commit()
        c.close()
        


    def append_data_sets(self, queue, call_back, lock):
        '''
        This can be seen as a consumer
        '''
        logger.debug("Queue started...",)
        while True:
            d = queue.get()
            if d is None:
                # Poison pill means shutdown
                
                queue.close()
                logger.debug("Queue exiting...",)
                break
            logger.debug("Queue got: {}".format(d))
            if call_back is not None:
                attr_list = d.as_string_list()
                
                lock.acquire()
                try:
                    with self.conn:
                        cursor = self.conn.cursor()
                        type_id = self.data_set_cls.data_type_cls.get_likely_type(d.id, cursor)
                        cursor.close()
                except sqlite3.OperationalError as e:
                    logger.error("DataCatalog (Queue): Error while inserting on the DB:\n{}".format(str(traceback.format_exc())))
                lock.release()
            
                
                attr_list += type_id, 
                call_back(attr_list)
            self.catalog.append(d)
        

    def list_data_sets(self, data_dir=None, call_back=None, process_files=True):
        """
            Process a data directory
        """
        self.catalog = []

        if self.conn is None:
            logger.info("DataCatalog: Could not access local data catalog")
            return

        if not os.path.isdir(data_dir):
            return
        
        logger.debug("Listing directory: {}".format(data_dir))

        try:
            jobs = []
            lock = Lock()
            queue = Queue()
            
            # starts the thread to process the queue
            qp = Process(target=self.append_data_sets, args=(queue, call_back, lock))
            qp.start()
            
            for f in os.listdir(data_dir):
                for extension in self.extension:
                    if f.endswith(extension):
                        file_path = os.path.join(data_dir, f)
                        # I guess this is never called (?)
                        if hasattr(self.data_set_cls, "find_with_api"):
                            d = self.data_set_cls.find_with_api(file_path,
                                                                process_files=process_files)
                        else:
                            p = Process(
                                target=self.data_set_cls.find,
                                # file_path, db_connection, process_files
                                args=(file_path, self.conn, process_files, queue, lock, )
                            )
                            jobs.append(p)
                            p.start()
                            
            
            
            # Waits for all jobs to finish
            for p in jobs:
                p.join()
        
            # All jobs finished, puts not in the queue: Poison pill
            # and waits for it to finish
            queue.put(None)
            qp.join()

        except Exception as msg:
            logger.error("DataCatalog: Error working with the local data catalog:\n{}".format(str(traceback.format_exc())))
