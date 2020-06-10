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

#include "kmmsqlquery.h"
#undef QSqlQuery

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

int KMMSqlQuery::queryId = 1;

KMMSqlQuery::KMMSqlQuery(QSqlResult *r, char *file, int line)
  : QSqlQuery(r)
{
  id = queryId++;
  qDebug() << "Created" << id << file << line;
}

KMMSqlQuery::KMMSqlQuery(const QString& query, QSqlDatabase db)
  : QSqlQuery(query, db)
{
  id = queryId++;
  qDebug() << "Created" << id;
}

KMMSqlQuery::KMMSqlQuery(QSqlDatabase db)
  : QSqlQuery(db)
{
  id = queryId++;
  qDebug() << "Created" << id;
}

KMMSqlQuery::~KMMSqlQuery()
{
  qDebug() << "Destroyed" << id;
}

bool KMMSqlQuery::prepare(const QString& _cmd)
{
  cmd = _cmd;
  return QSqlQuery::prepare(cmd);
}

bool KMMSqlQuery::exec()
{
  bool rc = QSqlQuery::exec();
  qDebug() << id << "executed" << cmd;
  return rc;
}

bool KMMSqlQuery::exec(const QString& query)
{
  bool rc = QSqlQuery::exec(query);
  qDebug() << id << "executed" << query;
  return rc;
}

void KMMSqlQuery::finish()
{
  QSqlQuery::finish();
  qDebug() << id << "finished";
}
