/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KCM_FORECASTVIEW_H
#define KCM_FORECASTVIEW_H

#include <KCModule>
#include <QWidget>
#include "ui_forecastviewsettings.h"

class ForecastViewSettingsWidget : public QWidget, public Ui::ForecastViewSettings
{
  Q_OBJECT
public:
  explicit ForecastViewSettingsWidget(QWidget* parent = nullptr);
};

class KCMForecastView : public KCModule
{
public:
  explicit KCMForecastView(QWidget* parent, const QVariantList& args);
  ~KCMForecastView();
};

#endif

