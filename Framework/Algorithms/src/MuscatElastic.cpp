// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation MuscatElastic,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/MuscatElastic.h"
#include "MantidAPI/EqualBinSizesValidator.h"
#include "MantidAPI/ISpectrum.h"
#include "MantidAPI/InstrumentValidator.h"
#include "MantidAPI/Sample.h"
#include "MantidAPI/SpectrumInfo.h"
#include "MantidAPI/WorkspaceGroup.h"
#include "MantidAPI/WorkspaceUnitValidator.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidDataObjects/WorkspaceCreation.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/DetectorInfo.h"
#include "MantidGeometry/Instrument/ReferenceFrame.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/CompositeValidator.h"
#include "MantidKernel/EnabledWhenProperty.h"
#include "MantidKernel/Material.h"
#include "MantidKernel/MersenneTwister.h"

using namespace Mantid::API;
using namespace Mantid::Kernel;
using Mantid::DataObjects::Workspace2D;

namespace {
constexpr int DEFAULT_NEVENTS = 1000;
constexpr int DEFAULT_SEED = 123456789;
constexpr size_t DEFAULT_NSCATTERINGS = 1;
constexpr int DEFAULT_LATITUDINAL_DETS = 5;
constexpr int DEFAULT_LONGITUDINAL_DETS = 10;
} // namespace

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(MuscatElastic)

/**
 * Initialize the algorithm
 */
void MuscatElastic::init() {
  // The input workspace must have an instrument and units of wavelength
  auto wsValidator = std::make_shared<CompositeValidator>();
  wsValidator->add<WorkspaceUnitValidator>("Wavelength");
  wsValidator->add<InstrumentValidator>();

  declareProperty(std::make_unique<WorkspaceProperty<>>(
                      "InputWorkspace", "", Direction::Input, wsValidator),
                  "The name of the input workspace.  The input workspace must "
                  "have X units of wavelength.");

  auto wsQValidator = std::make_shared<CompositeValidator>();
  wsQValidator->add<WorkspaceUnitValidator>("MomentumTransfer");
  wsQValidator->add<EqualBinSizesValidator>(1.0E-07);

  declareProperty(std::make_unique<WorkspaceProperty<>>(
                      "SofqWorkspace", "", Direction::Input, wsQValidator),
                  "The name of the workspace containing S(q).  The input "
                  "workspace must "
                  "have X units of momentum transfer.");
  declareProperty(std::make_unique<WorkspaceProperty<WorkspaceGroup>>(
                      "OutputWorkspace", "", Direction::Output),
                  "Name for the WorkspaceGroup that will be created.");
  declareProperty(
      std::make_unique<WorkspaceProperty<>>(
          "ScatteringCrossSection", "", Direction::Input,
          PropertyMode::Optional, wsQValidator),
      "A workspace containing the scattering cross section as a function of "
      "k.");

  auto positiveInt = std::make_shared<Kernel::BoundedValidator<int>>();
  positiveInt->setLower(1);
  declareProperty(
      "NeutronEventsSingle", DEFAULT_NEVENTS, positiveInt,
      "The number of \"neutron\" events to generate for single scattering");
  declareProperty(
      "NeutronEventsMultiple", DEFAULT_NEVENTS, positiveInt,
      "The number of \"neutron\" events to generate for multiple scattering");
  declareProperty("SeedValue", DEFAULT_SEED, positiveInt,
                  "Seed the random number generator with this value");
  auto nScatteringsValidator =
      std::make_shared<Kernel::BoundedValidator<size_t>>();
  nScatteringsValidator->setLower(1);
  nScatteringsValidator->setUpper(5);
  declareProperty("NumberScatterings", DEFAULT_NSCATTERINGS,
                  nScatteringsValidator, "Number of scatterings");

  declareProperty("SparseInstrument", false,
                  "Enable simulation on special "
                  "instrument with a sparse grid of "
                  "detectors interpolating the "
                  "results to the real instrument.");
  auto threeOrMore = std::make_shared<Kernel::BoundedValidator<int>>();
  threeOrMore->setLower(3);
  declareProperty(
      "NumberOfDetectorRows", DEFAULT_LATITUDINAL_DETS, threeOrMore,
      "Number of detector rows in the detector grid of the sparse instrument.");
  setPropertySettings(
      "NumberOfDetectorRows",
      std::make_unique<EnabledWhenProperty>(
          "SparseInstrument", ePropertyCriterion::IS_NOT_DEFAULT));
  auto twoOrMore = std::make_shared<Kernel::BoundedValidator<int>>();
  twoOrMore->setLower(2);
  declareProperty("NumberOfDetectorColumns", DEFAULT_LONGITUDINAL_DETS,
                  twoOrMore,
                  "Number of detector columns in the detector grid "
                  "of the sparse instrument.");
  setPropertySettings(
      "NumberOfDetectorColumns",
      std::make_unique<EnabledWhenProperty>(
          "SparseInstrument", ePropertyCriterion::IS_NOT_DEFAULT));
}

/**
 * Execution code
 */
void MuscatElastic::exec() {
  const MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");
  const MatrixWorkspace_sptr SQWS = getProperty("SofqWorkspace");
  // take log of S(Q) and store it this way
  auto y = SQWS->y(0);
  std::transform(y.begin(), y.end(), y.begin(),
                 static_cast<double (*)(double)>(std::log));

  const MatrixWorkspace_sptr sigmaSSWS = getProperty("ScatteringCrossSection");
  if (sigmaSSWS) {
    // take log of sigmaSSWS and store it this way
    auto &y = sigmaSSWS->mutableY(0);
    std::transform(y.begin(), y.end(), y.begin(),
                   static_cast<double (*)(double)>(std::log));
  }

  const bool useSparseInstrument = getProperty("SparseInstrument");
  SparseWorkspace_sptr sparseWS;
  if (useSparseInstrument) {
    const int latitudinalDets = getProperty("NumberOfDetectorRows");
    const int longitudinalDets = getProperty("NumberOfDetectorColumns");
    const auto inputNbins = static_cast<int>(inputWS->blocksize());
    sparseWS = createSparseWorkspace(*inputWS, inputNbins, latitudinalDets,
                                     longitudinalDets);
  }
  const size_t nScatters = getProperty("NumberScatterings");
  std::vector<MatrixWorkspace_sptr> simulationWSs;
  std::vector<MatrixWorkspace_sptr> outputWSs;

  auto noAbsOutputWS = createOutputWorkspace(*inputWS);
  auto noAbsSimulationWS =
      useSparseInstrument ? sparseWS->clone() : noAbsOutputWS;
  for (auto i = 0; i < nScatters; i++) {
    auto outputWS = createOutputWorkspace(*inputWS);
    MatrixWorkspace_sptr simulationWS =
        useSparseInstrument ? sparseWS->clone() : outputWS;
    simulationWSs.push_back(simulationWS);
    outputWSs.push_back(outputWS);
  }
  const MatrixWorkspace &instrumentWS =
      useSparseInstrument ? *sparseWS : *inputWS;

  auto instrument = inputWS->getInstrument();
  const auto nhists =
      useSparseInstrument
          ? static_cast<int64_t>(sparseWS->getNumberHistograms())
          : inputWS->getNumberHistograms();
  auto nbins = inputWS->blocksize();

  auto &sample = inputWS->sample();

  const int nSingleScatterEvents = getProperty("NeutronEventsSingle");
  const int nMultiScatterEvents = getProperty("NeutronEventsMultiple");

  const int seed = getProperty("SeedValue");

  Progress prog(this, 0.0, 1.0, nhists * nbins);
  prog.setNotifyStep(0.01);
  const std::string reportMsg = "Computing corrections";

  bool enableParallelFor = true;
  for (auto &ws : simulationWSs) {
    enableParallelFor = enableParallelFor && Kernel::threadSafe(*ws);
  }
  enableParallelFor = enableParallelFor && Kernel::threadSafe(*noAbsOutputWS);

  PARALLEL_FOR_IF(enableParallelFor)
  for (int64_t i = 0; i < nhists; ++i) {
    PARALLEL_START_INTERUPT_REGION
    MersenneTwister rng(seed + int(i));
    // no two theta for monitors
    if (!instrumentWS.spectrumInfo().isMonitor(i)) {
      for (int bin = 0; bin < nbins; bin++) {
        double kinc = 2 * M_PI / instrumentWS.histogram(i).x()[bin];

        auto detPos = instrumentWS.detectorInfo().position(i);

        double total(0);
        total = simulateEvents(nSingleScatterEvents, 1, sample, *instrument,
                               rng, sigmaSSWS, SQWS, kinc, detPos, true);
        noAbsSimulationWS->getSpectrum(i).dataY()[bin] = total;
        /*if (!useSparseInstrument && lambdaStepSize > 1) {
        }*/

        for (size_t ne = 0; ne < nScatters; ne++) {
          int nEvents = ne == 0 ? nSingleScatterEvents : nMultiScatterEvents;

          total = simulateEvents(nEvents, ne + 1, sample, *instrument, rng,
                                 sigmaSSWS, SQWS, kinc, detPos, false);
          simulationWSs[ne]->getSpectrum(i).dataY()[bin] = total;

          // interpolate through points not simulated
          /*if (!useSparseInstrument && lambdaStepSize > 1) {
            auto histnew = simulationWSs[ne]->histogram(i);
            outputWSs[ne]->setHistogram(i, histnew);
          }*/
        }

        prog.report(reportMsg);
      }
    }

    PARALLEL_END_INTERUPT_REGION
  }
  PARALLEL_CHECK_INTERUPT_REGION

  // if (useSparseInstrument) {
  //  interpolateFromSparse(*outputWS, *sparseWS, interpolateOpt);
  //}

  // Create workspace group that holds output workspaces
  auto wsgroup = std::make_shared<WorkspaceGroup>();

  std::string wsNamePrefix = "Scatter_";
  std::string wsname = wsNamePrefix + "1_NoAbs";
  API::AnalysisDataService::Instance().addOrReplace(wsname, noAbsOutputWS);
  wsgroup->addWorkspace(noAbsOutputWS);
  for (auto i = 0; i < outputWSs.size(); i++) {
    std::string wsname = wsNamePrefix + std::to_string(i + 1);
    API::AnalysisDataService::Instance().addOrReplace(wsname, outputWSs[i]);
    wsgroup->addWorkspace(outputWSs[i]);
  }
  // set the output property
  setProperty("OutputWorkspace", wsgroup);

  g_log.warning() << "Calls to interceptSurface= " +
                         std::to_string(m_callsToInterceptSurface) + "\n";
}

/**
 * Calculate a new mean free path using a k-specific scattering cross section
 * Note - a separate tabulated scattering cross section is used elsewhere in the
 * calculation
 * @param sigmaSSWS Workspace containing log of scattering cross section as a
 * function of k
 * @param material The sample material
 * @param kinc The incident wavenumber
 * @oaram specialSingleScatterCalc Boolean indicating whether special single
 * scatter calculation should be performed
 * @return A tuple containing the mean free path (metres), the total cross
 * section (barns)
 */
double MuscatElastic::new_vector(const MatrixWorkspace_sptr sigmaSSWS,
                                 const Material &material, double kinc,
                                 bool specialSingleScatterCalc) {
  double scatteringXSection, absorbXsection;
  if (specialSingleScatterCalc) {
    absorbXsection = 0;
  } else {
    double wavelength = 2 * M_PI / kinc;
    absorbXsection = material.absorbXSection(wavelength);
  }
  if (sigmaSSWS) {
    scatteringXSection = interpolateLogQuadratic(sigmaSSWS, kinc);
  } else {
    scatteringXSection = material.totalScatterXSection();
  }

  auto sig_total = scatteringXSection + absorbXsection;
  return sig_total;
}

double MuscatElastic::interpolateLogQuadratic(
    const MatrixWorkspace_sptr workspaceToInterpolate, double x) {
  if (x > workspaceToInterpolate->x(0).back()) {
    return workspaceToInterpolate->y(0).back();
  }
  if (x < workspaceToInterpolate->x(0).front()) {
    return workspaceToInterpolate->y(0).front();
  }
  // assume log(cross section) is quadratic in k
  size_t idx;
  // avoid passing tolerance into yIndexOfX for histograms (SURELY SHOULDN'T
  // HAVE TO DO THIS??)
  if (!workspaceToInterpolate->isHistogramData()) {
    auto binWidth =
        workspaceToInterpolate->x(0)[1] - workspaceToInterpolate->x(0)[0];
    idx = workspaceToInterpolate->yIndexOfX(x, 0, 0.5 * binWidth);
  } else {
    idx = workspaceToInterpolate->yIndexOfX(x);
  }
  // need at least two points to the right of the x value for the quadratic
  // interpolation to work
  auto ny = workspaceToInterpolate->blocksize();
  if (ny < 3) {
    throw std::runtime_error(
        "Need at least 3 y values to perform quadratic interpolation");
  }
  if (idx > ny - 3) {
    idx = ny - 3;
  }
  // this interpolation assumes the set of 3 bins\point have the same width
  auto binWidth =
      workspaceToInterpolate->x(0)[1] - workspaceToInterpolate->x(0)[0];
  // U=0 on at point or bin edge to the left of where x lies
  auto U = (x - workspaceToInterpolate->x(0)[idx]) / binWidth;
  auto &y = workspaceToInterpolate->y(0);
  auto A = (y[idx] - 2 * y[idx + 1] + y[idx + 2]) / 2;
  auto B = (-3 * y[idx] - 4 * y[idx + 1] - y[idx + 2]) / 2;
  auto C = y[idx];
  return exp(A * U * U + B * U + C);
}

double MuscatElastic::simulateEvents(const int nEvents, const size_t nScatters,
                                     const Sample &sample,
                                     const Geometry::Instrument &instrument,
                                     Kernel::PseudoRandomNumberGenerator &rng,
                                     const MatrixWorkspace_sptr sigmaSSWS,
                                     const MatrixWorkspace_sptr SOfQ,
                                     const double kinc, Kernel::V3D detPos,
                                     bool specialSingleScatterCalc) {
  double sumOfWeights = 0;
  auto sourcePos = instrument.getSource()->getPos();

  double sigma_total;
  double scatteringXSection = sample.getMaterial().totalScatterXSection();
  sigma_total = new_vector(sigmaSSWS, sample.getMaterial(), kinc,
                           specialSingleScatterCalc);

  for (int ie = 0; ie < nEvents; ie++) {
    auto [success, weight] = scatter(nScatters, sample, instrument, sourcePos,
                                     rng, sigma_total, scatteringXSection, SOfQ,
                                     kinc, detPos, specialSingleScatterCalc);
    if (success)
      sumOfWeights += weight;
    else
      ie--;
  }
  return sumOfWeights / nEvents;
}

std::tuple<bool, double> MuscatElastic::scatter(
    const size_t nScatters, const Sample &sample,
    const Geometry::Instrument &instrument, const V3D sourcePos,
    Kernel::PseudoRandomNumberGenerator &rng, const double sigma_total,
    double scatteringXSection, const MatrixWorkspace_sptr SOfQ,
    const double kinc, Kernel::V3D detPos, bool specialSingleScatterCalc) {
  double weight = 1;
  double numberDensity = sample.getMaterial().numberDensityEffective();
  // if scale up scatteringXSection by 100*numberDensity then may not need
  // sigma_total any more but leave it alone now to align with original code
  double vmu = 100 * numberDensity * sigma_total;
  auto track = start_point(sample.getShape(), instrument.getReferenceFrame(),
                           sourcePos, rng);
  updateWeightAndPosition(track, weight, vmu, sigma_total, rng);

  double QSS = 0;
  for (size_t is = 0; is < nScatters - 1; is++) {
    q_dir(track, SOfQ, kinc, scatteringXSection, rng, QSS, weight);
    int nlinks = sample.getShape().interceptSurface(track);
    m_callsToInterceptSurface++;
    if (nlinks == 0) {
      return {false, 0};
    }
    updateWeightAndPosition(track, weight, vmu, sigma_total, rng);
  }
  // divide by QSS here rather than outside scatter (as was done in original
  // Fortan) to avoid the magic 1/(nscatter-1)^(nscatter-1) factors
  weight = weight / pow(QSS, nScatters - 1);

  Kernel::V3D directionToDetector = detPos - track.startPoint();
  Kernel::V3D prevDirection = track.direction();
  directionToDetector.normalize();
  track.reset(track.startPoint(), directionToDetector);
  int nlinks = sample.getShape().interceptSurface(track);
  m_callsToInterceptSurface++;
  // due to VALID_INTERCEPT_POINT_SHIFT some tracks that skim the surface
  // of a CSGObject sample may not generate valid tracks. Start over again
  // for this event
  if (nlinks == 0) {
    return {false, 0};
  }
  double dl = track.front().distInsideObject;
  auto q = (directionToDetector - prevDirection) * kinc;
  auto SQ = interpolateLogQuadratic(SOfQ, q.norm());
  if (specialSingleScatterCalc)
    vmu = 0;
  auto AT2 = exp(-dl * vmu);
  weight = weight * AT2 * SQ * scatteringXSection / (4 * M_PI);
  return {true, weight};
}

// update track direction, QSS and weight
void MuscatElastic::q_dir(Geometry::Track track,
                          const MatrixWorkspace_sptr SOfQ, const double kinc,
                          double scatteringXSection,
                          Kernel::PseudoRandomNumberGenerator &rng, double &QSS,
                          double &weight) {
  auto qvalues = SOfQ->histogram(0).x().rawData();
  // For elastic just select a q value in range 0 to 2k
  // The following will eventually be used for inelastic where it's less trivial
  double QQ, cosT;
  bool foundQ = false;
  for (auto m = 0; m < 1000; m++) {
    QQ = qvalues.back() * rng.nextValue();
    // T = 2theta
    cosT = 1 - QQ * QQ / (2 * kinc * kinc);
    if (abs(cosT) <= 1) {
      foundQ = true;
      break;
    }
  }
  if (!foundQ) {
    throw std::runtime_error("Unable to select a new q for kinc=" +
                             std::to_string(kinc));
  } else {
    double SQ = interpolateLogQuadratic(SOfQ, QQ);
    QSS += QQ * SQ;
    weight = weight * scatteringXSection * SQ * QQ;
    auto phi = rng.nextValue() * 2 * M_PI;
    auto B3 = sqrt(1 - cosT * cosT);
    auto B2 = cosT;
    // possible to do this using the Quat class instead??
    // Quat(const double _deg, const V3D &_axis);
    // Quat(acos(cosT)*180/M_PI,
    // Kernel::V3D(track.direction()[],track.direction()[],0))

    // Rodrigues formula with final term equal to zero
    // v_rot = cosT * v + sinT(k x v)
    // with rotation axis k orthogonal to v and defined as:
    // sin(phi) * (vy, -vx, 0) + cos(phi) * (-vx * vz, -vy * vz, 1 - vz * vz)
    auto horiz = track.direction()[0];
    auto up = track.direction()[1];
    auto beam = track.direction()[2];
    double UKX, UKY, UKZ;
    if (up < 1) {
      auto A2 = sqrt(1 - up * up);
      auto UQTZ = cos(phi) * A2;
      auto UQTX = -cos(phi) * up * beam / A2 + sin(phi) * horiz / A2;
      auto UQTY = -cos(phi) * up * horiz / A2 - sin(phi) * beam / A2;
      UKX = B2 * beam + B3 * UQTX;
      UKY = B2 * horiz + B3 * UQTY;
      UKZ = B2 * up + B3 * UQTZ;
    } else {
      UKX = B2 * cos(phi);
      UKY = B3 * sin(phi);
      UKZ = B2;
    }
    track.reset(track.startPoint(), Kernel::V3D(UKX, UKY, UKZ));
  }
}

Geometry::Track MuscatElastic::start_point(
    const Geometry::IObject &shape,
    std::shared_ptr<const Geometry::ReferenceFrame> frame, const V3D sourcePos,
    Kernel::PseudoRandomNumberGenerator &rng) {
  int MAX_ATTEMPTS = 100;
  for (int i = 0; i < MAX_ATTEMPTS; i++) {
    auto t = generateInitialTrack(shape, frame, sourcePos, rng);
    int nlinks = shape.interceptSurface(t);
    m_callsToInterceptSurface++;
    if (nlinks > 0) {
      if (i > 0) {
        g_log.warning() << "Generating initial track required " +
                               std::to_string(i + 1) + " attempts.\n";
      }
      return t;
    }
  }
  throw std::runtime_error("MuscatElastic::start_point() - Unable to "
                           "generate entry point into sample");
}

// update track start point and weight
void MuscatElastic::updateWeightAndPosition(
    Geometry::Track &track, double &weight, const double vmu,
    const double sigma_total, Kernel::PseudoRandomNumberGenerator &rng) {
  double dl = track.front().distInsideObject;
  double b4 = (1.0 - exp(-dl * vmu));
  double vmfp = 1.0 / vmu;
  double vl = -(vmfp * log(1 - rng.nextValue() * b4));
  weight = weight * b4 / sigma_total;
  inc_xyz(track, vl);
}

/**
 * Generate an initial track starting at the source and entering
 * the sample at a random point on its front surface
 * @param sample The sample
 * @param frame The instrument's reference frame
 * @param sourcePos The source position
 * @param rng Random number generator
 * @return a track
 */
Geometry::Track MuscatElastic::generateInitialTrack(
    const Geometry::IObject &shape,
    std::shared_ptr<const Geometry::ReferenceFrame> frame, const V3D sourcePos,
    Kernel::PseudoRandomNumberGenerator &rng) {
  auto sampleBox = shape.getBoundingBox();
  // generate random point on front surface of sample bounding box
  auto ptx = sampleBox.minPoint()[frame->pointingHorizontal()] +
             rng.nextValue() * sampleBox.width()[frame->pointingHorizontal()];
  auto pty = sampleBox.minPoint()[frame->pointingUp()] +
             rng.nextValue() * sampleBox.width()[frame->pointingUp()];
  // perhaps eventually also generate random point on the beam profile?
  auto ptOnBeamProfile = Kernel::V3D();
  ptOnBeamProfile[frame->pointingHorizontal()] = ptx;
  ptOnBeamProfile[frame->pointingUp()] = pty;
  ptOnBeamProfile[frame->pointingAlongBeam()] =
      sourcePos[frame->pointingAlongBeam()];
  auto toSample = Kernel::V3D();
  toSample[frame->pointingAlongBeam()] = 1.;
  Geometry::Track trackToSample = Geometry::Track(ptOnBeamProfile, toSample);
  return trackToSample;
}

/**
 * Update the x, y, z position of the neutron (or dV volume element
 * to integrate over). Save new start point in to the track object
 * supplied along
 * @param track A track defining the current trajectory
 * @param vl A distance to move along the current trajectory
 */
void MuscatElastic::inc_xyz(Geometry::Track &track, double vl) {
  Kernel::V3D position = track.front().entryPoint;
  Kernel::V3D direction = track.direction();
  auto x = position[0] + vl * direction[0];
  auto y = position[1] + vl * direction[1];
  auto z = position[2] + vl * direction[2];
  auto startPoint = track.startPoint();
  startPoint = V3D(x, y, z);
  track.clearIntersectionResults();
  track.reset(startPoint, track.direction());
}

/**
 * Validate the input properties.
 * @return a map where keys are property names and values the found issues
 */
std::map<std::string, std::string> MuscatElastic::validateInputs() {
  MatrixWorkspace_sptr inputWS = getProperty("InputWorkspace");
  std::map<std::string, std::string> issues;
  Geometry::IComponent_const_sptr sample =
      inputWS->getInstrument()->getSample();
  if (!sample) {
    issues["InputWorkspace"] = "Input workspace does not have a Sample";
  } else {
    if (inputWS->sample().hasEnvironment()) {
      issues["InputWorkspace"] = "Sample must not have a sample environment";
    }

    if (inputWS->sample().getMaterial().numberDensity() == 0) {
      issues["InputWorkspace"] =
          "Sample must have a material set up with a non-zero number density";
    }
  }
  MatrixWorkspace_sptr SQWS = getProperty("SofqWorkspace");
  auto y = SQWS->y(0);
  if (std::any_of(y.cbegin(), y.cend(),
                  [](const auto yval) { return (yval <= 0); })) {
    issues["SofqWorkspace"] = "S(Q) workspace must have all y > 0";
  }
  return issues;
}

/**
 * Factory method to return an instance of the required SparseInstrument class
 * @param modelWS The full workspace that the sparse one will be based on
 * @param wavelengthPoints The number of wavelength points to include in the
 * histograms in the sparse workspace
 * @param rows The number of rows of detectors to create
 * @param columns The number of columns of detectors to create
 * @return a pointer to an SparseInstrument object
 */
std::shared_ptr<SparseWorkspace>
MuscatElastic::createSparseWorkspace(const API::MatrixWorkspace &modelWS,
                                     const size_t wavelengthPoints,
                                     const size_t rows, const size_t columns) {
  auto sparseWS = std::make_shared<SparseWorkspace>(modelWS, wavelengthPoints,
                                                    rows, columns);
  return sparseWS;
}

MatrixWorkspace_sptr
MuscatElastic::createOutputWorkspace(const MatrixWorkspace &inputWS) const {
  MatrixWorkspace_uptr outputWS = DataObjects::create<Workspace2D>(inputWS);
  // The algorithm computes the signal values at bin centres so they should
  // be treated as a distribution
  outputWS->setDistribution(true);
  outputWS->setYUnit("");
  outputWS->setYUnitLabel("Scattered Intensity");
  return outputWS;
}

} // namespace Algorithms
} // namespace Mantid