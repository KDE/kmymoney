/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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

class KMMSqlQuery : public QSqlQuery
{
    static int queryId;
public:
    explicit KMMSqlQuery (QSqlResult *r, char *file, int line);
    explicit KMMSqlQuery (const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
    explicit KMMSqlQuery (QSqlDatabase db);
    virtual ~KMMSqlQuery();

    bool exec(const QString &query);
    bool exec();
    void finish();
    bool prepare(const QString& cmd);

private:
    int id;
    QString cmd;
};

#define QSqlQuery KMMSqlQuery
#endif // ENABLE_SQLTRACER
#endif // MYSQLQUERY_H
