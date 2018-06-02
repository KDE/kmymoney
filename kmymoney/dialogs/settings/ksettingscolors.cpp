/*
 * Copyright 2005-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
