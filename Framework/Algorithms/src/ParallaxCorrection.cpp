// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/ParallaxCorrection.h"
#include "MantidAPI/InstrumentValidator.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Progress.h"
#include "MantidAPI/WorkspaceUnitValidator.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/DetectorInfo.h"
#include "MantidHistogramData/HistogramMath.h"
#include "MantidKernel/ArrayLengthValidator.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/CompositeValidator.h"

#include <math.h>
#include <muParser.h>

namespace {
static const std::string PARALLAX_PARAMETER = "parallax";
static const std::string DIRECTION_PARAMETER = "direction";

/**
 * @brief validateFormula Checks if the formula is evaluable
 * @param parallax : formula
 * @param direction : x or y
 * @return empty string if valid, error message otherwise
 */
std::string validateFormula(const std::string &parallax,
                            const std::string &direction) {
  if (direction != "x" && direction != "y") {
    return "Direction must be x or y";
  }
  mu::Parser muParser;
  double t = 0.;
  muParser.DefineVar("t", &t);
  muParser.SetExpr(parallax);
  try {
    muParser.Eval();
  } catch (mu::Parser::exception_type &e) {
    return e.GetMsg();
  }
  return "";
}
} // namespace

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(ParallaxCorrection)

//----------------------------------------------------------------------------------------------

/// Algorithms name for identification. @see Algorithm::name
const std::string ParallaxCorrection::name() const {
  return "ParallaxCorrection";
}

/// Algorithm's version for identification. @see Algorithm::version
int ParallaxCorrection::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string ParallaxCorrection::category() const { return "SANS"; }

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string ParallaxCorrection::summary() const {
  return "Performs parallax correction for tube based SANS instruments.";
}

//----------------------------------------------------------------------------------------------
/** Initialize the algorithm's properties.
 */
void ParallaxCorrection::init() {
  auto validator = boost::make_shared<Kernel::CompositeValidator>();
  validator->add(Kernel::make_unique<API::InstrumentValidator>());
  validator->add(
      Kernel::make_unique<API::WorkspaceUnitValidator>("Wavelength"));
  auto lengthValidator =
      boost::make_shared<Kernel::ArrayLengthValidator<std::string>>();
  lengthValidator->setLengthMin(1);
  declareProperty(
      Kernel::make_unique<API::WorkspaceProperty<API::MatrixWorkspace>>(
          "InputWorkspace", "", Kernel::Direction::Input, validator),
      "An input workspace.");
  declareProperty(
      Kernel::make_unique<Kernel::ArrayProperty<std::string>>("ComponentNames",
                                                              lengthValidator),
      "List of instrument components to perform the corrections for.");
  declareProperty(
      Kernel::make_unique<API::WorkspaceProperty<API::MatrixWorkspace>>(
          "OutputWorkspace", "", Kernel::Direction::Output),
      "An output workspace.");
}

//----------------------------------------------------------------------------------------------
/** Execute the algorithm.
 */
void ParallaxCorrection::exec() {
  API::MatrixWorkspace_const_sptr inputWorkspace =
      getProperty("InputWorkspace");
  API::MatrixWorkspace_sptr outputWorkspace = getProperty("OutputWorkspace");
  if (inputWorkspace != outputWorkspace) {
    outputWorkspace = inputWorkspace->clone();
  }
  const std::vector<std::string> componentNames = getProperty("ComponentNames");
  const auto &instrument = inputWorkspace->getInstrument();
  const auto &detectorInfo = inputWorkspace->detectorInfo();
  auto progress =
      Kernel::make_unique<API::Progress>(this, 0., 1., componentNames.size());
  for (const auto &componentName : componentNames) {
    progress->report("Performing parallax correction for component " +
                     componentName);
    const auto component = instrument->getComponentByName(componentName);
    if (!component) {
      g_log.error() << "No component defined with name " << componentName
                    << "\n";
      continue;
    }
    if (!component->hasParameter(PARALLAX_PARAMETER) ||
        !component->hasParameter(DIRECTION_PARAMETER)) {
      g_log.error() << "No parallax correction defined in IPF for component "
                    << componentName << "\n";
      continue;
    }
    std::vector<detid_t> detIDs;
    std::vector<Geometry::IDetector_const_sptr> dets;
    instrument->getDetectorsInBank(dets, componentName);
    if (dets.empty()) {
      const auto det =
          boost::dynamic_pointer_cast<const Geometry::IDetector>(component);
      if (!det) {
        g_log.error() << "No detectors found in component " << componentName
                      << "\n";
        continue;
      }
      dets.emplace_back(det);
    }
    if (!dets.empty()) {
      detIDs.reserve(dets.size());
      for (const auto &det : dets) {
        detIDs.emplace_back(det->getID());
      }
      const auto indices = inputWorkspace->getIndicesFromDetectorIDs(detIDs);
      const std::string parallax =
          component->getStringParameter(PARALLAX_PARAMETER)[0];
      const std::string direction =
          component->getStringParameter(DIRECTION_PARAMETER)[0];
      const auto valid = validateFormula(parallax, direction);
      if (!valid.empty()) {
        g_log.error() << "Unable to parse the parallax formula and direction "
                         "for component "
                      << componentName << ". Reason: " << valid << "\n";
        continue;
      }
      double t;
      mu::Parser muParser;
      muParser.DefineVar("t", &t);
      muParser.SetExpr(parallax);
      // note that this is intenionally serial
      for (auto wsIndex : indices) {
        const Kernel::V3D pos = detectorInfo.position(wsIndex);
        if (direction == "y") {
          t = std::fabs(std::atan2(pos.X(), pos.Z()));
        } else {
          t = std::fabs(std::atan2(pos.Y(), pos.Z()));
        }
        const double correction = muParser.Eval();
        if (correction > 0.) {
          auto &spectrum = outputWorkspace->mutableY(wsIndex);
          auto &errors = outputWorkspace->mutableE(wsIndex);
          spectrum /= correction;
          errors /= correction;
        } else {
          g_log.warning() << "Correction is <=0 for workspace index " << wsIndex
                          << ". Skipping the correction.\n";
        }
      }
    }
  }
  setProperty("OutputWorkspace", outputWorkspace);
}

} // namespace Algorithms
} // namespace Mantid
