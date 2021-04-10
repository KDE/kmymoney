/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz lukasz.wojnilowicz @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_forecastview.h"
#include <config-kmymoney-version.h>

// KDE includes
#include <KPluginFactory>
#include <KAboutData>
#include "kmymoneysettings.h"

ForecastViewSettingsWidget::ForecastViewSettingsWidget(QWidget* parent) :
    QWidget(parent)
{
    setupUi(this);
}

KCMForecastView::KCMForecastView(QWidget *parent, const QVariantList& args)
    : KCModule(parent, args)
{
    ForecastViewSettingsWidget* w = new ForecastViewSettingsWidget(this);
    // addConfig(ForecastViewSettings::self(), w);
    addConfig(KMyMoneySettings::self(), w);
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(w);
    setButtons(NoAdditionalButton);
    load();
}

KCMForecastView::~KCMForecastView()
{
}

K_PLUGIN_CLASS_WITH_JSON(KCMForecastView, "kcm_forecastview.json")

#include "kcm_forecastview.moc"
