/***************************************************************************
                             ksettingsforecast.cpp
                             --------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#include "ksettingsforecast.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsforecast.h"

KSettingsForecast::KSettingsForecast(QWidget* parent) :
  QWidget(parent),
  ui(new Ui::KSettingsForecast)
{
  ui->setupUi(this);
}

KSettingsForecast::~KSettingsForecast()
{
  delete ui;
}
