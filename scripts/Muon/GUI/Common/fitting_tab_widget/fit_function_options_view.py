# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2021 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
from mantidqt.utils.qt import load_ui
from mantidqt.widgets.functionbrowser import FunctionBrowser

from Muon.GUI.Common.utilities import table_utils

from qtpy import QtWidgets, QtCore

ui_fit_function_options, _ = load_ui(__file__, "fit_function_options.ui")

ALLOWED_MINIMIZERS = ["Levenberg-Marquardt", "BFGS", "Conjugate gradient (Fletcher-Reeves imp.)",
                      "Conjugate gradient (Polak-Ribiere imp.)", "Damped GaussNewton",
                      "Levenberg-MarquardtMD", "Simplex", "SteepestDescent", "Trust Region"]
DEFAULT_FREQUENCY_FIT_END_X = 250
FIT_START_TABLE_ROW = 0
FIT_END_TABLE_ROW = 1
MINIMIZER_TABLE_ROW = 2
RAW_DATA_TABLE_ROW = 3
EVALUATE_AS_TABLE_ROW = 4


class FitFunctionOptionsView(QtWidgets.QWidget, ui_fit_function_options):

    def __init__(self, parent=None, is_frequency_domain=False):
        super(FitFunctionOptionsView, self).__init__(parent)
        self.setupUi(self)

        self.start_x = None
        self.end_x = None
        self.minimizer_combo = None
        self.fit_to_raw_data_checkbox = None
        self.evaluation_combo = None

        self._setup_fit_options_table()

        self.function_browser = FunctionBrowser(self, True)
        self.function_browser_layout.addWidget(self.function_browser)
        self.function_browser.setErrorsEnabled(True)
        self.function_browser.hideGlobalCheckbox()

        if is_frequency_domain:
            self.fit_options_table.hideRow(RAW_DATA_TABLE_ROW)
            table_utils.setRowName(self.fit_options_table, FIT_START_TABLE_ROW, "Start X")
            table_utils.setRowName(self.fit_options_table, FIT_END_TABLE_ROW, "End X")
            self.end_time = DEFAULT_FREQUENCY_FIT_END_X

    def set_slot_for_fit_name_changed(self, slot):
        """Connect the slot for the fit name being changed by the user."""
        self.function_name_line_edit.textChanged.connect(slot)

    def set_slot_for_function_structure_changed(self, slot):
        """Connect the slot for the function structure changing."""
        self.function_browser.functionStructureChanged.connect(slot)

    def set_slot_for_function_parameter_changed(self, slot):
        """Connect the slot for a function parameter changing."""
        self.function_browser.parameterChanged.connect(slot)

    def set_slot_for_start_x_updated(self, slot):
        """Connect the slot for the start x option."""
        self.start_x.editingFinished.connect(slot)

    def set_slot_for_end_x_updated(self, slot):
        """Connect the slot for the end x option."""
        self.end_x.editingFinished.connect(slot)

    def set_slot_for_minimizer_changed(self, slot):
        """Connect the slot for changing the Minimizer."""
        self.minimizer_combo.currentIndexChanged.connect(slot)

    def set_slot_for_evaluation_type_changed(self, slot):
        """Connect the slot for changing the Evaluation type."""
        self.evaluation_combo.currentIndexChanged.connect(slot)

    def set_slot_for_use_raw_changed(self, slot):
        """Connect the slot for the Use raw option."""
        self.fit_to_raw_data_checkbox.stateChanged.connect(slot)

    def set_datasets_in_function_browser(self, dataset_names):
        """Sets the datasets stored in the FunctionBrowser."""
        index_list = range(self.function_browser.getNumberOfDatasets())
        self.function_browser.removeDatasets(index_list)
        self.function_browser.addDatasets(dataset_names)

    def set_current_dataset_index(self, dataset_index):
        """Sets the index of the current dataset."""
        self.function_browser.setCurrentDataset(dataset_index)

    def update_function_browser_parameters(self, is_simultaneous_fit, fit_function):
        """Updates the parameters in the function browser."""
        if is_simultaneous_fit:
            self.function_browser.blockSignals(True)
            self.function_browser.updateMultiDatasetParameters(fit_function)
            self.function_browser.blockSignals(False)
        else:
            self.function_browser.blockSignals(True)
            self.function_browser.updateParameters(fit_function)
            self.function_browser.blockSignals(False)
        self.function_browser.setErrorsEnabled(True)

    @property
    def fit_object(self):
        """Returns the global fitting function."""
        return self.function_browser.getGlobalFunction()

    @property
    def minimizer(self):
        """Returns the selected minimizer."""
        return str(self.minimizer_combo.currentText())

    @property
    def start_time(self):
        """Returns the selected start X."""
        return float(self.start_x.text())

    @start_time.setter
    def start_time(self, value):
        """Sets the selected start X."""
        self.start_x.setText(str(value))

    @property
    def end_time(self):
        """Returns the selected end X."""
        return float(self.end_x.text())

    @end_time.setter
    def end_time(self, value):
        """Sets the selected end X."""
        self.end_x.setText(str(value))

    @property
    def evaluation_type(self):
        """Returns the selected evaluation type."""
        return str(self.evaluation_combo.currentText())

    @property
    def fit_to_raw(self):
        """Returns whether or not fitting to raw data is ticked."""
        return self.fit_to_raw_data_checkbox.isChecked()

    @fit_to_raw.setter
    def fit_to_raw(self, value):
        """Sets whether or not you are fitting to raw data."""
        self.fit_to_raw_data_checkbox.setCheckState(QtCore.Qt.Checked if value else QtCore.Qt.Unchecked)

    @property
    def function_name(self):
        """Returns the function name being used."""
        return str(self.function_name_line_edit.text())

    @function_name.setter
    def function_name(self, function_name):
        """Sets the function name being used."""
        self.function_name_line_edit.blockSignals(True)
        self.function_name_line_edit.setText(function_name)
        self.function_name_line_edit.blockSignals(False)

    def get_global_parameters(self):
        """Returns a list of global parameters."""
        return self.function_browser.getGlobalParameters()

    def switch_to_simultaneous(self):
        """Switches the view to simultaneous mode."""
        self.function_browser.showGlobalCheckbox()

    def switch_to_single(self):
        """Switches the view to single mode."""
        self.function_browser.hideGlobalCheckbox()

    def _setup_fit_options_table(self):
        """Setup the fit options table with the appropriate options."""
        self.fit_options_table.setRowCount(5)
        self.fit_options_table.setColumnCount(2)
        self.fit_options_table.setColumnWidth(0, 150)
        self.fit_options_table.setColumnWidth(1, 300)
        self.fit_options_table.verticalHeader().setVisible(False)
        self.fit_options_table.horizontalHeader().setStretchLastSection(True)
        self.fit_options_table.setHorizontalHeaderLabels(["Property", "Value"])

        table_utils.setRowName(self.fit_options_table, FIT_START_TABLE_ROW, "Time Start")
        self.start_x = table_utils.addDoubleToTable(self.fit_options_table, 0.0, FIT_START_TABLE_ROW, 1)

        table_utils.setRowName(self.fit_options_table, FIT_END_TABLE_ROW, "Time End")
        self.end_x = table_utils.addDoubleToTable(self.fit_options_table, 15.0, FIT_END_TABLE_ROW, 1)

        table_utils.setRowName(self.fit_options_table, MINIMIZER_TABLE_ROW, "Minimizer")
        self.minimizer_combo = table_utils.addComboToTable(self.fit_options_table, MINIMIZER_TABLE_ROW, [])
        self.minimizer_combo.addItems(ALLOWED_MINIMIZERS)

        table_utils.setRowName(self.fit_options_table, RAW_DATA_TABLE_ROW, "Fit To Raw Data")
        self.fit_to_raw_data_checkbox = table_utils.addCheckBoxWidgetToTable(
            self.fit_options_table, True, RAW_DATA_TABLE_ROW)

        table_utils.setRowName(self.fit_options_table, EVALUATE_AS_TABLE_ROW, "Evaluate Function As")
        self.evaluation_combo = table_utils.addComboToTable(self.fit_options_table, EVALUATE_AS_TABLE_ROW,
                                                            ['CentrePoint', 'Histogram'])