/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYSQLQUERY_H
#define MYSQLQUERY_H

#include "config-kmymoney.h"

#if ENABLE_SQLTRACER

#undef QSqlQuery

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QSqlResult;

class MySqlQuery : public QSqlQuery
{
  static int queryId;
public:
  explicit MySqlQuery(QSqlResult *r, char *file, int line);
  explicit MySqlQuery(const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
  explicit MySqlQuery(QSqlDatabase db);
  virtual ~MySqlQuery();

  bool exec(const QString &query);
  bool exec();
  void finish();
  bool prepare(const QString& cmd);

private:
  int id;
  QString cmd;
};

#define QSqlQuery MySqlQuery
#endif // ENABLE_SQLTRACER
#endif // MYSQLQUERY_H
