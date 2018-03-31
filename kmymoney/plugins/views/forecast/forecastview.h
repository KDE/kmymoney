/***************************************************************************
                             forecastview.h
                             -------------------
    copyright            : (C) 2018 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FORECASTVIEW_H
#define FORECASTVIEW_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class KForecastView;

class ForecastView : public KMyMoneyPlugin::Plugin
{
  Q_OBJECT

public:
  explicit ForecastView(QObject *parent, const QVariantList &args);
  ~ForecastView() override;

  void plug() override;
  void unplug() override;

private:
  KForecastView* m_view;
};

#endif
