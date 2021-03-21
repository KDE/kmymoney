//krazy:skip
/****************************************************************************
**
** SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial
**
****************************************************************************/

#ifndef QSQL_SQLITE_H
#define QSQL_SQLITE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtSql/qsqldriver.h>

struct sqlite3;

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_SQLITE
#else
#define Q_EXPORT_SQLDRIVER_SQLITE Q_SQL_EXPORT
#endif

QT_BEGIN_NAMESPACE

class QSqlResult;
class QSQLiteDriverPrivate;

class Q_EXPORT_SQLDRIVER_SQLITE QSQLiteDriver : public QSqlDriver
{
    Q_DECLARE_PRIVATE(QSQLiteDriver)
    Q_OBJECT
    friend class QSQLiteResultPrivate;
public:
    explicit QSQLiteDriver(QObject *parent = 0);
    explicit QSQLiteDriver(sqlite3 *connection, QObject *parent = 0);
    ~QSQLiteDriver();
    bool hasFeature(DriverFeature f) const Q_DECL_OVERRIDE;
    bool open(const QString & db,
              const QString & user,
              const QString & password,
              const QString & host,
              int port,
              const QString & connOpts) Q_DECL_OVERRIDE;
    void close() Q_DECL_OVERRIDE;
    QSqlResult *createResult() const Q_DECL_OVERRIDE;
    bool beginTransaction() Q_DECL_OVERRIDE;
    bool commitTransaction() Q_DECL_OVERRIDE;
    bool rollbackTransaction() Q_DECL_OVERRIDE;
    QStringList tables(QSql::TableType) const Q_DECL_OVERRIDE;

    QSqlRecord record(const QString& tablename) const Q_DECL_OVERRIDE;
    QSqlIndex primaryIndex(const QString &table) const Q_DECL_OVERRIDE;
    QVariant handle() const Q_DECL_OVERRIDE;
    QString escapeIdentifier(const QString &identifier, IdentifierType) const Q_DECL_OVERRIDE;

    bool subscribeToNotification(const QString &name) Q_DECL_OVERRIDE;
    bool unsubscribeFromNotification(const QString &name) Q_DECL_OVERRIDE;
    QStringList subscribedToNotifications() const Q_DECL_OVERRIDE;
private Q_SLOTS:
    void handleNotification(const QString &tableName, qint64 rowid);
};

QT_END_NAMESPACE

#endif // QSQL_SQLITE_H
