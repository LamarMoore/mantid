# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
import unittest
from mantid.kernel import UnitConversion, DeltaEModeType
import math


class UnitConversionTest(unittest.TestCase):
    def test_run_accepts_string_units(self):
        src_unit = "Wavelength"
        src_value = 1.5
        dest_unit = "Momentum"

        l1 = l2 = theta = efixed = 0.0
        emode = DeltaEModeType.Indirect
        expected = 2.0 * math.pi / src_value

        result = UnitConversion.run(src_unit, dest_unit, src_value, l1, l2, theta, emode, efixed)
        self.assertAlmostEqual(result, expected, 12)


if __name__ == '__main__':
    unittest.main()
