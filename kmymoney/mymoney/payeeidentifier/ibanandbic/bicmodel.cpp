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

#include "bicmodel.h"

#include <KDebug>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QString>
#include <QtCore/QStringList>

bicModel::bicModel(QObject* parent)
  : QSqlQueryModel(parent)
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "bicModel");
  db.setDatabaseName( ":memory:" );
  db.setConnectOptions("QSQLITE_OPEN_READONLY=1;QSQLITE_ENABLE_SHARED_CACHE=1;");
  const bool opened = db.open();
  if ( !opened ) {
    kWarning() << QString("Could not open in-memory database for bic data.");
  }
  QSqlQuery query(db);
  query.prepare("ATTACH DATABASE ? AS ?");

  QStringList databases;
  databases << "/home/christian/Develop/install/kmymoney/share/apps/kmymoney/ibanbicdata/bankdata.de.db"
            << "/home/christian/Develop/install/kmymoney/share/apps/kmymoney/ibanbicdata/bankdata.ch.db";
  query.addBindValue(databases);
  
  QStringList dbNames;
  dbNames << "db1";
  dbNames << "db2";
  query.addBindValue(dbNames);

  qDebug() << "Attached databases" << query.execBatch() << query.lastError().text();

  QStringList queries;
  foreach (QString dbName, dbNames) {
    queries.append(QString("SELECT bic, name FROM %1.institutions").arg(dbName));
  }
  query.exec(queries.join( QLatin1String(" UNION ") ));
  
  setQuery(query);
}

QVariant bicModel::data(const QModelIndex& item, int role) const
{
  if ( role == InstitutionNameRole )
    return QSqlQueryModel::data(createIndex(item.row(), 1), Qt::DisplayRole);
  return QSqlQueryModel::data(item, role);
}
