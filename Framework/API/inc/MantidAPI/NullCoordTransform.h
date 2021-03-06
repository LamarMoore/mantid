// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/CoordTransform.h"

namespace Mantid {
namespace API {

/** NullCoordTransform : A transform that sets the outVector to have the same
 * values as the inputVector.
 * Therefore has no-effect, for where transforms are not required.
 * @author Owen Arnold
 * @date 14/09/2011
 */
class MANTID_API_DLL NullCoordTransform : public Mantid::API::CoordTransform {
public:
  NullCoordTransform(size_t ndims = 3);
  std::string toXMLString() const override;
  std::string id() const override;
  void apply(const Mantid::coord_t *inputVector,
             Mantid::coord_t *outVector) const override;
  CoordTransform *clone() const override;

private:
  /// Number of dimensions.
  size_t m_ndims;
};
} // namespace API
} // namespace Mantid
