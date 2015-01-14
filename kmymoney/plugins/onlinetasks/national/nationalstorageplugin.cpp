/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
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

#include "nationalstorageplugin.h"
#include <QSqlQuery>
#include <QSqlError>

const QString nationalStoragePlugin::iid = QLatin1String("org.kmymoney.creditTransfer.germany.sqlStoragePlugin");

nationalStoragePlugin::nationalStoragePlugin(QObject* parent, const QVariantList& options)
  : storagePlugin( parent )
{
  Q_UNUSED( options );
}

bool nationalStoragePlugin::removePluginData(QSqlDatabase connection)
{
  QSqlQuery query(connection);
  query.prepare("DROP TABLE IF EXISTS kmmNationalOrders;");
  if (!query.exec()) {
    qWarning("Could not execute query for nationalStoragePlugin: %s", qPrintable(query.lastError().text()));
    return false;
  }

  query.prepare("DELETE FROM versionMajor WHERE iid = ?");
  query.bindValue(0, iid);
  if (!query.exec()) {
    qWarning("Could not execute query for nationalStoragePlugin: %s", qPrintable(query.lastError().text()));
    return false;
  }

  return true;
}

bool nationalStoragePlugin::setupDatabase(QSqlDatabase connection)
{
  // Get current version
  QSqlQuery query = QSqlQuery(connection);
  query.prepare("SELECT versionMajor FROM kmmPluginInfo WHERE iid = ?");
  query.bindValue(0, iid);
  if (!query.exec()) {
    qWarning("Could not execute query for nationalStoragePlugin: %s", qPrintable(query.lastError().text()));
    return false;
  }

  int currentVersion = 0;
  if ( query.next() )
    currentVersion = query.value(0).toInt();

  // Create database in it's most recent version if version is 0
  // (version 0 means the database was not installed)
  if ( currentVersion == 0 ) {
    if (!query.exec(
      "CREATE TABLE kmmNationalOrders ("
      "  id varchar(32) NOT NULL PRIMARY KEY REFERENCES kmmOnlineJobs( id ),"
      "  originAccount varchar(32) REFERENCES kmmAccounts( id ) ON UPDATE CASCADE ON DELETE SET NULL,"
      "  value text DEFAULT '0',"
      "  purpose text,"
      "  beneficiaryName varchar(27),"
      "  beneficiaryAccountNumber char(10),"
      "  beneficiaryBankCode char(8),"
      "  textKey int,"
      "  subTextKey int"
      " );"
    )) {
      qWarning("Error while creating table 'kmmNationalOrders': %s", qPrintable(query.lastError().text()));
      return false;
    }

    query.prepare("INSERT INTO kmmPluginInfo (iid, versionMajor, versionMinor, uninstallQuery) VALUES(?, ?, ?, ?)");
    query.bindValue(0, iid);
    query.bindValue(1, 1);
    query.bindValue(2, 0);
    query.bindValue(3, "DROP TABLE IF EXISTS kmmNationalOrders;");
    if ( query.exec() )
      return true;
    qWarning("Error while saving plugin info for %s: %s", qPrintable(iid), qPrintable(query.lastError().text()));
    return false;
  }

  // Check if version is valid with this plugin
  switch( currentVersion ) {
    case 1: return true;
  }

  return false;
}
