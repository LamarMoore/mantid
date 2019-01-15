# -*- coding: utf-8 -*-
# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +

from __future__ import (absolute_import, division, print_function)

from mantid.api import mtd
import numpy.testing
from testhelpers import (assertRaisesNothing, create_algorithm, illhelpers)
import unittest


class ReflectometryILLPreprocessTest(unittest.TestCase):
    def tearDown(self):
        mtd.clear()

    def testDefaultRunExecutesSuccessfully(self):
        args = {
            'Run': 'ILL/D17/317370.nxs',
            'OutputWorkspace': 'outWS',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getAxis(0).getUnit().caption(), 'Wavelength')

    def testDefaultRunFIGARO(self):
        args = {
            'Run': 'ILL/Figaro/000002.nxs',
            'OutputWorkspace': 'outWS',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getAxis(0).getUnit().caption(), 'Wavelength')

    def testFlatBackgroundSubtraction(self):
        inWSName = 'ReflectometryILLPreprocess_test_ws'
        self.create_sample_workspace(inWSName)
        # Add a peak to the sample workspace.
        ws = mtd[inWSName]
        ys = ws.dataY(49)
        ys += 10.0
        args = {
            'InputWorkspace': inWSName,
            'OutputWorkspace': 'unused_for_child',
            'BeamCentre': 49,
            'FluxNormalisation': 'Normalisation OFF',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getNumberHistograms(), 100)
        for i in range(outWS.getNumberHistograms()):
            ys = outWS.readY(i)
            if i != 49:
                numpy.testing.assert_equal(ys, [0.] * outWS.blocksize())
            else:
                numpy.testing.assert_almost_equal(ys, [10.] * outWS.blocksize())

    def _backgroundSubtraction(self, subtractionType):
        inWSName = 'ReflectometryILLPreprocess_test_ws'
        self.create_sample_workspace(inWSName)
        # Add a peak to the sample workspace.
        ws = mtd[inWSName]
        ys = ws.dataY(49)
        ys += 10.0
        args = {
            'InputWorkspace': inWSName,
            'OutputWorkspace': 'unused_for_child',
            'BeamCentre': 49,
            'FluxNormalisation': 'Normalisation OFF',
            'FlatBackground': subtractionType,
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getNumberHistograms(), 100)
        for i in range(outWS.getNumberHistograms()):
            ys = outWS.readY(i)
            if i != 49:
                numpy.testing.assert_almost_equal(ys, [0] * outWS.blocksize())
            else:
                numpy.testing.assert_almost_equal(ys, [10.] * outWS.blocksize())

    def testLinearFlatBackgroundSubtraction(self):
        self._backgroundSubtraction('Background Linear Fit')

    def testConstantFlatBackgroundSubtraction(self):
        self._backgroundSubtraction('Background Constant Fit')

    def testDisableFlatBackgroundSubtraction(self):
        inWSName = 'ReflectometryILLPreprocess_test_ws'
        self.create_sample_workspace(inWSName)
        # Add a peak to the sample workspace.
        ws = mtd[inWSName]
        bkgLevel = ws.readY(0)[0]
        self.assertGreater(bkgLevel, 0.1)
        args = {
            'InputWorkspace': inWSName,
            'OutputWorkspace': 'unused_for_child',
            'BeamCentre': 49,
            'FluxNormalisation': 'Normalisation OFF',
            'FlatBackground': 'Background OFF',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getNumberHistograms(), 100)
        for i in range(outWS.getNumberHistograms()):
            ys = outWS.readY(i)
            numpy.testing.assert_equal(ys, bkgLevel)

    def testForegroundBackgroundRanges(self):
        inWSName = 'ReflectometryILLPreprocess_test_ws'
        self.create_sample_workspace(inWSName)
        ws = mtd[inWSName]
        # Add special background fitting zones around the exclude zones.
        lowerBkgIndices = [26]
        for i in lowerBkgIndices:
            ys = ws.dataY(i)
            ys += 5.0
        # Add negative 'exclude zone' around the peak.
        lowerExclusionIndices = [27, 28]
        for i in lowerExclusionIndices:
            ys = ws.dataY(i)
            ys -= 1000.0
        # Add a peak to the sample workspace.
        foregroundIndices = [29, 30, 31]
        for i in foregroundIndices:
            ys = ws.dataY(i)
            ys += 1000.0
        # The second exclusion zone is wider.
        upperExclusionIndices = [32, 33, 34]
        for i in upperExclusionIndices:
            ys = ws.dataY(i)
            ys -= 1000.0
        # The second fitting zone is wider.
        upperBkgIndices = [35, 36]
        for i in upperBkgIndices:
            ys = ws.dataY(i)
            ys += 5.0
        args = {
            'InputWorkspace': inWSName,
            'OutputWorkspace': 'unused_for_child',
            'BeamCentre': 30,
            'ForegroundHalfWidth': [1],
            'LowAngleBkgOffset': len(lowerExclusionIndices),
            'LowAngleBkgWidth': len(lowerBkgIndices),
            'HighAngleBkgOffset': len(upperExclusionIndices),
            'HighAngleBkgWidth': len(upperBkgIndices),
            'FluxNormalisation': 'Normalisation OFF',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getNumberHistograms(), 100)
        for i in range(outWS.getNumberHistograms()):
            ys = outWS.readY(i)
            if i in lowerBkgIndices:
                numpy.testing.assert_equal(ys, [0.0] * outWS.blocksize())
            elif i in lowerExclusionIndices:
                numpy.testing.assert_equal(ys, [-1005.0] * outWS.blocksize())
            elif i in foregroundIndices:
                numpy.testing.assert_equal(ys, [995.0] * outWS.blocksize())
            elif i in upperExclusionIndices:
                numpy.testing.assert_equal(ys, [-1005.0] * outWS.blocksize())
            elif i in upperBkgIndices:
                numpy.testing.assert_equal(ys, [0.0] * outWS.blocksize())
            else:
                numpy.testing.assert_equal(ys, [-5.0] * outWS.blocksize())

    def testAsymmetricForegroundRanges(self):
        inWSName = 'ReflectometryILLPreprocess_test_ws'
        self.create_sample_workspace(inWSName)
        ws = mtd[inWSName]
        # Add special background fitting zones around the exclude zones.
        foregroundIndices = [21, 22, 23, 24]
        for i in range(ws.getNumberHistograms()):
            ys = ws.dataY(i)
            es = ws.dataE(i)
            if i in foregroundIndices:
                ys.fill(1000.0)
                es.fill(numpy.sqrt(1000.0))
            else:
                ys.fill(-100)
                es.fill(numpy.sqrt(100))
        args = {
            'InputWorkspace': inWSName,
            'OutputWorkspace': 'unused_for_child',
            'BeamCentre': 23,
            'ForegroundHalfWidth': [2, 1],
            'FlatBackground': 'Background OFF',
            'FluxNormalisation': 'Normalisation OFF',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getNumberHistograms(), 100)
        logs = outWS.run()
        properties = ['foreground.first_workspace_index',
                      'foreground.centre_workspace_index',
                      'foreground.last_workspace_index']
        values = [21, 23, 24]
        for p, val in zip(properties, values):
            self.assertTrue(logs.hasProperty(p))
            self.assertEqual(logs.getProperty(p).value, val)

    def testWaterWorkspace(self):
        inWSName = 'ReflectometryILLPreprocess_test_ws'
        self.create_sample_workspace(inWSName)
        # Add a peak to the sample workspace.
        ws = mtd[inWSName]
        for i in range(ws.getNumberHistograms()):
            ys = ws.dataY(i)
            ys.fill(10.27)
        args = {
            'InputWorkspace': inWSName,
            'OutputWorkspace': 'unused_for_child',
            'WaterWorkspace': inWSName,
            'FluxNormalisation': 'Normalisation OFF',
            'FlatBackground': 'Background OFF',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getNumberHistograms(), 100)
        for i in range(outWS.getNumberHistograms()):
            ys = outWS.readY(i)
            numpy.testing.assert_equal(ys, [1.0] * outWS.blocksize())

    def testCleanupOFF(self):
        # test if intermediate workspaces exist:
        # not tested: position tables
        # normalise_to_slits, normalise_to_monitor, '_normalised_to_time_','transposed_flat_background'
        outWSName = 'outWS'
        ws = illhelpers.create_poor_mans_d17_workspace()
        illhelpers._fillTemplateReflectometryWorkspace(ws)
        # Add a peak to the workspace.
        for i in range(33, 100):
            ys = ws.dataY(i)
            ys += 10.0

        arg = {'OutputWorkspace': 'peakTable'}
        alg = create_algorithm('CreateEmptyTableWorkspace', **arg)
        alg.execute()
        table = mtd['peakTable']
        table.addColumn('double', 'PeakCentre')
        table.addRow((3.0,))
        args = {
            'InputWorkspace': ws,
            'BeamPositionWorkspace': 'peakTable',
            'OutputWorkspace': outWSName,
            'Cleanup': 'Cleanup OFF',
            'WaterWorkspace': ws,
            'ForegroundHalfWidth': [41],
            'LowAngleBkgOffset': 10,
            'LowAngleBkgWidth': 10,
            'HighAngleBkgOffset': 70,
            'HighAngleBkgWidth': 5,
            'FluxNormalisation': 'Normalisation OFF',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        print(mtd.getObjectNames())
        self.assertEquals(mtd.getObjectNames(), ['outWS_cloned_for_flat_bkg_',
                                                 'outWS_detector_moved_',
                                                 'outWS_detectors_',
                                                 'outWS_flat_background_',
                                                 'outWS_flat_background_subtracted_',
                                                 'outWS_in_wavelength_',
                                                 'outWS_monitors_',
                                                 'outWS_transposed_clone_',
                                                 'outWS_transposed_flat_background_',
                                                 'outWS_water_calibrated_',
                                                 'outWS_water_rebinned_',
                                                 'peakTable'])
        mtd.clear()

    def testTwoInputFiles(self):
        outWSName = 'outWS'
        args = {
            'Run': 'ILL/D17/317369, ILL/D17/317370.nxs',
            'OutputWorkspace': outWSName,
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getAxis(0).getUnit().caption(), 'Wavelength')

    def create_sample_workspace(self, name, numMonitors=0):
        args = {
            'OutputWorkspace': name,
            'Function': 'Flat background',
            'NumMonitors': numMonitors,
            'NumBanks': 1,
        }
        alg = create_algorithm('CreateSampleWorkspace', **args)
        alg.setLogging(False)
        alg.execute()
        loadInstrArgs = {
            'Workspace': name,
            'InstrumentName': 'FIGARO',
            'RewriteSpectraMap': False
        }
        loadInstrument = create_algorithm('LoadInstrument', **loadInstrArgs)
        loadInstrument.setLogging(False)
        loadInstrument.execute()

    def testBeamCentreBraggAngelInput(self):
        args = {
            'Run': 'ILL/D17/317369',
            'BeamCentre': 10.23,
            'BraggAngle': 20.23,
            'OutputWorkspace': 'outWS',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertEquals(outWS.getRun().getProperty('peak_position').value, 10.23)
        self.assertEquals(outWS.getRun().getProperty('foreground.centre_workspace_index').value, 10)
        self.assertEquals(outWS.getAxis(0).getUnit().caption(), 'Wavelength')

    def testBeamCentreFit(self):
        args = {
            'Run': 'ILL/D17/317369',
            'OutputWorkspace': 'outWS',
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertAlmostEquals(outWS.getRun().getProperty('peak_position').value, 202.1773407538167, delta=1.e-13)
        self.assertEquals(outWS.getRun().getProperty('foreground.centre_workspace_index').value, 202)
        self.assertEquals(outWS.getAxis(0).getUnit().caption(), 'Wavelength')

    def testUnitConversion(self):
        args = {
            'Run': 'ILL/D17/317369',
            'OutputWorkspace': 'outWS',
            'RangeLower': 2.,
            'RangeUpper': 20,
            'rethrow': True,
            'child': True
        }
        alg = create_algorithm('ReflectometryILLPreprocess', **args)
        assertRaisesNothing(self, alg.execute)
        outWS = alg.getProperty('OutputWorkspace').value
        self.assertAlmostEquals(outWS.getRun().getProperty('peak_position').value, 202.17752545515665, delta=1.e-13)
        self.assertEquals(outWS.getRun().getProperty('foreground.centre_workspace_index').value, 202)
        self.assertEquals(outWS.getAxis(0).getUnit().caption(), 'Wavelength')

if __name__ == "__main__":
    unittest.main()
