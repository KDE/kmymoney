/*

    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "forecastview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "kforecastview.h"

ForecastView::ForecastView(QObject *parent, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, "forecastview"/*must be the same as X-KDE-PluginInfo-Name*/),
    m_view(nullptr)
{
    Q_UNUSED(args)
    setComponentName("forecastview", i18n("Forecast view"));
    // For information, announce that we have been loaded.
    qDebug("Plugins: forecastview loaded");
}

ForecastView::~ForecastView()
{
    qDebug("Plugins: forecastview unloaded");
}

void ForecastView::plug()
{
    m_view = new KForecastView;
    viewInterface()->addView(m_view, i18n("Forecast"), View::Forecast, Icons::Icon::Forecast);
}

void ForecastView::unplug()
{
    viewInterface()->removeView(View::Forecast);
}

K_PLUGIN_FACTORY_WITH_JSON(ForecastViewFactory, "forecastview.json", registerPlugin<ForecastView>();)

#include "forecastview.moc"
