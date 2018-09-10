/***************************************************************************
                             sqlstorage.h
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

#ifndef SQLSTORAGE_H
#define SQLSTORAGE_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyStorageMgr;
class QUrlQuery;

class SQLStorage : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::StoragePlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::StoragePlugin)

public:
  explicit SQLStorage(QObject *parent, const QVariantList &args);
  ~SQLStorage() override;

  QAction *m_openDBaction;
  QAction *m_saveAsDBaction;
  QAction *m_generateDB;

  MyMoneyStorageMgr *open(const QUrl &url) override;
  bool save(const QUrl &url) override;
  bool saveAs() override;
  eKMyMoney::StorageType storageType() const override;
  QString fileExtension() const override;

protected:
  void createActions();

private:
  /**
   * Saves the data into permanent storage on a new or empty SQL database.
   *
   * @param url The pseudo URL of the database
   *
   * @retval false save operation failed
   * @retval true save operation was successful
   */
  bool saveAsDatabase(const QUrl &url);

  QUrlQuery convertOldUrl(const QUrl& url);

private Q_SLOTS:
  void slotOpenDatabase();
  void slotGenerateSql();
};

#endif
