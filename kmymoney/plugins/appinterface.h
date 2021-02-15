/***************************************************************************
                          appinterface.h
                             -------------------
    copyright            : (C) 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
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

class IMyMoneyOperationsFormat;
typedef  void (*KMyMoneyAppCallback)(int, int, const QString &);

namespace KMyMoneyPlugin
{
  class KMM_PLUGIN_EXPORT AppInterface : public QObject
  {
    Q_OBJECT

  public:
    explicit AppInterface(QObject* parent, const char* name = 0);
    virtual ~AppInterface();

    /**
      * Makes sure that a MyMoneyFile is open and has been created successfully.
      *
      * @return Whether the file is open and initialised
      */
    virtual bool fileOpen() = 0;

    virtual bool isDatabase() = 0;
    virtual bool isNativeFile() = 0;
    virtual QUrl filenameURL() const = 0;
    virtual void writeFilenameURL(const QUrl &url) = 0;
    virtual QUrl lastOpenedURL() = 0;
    virtual void writeLastUsedFile(const QString& fileName) = 0;
    virtual void slotFileOpenRecent(const QUrl &url) = 0;
    virtual void addToRecentFiles(const QUrl& url) = 0;
    virtual KMyMoneyAppCallback progressCallback() = 0;
    virtual void writeLastUsedDir(const QString &directory) = 0;
    virtual QString readLastUsedDir() const = 0;
    virtual void consistencyCheck(bool alwaysDisplayResult) = 0;

   Q_SIGNALS:
    void kmmFilePlugin(unsigned int);
  };

}

#endif
