/***************************************************************************
                            reportsview.cpp
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

#include "reportsview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "kreportsview.h"

ReportsView::ReportsView(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "reportsview"/*must be the same as X-KDE-PluginInfo-Name*/),
  m_view(nullptr)
{
  Q_UNUSED(args)
  setComponentName("reportsview", i18n("Reports view"));
  // For information, announce that we have been loaded.
  qDebug("Plugins: reportsview loaded");
}

ReportsView::~ReportsView()
{
  qDebug("Plugins: reportsview unloaded");
}

void ReportsView::plug()
{
  m_view = new KReportsView;
  viewInterface()->addView(m_view, i18n("Reports"), View::Reports);
}

void ReportsView::unplug()
{
  viewInterface()->removeView(View::Reports);
}

K_PLUGIN_FACTORY_WITH_JSON(ReportsViewFactory, "reportsview.json", registerPlugin<ReportsView>();)

#include "reportsview.moc"
