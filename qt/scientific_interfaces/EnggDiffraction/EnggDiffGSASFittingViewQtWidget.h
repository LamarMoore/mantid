// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "DllConfig.h"
#include "EnggDiffMultiRunFittingQtWidget.h"
#include "IEnggDiffGSASFittingPresenter.h"
#include "IEnggDiffGSASFittingView.h"
#include "IEnggDiffractionParam.h"
#include "IEnggDiffractionPythonRunner.h"
#include "IEnggDiffractionUserMsg.h"

#include <memory>
#include <qwt_plot_curve.h>
#include <qwt_plot_zoomer.h>

#include "ui_EnggDiffractionQtTabGSAS.h"

namespace MantidQt {
namespace CustomInterfaces {

class MANTIDQT_ENGGDIFFRACTION_DLL EnggDiffGSASFittingViewQtWidget
    : public QWidget,
      public IEnggDiffGSASFittingView {
  Q_OBJECT

public:
  EnggDiffGSASFittingViewQtWidget(
      std::shared_ptr<IEnggDiffractionUserMsg> userMessageProvider,
      const std::shared_ptr<IEnggDiffractionPythonRunner> &pythonRunner,
      std::shared_ptr<IEnggDiffractionParam> mainSettings);

  ~EnggDiffGSASFittingViewQtWidget() override;

  void addWidget(IEnggDiffMultiRunFittingWidgetView *widget) override;

  void displayLatticeParams(
      const Mantid::API::ITableWorkspace_sptr latticeParams) const override;

  void displayGamma(const double gamma) const override;

  void displayRwp(const double rwp) const override;

  void displaySigma(const double sigma) const override;

  std::vector<std::string> getFocusedFileNames() const override;

  std::string getGSASIIProjectPath() const override;

  std::string getInstrumentFileName() const override;

  std::string getPathToGSASII() const override;

  boost::optional<double> getPawleyDMin() const override;

  boost::optional<double> getPawleyNegativeWeight() const override;

  std::vector<std::string> getPhaseFileNames() const override;

  GSASRefinementMethod getRefinementMethod() const override;

  bool getRefineGamma() const override;

  bool getRefineSigma() const override;

  boost::optional<double> getXMax() const override;

  boost::optional<double> getXMin() const override;

  void setEnabled(const bool enabled) override;

  void showStatus(const std::string &status) const override;

  void userError(const std::string &errorTitle,
                 const std::string &errorDescription) const override;

  void userWarning(const std::string &warningTitle,
                   const std::string &warningDescription) const override;

private slots:
  void browseFocusedRun();
  void browseGSASHome();
  void browseGSASProj();
  void browseInstParams();
  void browsePhaseFiles();
  void disableLoadIfInputEmpty();
  void doRefinement();
  void loadFocusedRun();
  void refineAll();
  void selectRun();

private:
  bool runFileLineEditEmpty() const;

  void setLoadEnabled(const bool enabled);

  static const char GSAS_HOME_SETTING_NAME[];
  static const char SETTINGS_NAME[];

  std::shared_ptr<EnggDiffMultiRunFittingQtWidget> m_multiRunWidgetView;

  std::shared_ptr<IEnggDiffGSASFittingPresenter> m_presenter;

  Ui::EnggDiffractionQtTabGSAS m_ui;

  std::shared_ptr<IEnggDiffractionUserMsg> m_userMessageProvider;

  void setFocusedRunFileNames(const QStringList &filenames);

  void setupUI();
};

} // namespace CustomInterfaces
} // namespace MantidQt
