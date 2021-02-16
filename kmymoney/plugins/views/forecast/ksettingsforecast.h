/*
    SPDX-FileCopyrightText: 2007 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

