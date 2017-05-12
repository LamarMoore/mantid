#include "MantidCurveFitting/Functions/CrystalFieldControl.h"
#include "MantidCurveFitting/Functions/CrystalFieldPeaks.h"
#include "MantidKernel/Exception.h"
#include "MantidKernel/StringTokenizer.h"
#include "MantidKernel/Strings.h"

namespace Mantid {
namespace CurveFitting {
namespace Functions {

using namespace API;

CrystalFieldControl::CrystalFieldControl() : CompositeFunction() {
  // A comma-separated list of ion names
  declareAttribute("Ions", Attribute(""));
  // A comma-separated list of symmetry names
  declareAttribute("Symmetries", Attribute(""));
  // Temperature values for each spectrum.
  declareAttribute("Temperatures", Attribute(std::vector<double>()));
  // Default widths for peaks in each spectrum. If given it must have
  // the same size as Temperatures or size == 1 in which case it's used
  // for all spectra.
  declareAttribute("FWHMs", Attribute(std::vector<double>()));
  // Variation in FWHM of peaks when with model is used (FWHMX and FWHMY)
  declareAttribute("FWHMVariation", Attribute(0.1));
  // Definition of the background function
  declareAttribute("Background", Attribute(""));
  // Name of a IPeakFunction to use for peaks
  declareAttribute("PeakShape", Attribute("Lorentzian"));
  // Energy tolerance in crystal field calculations
  declareAttribute("ToleranceEnergy", Attribute(1.0e-10));
  // Intensity tolerance in crystal field calculations
  declareAttribute("ToleranceIntensity", Attribute(1.0e-1));
  // A comma-separated list of physical properties
  declareAttribute("PhysicalProperties", Attribute(""));
}

/// Parse a comma-separated list attribute
/// @param attName :: A name of the attribute to parse.
/// @param cache :: A vector to chache the parsed values.
void CrystalFieldControl::parseStringListAttribute(const std::string &attName, std::vector<std::string> &cache) {
  // Split the attribute value into separate names, white spaces removed
  auto attr = getAttribute(attName).asString();
  Kernel::StringTokenizer tokenizer(attr, ",",
                                    Kernel::StringTokenizer::TOK_TRIM);
  cache.clear();
  cache.insert(cache.end(), tokenizer.begin(), tokenizer.end());
  auto attrValue = Kernel::Strings::join(cache.begin(), cache.end(), ",");
  // Store back the trimmed names
  setAttribute(attName, Attribute(attrValue));
}


/// Cache function attributes.
void CrystalFieldControl::cacheAttributes() {
  parseStringListAttribute("Ions", m_ions);
  parseStringListAttribute("Symmetries", m_symmetries);
  parseStringListAttribute("PhysicalProperties", m_physProps);
  m_temperatures = getAttribute("Temperatures").asVector();
  buildControls();
  m_FWHMs = getAttribute("FWHMs").asVector();
  const auto nSpec = m_temperatures.size();
  m_fwhmX.clear();
  m_fwhmY.clear();
  for (size_t i = 0; i < nSpec; ++i) {
    auto &control = *getFunction(i);
    auto fwhmX = control.getAttribute("FWHMX").asVector();
    auto fwhmY = control.getAttribute("FWHMY").asVector();
    m_fwhmX.push_back(fwhmX);
    m_fwhmY.push_back(fwhmY);
  }
}

/// Check that all attributes are consistent
void CrystalFieldControl::checkConsistent()  {
  cacheAttributes();
  if (m_ions.empty()) {
    throw std::runtime_error("No ions are set.");
  }
  if (m_ions.size() != m_symmetries.size()) {
    throw std::runtime_error(
        "Number of ions is different from number of symmetries.");
  }
  if (!m_temperatures.empty()) {
    // If temperatures are given then there will be spectra with peaks
    // and peaks need some width information. It comes either from
    // FWHMs attribute or FWHMX and FWHMY attributes of the spectrum
    // control functions.
    const auto nSpec = m_temperatures.size();
    if (nSpec > nFunctions()) {
      // Some of the member control funtions must be spectra
      throw std::logic_error("Too few spectrum functions.");
    }
    // Check if the FWHMX and FWHMY attributes of the spectrum
    // control functions are set and if they are they have equal lengths
    bool allXYEmpty = true;
    for(size_t i = 0; i < nSpec; ++i) {
      auto specFun = getFunction(i).get();
      if (!dynamic_cast<CrystalFieldSpectrumControl*>(specFun)) {
        throw std::logic_error("CrystalFieldSpectrumControl function expected");
      }
      if (m_fwhmX[i].size() != m_fwhmY[i].size()) {
        throw std::runtime_error("Vectors in each pair of (FWHMX, FWHMY) attributes must have the same size");
      }
      allXYEmpty = allXYEmpty && m_fwhmX[i].empty() && m_fwhmY[i].empty();
    }
    if (m_FWHMs.empty()) {
      if (allXYEmpty) {
        // No width information given
        throw std::runtime_error("No peak width settings (FWHMs and FWHMX and "
                                 "FWHMY attributes not set).");
      }
    } else if (m_FWHMs.size() != nSpec) {
      if (m_FWHMs.size() == 1) {
        // If single value then use it for all spectra
        m_FWHMs.assign(nSpec, m_FWHMs.front());
      } else {
        // Otherwise it's an error
        throw std::runtime_error(
            "Vector of FWHMs must either have same size as "
            "Temperatures (" +
            std::to_string(nSpec) + ") or have size 1.");
      }
    } else if (!allXYEmpty) {
      // Conflicting width attributes
      throw std::runtime_error("Either FWHMs or (FWHMX and FWHMY) can be set but not all.");
    }
  }
}

/// Build control functions for individual spectra.
void CrystalFieldControl::buildControls() {
  if (m_temperatures.empty()) {
    throw std::runtime_error("No temperatures were defined.");
  }
  const auto nSpec = m_temperatures.size();
  this->clear();
  for(size_t i = 0; i < nSpec; ++i) {
    auto control = API::IFunction_sptr(new CrystalFieldSpectrumControl);
    addFunction(control);
  }
  const auto nPProps = m_physProps.size();
  for(size_t i = 0; i < nPProps; ++i) {
    auto control = API::IFunction_sptr(new CrystalFieldPhysPropControl);
    addFunction(control);
  }
}


/// Check if the function is set up for a multi-site calculations.
/// (Multiple ions defined)
bool CrystalFieldControl::isMultiSite() const { return m_ions.size() > 1; }

bool CrystalFieldControl::isMultiSpectrum() const {
  return m_temperatures.size() > 1 || !m_physProps.empty();
}

/// Build the source function.
API::IFunction_sptr CrystalFieldControl::buildSource()  {
  checkConsistent();
  if (isMultiSite()) {
    return buildMultiSite();
  } else {
    return buildSingleSite();
  }
}

/// Build the source function in a single site case.
API::IFunction_sptr CrystalFieldControl::buildSingleSite() {
  if (isMultiSpectrum()) {
    return buildSingleSiteMultiSpectrum();
  } else {
    return buildSingleSiteSingleSpectrum();
  }
}

/// Build the source function in a multi site case.
API::IFunction_sptr CrystalFieldControl::buildMultiSite() {
  if (isMultiSpectrum()) {
    return buildMultiSiteMultiSpectrum();
  } else {
    return buildMultiSiteSingleSpectrum();
  }
}

/// Build the source function in a single site - single spectrum case.
API::IFunction_sptr CrystalFieldControl::buildSingleSiteSingleSpectrum() {
  if (m_temperatures.empty()) {
    throw std::runtime_error("No tmperature was set.");
  }
  auto source = IFunction_sptr(new CrystalFieldPeaks);
  source->setAttributeValue("Ion", m_ions[0]);
  source->setAttributeValue("Symmetry", m_symmetries[0]);
  source->setAttribute("ToleranceEnergy", IFunction::getAttribute("ToleranceEnergy"));
  source->setAttribute("ToleranceIntensity", IFunction::getAttribute("ToleranceIntensity"));
  source->setAttributeValue("Temperature", m_temperatures[0]);
  return source;
}

/// Build the source function in a single site - multi spectrum case.
API::IFunction_sptr CrystalFieldControl::buildSingleSiteMultiSpectrum() {
  auto source = IFunction_sptr(new CrystalFieldPeaks);
  source->setAttributeValue("Ion", m_ions[0]);
  source->setAttributeValue("Symmetry", m_symmetries[0]);
  source->setAttribute("ToleranceEnergy", IFunction::getAttribute("ToleranceEnergy"));
  source->setAttribute("ToleranceIntensity", IFunction::getAttribute("ToleranceIntensity"));
  return source;
}

/// Build the source function in a multi site - single spectrum case.
API::IFunction_sptr CrystalFieldControl::buildMultiSiteSingleSpectrum() {
  auto multiSite = CompositeFunction_sptr(new CompositeFunction);
  auto nSites = m_ions.size();
  auto temperatures = getAttribute("Temperatures").asVector();
  auto temperature = temperatures[0];
  for(size_t i = 0; i < nSites; ++i) {
    auto peakSource = (IFunction_sptr(new CrystalFieldPeaks));
    multiSite->addFunction(peakSource);
    peakSource->setAttributeValue("Ion", m_ions[i]);
    peakSource->setAttributeValue("Symmetry", m_symmetries[i]);
    peakSource->setAttribute("ToleranceEnergy", IFunction::getAttribute("ToleranceEnergy"));
    peakSource->setAttribute("ToleranceIntensity", IFunction::getAttribute("ToleranceIntensity"));
    peakSource->setAttributeValue("Temperature", temperature);
  }
  return multiSite;
}

/// Build the source function in a multi site - multi spectrum case.
API::IFunction_sptr CrystalFieldControl::buildMultiSiteMultiSpectrum() {
  throw std::logic_error("buildMultiSiteMultiSpectrumSource not implemented yet.");
}

// ----------------------------------------------------------------------------------- //

CrystalFieldSpectrumControl::CrystalFieldSpectrumControl() : ParamFunction() {
  declareAttribute("FWHMX", Attribute(std::vector<double>()));
  declareAttribute("FWHMY", Attribute(std::vector<double>()));
  declareParameter("IntensityScaling", 1.0,
                   "Scales intensities of peaks in a spectrum.");
}

std::string CrystalFieldSpectrumControl::name() const {
  throw Kernel::Exception::NotImplementedError(
      "This method is intentionally not implemented.");
}

void CrystalFieldSpectrumControl::function(const API::FunctionDomain &,
                                           API::FunctionValues &) const {
  throw Kernel::Exception::NotImplementedError(
      "This method is intentionally not implemented.");
}

// ----------------------------------------------------------------------------------- //

CrystalFieldPhysPropControl::CrystalFieldPhysPropControl() : ParamFunction() {
}

std::string CrystalFieldPhysPropControl::name() const {
  throw Kernel::Exception::NotImplementedError(
      "This method is intentionally not implemented.");
}

void CrystalFieldPhysPropControl::function(const API::FunctionDomain &,
                                           API::FunctionValues &) const {
  throw Kernel::Exception::NotImplementedError(
      "This method is intentionally not implemented.");
}

} // namespace Functions
} // namespace CurveFitting
} // namespace Mantid
