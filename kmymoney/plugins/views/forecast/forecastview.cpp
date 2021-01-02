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
  const auto componentName = QLatin1String("forecastview");
  const auto rcFileName = QLatin1String("forecastview.rc");
  setComponentName(componentName, i18n("Forecast view"));

  #ifdef IS_APPIMAGE
  const QString rcFilePath = QString("%1/../share/kxmlgui5/%2/%3").arg(QCoreApplication::applicationDirPath(), componentName, rcFileName);
  setXMLFile(rcFilePath);

  const QString localRcFilePath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first() + QLatin1Char('/') + componentName + QLatin1Char('/') + rcFileName;
  setLocalXMLFile(localRcFilePath);
  #else
  setXMLFile(rcFileName);
  #endif

  // create my actions and menus
  m_view->createActions(this);

  viewInterface()->addView(m_view, i18n("Forecast"), View::Forecast, Icons::Icon::Forecast);
}

void ForecastView::unplug()
{
  m_view->removeActions();
  viewInterface()->removeView(View::Forecast);
}

K_PLUGIN_FACTORY_WITH_JSON(ForecastViewFactory, "forecastview.json", registerPlugin<ForecastView>();)

#include "forecastview.moc"
