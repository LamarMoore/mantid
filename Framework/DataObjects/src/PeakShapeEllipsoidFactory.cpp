// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidDataObjects/PeakShapeEllipsoidFactory.h"
#include "MantidDataObjects/PeakShapeEllipsoid.h"
#include "MantidKernel/SpecialCoordinateSystem.h"

#include <json/json.h>
#include <stdexcept>

using namespace Mantid::Kernel;

namespace Mantid {
namespace DataObjects {

/**
 * @brief Create the PeakShape
 * @param source : source JSON
 * @return PeakShape via this factory or it's successors
 */
Mantid::Geometry::PeakShape *
PeakShapeEllipsoidFactory::create(const std::string &source) const {
  Json::Reader reader;
  Json::Value root;
  Mantid::Geometry::PeakShape *product = nullptr;
  if (reader.parse(source, root)) {
    const std::string shape = root["shape"].asString();
    if (shape == PeakShapeEllipsoid::ellipsoidShapeName()) {

      const std::string algorithmName(root["algorithm_name"].asString());
      const int algorithmVersion(root["algorithm_version"].asInt());
      const auto frame(
          static_cast<SpecialCoordinateSystem>(root["frame"].asInt()));
      std::array<double, 3> abcRadii;
      abcRadii[0] = root["radius0"].asDouble();
      abcRadii[1] = root["radius1"].asDouble();
      abcRadii[2] = root["radius2"].asDouble();

      std::array<double, 3> abcRadiiBackgroundInner;
      abcRadiiBackgroundInner[0] = root["background_inner_radius0"].asDouble();
      abcRadiiBackgroundInner[1] = root["background_inner_radius1"].asDouble();
      abcRadiiBackgroundInner[2] = root["background_inner_radius2"].asDouble();

      std::array<double, 3> abcRadiiBackgroundOuter;
      abcRadiiBackgroundOuter[0] = root["background_outer_radius0"].asDouble();
      abcRadiiBackgroundOuter[1] = root["background_outer_radius1"].asDouble();
      abcRadiiBackgroundOuter[2] = root["background_outer_radius2"].asDouble();

      std::array<V3D, 3> directions;
      directions[0].fromString(root["direction0"].asString());
      directions[1].fromString(root["direction1"].asString());
      directions[2].fromString(root["direction2"].asString());

      product = new PeakShapeEllipsoid(
          directions, abcRadii, abcRadiiBackgroundInner,
          abcRadiiBackgroundOuter, frame, algorithmName, algorithmVersion);

    } else {
      if (m_successor) {
        product = m_successor->create(source);
      } else {
        throw std::invalid_argument("PeakShapeSphericalFactory:: No successor "
                                    "factory able to process : " +
                                    source);
      }
    }

  } else {

    throw std::invalid_argument("PeakShapeSphericalFactory:: Source JSON for "
                                "the peak shape is not valid: " +
                                source);
  }
  return product;
}

/**
 * @brief Set successor
 * @param successorFactory : successor
 */
void PeakShapeEllipsoidFactory::setSuccessor(
    std::shared_ptr<const PeakShapeFactory> successorFactory) {
  this->m_successor = successorFactory;
}

} // namespace DataObjects
} // namespace Mantid
