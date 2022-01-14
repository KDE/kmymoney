/*

    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
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

ForecastView::ForecastView(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args) :
    KMyMoneyPlugin::Plugin(parent, metaData, args),
    m_view(nullptr)
{
    Q_INIT_RESOURCE(forecastview);

    // For information, announce that we have been loaded.
    qDebug("Plugins: forecastview loaded");
}

ForecastView::~ForecastView()
{
    qDebug("Plugins: forecastview unloaded");
}

void ForecastView::plug(KXMLGUIFactory* guiFactory)
{
    Q_UNUSED(guiFactory)
    m_view = new KForecastView;

    // Tell the host application to load my GUI component
    const auto rcFileName = QLatin1String("forecastview.rc");
    setXMLFile(rcFileName);

    // create my actions and menus
    m_view->createActions(this);

    viewInterface()->addView(m_view, i18n("Forecast"), View::Forecast, Icons::Icon::Forecast);
}

void ForecastView::unplug()
{
    m_view->removeActions();
    viewInterface()->removeView(View::Forecast);
}

K_PLUGIN_CLASS_WITH_JSON(ForecastView, "forecastview.json")

#include "forecastview.moc"
