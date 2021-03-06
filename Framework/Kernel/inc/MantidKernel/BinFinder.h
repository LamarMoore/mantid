// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidKernel/DllConfig.h"
#include <vector>

namespace Mantid {
namespace Kernel {

/**
 * The BinFinder class is a helper class that allows one to quickly
 * find a bin index for many events.
 *
 * The class is constructed by giving it rebinning parameters. This sets
 * it up to be called repeatedly later to return the bin index.
 *
 * Only works for linear and logarithmic binning - not arbitrary.
 *
 * Does work for consecutive bins of different steps, or mixing lin and log
 *binning.
 */
class MANTID_KERNEL_DLL BinFinder {
public:
  BinFinder(const std::vector<double> &binParams);

  int bin(double x);

  int lastBinIndex();

private:
  /// Boundaries between binning regions
  std::vector<double> boundaries;
  /// Step sizes in binning regions; 1 smaller than boundaries
  std::vector<double> stepSizes;
  /// Log of the step size (used by log binning)
  std::vector<double> logSteps;
  /// Log of the boundary (used by log binning)
  std::vector<double> logBoundaries;
  /// Index of the last boundary in the bins
  std::vector<int> endBinIndex;
  /// How many regions?
  int numRegions;
};

} // namespace Kernel
} // namespace Mantid
