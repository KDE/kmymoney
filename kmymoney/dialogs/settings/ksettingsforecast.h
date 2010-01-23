/***************************************************************************
                          ksettingsforecast.h
                             -------------------
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

#ifndef KSETTINGSFORECAST_H
#define KSETTINGSFORECAST_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsforecastdecl.h"

class KSettingsForecastDecl : public QWidget, public Ui::KSettingsForecastDecl
{
public:
  KSettingsForecastDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};


class KSettingsForecast : public KSettingsForecastDecl
{
  Q_OBJECT

public:
  KSettingsForecast(QWidget* parent = 0);
  ~KSettingsForecast();
};
#endif

