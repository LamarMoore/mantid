// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_ALGORITHMS_PARALLAXCORRECTION_H_
#define MANTID_ALGORITHMS_PARALLAXCORRECTION_H_

#include "MantidAlgorithms/DllConfig.h"
#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace Algorithms {

/** ParallaxCorrection : Performs geometrical correction for parallax effect in
 * tube based SANS instruments.
 */
class MANTID_ALGORITHMS_DLL ParallaxCorrection : public API::Algorithm {
public:
  const std::string name() const override;
  int version() const override;
  const std::string category() const override;
  const std::string summary() const override;

private:
  void init() override;
  void exec() override;
};

} // namespace Algorithms
} // namespace Mantid

#endif /* MANTID_ALGORITHMS_PARALLAXCORRECTION_H_ */
