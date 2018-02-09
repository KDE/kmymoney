/***************************************************************************
                          appinterface.h
                             -------------------
    copyright            : (C) 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef APPINTERFACE_H
#define APPINTERFACE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <kmm_plugin_export.h>

class QTimer;

namespace KMyMoneyPlugin
{
  class KMM_PLUGIN_EXPORT AppInterface : public QObject
  {
    Q_OBJECT

  public:
    explicit AppInterface(QObject* parent, const char* name = 0);
    virtual ~AppInterface();

    virtual QUrl filenameURL() const = 0;
    virtual QUrl lastOpenedURL() = 0;
    virtual void writeLastUsedFile(const QString& fileName) = 0;
    virtual void slotFileOpenRecent(const QUrl &url) = 0;
    virtual void addToRecentFiles(const QUrl& url) = 0;
    virtual void updateCaption(bool skipActions = false) = 0;
    virtual QTimer* autosaveTimer() = 0;
  };

}

#endif
