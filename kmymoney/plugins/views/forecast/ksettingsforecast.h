/***************************************************************************
                          ksettingsforecast.h
                             -------------------
    copyright            : (C) 2007 by Alvaro Soliverez
    email                : asoliverez@gmail.com
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSFORECAST_H
#define KSETTINGSFORECAST_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KSettingsForecast; }

class KSettingsForecast : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsForecast)

public:
  explicit KSettingsForecast(QWidget* parent = nullptr);
  ~KSettingsForecast();

private:
  Ui::KSettingsForecast       *ui;
};
#endif

