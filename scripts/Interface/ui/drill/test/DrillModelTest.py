# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +

import unittest
import mock

from mantid.kernel import *
from mantid.api import *

from Interface.ui.drill.model.DrillModel import DrillModel, DrillException


class DrillModelTest(unittest.TestCase):

    TECHNIQUE = {
            "i1" : "t1"
            }

    ACQUISITION_MODES = {
            "i1": ["a1", "a2"]
            }

    COLUMNS = {
            "a1": ["c1", "c2", "CustomOptions"],
            "a2": ["c3", "c4"]
            }

    ALGORITHM = {
            "a1": "a1",
            "a2": "a2"
            }

    SETTINGS = {
            "a1": {
                "str": "test",
                "int": 1,
                "float": "0.9",
                "bool": False
                },
            "a2": {
                "test": "test"
                }
            }

    def setUp(self):
        # mock sapi
        patch = mock.patch('Interface.ui.drill.model.DrillModel.sapi')
        self.mSapi = patch.start()
        self.addCleanup(patch.stop)

        # mock open
        patch = mock.patch('Interface.ui.drill.model.DrillModel.open')
        self.mOpen = patch.start()
        self.addCleanup(patch.stop)

        # mock json
        patch = mock.patch('Interface.ui.drill.model.DrillModel.json')
        self.mJson = patch.start()
        self.addCleanup(patch.stop)

        # mock parameter controller
        patch = mock.patch(
                'Interface.ui.drill.model.DrillModel.ParameterController'
                )
        self.mController = patch.start()
        self.addCleanup(patch.stop)

        # mock config
        patch = mock.patch('Interface.ui.drill.model.DrillModel.config')
        self.mConfig = patch.start()
        self.mConfig.__getitem__.return_value = "i1"
        self.addCleanup(patch.stop)

        # mock specifications
        patch = mock.patch.dict(
                'Interface.ui.drill.model.DrillModel.RundexSettings.TECHNIQUE',
                self.TECHNIQUE, clear=True
                )
        self.mTechniques = patch.start()
        self.addCleanup(patch.stop)

        patch = mock.patch.dict(
                'Interface.ui.drill.model.DrillModel.RundexSettings.ACQUISITION_MODES',
                self.ACQUISITION_MODES, clear=True
                )
        self.mAcq = patch.start()
        self.addCleanup(patch.stop)

        patch = mock.patch.dict(
                'Interface.ui.drill.model.DrillModel.RundexSettings.COLUMNS',
                self.COLUMNS, clear=True
                )
        self.mColumns = patch.start()
        self.addCleanup(patch.stop)

        patch = mock.patch.dict(
                'Interface.ui.drill.model.DrillModel.RundexSettings.ALGORITHM',
                self.ALGORITHM, clear=True
                )
        self.mAlgo = patch.start()
        self.addCleanup(patch.stop)

        patch = mock.patch.dict(
                'Interface.ui.drill.model.DrillModel.RundexSettings.SETTINGS',
                self.SETTINGS, clear=True
                )
        self.mSettings = patch.start()
        self.addCleanup(patch.stop)

        # mock drill task
        patch = mock.patch('Interface.ui.drill.model.DrillModel.DrillTask')
        self.mDrillTask = patch.start()
        self.addCleanup(patch.stop)

        # mock drill tasks pool
        patch = mock.patch(
                'Interface.ui.drill.model.DrillModel.DrillAlgorithmPool'
                )
        self.mTasksPool = patch.start()
        self.addCleanup(patch.stop)

        self.model = DrillModel()

    def test_setInstrument(self):
        # invalid instrument
        self.mController.reset_mock()
        self.model.setInstrument("test")
        self.assertIsNone(self.model.instrument)
        self.assertIsNone(self.model.acquisitionMode)
        self.assertEqual(self.model.columns, list())
        self.assertIsNone(self.model.algorithm)
        self.assertEqual(self.model.settings, dict())
        self.mController.assert_not_called()

        # valid instrument
        self.mController.reset_mock()
        self.model.setInstrument("i1")
        self.assertEqual(self.model.instrument, "i1")
        self.assertEqual(self.model.acquisitionMode,
                         self.ACQUISITION_MODES["i1"][0])
        self.assertEqual(self.model.columns, self.COLUMNS["a1"])
        self.assertEqual(self.model.algorithm, self.ALGORITHM["a1"])
        self.mController.assert_called_once()

    def test_getInstrument(self):
        self.assertEqual(self.model.getInstrument(), self.model.instrument)

    def test_getAvailableTechniques(self):
        self.assertEqual(self.model.getAvailableTechniques(), ["t1"])

    def test_setAcquisitionMode(self):
        # invalid aquisition mode
        self.mController.reset_mock()
        ac = self.model.acquisitionMode
        al = self.model.algorithm
        co = self.model.columns
        se = self.model.settings
        self.model.setAcquisitionMode("test_invalid")
        self.assertEqual(self.model.acquisitionMode, ac)
        self.assertEqual(self.model.algorithm, al)
        self.assertEqual(self.model.columns, co)
        self.assertEqual(self.model.settings, se)
        self.mController.assert_not_called()

        # valide acquisition mode
        self.mController.reset_mock()
        self.model.setAcquisitionMode("a2")
        self.assertNotEqual(self.model.acquisitionMode, ac)
        self.assertEqual(self.model.acquisitionMode, "a2")
        self.assertNotEqual(self.model.algorithm, al)
        self.assertEqual(self.model.algorithm, self.ALGORITHM["a2"])
        self.assertNotEqual(self.model.columns, co)
        self.assertEqual(self.model.columns, self.COLUMNS["a2"])
        self.assertNotEqual(self.model.settings, se)
        self.assertEqual(self.model.settings, self.SETTINGS["a2"])
        self.mController.assert_called_once()

    def test_getAcquisitionMode(self):
        self.assertEqual(self.model.getAcquisitionMode(),
                         self.model.acquisitionMode)

    def test_getAvailableAcquisitionModes(self):
        self.model.setInstrument("i1")
        self.assertEqual(self.model.getAvailableAcquisitionModes(),
                         ["a1", "a2"])
        self.model.setInstrument("test")
        self.assertEqual(self.model.getAvailableAcquisitionModes(), [])

    def test_initController(self):
        self.mController.reset_mock()
        self.model.algorithm = None
        self.model._initController()
        self.mController.assert_not_called()
        self.model.algorithm = "a1"
        self.model._initController()
        self.mController.assert_called_once_with("a1")

    def test_setSettings(self):
        self.assertEqual(self.model.settings, self.SETTINGS["a1"])
        self.model.setSettings({"str": "test2"})
        self.assertNotEqual(self.model.settings, self.SETTINGS["a1"])
        self.model.setSettings({"str": "test"})
        self.assertEqual(self.model.settings, self.SETTINGS["a1"])
        self.model.setSettings({"str2": "test"})
        self.assertEqual(self.model.settings, self.SETTINGS["a1"])

    def test_getSettings(self):
        self.assertEqual(self.model.getSettings(), self.SETTINGS["a1"])
        self.assertEqual(self.model.getSettings(), self.model.settings)

    def test_getSettingsTypes(self):
        alg = self.mSapi.AlgorithmManager.createUnmanaged.return_value
        self.model.getSettingsTypes()
        self.mSapi.AlgorithmManager.createUnmanaged.assert_called_once_with(
                self.model.algorithm)
        alg.initialize.assert_called_once()

        # file property
        self.mSapi.reset_mock()
        prop = mock.Mock(spec=FileProperty)
        prop.allowedValues = ["test", "test"]
        prop.documentation = "test doc"
        alg.getProperty.return_value = prop
        types, values, docs = self.model.getSettingsTypes()
        self.assertDictEqual(types, {"str": "file",
                                     "int": "file",
                                     "float": "file",
                                     "bool": "file"})
        self.assertDictEqual(values, {"str": ["test", "test"],
                                      "int": ["test", "test"],
                                      "float": ["test", "test"],
                                      "bool": ["test", "test"]})
        self.assertDictEqual(docs, {"str": "test doc",
                                    "int": "test doc",
                                    "float": "test doc",
                                    "bool": "test doc"})

        # workspace property
        self.mSapi.reset_mock()
        prop = mock.Mock(spec=WorkspaceGroupProperty)
        prop.allowedValues = ["test", "test"]
        prop.documentation = "test doc"
        alg.getProperty.return_value = prop
        types, values, docs = self.model.getSettingsTypes()
        self.assertDictEqual(types, {"str": "workspace",
                                     "int": "workspace",
                                     "float": "workspace",
                                     "bool": "workspace"})
        self.assertDictEqual(values, {"str": ["test", "test"],
                                      "int": ["test", "test"],
                                      "float": ["test", "test"],
                                      "bool": ["test", "test"]})
        self.assertDictEqual(docs, {"str": "test doc",
                                    "int": "test doc",
                                    "float": "test doc",
                                    "bool": "test doc"})
        self.mSapi.reset_mock()
        prop = mock.Mock(spec=MatrixWorkspaceProperty)
        prop.allowedValues = ["test", "test"]
        prop.documentation = "test doc"
        alg.getProperty.return_value = prop
        types, values, docs = self.model.getSettingsTypes()
        self.assertDictEqual(types, {"str": "workspace",
                                     "int": "workspace",
                                     "float": "workspace",
                                     "bool": "workspace"})
        self.assertDictEqual(values, {"str": ["test", "test"],
                                      "int": ["test", "test"],
                                      "float": ["test", "test"],
                                      "bool": ["test", "test"]})
        self.assertDictEqual(docs, {"str": "test doc",
                                    "int": "test doc",
                                    "float": "test doc",
                                    "bool": "test doc"})

        # combobox property
        self.mSapi.reset_mock()
        prop = mock.Mock(spec=StringPropertyWithValue)
        prop.allowedValues = ["test", "test"]
        prop.documentation = "test doc"
        alg.getProperty.return_value = prop
        types, values, docs = self.model.getSettingsTypes()
        self.assertDictEqual(types, {"str": "combobox",
                                     "int": "combobox",
                                     "float": "combobox",
                                     "bool": "combobox"})
        self.assertDictEqual(values, {"str": ["test", "test"],
                                      "int": ["test", "test"],
                                      "float": ["test", "test"],
                                      "bool": ["test", "test"]})
        self.assertDictEqual(docs, {"str": "test doc",
                                    "int": "test doc",
                                    "float": "test doc",
                                    "bool": "test doc"})

        # bool property
        self.mSapi.reset_mock()
        prop = mock.Mock(spec=BoolPropertyWithValue)
        prop.allowedValues = ["test", "test"]
        prop.documentation = "test doc"
        alg.getProperty.return_value = prop
        types, values, docs = self.model.getSettingsTypes()
        self.assertDictEqual(types, {"str": "bool",
                                     "int": "bool",
                                     "float": "bool",
                                     "bool": "bool"})
        self.assertDictEqual(values, {"str": ["test", "test"],
                                      "int": ["test", "test"],
                                      "float": ["test", "test"],
                                      "bool": ["test", "test"]})
        self.assertDictEqual(docs, {"str": "test doc",
                                    "int": "test doc",
                                    "float": "test doc",
                                    "bool": "test doc"})

        # other property
        self.mSapi.reset_mock()
        prop = mock.Mock()
        prop.allowedValues = ["test", "test"]
        prop.documentation = "test doc"
        alg.getProperty.return_value = prop
        types, values, docs = self.model.getSettingsTypes()
        self.assertDictEqual(types, {"str": "string",
                                     "int": "string",
                                     "float": "string",
                                     "bool": "string"})
        self.assertDictEqual(values, {"str": ["test", "test"],
                                      "int": ["test", "test"],
                                      "float": ["test", "test"],
                                      "bool": ["test", "test"]})
        self.assertDictEqual(docs, {"str": "test doc",
                                    "int": "test doc",
                                    "float": "test doc",
                                    "bool": "test doc"})

    def test_changeParameter(self):
        self.model.samples = [{}, {}]
        self.model.changeParameter(0, 0, "v1")
        self.assertEqual(self.model.samples[0]["c1"], "v1")
        self.mController.return_value.addParameter.assert_called_once()

        self.mController.reset_mock()
        self.model.changeParameter(0, 0, "")
        self.assertEqual(self.model.samples, [{}, {}])
        self.mController.return_value.addParameter.assert_not_called()

        # invalid custom option
        self.mController.reset_mock()
        samples = [{"c1": "test2", "CustomOptions": {"int": 2}}]
        self.model.samples = [{"c1": "test2", "CustomOptions": {"int": 2}}]
        self.model.changeParameter(0,
                                   self.model.columns.index("CustomOptions"),
                                   'invalid')
        self.assertEqual(self.model.samples, samples)
        self.mController.return_value.addParameter.assert_not_called()
        self.model.changeParameter(0,
                                   self.model.columns.index("CustomOptions"),
                                   'invalid=invalid')
        self.mController.return_value.addParameter.assert_not_called()

        # valid custom options
        self.model.changeParameter(0,
                                   self.model.columns.index("CustomOptions"),
                                   "int=4")
        samples[0]["CustomOptions"].update({"int": "4"})
        self.assertEqual(self.model.samples, samples)
        self.mController.return_value.addParameter.assert_called_once()
        self.mController.reset_mock()
        self.model.changeParameter(0,
                                   self.model.columns.index("CustomOptions"),
                                   "str=test1,test2;bool=True")
        samples[0]["CustomOptions"] = {"str": "test1,test2", "bool": True}
        self.assertEqual(self.model.samples, samples)
        self.mController.return_value.addParameter.assert_called()

    def test_getProcessingParameters(self):
        params = dict()
        params.update(self.SETTINGS["a1"])
        params["OutputWorkspace"] = "sample_1"
        self.model.samples = [{}]
        self.assertEqual(self.model.getProcessingParameters(0), params)
        self.model.samples = [{"c1": "test2"}]
        params.update({"c1": "test2"})
        self.assertEqual(self.model.getProcessingParameters(0), params)
        self.model.samples = [{"c1": "test2", "CustomOptions": {"int": 2}}]
        params["int"] = 2
        self.assertEqual(self.model.getProcessingParameters(0), params)

    def test_process(self):
        self.model.samples = [{}]
        try:
            self.model.process([0])
        except DrillException as e:
            assert True
            self.assertEqual(e.elements[0][0], 0)
        else:
            assert False
        self.mDrillTask.assert_not_called()
        self.model.tasksPool.addProcess.assert_not_called()
        self.mDrillTask.reset_mock()
        self.model.tasksPool.reset_mock()

        self.model.samples = [{"c1": "v1"}]
        self.model.process([0])
        self.mDrillTask.assert_called()
        self.model.tasksPool.addProcess.assert_called_once()
        self.mDrillTask.reset_mock()
        self.model.tasksPool.reset_mock()

        self.model.samples = [{"c1": "v1"}, {"c1": "v1"}]
        self.model.process([0, 1])
        self.mDrillTask.assert_called()
        self.model.tasksPool.addProcess.assert_called()
        self.mDrillTask.reset_mock()
        self.model.tasksPool.reset_mock()

        self.mDrillTask.side_effect = Exception()
        self.model.samples = [{"c1": "v1"}, {"c1": "v1"}]
        try:
            self.model.process([0, 1])
        except DrillException:
            assert True
        else:
            assert False

    def test_stopProcess(self):
        self.model.stopProcess()
        self.model.tasksPool.abortProcessing.assert_called_once()

    def test_importRundexData(self):
        self.mJson.load.return_value = {
                "Instrument": "i1",
                "AcquisitionMode": "a1",
                "GlobalSettings": {},
                "Samples": []
                }
        self.model.importRundexData("test")
        self.mOpen.assert_called_once_with("test")
        self.mJson.load.assert_called_once()
        self.assertDictEqual(self.model.settings, self.SETTINGS["a1"])
        self.assertEqual(self.model.samples, list())
        self.assertEqual(self.model.instrument, "i1")
        self.assertEqual(self.model.acquisitionMode, "a1")

    def test_exportRundexData(self):
        self.model.exportRundexData("test")
        self.mOpen.assert_called_once_with("test", 'w')
        self.mJson.dump.assert_called_once()
        written = self.mJson.dump.call_args[0][0]
        self.assertEquals(written, {
            "Instrument": "i1",
            "AcquisitionMode": "a1",
            "GlobalSettings": self.SETTINGS["a1"],
            "Samples": []
            })


if __name__ == "__main__":
    unittest.main()
