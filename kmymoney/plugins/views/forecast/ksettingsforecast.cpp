/*
    SPDX-FileCopyrightText: 2007 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
