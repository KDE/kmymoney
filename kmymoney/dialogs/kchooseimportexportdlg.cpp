/*
 * SPDX-FileCopyrightText: 2001-2002 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2001-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kchooseimportexportdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kchooseimportexportdlg.h"

class KChooseImportExportDlgPrivate
{
  Q_DISABLE_COPY(KChooseImportExportDlgPrivate)

public:
  KChooseImportExportDlgPrivate() :
    ui(new Ui::KChooseImportExportDlg)
  {
  }

  ~KChooseImportExportDlgPrivate()
  {
    delete ui;
  }

  void readConfig()
  {
    auto config = KSharedConfig::openConfig();
    auto grp = config->group("Last Use Settings");
    m_lastType = grp.readEntry("KChooseImportExportDlg_LastType");
  }

  void writeConfig() const
  {
    auto config = KSharedConfig::openConfig();
    auto grp = config->group("Last Use Settings");
    grp.writeEntry("KChooseImportExportDlg_LastType", ui->typeCombo->currentText());
    config->sync();
  }

  Ui::KChooseImportExportDlg *ui;
  QString m_lastType;
};


KChooseImportExportDlg::KChooseImportExportDlg(int type, QWidget *parent) :
  QDialog(parent),
  d_ptr(new KChooseImportExportDlgPrivate)
{
  Q_D(KChooseImportExportDlg);
  d->ui->setupUi(this);
  setModal(true);

  if (type == 0) { // import
    d->ui->topLabel->setText(i18n("Please choose the type of import you wish to perform.  A simple explanation\n"
                                 "of the import type is available at the bottom of the screen and is updated when\n"
                                 "you select an item from the choice box."
                                 "\n\nOnce you have chosen an import type please press the OK button."));
    d->ui->promptLabel->setText(i18n("Choose import type:"));
    setWindowTitle(i18n("Choose Import Type Dialog"));
  } else { // export
    d->ui->topLabel->setText(i18n("Please choose the type of export you wish to perform.  A simple explanation\n"
                                 "of the export type is available at the bottom of the screen and is updated when\n"
                                 "you select an item from the choice box."
                                 "\n\nOnce you have chosen an export type please press the OK button."));
    d->ui->promptLabel->setText(i18n("Choose export type:"));
    setWindowTitle(i18n("Choose Export Type Dialog"));
  }

  d->readConfig();
  slotTypeActivated(d->m_lastType);
  d->ui->typeCombo->setCurrentItem(((d->m_lastType == "QIF") ? i18n("QIF") : i18n("CSV")), false);

  connect(d->ui->typeCombo, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated), this, &KChooseImportExportDlg::slotTypeActivated);
}

KChooseImportExportDlg::~KChooseImportExportDlg()
{
  Q_D(KChooseImportExportDlg);
  d->writeConfig();
  delete d;
}

void KChooseImportExportDlg::slotTypeActivated(const QString& text)
{
  Q_D(KChooseImportExportDlg);
  if (text == "QIF") {
    d->ui->descriptionLabel->setText(i18n("QIF files are created by the popular accounting program Quicken.\n"
                                         "Another dialog will appear, if you choose this type, asking for further\n"
                                         "information relevant to the Quicken format."));
  } else {
    d->ui->descriptionLabel->setText(i18n("The CSV type uses a comma delimited text file that can be used by\n"
                                         "most popular spreadsheet programs available for Linux and other operating\n"
                                         "systems."));
  }
}

QString KChooseImportExportDlg::importExportType() const
{
  Q_D(const KChooseImportExportDlg);
  return d->ui->typeCombo->currentText();
}
