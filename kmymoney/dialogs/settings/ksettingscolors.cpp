/*
 * SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ksettingscolors.h"
#include "kmymoneysettings.h"

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
  ui->kcfg_transactionErroneousColor->setColor(KMyMoneySettings::schemeColor(SchemeColor::TransactionErroneous));
  ui->kcfg_missingConversionRateColor->setColor(KMyMoneySettings::schemeColor(SchemeColor::MissingConversionRate));
  ui->kcfg_groupMarkerColor->setColor(KMyMoneySettings::schemeColor(SchemeColor::GroupMarker));
  ui->kcfg_fieldRequiredColor->setColor(KMyMoneySettings::schemeColor(SchemeColor::FieldRequired));
  ui->kcfg_transactionImportedColor->setColor(KMyMoneySettings::schemeColor(SchemeColor::TransactionImported));
  ui->kcfg_transactionMatchedColor->setColor(KMyMoneySettings::schemeColor(SchemeColor::TransactionMatched));
}
