/***************************************************************************
                             ksettingscolors.cpp
                             --------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowiczd@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ksettingscolors.h"
#include "kmymoneyglobalsettings.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingscolors.h"

KSettingsColors::KSettingsColors(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::KSettingsColors)
{
  ui->setupUi(this);
  connect(ui->kcfg_useCustomColors, &QGroupBox::toggled, this, &KSettingsColors::slotCustomColorsToggled);
}

KSettingsColors::~KSettingsColors()
{
  delete ui;
}

void KSettingsColors::slotCustomColorsToggled(bool)
{
  ui->kcfg_transactionErroneousColor->setColor(KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionErroneous));
  ui->kcfg_missingConversionRateColor->setColor(KMyMoneyGlobalSettings::schemeColor(SchemeColor::MissingConversionRate));
  ui->kcfg_groupMarkerColor->setColor(KMyMoneyGlobalSettings::schemeColor(SchemeColor::GroupMarker));
  ui->kcfg_fieldRequiredColor->setColor(KMyMoneyGlobalSettings::schemeColor(SchemeColor::FieldRequired));
  ui->kcfg_transactionImportedColor->setColor(KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionImported));
  ui->kcfg_transactionMatchedColor->setColor(KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionMatched));
}
