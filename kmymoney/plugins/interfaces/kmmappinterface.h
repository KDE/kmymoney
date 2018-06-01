/***************************************************************************
                          kmmappinterface.h
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
    void updateCaption(bool skipActions = false) override;
    QTimer* autosaveTimer() override;
    KMyMoneyAppCallback progressCallback() override;
    void writeLastUsedDir(const QString &directory) override;
    QString readLastUsedDir() const override;
    void consistencyCheck(bool alwaysDisplayResult) override;

  private:
    KMyMoneyApp* m_app;
  };

}

#endif
