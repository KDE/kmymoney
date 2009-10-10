/***************************************************************************
                             ksettingsforecast.cpp
                             --------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingsforecast.h"

KSettingsForecast::KSettingsForecast(QWidget* parent) :
  KSettingsForecastDecl(parent)
{
  kcfg_forecastMethod->setId(radioButton9, 0); // scheduled and future transactions
  kcfg_forecastMethod->setId(radioButton10, 1); // history based

  kcfg_historyMethod->setId(radioButton11, 0); // simple moving average
  kcfg_historyMethod->setId(radioButton12, 1); // weighted moving average
  kcfg_historyMethod->setId(radioButton13, 2); // linear regression
}

KSettingsForecast::~KSettingsForecast()
{
}

#include "ksettingsforecast.moc"
