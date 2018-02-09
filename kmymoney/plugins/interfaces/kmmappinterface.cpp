/***************************************************************************
                          kmmappinterface.cpp
                             -------------------
    begin                : Mon Apr 14 2008
    copyright            : (C) 2008 Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmmappinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"

KMyMoneyPlugin::KMMAppInterface::KMMAppInterface(KMyMoneyApp* app, QObject* parent, const char* name) :
    AppInterface(parent, name),
    m_app(app)
{
}

QUrl KMyMoneyPlugin::KMMAppInterface::filenameURL() const
{
  return m_app->filenameURL();
}

QUrl KMyMoneyPlugin::KMMAppInterface::lastOpenedURL()
{
  return m_app->lastOpenedURL();
}

void KMyMoneyPlugin::KMMAppInterface::writeLastUsedFile(const QString& fileName)
{
  m_app->writeLastUsedFile(fileName);
}

void KMyMoneyPlugin::KMMAppInterface::slotFileOpenRecent(const QUrl &url)
{
  m_app->slotFileOpenRecent(url);
}

void KMyMoneyPlugin::KMMAppInterface::addToRecentFiles(const QUrl& url)
{
  m_app->addToRecentFiles(url);
}

void KMyMoneyPlugin::KMMAppInterface::updateCaption(bool skipActions)
{
 m_app->updateCaption(skipActions);
}

QTimer* KMyMoneyPlugin::KMMAppInterface::autosaveTimer()
{
 return m_app->autosaveTimer();
}
