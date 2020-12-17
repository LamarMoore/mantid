# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
from mantidqt.utils.observer_pattern import Observer, Observable, GenericObservable, GenericObserver
from Muon.GUI.Common.utilities.run_string_utils import run_string_to_list


class EAGroupingTabPresenter(object):
    """
    The grouping tab presenter is responsible for synchronizing the group table.
    """

    @staticmethod
    def string_to_list(text):
        return run_string_to_list(text)

    def __init__(self, view, model,
                 grouping_table_widget=None):
        self._view = view
        self._model = model

        self.grouping_table_widget = grouping_table_widget

        self._view.set_description_text(self.text_for_description())

        # monitors for loaded data changing
        self.loadObserver = EAGroupingTabPresenter.LoadObserver(self)
        self.instrumentObserver = EAGroupingTabPresenter.InstrumentObserver(self)

        # notifiers
        self.groupingNotifier = EAGroupingTabPresenter.GroupingNotifier(self)
        self.enable_editing_notifier = EAGroupingTabPresenter.EnableEditingNotifier(self)
        self.disable_editing_notifier = EAGroupingTabPresenter.DisableEditingNotifier(self)
        self.calculation_finished_notifier = GenericObservable()

        self.message_observer = EAGroupingTabPresenter.MessageObserver(self)
        self.gui_variables_observer = EAGroupingTabPresenter.GuiVariablesChangedObserver(self)
        self.enable_observer = EAGroupingTabPresenter.EnableObserver(self)
        self.disable_observer = EAGroupingTabPresenter.DisableObserver(self)

        self.disable_tab_observer = GenericObserver(self.disable_editing_without_notifying_subscribers)
        self.enable_tab_observer = GenericObserver(self.enable_editing_without_notifying_subscribers)

        self.update_view_from_model_observer = GenericObserver(self.update_view_from_model)

    def update_view_from_model(self):
        self.grouping_table_widget.update_view_from_model()

    def show(self):
        self._view.show()

    def text_for_description(self):
        """
        Generate the text for the description edit at the top of the widget.
        """
        text = "\u03BCx: exp2k : file type .dat"
        return text

    def update_description_text(self, description_text=''):
        if not description_text:
            description_text = self.text_for_description()
        self._view.set_description_text(description_text)

    def disable_editing(self):
        self.grouping_table_widget.disable_editing()
        self.disable_editing_notifier.notify_subscribers()

    def enable_editing(self):
        self.grouping_table_widget.enable_editing()
        self.enable_editing_notifier.notify_subscribers()

    def disable_editing_without_notifying_subscribers(self):
        self.grouping_table_widget.disable_editing()

    def enable_editing_without_notifying_subscribers(self):
        self.grouping_table_widget.enable_editing()

    def error_callback(self, error_message):
        self.enable_editing()
        self._view.display_warning_box(error_message)

    def handle_update_finished(self):
        self.enable_editing()
        self.groupingNotifier.notify_subscribers()
        self.calculation_finished_notifier.notify_subscribers()

    def on_clear_requested(self):
        self._model.clear()
        self.grouping_table_widget.update_view_from_model()
        self.update_description_text()

    def handle_new_data_loaded(self):
        if self._model.is_data_loaded():
            self.update_view_from_model()
            self.update_description_text()
            self.plot_default_groups()
        else:
            self.on_clear_requested()

    def plot_default_groups(self):
        # if we have no groups selected, generate a default plot
        if len(self._model.selected_groups) == 0:
            self.grouping_table_widget.plot_default_case()

    # ------------------------------------------------------------------------------------------------------------------
    # Observer / Observable
    # ------------------------------------------------------------------------------------------------------------------

    class LoadObserver(Observer):

        def __init__(self, outer):
            Observer.__init__(self)
            self.outer = outer

        def update(self, observable, arg):
            self.outer.handle_new_data_loaded()

    class InstrumentObserver(Observer):

        def __init__(self, outer):
            Observer.__init__(self)
            self.outer = outer

        def update(self, observable, arg):
            self.outer.on_clear_requested()

    class GuiVariablesChangedObserver(Observer):
        def __init__(self, outer):
            Observer.__init__(self)
            self.outer = outer

    class GroupingNotifier(Observable):

        def __init__(self, outer):
            Observable.__init__(self)
            self.outer = outer  # handle to containing class

        def notify_subscribers(self, *args, **kwargs):
            Observable.notify_subscribers(self, *args, **kwargs)

    class MessageObserver(Observer):

        def __init__(self, outer):
            Observer.__init__(self)
            self.outer = outer

        def update(self, observable, arg):
            self.outer._view.display_warning_box(arg)

    class EnableObserver(Observer):
        def __init__(self, outer):
            Observer.__init__(self)
            self.outer = outer

        def update(self, observable, arg):
            self.outer.enable_editing()

    class DisableObserver(Observer):
        def __init__(self, outer):
            Observer.__init__(self)
            self.outer = outer

        def update(self, observable, arg):
            self.outer.disable_editing()

    class DisableEditingNotifier(Observable):

        def __init__(self, outer):
            Observable.__init__(self)
            self.outer = outer  # handle to containing class

        def notify_subscribers(self, *args, **kwargs):
            Observable.notify_subscribers(self, *args, **kwargs)

    class EnableEditingNotifier(Observable):

        def __init__(self, outer):
            Observable.__init__(self)
            self.outer = outer  # handle to containing class

        def notify_subscribers(self, *args, **kwargs):
            Observable.notify_subscribers(self, *args, **kwargs)