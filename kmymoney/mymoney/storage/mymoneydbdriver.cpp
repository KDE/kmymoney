/***************************************************************************
                          mymoneydbdriver.cpp
                          ---------------------
    begin                : 19 February 2010
    copyright            : (C) 2010 by Fernando Vilas
    email                : Fernando Vilas <fvilas@iname.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneydbdriver.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QSqlQuery>
#include <QVariant>
#include <QtDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneyexception.h"

//********************* The driver subclass definitions ********************
// It is only necessary to implement the functions that deviate from the
// default of the base type

class MyMoneyDb2Driver : public MyMoneyDbDriver
{
public:
  MyMoneyDb2Driver() {
  }

  virtual const QString textString(const MyMoneyDbTextColumn& c) const;
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

  virtual bool isTested() const;
  virtual bool canAutocreate() const;
  virtual const QString defaultDbName() const;
  virtual const QString createDbString(const QString& name) const;
  virtual const QString dropPrimaryKeyString(const QString& name) const;
  virtual const QString dropIndexString(const QString& tableName, const QString& indexName) const;
  virtual const QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const;
  virtual const QString intString(const MyMoneyDbIntColumn& c) const;
  virtual const QString timestampString(const MyMoneyDbDatetimeColumn& c) const;
  virtual const QString tableOptionString() const;
  virtual const QStringList tables(QSql::TableType tt, const QSqlDatabase& db) const;
};

class MyMoneyOracleDriver : public MyMoneyDbDriver
{
public:
  MyMoneyOracleDriver() {
  }

  virtual const QString dropPrimaryKeyString(const QString& name) const;
  virtual const QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const;
  virtual const QString intString(const MyMoneyDbIntColumn& c) const;
  virtual const QString textString(const MyMoneyDbTextColumn& c) const;
};

class MyMoneyODBCDriver : public MyMoneyDbDriver
{
public:
  MyMoneyODBCDriver() {
  }

  virtual const QString timestampString(const MyMoneyDbDatetimeColumn& c) const;
};

class MyMoneyPostgresqlDriver : public MyMoneyDbDriver
{
public:
  MyMoneyPostgresqlDriver() {
  }

  virtual bool isTested() const;
  virtual bool canAutocreate() const;
  virtual const QString defaultDbName() const;
  virtual const QString createDbString(const QString& name) const;
  virtual const QString dropPrimaryKeyString(const QString& name) const;
  virtual const QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const;
  virtual const QString intString(const MyMoneyDbIntColumn& c) const;
  virtual const QString textString(const MyMoneyDbTextColumn& c) const;
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

  virtual bool isTested() const;
  virtual const QString forUpdateString() const;
  virtual const QString intString(const MyMoneyDbIntColumn& c) const;
  virtual bool requiresExternalFile() const;
  virtual bool requiresCreation() const;
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
  map["QSQLITE"] = QString("SQLite Version 3");

  return map;
}

//********************* The factory *********************
KSharedPtr<MyMoneyDbDriver> MyMoneyDbDriver::create(const QString& type)
{
  if (type == "QDB2")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneyDb2Driver());
  else if (type == "QIBASE")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneyInterbaseDriver());
  else if (type == "QMYSQL")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneyMysqlDriver());
  else if (type == "QOCI")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneyOracleDriver());
  else if (type == "QODBC")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneyODBCDriver());
  else if (type == "QPSQL")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneyPostgresqlDriver());
  else if (type == "QTDS")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneySybaseDriver());
  else if (type == "QSQLITE")
    return KSharedPtr<MyMoneyDbDriver> (new MyMoneySqlite3Driver());
  else throw new MYMONEYEXCEPTION(QString("Unknown database driver type").arg(type));
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
const QString MyMoneyDbDriver::defaultDbName() const
{
  return "";
}

// The default db for Mysql is "mysql"
const QString MyMoneyMysqlDriver::defaultDbName() const
{
  return "mysql";
}

// The default db for Postgres is "template1"
const QString MyMoneyPostgresqlDriver::defaultDbName() const
{
  return "template1";
}

//*******************************************************
// By default, just attempt to create the database
// Mysql and Postgres need the character set specified.
const QString MyMoneyDbDriver::createDbString(const QString& name) const
{
  return QString("CREATE DATABASE %1").arg(name);
}

const QString MyMoneyMysqlDriver::createDbString(const QString& name) const
{
  return MyMoneyDbDriver::createDbString(name) + " CHARACTER SET 'utf8' COLLATE 'utf8_unicode_ci'";
}

const QString MyMoneyPostgresqlDriver:: createDbString(const QString& name) const
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
const QString MyMoneyDbDriver::dropPrimaryKeyString(const QString& name) const
{
  Q_UNUSED(name);
  return "";
}

const QString MyMoneyMysqlDriver::dropPrimaryKeyString(const QString& name) const
{
  return QString("ALTER TABLE %1 DROP PRIMARY KEY;").arg(name);
}

const QString MyMoneyOracleDriver::dropPrimaryKeyString(const QString& name) const
{
  return QString("ALTER TABLE %1 DROP PRIMARY KEY;").arg(name);
}

const QString MyMoneyPostgresqlDriver::dropPrimaryKeyString(const QString& name) const
{
  return QString("ALTER TABLE %1 DROP CONSTRAINT %2_pkey;").arg(name).arg(name);
}

//*******************************************************
// There is apparently no standard for dropping an index.
// If it is supported, it will have to be implemented for each DBMS
const QString MyMoneyDbDriver::dropIndexString(const QString& tableName, const QString& indexName) const
{
  Q_UNUSED(tableName);
  return QString("DROP INDEX %1;").arg(indexName);
}

const QString MyMoneyMysqlDriver::dropIndexString(const QString& tableName, const QString& indexName) const
{
  return QString("DROP INDEX %1 ON %2;").arg(indexName).arg(tableName);
}

//*******************************************************
// There is no standard for modifying a column
// If it is supported, it will have to be implemented for each DBMS
const QString MyMoneyDbDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{

  Q_UNUSED(tableName);
  Q_UNUSED(columnName);
  Q_UNUSED(newDef);

  return "";
}

const QString MyMoneyMysqlDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
  return QString("ALTER TABLE %1 CHANGE %2 %3")
         .arg(tableName)
         .arg(columnName)
         .arg(newDef.generateDDL(KSharedPtr<MyMoneyDbDriver>(const_cast<MyMoneyMysqlDriver*>(this))));
}

const QString MyMoneyPostgresqlDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
  return QString("ALTER TABLE %1 ALTER COLUMN %2 TYPE %3")
         .arg(tableName)
         .arg(columnName)
         .arg(newDef.generateDDL(KSharedPtr<MyMoneyDbDriver>(const_cast<MyMoneyPostgresqlDriver*>(this))));
}

const QString MyMoneyOracleDriver::modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const
{
  return QString("ALTER TABLE %1 MODIFY %2 %3")
         .arg(tableName)
         .arg(columnName)
         .arg(newDef.generateDDL(KSharedPtr<MyMoneyDbDriver>(const_cast<MyMoneyOracleDriver*>(this))));
}

//*******************************************************
// Define the integer column types in terms of the standard
// Each DBMS typically has its own variation of this
const QString MyMoneyDbDriver::intString(const MyMoneyDbIntColumn& c) const
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

const QString MyMoneyMysqlDriver::intString(const MyMoneyDbIntColumn& c) const
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

const QString MyMoneySqlite3Driver::intString(const MyMoneyDbIntColumn& c) const
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

const QString MyMoneyPostgresqlDriver::intString(const MyMoneyDbIntColumn& c) const
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

const QString MyMoneyOracleDriver::intString(const MyMoneyDbIntColumn& c) const
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
const QString MyMoneyDbDriver::textString(const MyMoneyDbTextColumn& c) const
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

const QString MyMoneyDb2Driver::textString(const MyMoneyDbTextColumn& c) const
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

const QString MyMoneyOracleDriver::textString(const MyMoneyDbTextColumn& c) const
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

const QString MyMoneyPostgresqlDriver::textString(const MyMoneyDbTextColumn& c) const
{
  QString qs = QString("%1 text").arg(c.name());

  if (c.isNotNull()) qs += " NOT NULL";

  return qs;
}

//*******************************************************
// Define the timestamp column types in terms of the standard
// Each DBMS typically has its own variation of this
const QString MyMoneyDbDriver::timestampString(const MyMoneyDbDatetimeColumn& c) const
{
  QString qs = QString("%1 timestamp").arg(c.name());

  if (c.isNotNull()) qs += " NOT NULL";

  return qs;
}

// Mysql has a timestamp type, but datetime is closer to the standard
const QString MyMoneyMysqlDriver::timestampString(const MyMoneyDbDatetimeColumn& c) const
{
  QString qs = QString("%1 datetime").arg(c.name());

  if (c.isNotNull()) qs += " NOT NULL";

  return qs;
}

const QString MyMoneyODBCDriver::timestampString(const MyMoneyDbDatetimeColumn& c) const
{
  QString qs = QString("%1 datetime").arg(c.name());

  if (c.isNotNull()) qs += " NOT NULL";

  return qs;
}

//***********************************************
// Define the FOR UPDATE string
// So far, only SQLite requires special handling.
const QString MyMoneyDbDriver::forUpdateString() const
{
  return " FOR UPDATE";
}

const QString MyMoneySqlite3Driver::forUpdateString() const
{
  return "";
}

//***********************************************
// Define the table option string
// So far, only mysql requires special handling.
const QString MyMoneyMysqlDriver::tableOptionString() const
{
  return " ENGINE = InnoDB";
}

const QString MyMoneyDbDriver::tableOptionString() const
{
  return "";
}

//*************************************************
// replace the QSqlDatabase::tables() call for Mysql only
// see bug 252841
const QStringList MyMoneyDbDriver::tables(QSql::TableType tt, const QSqlDatabase& db) const {
  return (db.tables(tt));
}

const QStringList MyMoneyMysqlDriver::tables(QSql::TableType tt, const QSqlDatabase& db) const {
  QStringList tableList;
  QSqlQuery q(db);
  QString selectString;
  switch (tt) {
  case QSql::AllTables:
    selectString = QString("SELECT TABLE_NAME FROM information_schema.TABLES WHERE TABLE_SCHEMA = '%1'").arg(db.databaseName());
    if (!q.exec(selectString)) {
      throw new MYMONEYEXCEPTION("select names failed in mymoneydbdriver.cpp");
    }
    while (q.next())
      tableList.append(q.value(0).toString());
    break;
  case QSql::Tables:
  case QSql::SystemTables:
  case QSql::Views:
    qFatal("Programming error in mymoneydbdriver.cpp"); // KMM only uses AllTables; implement other options if required
  }

  return (tableList);
}
