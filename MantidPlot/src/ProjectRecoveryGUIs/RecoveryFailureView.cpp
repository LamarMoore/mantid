// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "RecoveryFailureView.h"
#include "ApplicationWindow.h"
#include "MantidKernel/UsageService.h"
#include "Script.h"
#include "ScriptingWindow.h"
#include "ui_RecoveryFailure.h"
#include <boost/smart_ptr/make_shared.hpp>
#include <memory>

RecoveryFailureView::RecoveryFailureView(QWidget *parent,
                                         ProjectRecoveryPresenter *presenter)
    : QDialog(parent), ui(std::make_unique<Ui::RecoveryFailure>()),
      m_presenter(presenter) {
  ui->setupUi(this);
  ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->tableWidget->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  // Make sure the UI has all the data it needs to display
  m_presenter->fillAllRows();
  // Set the table information
  addDataToTable();
  Mantid::Kernel::UsageService::Instance().registerFeatureUsage(
      "Interface", "ProjectRecoveryFailureWindow", true);
}

void RecoveryFailureView::addDataToTable() {
  // This table's size was generated for 5 which is the default but will take
  // more or less than 5, but won't look as neat
  const auto numberOfRows = m_presenter->getNumberOfCheckpoints();
  for (auto i = 0; i < numberOfRows; ++i) {
    const auto row = m_presenter->getRow(i);
    for (auto j = 0; j < row.size(); ++j) {
      ui->tableWidget->setItem(i, j, new QTableWidgetItem(row[j]));
    }
  }
}

void RecoveryFailureView::onClickLastCheckpoint() {
  // Recover last checkpoint
  m_presenter->recoverLast();
  Mantid::Kernel::UsageService::Instance().registerFeatureUsage(
      "Feature", "ProjectRecoveryFailureWindow->RecoverLastCheckpoint", false);
}

void RecoveryFailureView::onClickSelectedCheckpoint() {
  // Recover Selected
  QList<QTableWidgetItem *> selectedRows = ui->tableWidget->selectedItems();
  if (selectedRows.size() > 0) {
    const QString text = selectedRows[0]->text();
    if (text.toStdString().empty()) {
      return;
    }
    m_presenter->recoverSelectedCheckpoint(text);
  }
  Mantid::Kernel::UsageService::Instance().registerFeatureUsage(
      "Feature", "ProjectRecoveryFailureWindow->RecoverSelectedCheckpoint",
      false);
}

void RecoveryFailureView::onClickOpenSelectedInScriptWindow() {
  // Open checkpoint in script window
  QList<QTableWidgetItem *> selectedRows = ui->tableWidget->selectedItems();
  if (selectedRows.size() > 0) {
    const QString text = selectedRows[0]->text();
    if (text.toStdString().empty()) {
      return;
    }
    m_presenter->openSelectedInEditor(text);
  }
  Mantid::Kernel::UsageService::Instance().registerFeatureUsage(
      "Feature", "ProjectRecoveryFailureWindow->OpenSelectedInScriptWindow",
      false);
}

void RecoveryFailureView::onClickStartMantidNormally() {
  // Start save and close this, clear checkpoint that was offered for load
  m_presenter->startMantidNormally();
  Mantid::Kernel::UsageService::Instance().registerFeatureUsage(
      "Feature", "ProjectRecoveryFailureWindow->StartMantidNormally", false);
}

void RecoveryFailureView::reject() {
  // Do nothing just absorb request
  m_presenter->startMantidNormally();
  Mantid::Kernel::UsageService::Instance().registerFeatureUsage(
      "Feature", "ProjectRecoveryFailureWindow->StartMantidNormally", false);
}

void RecoveryFailureView::updateProgressBar(const int newValue,
                                            const bool err) {
  if (!err) {
    ui->progressBar->setValue(newValue);
  }
}

void RecoveryFailureView::setProgressBarMaximum(const int newValue) {
  ui->progressBar->setMaximum(newValue);
}

void RecoveryFailureView::connectProgressBar() {
  connect(&m_presenter->m_mainWindow->getScriptWindowHandle()
               ->getCurrentScriptRunner(),
          SIGNAL(currentLineChanged(int, bool)), this,
          SLOT(updateProgressBar(int, bool)));
}

void RecoveryFailureView::emitAbortScript() {
  connect(this, SIGNAL(abortProjectRecoveryScript()),
          m_presenter->m_mainWindow->getScriptWindowHandle(),
          SLOT(abortCurrent()));
  emit(abortProjectRecoveryScript());
}

void RecoveryFailureView::changeStartMantidButton(const QString &string) {
  ui->pushButton_3->setText(string);
}