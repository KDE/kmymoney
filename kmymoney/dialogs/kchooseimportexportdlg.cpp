/***************************************************************************
                          kchooseimportexportdlg.cpp  -  description
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 2000-2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@users.sourceforge.net>
                             Felix Rodriguez <frodriguez@users.sourceforge.net>
                             John C <thetacoturtle@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

#include "ui_kchooseimportexportdlgdecl.h"

struct KChooseImportExportDlg::Private {
  Ui::KChooseImportExportDlgDecl ui;
};

KChooseImportExportDlg::KChooseImportExportDlg(int type, QWidget *parent)
    : QDialog(parent), d(new Private)
{
  d->ui.setupUi(this);

  QString filename;
  setModal(true);

  if (type == 0) { // import
    d->ui.topLabel->setText(i18n("Please choose the type of import you wish to perform.  A simple explanation\n"
                                 "of the import type is available at the bottom of the screen and is updated when\n"
                                 "you select an item from the choice box."
                                 "\n\nOnce you have chosen an import type please press the OK button."));
    d->ui.promptLabel->setText(i18n("Choose import type:"));
    setWindowTitle(i18n("Choose Import Type Dialog"));
  } else { // export
    d->ui.topLabel->setText(i18n("Please choose the type of export you wish to perform.  A simple explanation\n"
                                 "of the export type is available at the bottom of the screen and is updated when\n"
                                 "you select an item from the choice box."
                                 "\n\nOnce you have chosen an export type please press the OK button."));
    d->ui.promptLabel->setText(i18n("Choose export type:"));
    setWindowTitle(i18n("Choose Export Type Dialog"));
  }

  readConfig();
  slotTypeActivated(m_lastType);
  d->ui.typeCombo->setCurrentItem(((m_lastType == "QIF") ? i18n("QIF") : i18n("CSV")), false);

  connect(d->ui.typeCombo, SIGNAL(activated(QString)), this, SLOT(slotTypeActivated(QString)));
  connect(d->ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(d->ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

KChooseImportExportDlg::~KChooseImportExportDlg()
{
  writeConfig();
  delete d;
}

void KChooseImportExportDlg::slotTypeActivated(const QString& text)
{
  if (text == "QIF") {
    d->ui.descriptionLabel->setText(i18n("QIF files are created by the popular accounting program Quicken.\n"
                                         "Another dialog will appear, if you choose this type, asking for further\n"
                                         "information relevant to the Quicken format."));
  } else {
    d->ui.descriptionLabel->setText(i18n("The CSV type uses a comma delimited text file that can be used by\n"
                                         "most popular spreadsheet programs available for Linux and other operating\n"
                                         "systems."));
  }
}

QString KChooseImportExportDlg::importExportType()
{
  return d->ui.typeCombo->currentText();
}

void KChooseImportExportDlg::readConfig()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  m_lastType = grp.readEntry("KChooseImportExportDlg_LastType");
}

void KChooseImportExportDlg::writeConfig()
{
  KSharedConfigPtr config = KSharedConfig::openConfig();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KChooseImportExportDlg_LastType", d->ui.typeCombo->currentText());
  config->sync();
}
