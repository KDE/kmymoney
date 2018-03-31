/***************************************************************************
                             ksettingsforecast.cpp
                             --------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
