// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2007 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidAPI/IFunctionMW.h"
#include "MantidAPI/IFunctionWithLocation.h"
#include "MantidAPI/IPeakFunction.h"
#include "MantidCurveFitting/DllConfig.h"
#include <cmath>

namespace Mantid {
namespace CurveFitting {
namespace Functions {
/**
 Provide Dynamic Kubo Toyabe function interface to IFunction1D for muon
 scientists.

 @author Raquel Alvarez, ISIS, RAL
 @date 18/02/2015
 */

class MANTID_CURVEFITTING_DLL DynamicKuboToyabe : public API::ParamFunction,
                                                  public API::IFunction1D {
public:
  /// Constructor
  DynamicKuboToyabe();

  /// overwrite base class methods
  std::string name() const override { return "DynamicKuboToyabe"; }
  const std::string category() const override { return "Muon\\MuonGeneric"; }

  /// Set a value to attribute attName
  void setAttribute(const std::string &attName, const Attribute &) override;

protected:
  void function1D(double *out, const double *xValues,
                  const size_t nData) const override;
  void functionDeriv1D(API::Jacobian *out, const double *xValues,
                       const size_t nData) override;
  void functionDeriv(const API::FunctionDomain &domain,
                     API::Jacobian &jacobian) override;
  void init() override;
  void setActiveParameter(size_t i, double value) override;

private:
  // Evaluate DKT function for a given t, G, F, v and eps
  double getDKT(double t, double G, double F, double v, double eps) const;

  /// Bin width
  double m_eps;
  double m_minEps, m_maxEps;
};

} // namespace Functions
} // namespace CurveFitting
} // namespace Mantid
