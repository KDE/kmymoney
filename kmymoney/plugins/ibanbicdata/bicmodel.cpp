/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bicmodel.h"

#include <KServiceTypeTrader>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QStandardPaths>
#include <QDebug>

/**
 * @warning At the moment the completion may fail if bicModel was created in more than one thread
 * (it uses a QSqlDatabase object over all instances of bicModel, so the first created bicModel defines
 * the thread)
 *
 * @todo Make thread safe.
 */
bicModel::bicModel(QObject* parent)
    : QSqlQueryModel(parent)
{

    QSqlDatabase db = QSqlDatabase::database("bicModel", true);
    // Save if the database was opened before
    bool attachDatabases = false;

    if (!db.isValid()) {
        db = QSqlDatabase::addDatabase("QSQLITE", "bicModel");
        db.setDatabaseName(":memory:");
        db.setConnectOptions("QSQLITE_OPEN_READONLY=1;QSQLITE_ENABLE_SHARED_CACHE=1;");
        if (db.open())
            attachDatabases = true; // Database was not opened before
    }

    if (!db.isOpen()) {
        qWarning() << QString("Could not open in-memory database for bic data.");
    }
    QSqlQuery query(db);

    // Get services which support iban2bic and have a database entry
    KService::List services = KServiceTypeTrader::self()->query("KMyMoney/IbanBicData",
                              QString("exist [X-KMyMoney-Bankdata-Database]")
                                                               );

    if (services.isEmpty()) {
        // Set a valid query
        if (query.exec("SELECT null;"))
            setQuery(query);
        return;
    }

    QStringList databases;
    QStringList dbNames;

    unsigned int databaseCount = 0;

    Q_FOREACH (KService::Ptr service, services) {
        QString database = service->property(QLatin1String("X-KMyMoney-Bankdata-Database")).toString();

        // Locate database
        QString path = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("kmymoney/ibanbicdata/") + database);
        if (path.isEmpty()) {
            qWarning() << QString("Could not locate database file \"%1\" to receive BIC data.").arg(database);
        } else {
            databases << path;
            dbNames << QString("db%1").arg(++databaseCount);
        }
    }

    if (attachDatabases) {
        query.prepare("ATTACH DATABASE ? AS ?");
        query.addBindValue(databases);
        query.addBindValue(dbNames);
        if (!query.execBatch()) {
            qWarning() << "Could not init bic for bicModel, last error:" << query.lastError().text();
            dbNames = QStringList(); // clear so no query will be set
        }
    }

    QStringList queries;
    Q_FOREACH (QString dbName, dbNames) {
        queries.append(QString("SELECT bic, name FROM %1.institutions").arg(dbName));
    }

    if (query.exec(queries.join(QLatin1String(" UNION "))))
        setQuery(query);
}

QVariant bicModel::data(const QModelIndex& item, int role) const
{
    if (role == InstitutionNameRole)
        return QSqlQueryModel::data(createIndex(item.row(), 1), Qt::DisplayRole);
    return QSqlQueryModel::data(item, role);
}
