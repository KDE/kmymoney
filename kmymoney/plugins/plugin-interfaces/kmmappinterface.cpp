/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmmappinterface.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoney.h"

KMyMoneyPlugin::KMMAppInterface::KMMAppInterface(KMyMoneyApp* app, QObject* parent, const char* name)
    : AppInterface(parent, name)
    , m_app(app)
{
    connect(m_app, &KMyMoneyApp::kmmFilePlugin, this, &AppInterface::kmmFilePlugin);
}

bool KMyMoneyPlugin::KMMAppInterface::fileOpen()
{
    return m_app->fileOpen();
}

bool KMyMoneyPlugin::KMMAppInterface::isDatabase()
{
    return m_app->isDatabase();
}

bool KMyMoneyPlugin::KMMAppInterface::isNativeFile()
{
    return m_app->isNativeFile();
}

QUrl KMyMoneyPlugin::KMMAppInterface::filenameURL() const
{
    return m_app->filenameURL();
}

void KMyMoneyPlugin::KMMAppInterface::writeFilenameURL(const QUrl& url)
{
    m_app->writeFilenameURL(url);
}

QUrl KMyMoneyPlugin::KMMAppInterface::lastOpenedURL()
{
    return m_app->lastOpenedURL();
}

void KMyMoneyPlugin::KMMAppInterface::writeLastUsedFile(const QString& fileName)
{
    m_app->writeLastUsedFile(fileName);
}

void KMyMoneyPlugin::KMMAppInterface::slotFileOpenRecent(const QUrl& url)
{
    m_app->slotFileOpenRecent(url);
}

void KMyMoneyPlugin::KMMAppInterface::addToRecentFiles(const QUrl& url)
{
    m_app->addToRecentFiles(url);
}

KMyMoneyAppCallback KMyMoneyPlugin::KMMAppInterface::progressCallback()
{
    return m_app->progressCallback();
}

void KMyMoneyPlugin::KMMAppInterface::writeLastUsedDir(const QString& directory)
{
    m_app->writeLastUsedDir(directory);
}

QString KMyMoneyPlugin::KMMAppInterface::readLastUsedDir() const
{
    return m_app->readLastUsedDir();
}

void KMyMoneyPlugin::KMMAppInterface::consistencyCheck(bool alwaysDisplayResult)
{
    m_app->consistencyCheck(alwaysDisplayResult);
}
