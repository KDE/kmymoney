/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#include "ibanbicdata.h"

#include <KServiceTypeTrader>
#include <QtSql/QSqlQuery>
#include <QSqlError>
#include <QtCore/QStandardPaths>
#include <QtCore/QDebug>

#include "ibanbic.h"

ibanBicData::~ibanBicData()
{
}

int ibanBicData::bankIdentifierLength(const QString& countryCode)
{
  const QVariant value = findPropertyByCountry(countryCode, QLatin1String("X-KMyMoney-BankIdentifier-Length"), QVariant::Int);
  if (value.isValid())
    return value.toInt();
  return 0;
}

int ibanBicData::bankIdentifierPosition(const QString& countryCode)
{
  const QVariant value = findPropertyByCountry(countryCode, QLatin1String("X-KMyMoney-BankIdentifier-Position"), QVariant::Int);
  if (value.isValid())
    return value.toInt();
  return -1;
}

int ibanBicData::bbanLength(const QString& countryCode)
{
  const QVariant value = findPropertyByCountry(countryCode, QLatin1String("X-KMyMoney-BBAN-Length"), QVariant::Int);
  if (value.isValid())
    return value.toInt();
  // Something went wrong, so return the allowed maximum
  return 30;
}

QString ibanBicData::iban2Bic(const QString& iban)
{
  Q_ASSERT(iban.length() < 1 || iban.at(0).isLetterOrNumber());
  Q_ASSERT(iban.length() < 2 || iban.at(1).isLetterOrNumber());
  Q_ASSERT(iban == payeeIdentifiers::ibanBic::ibanToElectronic(iban));

  if (iban.length() <= 4)   // This iban is to short to extract a BIC
    return QString("");

  // Get bank identifier
  const QString bankCode = extractBankIdentifier(iban);
  if (bankCode.isEmpty())
    return bankCode; // keep .isEmpty() or .isNull()

  // Get countryCode
  const QString countryCode = iban.left(2);

  // Get services which support iban2bic and have a database entry
  KService::List services = KServiceTypeTrader::self()->query("KMyMoney/IbanBicData",
                            QString("(\'%1' ~in [X-KMyMoney-CountryCodes] or '*' in [X-KMyMoney-CountryCodes]) and true == [X-KMyMoney-IBAN-2-BIC-supported] and exist [X-KMyMoney-Bankdata-Database]").arg(countryCode)
                                                             );

  if (services.isEmpty())
    return QString();

  QSqlDatabase db = createDatabaseConnection(services.first()->property(QLatin1String("X-KMyMoney-Bankdata-Database"), QVariant::String).toString());
  if (!db.isOpen())   // This is an error
    return QString();

  QSqlQuery query = QSqlQuery(db);
  query.prepare("SELECT bic FROM institutions WHERE bankcode=? and country=?");
  query.bindValue(0, bankCode);
  query.bindValue(1, countryCode);

  if (!query.exec()) {
    qWarning() << QString("Could not execute query on \"%1\" to receive BIC. Error: %2").arg(db.databaseName()).arg(query.lastError().text());
    return QString();
  }

  if (query.next()) {
    return query.value(0).toString();
  }

  return QString("");
}

QString ibanBicData::bankNameByBic(QString bic)
{
  if (bic.length() == 8)
    bic += QLatin1String("XXX");
  else if (bic.length() != 11)
    return QString();

  const QString countryCode = bic.mid(4, 2);

  // Get services which have a database entry
  KService::List services = KServiceTypeTrader::self()->query("KMyMoney/IbanBicData",
                            QString("(\'%1' ~in [X-KMyMoney-CountryCodes] or '*' in [X-KMyMoney-CountryCodes]) and exist [X-KMyMoney-Bankdata-Database]").arg(countryCode)
                                                             );

  if (services.isEmpty())
    return QString();

  QSqlDatabase db = createDatabaseConnection(services.first()->property("X-KMyMoney-Bankdata-Database", QVariant::String).toString());
  if (!db.isOpen())   // This is an error
    return QString();

  QSqlQuery query = QSqlQuery(db);
  query.prepare("SELECT name FROM institutions WHERE bic=?");
  query.bindValue(0, bic);

  if (!query.exec()) {
    qWarning() << QString("Could not execute query on \"%1\" to receive bank name. Error: %2").arg(db.databaseName()).arg(query.lastError().text());
    return QString();
  }

  if (query.next()) {
    return query.value(0).toString();
  }

  return QString("");
}

QPair< QString, QString > ibanBicData::bankNameAndBic(const QString& iban)
{
  Q_ASSERT(iban.length() < 1 || iban.at(0).isLetterOrNumber());
  Q_ASSERT(iban.length() < 2 || iban.at(1).isLetterOrNumber());
  Q_ASSERT(iban == payeeIdentifiers::ibanBic::ibanToElectronic(iban));

  if (iban.length() <= 4)   // This iban is to short to extract a BIC
    return QPair<QString, QString>();

  // Get bank identifier
  const QString bankCode = extractBankIdentifier(iban);
  if (bankCode.isEmpty())
    return QPair<QString, QString>(bankCode, bankCode); // keep .isEmpty() or .isNull()

  // Get countryCode
  const QString countryCode = iban.left(2);

  // Get services which support iban2bic and have a database entry
  KService::List services = KServiceTypeTrader::self()->query("KMyMoney/IbanBicData",
                            QString("(\'%1' ~in [X-KMyMoney-CountryCodes] or '*' in [X-KMyMoney-CountryCodes]) and true == [X-KMyMoney-IBAN-2-BIC-supported] and exist [X-KMyMoney-Bankdata-Database]").arg(countryCode)
                                                             );

  if (services.isEmpty())
    return QPair<QString, QString>();

  QSqlDatabase db = createDatabaseConnection(services.first()->property(QLatin1String("X-KMyMoney-Bankdata-Database"), QVariant::String).toString());
  if (!db.isOpen())   // This is an error
    return QPair<QString, QString>();

  QSqlQuery query(db);
  query.prepare("SELECT bic, name FROM institutions WHERE bankcode=? and country=?");
  query.bindValue(0, bankCode);
  query.bindValue(1, countryCode);

  if (!query.exec()) {
    qWarning() << QString("Could not execute query on \"%1\" to receive BIC and name. Error: %2").arg(db.databaseName()).arg(query.lastError().text());
    return QPair<QString, QString>();
  }

  if (query.next()) {
    return QPair<QString, QString>(query.value(0).toString(), query.value(1).toString());
  }

  return QPair<QString, QString>(QString(""), QString(""));
}

ibanBicData::bicAllocationStatus ibanBicData::isBicAllocated(const QString& bic)
{
  // Get countryCode
  const QString countryCode = bic.mid(4, 2);
  if (countryCode.length() != 2)
    return bicNotAllocated;

  // Get services which have a database entry
  KService::List services = KServiceTypeTrader::self()->query("KMyMoney/IbanBicData",
                            QString("(\'%1' ~in [X-KMyMoney-CountryCodes] or '*' in [X-KMyMoney-CountryCodes]) and exist [X-KMyMoney-Bankdata-Database]").arg(countryCode)
                                                             );

  if (services.isEmpty())
    return bicAllocationUncertain;

  QSqlDatabase db = createDatabaseConnection(services.first()->property(QLatin1String("X-KMyMoney-Bankdata-Database"), QVariant::String).toString());
  if (!db.isOpen())   // This is an error
    return bicAllocationUncertain;

  QSqlQuery query(db);
  query.prepare("SELECT ? IN (SELECT bic FROM institutions)");
  query.bindValue(0, bic);

  if (!query.exec() || !query.next()) {
    qWarning() << QString("Could not execute query on \"%1\" to check if bic exists. Error: %2").arg(db.databaseName()).arg(query.lastError().text());
    return bicAllocationUncertain;
  }

  if (query.value(0).toBool())   // Bic found
    return bicAllocated;

  // Bic not found, test if database is complete
  if (services.first()->property(QLatin1String("X-KMyMoney-Bankdata-IsComplete"), QVariant::Bool).toBool())
    return bicNotAllocated;

  return bicAllocationUncertain;
}

QVariant ibanBicData::findPropertyByCountry(const QString& countryCode, const QString& property, const QVariant::Type type)
{
  const KService::List services = KServiceTypeTrader::self()->query("KMyMoney/IbanBicData",
                                  QString("'%1' ~in [X-KMyMoney-CountryCodes] and exist [%2]").arg(countryCode).arg(property)
                                                                   );
  if (!services.isEmpty())
    return services.first()->property(property, type);

  // Something went wrong
  return QVariant();
}

QString ibanBicData::extractBankIdentifier(const QString& iban)
{
  const QString countryCode = iban.left(2);

  // Extract bank code
  const int start = bankIdentifierPosition(countryCode);
  if (start == -1)
    return QString("");

  return iban.mid(start + 4, bankIdentifierLength(countryCode));
}

QSqlDatabase ibanBicData::createDatabaseConnection(const QString& database)
{
  Q_ASSERT(QSqlDatabase::drivers().contains("QSQLITE"));

  // Try to use already created connection
  const QString connectionName = QLatin1String("ibanBicData/") + database;
  QSqlDatabase storedConnection = QSqlDatabase::database(connectionName);
  if (storedConnection.isValid() && storedConnection.isOpen())
    return storedConnection;

  // Need to create new connection, locate database
  QString path = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("kmymoney/ibanbicdata/") + database);
  if (path.isEmpty()) {
    qWarning() << QString("Could not locate database file \"%1\" to receive IBAN and BIC data.").arg(database);
    return QSqlDatabase();
  }

  // Connect
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
  db.setDatabaseName(path);
  db.setConnectOptions("QSQLITE_OPEN_READONLY=1;QSQLITE_ENABLE_SHARED_CACHE=1;");
  const bool opened = db.open();
  if (!opened) {
    qWarning() << QString("Could not open database \"%1\" to receive IBAN and BIC data.").arg(path);
  }

  return db;
}
