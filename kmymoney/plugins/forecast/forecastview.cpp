/***************************************************************************
                            forecastview.cpp
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
#include "kmymoneyglobalsettings.h"

ForecastView::ForecastView(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "forecastview"/*must be the same as X-KDE-PluginInfo-Name*/)
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

void ForecastView::injectExternalSettings(KMyMoneySettings* p)
{
  KMyMoneyGlobalSettings::injectExternalSettings(p);
}

void ForecastView::plug()
{
  m_view = new KForecastView;
  viewInterface()->addView(m_view, i18n("Forecast"), View::Forecast);
}

void ForecastView::unplug()
{
  viewInterface()->removeView(View::Forecast);
}

K_PLUGIN_FACTORY_WITH_JSON(ForecastViewFactory, "forecastview.json", registerPlugin<ForecastView>();)

#include "forecastview.moc"
