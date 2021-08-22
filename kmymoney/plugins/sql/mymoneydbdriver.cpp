/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <fvilas@iname.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneydbdriver.h"
#include <config-kmymoney.h>

// ----------------------------------------------------------------------------
// QT Includes
#include <QSqlQuery>
#include <QVariant>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneydbdef.h"
#include "mymoneyexception.h"

//********************* The driver subclass definitions ********************
// It is only necessary to implement the functions that deviate from the
// default of the base type

class MyMoneyDb2Driver : public MyMoneyDbDriver
{
public:
    MyMoneyDb2Driver() {
    }

    QString textString(const MyMoneyDbTextColumn& c) const final override;
};

class MyMoneyInterbaseDriver : public MyMoneyDbDriver
{
public:
    MyMoneyInterbaseDriver() {
    }
};

class MyMoneyMysqlDriver : public MyMoneyDbDriver
{
public:
    MyMoneyMysqlDriver() {
    }

    bool isTested() const final override;
    bool canAutocreate() const final override;
    QString defaultDbName() const final override;
    QString createDbString(const QString& name) const final override;
    QString dropPrimaryKeyString(const QString& name) const final override;
    QString dropIndexString(const QString& tableName, const QString& indexName) const final override;
    QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const final override;
    QString intString(const MyMoneyDbIntColumn& c) const final override;
    QString timestampString(const MyMoneyDbDatetimeColumn& c) const final override;
    QString tableOptionString() const final override;
    QString highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const final override;
    QStringList tables(QSql::TableType tt, const QSqlDatabase& db) const final override;
};

class MyMoneyOracleDriver : public MyMoneyDbDriver
{
public:
    MyMoneyOracleDriver() {
    }

    QString dropPrimaryKeyString(const QString& name) const final override;
    QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const final override;
    QString intString(const MyMoneyDbIntColumn& c) const final override;
    QString textString(const MyMoneyDbTextColumn& c) const final override;
    QString highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const final override;
};

class MyMoneyODBCDriver : public MyMoneyDbDriver
{
public:
    MyMoneyODBCDriver() {
    }

    QString timestampString(const MyMoneyDbDatetimeColumn& c) const final override;
};

class MyMoneyPostgresqlDriver : public MyMoneyDbDriver
{
public:
    MyMoneyPostgresqlDriver() {
    }

    bool isTested() const final override;
    bool canAutocreate() const final override;
    QString defaultDbName() const final override;
    QString createDbString(const QString& name) const final override;
    QString dropPrimaryKeyString(const QString& name) const final override;
    QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const final override;
    QString intString(const MyMoneyDbIntColumn& c) const final override;
    QString textString(const MyMoneyDbTextColumn& c) const final override;
    QString highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const final override;
};

class MyMoneySybaseDriver : public MyMoneyDbDriver
{
public:
    MyMoneySybaseDriver() {
    }
};

class MyMoneySqlite3Driver : public MyMoneyDbDriver
{
public:
    MyMoneySqlite3Driver() {
    }

    bool isTested() const final override;
    QString forUpdateString() const final override;
    QString intString(const MyMoneyDbIntColumn& c) const final override;
    bool requiresExternalFile() const final override;
    bool requiresCreation() const final override;
    bool isPasswordSupported() const override;
};

class MyMoneySqlCipher3Driver : public MyMoneySqlite3Driver
{
public:
    bool isPasswordSupported() const final override;
};

//********************* The driver map *********************
// This function is only used by the GUI types at the moment
const QMap<QString, QString> MyMoneyDbDriver::driverMap()
{
    QMap<QString, QString> map;

    map["QDB2"] = QString("IBM DB2");
    map["QIBASE"] = QString("Borland Interbase");
    map["QMYSQL"] = QString("MySQL");
    map["QOCI"] = QString("Oracle Call Interface");
    map["QODBC"] = QString("Open Database Connectivity");
    map["QPSQL"] = QString("PostgreSQL v7.3 and up");
    map["QTDS"] = QString("Sybase Adaptive Server and Microsoft SQL Server");
#ifndef ENABLE_SQLCIPHER
    map["QSQLITE"] = QString("SQLite Version 3"); // QSQLITE is overridden with QSQLCIPHER and won't work properly, so disable it
#else
    map["QSQLCIPHER"] = QString("SQLCipher Version 3 (encrypted SQLite)");
#endif
    return map;
}

//********************* The factory *********************
QExplicitlySharedDataPointer<MyMoneyDbDriver> MyMoneyDbDriver::create(const QString& type)
{
    if (type == "QDB2")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneyDb2Driver());
    else if (type == "QIBASE")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneyInterbaseDriver());
    else if (type == "QMYSQL")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneyMysqlDriver());
    else if (type == "QOCI")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneyOracleDriver());
    else if (type == "QODBC")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneyODBCDriver());
    else if (type == "QPSQL")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneyPostgresqlDriver());
    else if (type == "QTDS")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneySybaseDriver());
    else if (type == "QSQLITE")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneySqlite3Driver());
    else if (type == "QSQLCIPHER")
        return QExplicitlySharedDataPointer<MyMoneyDbDriver> (new MyMoneySqlCipher3Driver());
    else throw MYMONEYEXCEPTION_CSTRING("Unknown database driver type.");
}

MyMoneyDbDriver::MyMoneyDbDriver()
{
}

MyMoneyDbDriver::~MyMoneyDbDriver()
{
}

//*******************************************************
// By default, claim that the driver is not tested
// For Mysql, Pgsql, and SQLite, return true.
bool MyMoneyDbDriver::isTested() const
{
    return false;
}

bool MyMoneyMysqlDriver::isTested() const
{
    return true;
}

bool MyMoneyPostgresqlDriver::isTested() const
{
    return true;
}

bool MyMoneySqlite3Driver::isTested() const
{
    return true;
}

//*******************************************************
// By default, claim that the database cannot be created
// For Mysql and Pgsql, return true.
bool MyMoneyDbDriver::canAutocreate() const
{
    return false;
}

bool MyMoneyMysqlDriver::canAutocreate() const
{
    return true;
}

bool MyMoneyPostgresqlDriver::canAutocreate() const
{
    return true;
}

//*******************************************************
// By default, there is no default name
QString MyMoneyDbDriver::defaultDbName() const
{
    return "";
}

// The default db for Mysql is "mysql"
QString MyMoneyMysqlDriver::defaultDbName() const
{
    return "mysql";
}

// The default db for Postgres is "template1"
QString MyMoneyPostgresqlDriver::defaultDbName() const
{
    return "template1";
}

//*******************************************************
// By default, just attempt to create the database
// Mysql and Postgres need the character set specified.
QString MyMoneyDbDriver::createDbString(const QString& name) const
{
    return QString("CREATE DATABASE %1").arg(name);
}

QString MyMoneyMysqlDriver::createDbString(const QString& name) const
{
    return MyMoneyDbDriver::createDbString(name) + " CHARACTER SET 'utf8' COLLATE 'utf8_unicode_ci'";
}

QString MyMoneyPostgresqlDriver:: createDbString(const QString& name) const
{
    return MyMoneyDbDriver::createDbString(name) + " WITH ENCODING='UTF8' LC_CTYPE='C' TEMPLATE=template0";
}

//*******************************************************
// By default, the DBMS does not require an external file
// At present, only sqlite does
bool MyMoneyDbDriver::requiresExternalFile() const
{
    return false;
}

bool MyMoneySqlite3Driver::requiresExternalFile() const
{
    return true;
}

//*******************************************************
// By default, the DBMS requires creating before use
// At present, only sqlite doesn't AFAIK
bool MyMoneyDbDriver::requiresCreation() const
{
    return true;
}

bool MyMoneySqlite3Driver::requiresCreation() const
{
    return false;
}

//*******************************************************
// There is no standard for dropping a primary key.
// If it is supported, it will have to be implemented for each DBMS
QString MyMoneyDbDriver::dropPrimaryKeyString(const QString& name) const
{
    Q_UNUSED(name);
    return "";
}

QString MyMoneyMysqlDriver::dropPrimaryKeyString(const QString& name) const
{
    return QString("ALTER TABLE %1 DROP PRIMARY KEY;").arg(name);
}

QString MyMoneyOracleDriver::dropPrimaryKeyString(const QString& name) const
{
    return QString("ALTER TABLE %1 DROP PRIMARY KEY;").arg(name);
}

QString MyMoneyPostgresqlDriver::dropPrimaryKeyString(const QString& name) const
{
    return QString("ALTER TABLE %1 DROP CONSTRAINT %2_pkey;").arg(name).arg(name);
}

//*******************************************************
// There is apparently no standard for dropping an index.
// If it is supported, it will have to be implemented for each DBMS
QString MyMoneyDbDriver::dropIndexString(const QString& tableName, const QString& indexName) const
{
    Q_UNUSED(tableName);
    return QString("DROP INDEX %1;").arg(indexName);
}

QString MyMoneyMysqlDriver::dropIndexString(const QString& tableName, const QString& indexName) const
{
    return QString("DROP INDEX %1 ON %2;").arg(indexName).arg(tableName);
}

//*******************************************************
// There is no standard for modifying a column
// If it is supported, it will have to be implemented for each DBMS
QString MyMoneyDbDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{

    Q_UNUSED(tableName);
    Q_UNUSED(columnName);
    Q_UNUSED(newDef);

    return "";
}

QString MyMoneyMysqlDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
    return QString("ALTER TABLE %1 CHANGE %2 %3")
           .arg(tableName)
           .arg(columnName)
           .arg(newDef.generateDDL(QExplicitlySharedDataPointer<MyMoneyDbDriver>(const_cast<MyMoneyMysqlDriver*>(this))));
}

QString MyMoneyPostgresqlDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
    return QString("ALTER TABLE %1 ALTER COLUMN %2 TYPE %3")
           .arg(tableName)
           .arg(columnName)
           .arg(newDef.generateDDL(QExplicitlySharedDataPointer<MyMoneyDbDriver>(const_cast<MyMoneyPostgresqlDriver*>(this))));
}

QString MyMoneyOracleDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
    return QString("ALTER TABLE %1 MODIFY %2 %3")
           .arg(tableName)
           .arg(columnName)
           .arg(newDef.generateDDL(QExplicitlySharedDataPointer<MyMoneyDbDriver>(const_cast<MyMoneyOracleDriver*>(this))));
}

//*******************************************************
// Define the integer column types in terms of the standard
// Each DBMS typically has its own variation of this
QString MyMoneyDbDriver::intString(const MyMoneyDbIntColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbIntColumn::TINY:
    case MyMoneyDbIntColumn::SMALL:
        qs += " smallint";
        break;
    case MyMoneyDbIntColumn::BIG:
        qs += " bigint";
        break;
    case MyMoneyDbIntColumn::MEDIUM:
    default:
        qs += " int";
        break;
    }

    if (c.isNotNull()) qs += " NOT NULL";
    return qs;
}

QString MyMoneyMysqlDriver::intString(const MyMoneyDbIntColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbIntColumn::TINY:
        qs += " tinyint";
        break;
    case MyMoneyDbIntColumn::SMALL:
        qs += " smallint";
        break;
    case MyMoneyDbIntColumn::BIG:
        qs += " bigint";
        break;
    case MyMoneyDbIntColumn::MEDIUM:
    default:
        qs += " int";
        break;
    }

    if (! c.isSigned()) qs += " unsigned";

    if (c.isNotNull()) qs += " NOT NULL";
    return qs;
}

QString MyMoneySqlite3Driver::intString(const MyMoneyDbIntColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbIntColumn::TINY:
        qs += " tinyint";
        break;
    case MyMoneyDbIntColumn::SMALL:
        qs += " smallint";
        break;
    case MyMoneyDbIntColumn::BIG:
        qs += " bigint";
        break;
    case MyMoneyDbIntColumn::MEDIUM:
    default:
        qs += " int";
        break;
    }

    if (! c.isSigned()) qs += " unsigned";

    if (c.isNotNull()) qs += " NOT NULL";
    return qs;
}

QString MyMoneyPostgresqlDriver::intString(const MyMoneyDbIntColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbIntColumn::TINY:
    case MyMoneyDbIntColumn::SMALL:
        qs += " int2";
        break;
    case MyMoneyDbIntColumn::BIG:
        qs += " int8";
        break;
    case MyMoneyDbIntColumn::MEDIUM:
    default:
        qs += " int4";
        break;
    }

    if (c.isNotNull()) qs += " NOT NULL";

    if (! c.isSigned()) qs += QString(" check(%1 >= 0)").arg(c.name());

    return qs;
}

QString MyMoneyOracleDriver::intString(const MyMoneyDbIntColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbIntColumn::TINY:
        qs += " number(3)";
        break;
    case MyMoneyDbIntColumn::SMALL:
        qs += " number(5)";
        break;
    case MyMoneyDbIntColumn::BIG:
        qs += " number(20)";
        break;
    case MyMoneyDbIntColumn::MEDIUM:
    default:
        qs += " number(10)";
        break;
    }

    if (c.isNotNull()) qs += " NOT NULL";

    if (! c.isSigned()) qs += QString(" check(%1 >= 0)").arg(c.name());

    return qs;
}

//*******************************************************
// Define the text column types in terms of the standard
// Each DBMS typically has its own variation of this
QString MyMoneyDbDriver::textString(const MyMoneyDbTextColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbTextColumn::TINY:
        qs += " tinytext";
        break;
    case MyMoneyDbTextColumn::MEDIUM:
        qs += " mediumtext";
        break;
    case MyMoneyDbTextColumn::LONG:
        qs += " longtext";
        break;
    case MyMoneyDbTextColumn::NORMAL:
    default:
        qs += " text";
        break;
    }

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

QString MyMoneyDb2Driver::textString(const MyMoneyDbTextColumn& c) const
{
    QString qs = c.name();

    switch (c.type()) {
    case MyMoneyDbTextColumn::TINY:
        qs += " varchar(255)";
        break;
    case MyMoneyDbTextColumn::MEDIUM:
        qs += " clob(16M)";
        break;
    case MyMoneyDbTextColumn::LONG:
        qs += " clob(2G)";
        break;
    case MyMoneyDbTextColumn::NORMAL:
    default:
        qs += " clob(64K)";
        break;
    }

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

QString MyMoneyOracleDriver::textString(const MyMoneyDbTextColumn& c) const
{
    QString qs = c.name();
    switch (c.type()) {
    case MyMoneyDbTextColumn::TINY:
        qs += " varchar2(255)";
        break;
    case MyMoneyDbTextColumn::MEDIUM:
    case MyMoneyDbTextColumn::LONG:
    case MyMoneyDbTextColumn::NORMAL:
    default:
        qs += " clob";
        break;
    }

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

QString MyMoneyPostgresqlDriver::textString(const MyMoneyDbTextColumn& c) const
{
    QString qs = QString("%1 text").arg(c.name());

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

//*******************************************************
// Define the timestamp column types in terms of the standard
// Each DBMS typically has its own variation of this
QString MyMoneyDbDriver::timestampString(const MyMoneyDbDatetimeColumn& c) const
{
    QString qs = QString("%1 timestamp").arg(c.name());

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

// Mysql has a timestamp type, but datetime is closer to the standard
QString MyMoneyMysqlDriver::timestampString(const MyMoneyDbDatetimeColumn& c) const
{
    QString qs = QString("%1 datetime").arg(c.name());

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

QString MyMoneyODBCDriver::timestampString(const MyMoneyDbDatetimeColumn& c) const
{
    QString qs = QString("%1 datetime").arg(c.name());

    if (c.isNotNull()) qs += " NOT NULL";

    return qs;
}

//***********************************************
// Define the FOR UPDATE string
// So far, only SQLite requires special handling.
QString MyMoneyDbDriver::forUpdateString() const
{
    return " FOR UPDATE";
}

QString MyMoneySqlite3Driver::forUpdateString() const
{
    return "";
}

//***********************************************
// Define the table option string
// So far, only mysql requires special handling.
QString MyMoneyMysqlDriver::tableOptionString() const
{
    return " ENGINE = InnoDB";
}

QString MyMoneyDbDriver::tableOptionString() const
{
    return "";
}

//***********************************************
// Define the highestIdNum string
// PostgreSQL and Oracle return errors when a non-numerical string is cast to an integer, so a regex is used to skip strings that aren't entirely numerical after the prefix is removed
QString MyMoneyDbDriver::highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const
{
    return QString("SELECT MAX(CAST(SUBSTR(%1, %2) AS INTEGER)) FROM %3;").arg(tableField).arg(prefixLength + 1).arg(tableName);
}

QString MyMoneyMysqlDriver::highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const
{
    return QString("SELECT MAX(CAST(SUBSTR(%1, %2) AS UNSIGNED INTEGER)) FROM %3;").arg(tableField).arg(prefixLength + 1).arg(tableName);
}

QString MyMoneyPostgresqlDriver::highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const
{
    return QString("SELECT MAX(CAST(SUBSTR(%1, %2) AS INTEGER)) FROM %3 WHERE SUBSTR(%1, %2) ~ '^[0-9]+$';").arg(tableField).arg(prefixLength + 1).arg(tableName);
}

QString MyMoneyOracleDriver::highestNumberFromIdString(const QString& tableName, const QString& tableField, const int prefixLength) const
{
    return QString("SELECT MAX(TO_NUMBER(SUBSTR(%1, %2))) FROM %3 WHERE REGEXP_LIKE(SUBSTR(%1, %2), '^[0-9]+$');").arg(tableField).arg(prefixLength + 1).arg(tableName);
}

//*************************************************
// Define
bool MyMoneyDbDriver::isPasswordSupported() const
{
    return true;
}

bool MyMoneySqlite3Driver::isPasswordSupported() const
{
    return false;
}

bool MyMoneySqlCipher3Driver::isPasswordSupported() const
{
    return true;
}

//*************************************************
// replace the QSqlDatabase::tables() call for Mysql only
// see bug 252841
QStringList MyMoneyDbDriver::tables(QSql::TableType tt, const QSqlDatabase& db) const
{
    return (db.tables(tt));
}

QStringList MyMoneyMysqlDriver::tables(QSql::TableType tt, const QSqlDatabase& db) const
{
    QStringList tableList;
    QSqlQuery q(db);
    QString selectString;
    switch (tt) {
    case QSql::AllTables:
        selectString = QString("SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA = '%1'").arg(db.databaseName());
        if (!q.exec(selectString)) {
            throw MYMONEYEXCEPTION_CSTRING("select names failed in mymoneydbdriver.cpp");
        }
        while (q.next())
            tableList.append(q.value(0).toString());
        break;
    case QSql::Tables:
    case QSql::SystemTables:
    case QSql::Views:
        qWarning("Programming error in mymoneydbdriver.cpp"); // KMM only uses AllTables; implement other options if required
    }

    return (tableList);
}
