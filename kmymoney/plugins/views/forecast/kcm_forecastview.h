/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KCM_FORECASTVIEW_H
#define KCM_FORECASTVIEW_H

#include "kmm_kcmodule.h"
#include "ui_forecastviewsettings.h"

class ForecastViewSettingsWidget : public QWidget, public Ui::ForecastViewSettings
{
    Q_OBJECT
public:
    explicit ForecastViewSettingsWidget(QWidget* parent = nullptr);
};

class KCMForecastView : public KMMKCModule
{
public:
    explicit KCMForecastView(QObject* parent, const QVariantList& args = QVariantList());
    ~KCMForecastView();
};

#endif

