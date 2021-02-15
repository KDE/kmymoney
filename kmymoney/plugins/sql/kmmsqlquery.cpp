/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
