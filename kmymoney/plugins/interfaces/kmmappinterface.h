/*
    SPDX-FileCopyrightText: 2008 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMMAPPINTERFACE_H
#define KMMAPPINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QUrl>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "appinterface.h"

class KMyMoneyApp;

namespace KMyMoneyPlugin
{
/**
* This class represents the implementation of the
* AppInterface.
*/
class KMMAppInterface : public AppInterface
{
    Q_OBJECT

public:
    explicit KMMAppInterface(KMyMoneyApp* app, QObject* parent, const char* name = 0);
    ~KMMAppInterface() override = default;

    /**
      * Makes sure that a MyMoneyFile is open and has been created successfully.
      *
      * @return Whether the file is open and initialised
      */
    bool fileOpen() override;

    bool isDatabase() override;
    bool isNativeFile() override;
    QUrl filenameURL() const override;
    void writeFilenameURL(const QUrl &url) override;
    QUrl lastOpenedURL() override;
    void writeLastUsedFile(const QString& fileName) override;
    void slotFileOpenRecent(const QUrl &url) override;
    void addToRecentFiles(const QUrl& url) override;
    KMyMoneyAppCallback progressCallback() override;
    void writeLastUsedDir(const QString &directory) override;
    QString readLastUsedDir() const override;
    void consistencyCheck(bool alwaysDisplayResult) override;

private:
    KMyMoneyApp* m_app;
};

}

#endif
