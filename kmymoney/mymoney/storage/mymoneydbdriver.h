/***************************************************************************
                          mymoneydbdriver.h
                          -------------------
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

#ifndef MYMONEYDBDRIVER_H
#define MYMONEYDBDRIVER_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QMap>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes
#include <ksharedptr.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "mymoneydbdef.h"

/**
@author Tony Bloomfield
 */
typedef enum databaseTypeE { // database (driver) type
  Db2 = 0, //
  Interbase, //
  Mysql, //
  Oracle, //
  ODBC, //
  Postgresql, //
  Sqlite, //
  Sybase, //
  Sqlite3 //
} _databaseType;

/**
  * The MyMoneyDbDriver class hierarchy provides a way to implement DBMS
  * specific parts of SQL strings.
  */
class MyMoneyDbDriver : public KShared
{
public:
  /**
   * MyMoneyDbDriver factory to create the object
   *
   * @param type The name of the driver, according to Qt (QMYSQL, etc)
   *
   * @return A KSharedPtr to the driver for the desired implementation
   */
  static KSharedPtr<MyMoneyDbDriver> create(const QString& type);

  virtual ~MyMoneyDbDriver();

  /**
    *  @return a list ofsupported Qt database driver types, their qt names and useful names
    **/
  static const QMap<QString, QString> driverMap();

  // check database type
  // These are deprecated because there should be no reliance on the type
  // of the database. The class hierarchy should handle it automatically.
  KDE_DEPRECATED inline bool isMysql() const {
    return (m_dbType == Mysql);
  };
  KDE_DEPRECATED inline bool isSqlite3() const {
    return (m_dbType == Sqlite3);
  };

  /**
   * Has the database been tested by developers?
   *
   * @return true if the database has been tested, otherwise false.
   */
  virtual bool isTested() const;

  /**
   * Can the driver automatically create the database? If not, the user will
   * have to create it, then KMM can create the tables and populate them.
   *
   * @return true if the database can be automatically created.
   */
  virtual bool canAutocreate() const;

  /**
   * Some DBMS implementations require a connection to a default database
   * to create another. This gives that name, based on the implementation.
   * Note that this is not a default KMM db name.
   *
   * @return The name of the default database
   */
  virtual const QString defaultDbName() const;

  /**
   * @return The SQL string to create a database
   */
  virtual const QString createDbString(const QString& name) const;

  /**
   * @return " FOR UPDATE" in most cases, but "" for SQLITE
   */
  virtual const QString forUpdateString() const;

  /**
   * @param name The name of the table
   *
   * @return The SQL string to drop a primary key from a table
   */
  virtual const QString dropPrimaryKeyString(const QString& name) const;

  /**
   * modifyColumnString will generate the SQL to modify the type of a column,
   * including type, not null, etc. It is not for changing the column name.
   *
   * @param tableName The name of the table being altered
   * @param columnName The name of the column being altered
   * @param newDef The new definition of the column
   *
   * @return The SQL string to modify the column appropriately
   */
  virtual const QString modifyColumnString(const QString& tableName, const QString& columnName, const MyMoneyDbColumn& newDef) const;

  /**
   * @param c The integer column definition
   *
   * @return The SQL to create the appropriately sized integer column
   */
  virtual const QString intString(const MyMoneyDbIntColumn& c) const;

  /**
   * @param c The text column definition
   *
   * @return The SQL to create the appropriately sized text column
   */
  virtual const QString textString(const MyMoneyDbTextColumn& c) const;

  /**
   * @param c The integer column definition
   *
   * @return The SQL to create the appropriate timestamp/datetime column
   */
  virtual const QString timestampString(const MyMoneyDbDatetimeColumn& c) const;

protected:
  MyMoneyDbDriver(); // only allow create() and derived types to construct
  databaseTypeE m_dbType; // This goes away with the deprecated functions
};

#endif // MYMONEYDBDRIVER_H

