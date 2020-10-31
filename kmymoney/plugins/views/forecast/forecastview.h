/*
 * Copyright (C) 2018      Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright (C) 2020      Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
