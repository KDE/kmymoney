/***************************************************************************
                            onlinejoboutboxview.cpp
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

#include "onlinejoboutboxview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KPluginFactory>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "viewinterface.h"
#include "konlinejoboutboxview.h"

OnlineJobOutboxView::OnlineJobOutboxView(QObject *parent, const QVariantList &args) :
  KMyMoneyPlugin::Plugin(parent, "onlinejoboutboxview"/*must be the same as X-KDE-PluginInfo-Name*/),
  m_view(nullptr)
{
  Q_UNUSED(args)
  setComponentName("onlinejoboutboxview", i18n("Reports view"));
  // For information, announce that we have been loaded.
  qDebug("Plugins: onlinejoboutboxview loaded");
}

OnlineJobOutboxView::~OnlineJobOutboxView()
{
  qDebug("Plugins: onlinejoboutboxview unloaded");
}

void OnlineJobOutboxView::plug()
{
  m_view = new KOnlineJobOutboxView;
  viewInterface()->addView(m_view, i18n("Outbox"), View::OnlineJobOutbox);
}

void OnlineJobOutboxView::unplug()
{
  viewInterface()->removeView(View::OnlineJobOutbox);
}

K_PLUGIN_FACTORY_WITH_JSON(OnlineJobOutboxViewFactory, "onlinejoboutboxview.json", registerPlugin<OnlineJobOutboxView>();)

#include "onlinejoboutboxview.moc"
