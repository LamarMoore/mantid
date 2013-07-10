# Forced to use version 1 API.
IS_IN_MANTIDPLOT = False
try:
    import mantidplot
    from MantidFramework import *
    mtd.initialise(False)
    from mantidsimple import *
    import mantidplot

    IS_IN_MANTIDPLOT = True
except:
    pass

import os

class InstrumentParameters(object):
    instrument_name = None
    _instrument = None
    
    def __init__(self, inst_name):
        if self.instrument_name is None:
            self._load_param_file(inst_name)
        else:
            if self.instrument_name != inst_name:
                self._load_param_file(inst_name)
                
    def _load_param_file(self, inst_name):
        InstrumentParameters.instrument_name = inst_name
        ws_name = "__emptyInst_" + inst_name
        if IS_IN_MANTIDPLOT:
            idf_loc = ConfigService().getInstrumentDirectory()
            idf_pattern = os.path.join(idf_loc, "%s_Definition*.xml") % inst_name
            import glob
            idf_files = glob.glob(idf_pattern)
            output = LoadEmptyInstrument(Filename=idf_files[0], 
                                         OutputWorkspace=ws_name)
            InstrumentParameters._instrument = output.workspace().getInstrument()

    def _self_check(self):
        if self._instrument is None:
            raise ValueError("Instrument was not loaded, cannot retrieve parameters.")

    def get_parameter(self, name):
        default = -1
        try:
            self._self_check()
        except ValueError:
            return default
    
        type_name = self._instrument.getParameterType(name)
        if type_name == "double":
            val = self._instrument.getNumberParameter(name)
        elif type_name == "bool":
            val = self._instrument.getBoolParameter(name)
        elif type_name == "string":
            val = self._instrument.getStringParameter(name)
            if val[0] == "None" : 
                return None        
        elif type_name == "int" :
              val = self._instrument.getIntParameter(name)
        else :
            return default
        try:
            return val[0]
        except IndexError:
            return default
          
        
    def get_bool_param(self, parname):
        default = False
        try:
            self._self_check()
        except ValueError:
            return default            

        param = self.get_parameter(parname)
        if param < 0 :
            return False
        return bool(param)
        