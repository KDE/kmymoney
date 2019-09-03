/***************************************************************************
                          mymoneydbdef.h
                          -------------------
    begin                : 20 February 2010
    copyright            : (C) 2010 by Fernando Vilas
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYDBDEF_H
#define MYMONEYDBDEF_H

// ----------------------------------------------------------------------------
// System Includes
#include <limits>

// ----------------------------------------------------------------------------
// QT Includes
#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>

// ----------------------------------------------------------------------------
// KDE Includes
#include <ksharedptr.h>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyDbDriver;
class MyMoneyStorageSql;

/**
  * The MyMoneyDbColumn class is a base type for generic db columns.
  * Derived types exist for several common column types.
  */
class MyMoneyDbColumn : public KShared
{
public:
  explicit MyMoneyDbColumn(const QString& iname,
                           const QString& itype = QString(),
                           const bool iprimary = false,
                           const bool inotnull = false,
                           const int initVersion = 0,
                           const int lastVersion = std::numeric_limits<int>::max()
                          ):
      m_name(iname),
      m_type(itype),
      m_isPrimary(iprimary),
      m_isNotNull(inotnull),
      m_initVersion(initVersion),
      m_lastVersion(lastVersion) {}
  MyMoneyDbColumn() {}
  virtual ~MyMoneyDbColumn() {}

  /**
    * This method is used to copy column objects. Because there are several derived types,
    * clone() is more appropriate than a copy ctor in most cases.
    */
  virtual MyMoneyDbColumn* clone() const;

  /**
    * This method generates the DDL (Database Design Language) string for the column.
    *
    * @param dbType Database driver type
    *
    * @return QString of the DDL for the column, tailored for what the driver supports.
    */
  virtual const QString generateDDL(const KSharedPtr<MyMoneyDbDriver>& driver) const;

  const QString& name() const {
    return (m_name);
  }
  const QString& type() const {
    return (m_type);
  }
  bool isPrimaryKey() const {
    return (m_isPrimary);
  }
  bool isNotNull() const {
    return (m_isNotNull);
  }
  int initVersion() const {
    return (m_initVersion);
  }
  int lastVersion() const {
    return (m_lastVersion);
  }
private:
  QString m_name;
  QString m_type;
  bool m_isPrimary;
  bool m_isNotNull;
  int m_initVersion;
  int m_lastVersion;
};

/**
  * The MyMoneyDbDatetimeColumn class is a representation of datetime columns.
  */
class MyMoneyDbDatetimeColumn : public MyMoneyDbColumn
{
public:
  explicit MyMoneyDbDatetimeColumn(const QString& iname,
                                   const bool iprimary = false,
                                   const bool inotnull = false,
                                   const int initVersion = 0):
      MyMoneyDbColumn(iname, "", iprimary, inotnull, initVersion) {}
  virtual ~MyMoneyDbDatetimeColumn() {}
  virtual const QString generateDDL(const KSharedPtr<MyMoneyDbDriver>& driver) const;
  virtual MyMoneyDbDatetimeColumn* clone() const;
private:
  static const QString calcType();
};

/**
  * The MyMoneyDbColumn class is a representation of integer db columns.
  */
class MyMoneyDbIntColumn : public MyMoneyDbColumn
{
public:
  enum size {TINY, SMALL, MEDIUM, BIG};
  explicit MyMoneyDbIntColumn(const QString& iname,
                              const size type = MEDIUM,
                              const bool isigned = true,
                              const bool iprimary = false,
                              const bool inotnull = false,
                              const int initVersion = 0,
                              const int lastVersion = std::numeric_limits<int>::max() ):
      MyMoneyDbColumn(iname, "", iprimary, inotnull, initVersion, lastVersion),
      m_type(type),
      m_isSigned(isigned) {}
  virtual ~MyMoneyDbIntColumn() {}
  virtual const QString generateDDL(const KSharedPtr<MyMoneyDbDriver>& driver) const;
  virtual MyMoneyDbIntColumn* clone() const;
  size type() const {
    return m_type;
  }
  bool isSigned() const {
    return m_isSigned;
  }
private:
  size m_type;
  bool m_isSigned;
};

/**
  * The MyMoneyDbTextColumn class is a representation of text db columns,
  * for drivers that support it.  If the driver does not support it, it is
  * usually some sort of really large varchar or varchar2.
  */
class MyMoneyDbTextColumn : public MyMoneyDbColumn
{
public:
  enum size {TINY, NORMAL, MEDIUM, LONG};
  explicit MyMoneyDbTextColumn(const QString& iname,
                               const size type = MEDIUM,
                               const bool iprimary = false,
                               const bool inotnull = false,
                               const int initVersion = 0):
      MyMoneyDbColumn(iname, "", iprimary, inotnull, initVersion),
      m_type(type) {}
  virtual ~MyMoneyDbTextColumn() {}
  virtual const QString generateDDL(const KSharedPtr<MyMoneyDbDriver>& driver) const;
  virtual MyMoneyDbTextColumn* clone() const;
  size type() const {
    return m_type;
  }
private:
  size m_type;
};

/**
  * The MyMoneyDbIndex class is a representation of a db index.
  * To provide generic support for most databases, the table name,
  * name of the index, and list of columns for the index are required.
  * Additionally, the user can specify whether the index is unique or not.
  *
  * At this time, different types of index are not supported, since the
  * portability is fairly limited.
  */
class MyMoneyDbIndex
{
public:
  MyMoneyDbIndex(const QString& table,
                 const QString& name,
                 const QStringList& columns,
                 bool unique = false):
      m_table(table),
      m_unique(unique),
      m_name(name),
      m_columns(columns) {}
  MyMoneyDbIndex() {}
  inline const QString table() const {
    return m_table;
  }
  inline bool isUnique() const {
    return m_unique;
  }
  inline const QString name() const {
    return m_name;
  }
  inline const QStringList columns() const {
    return m_columns;
  }
  const QString generateDDL(const KSharedPtr<MyMoneyDbDriver>& driver) const;
private:
  QString m_table;
  bool m_unique;
  QString m_name;
  QStringList m_columns;
};

/**
  * The MyMoneyDbTable class is a representation of a db table.
  * It has a list of the columns (pointers to MyMoneyDbColumn types) and a
  * list of any indices that may be on the table.
  * Additionally, a string for a parameterized query for each of some common
  * tasks on a table is created by the ctor.
  *
  * Const iterators over the list of columns are provided as a convenience.
  */
class MyMoneyDbTable
{
public:
  MyMoneyDbTable(const QString& iname,
                 const QList<KSharedPtr <MyMoneyDbColumn> >& ifields,
                 const QString& initVersion = "1.0"):
      m_name(iname),
      m_fields(ifields),
      m_initVersion(initVersion) {}
  MyMoneyDbTable() {}

  inline const QString& name() const {
    return (m_name);
  }
  inline const QString& insertString() const {
    return (m_insertString);
  };
  inline const QString selectAllString(bool terminate = true) const {
    return (terminate ? QString(m_selectAllString + ";") : m_selectAllString);
  };
  inline const QString& updateString() const {
    return (m_updateString);
  };
  inline const QString& deleteString() const {
    return (m_deleteString);
  };
  /**
    * This method determines whether the table has a primary key field
    *
    * @param int database version which has to be checked
    *
    * @return bool table has a priimary key
    */
  bool hasPrimaryKey(int version = std::numeric_limits<int>::max()) const;
  /**
    * This method determines the string required to drop the primary key for the table
    * based on the db specific syntax.
    *
    * @param dbType The driver type of the database.
    *
    * @return QString for the syntax to drop the primary key.
    */
  const QString dropPrimaryKeyString(const KSharedPtr<MyMoneyDbDriver>& driver) const;
  /**
    * This method returns a comma-separated list of all column names in the table
    * which were present in a given version
    *
    * @param version version of database definition required
    *
    * @return QString column list
    */
  const QString columnList(const int version = std::numeric_limits<int>::max()) const;
  /**
    * This method returns the string for changing a column's definition.  It covers statements
    * like ALTER TABLE..CHANGE COLUMN, MODIFY COLUMN, etc.
    *
    * @param dbType The driver type of the database.
    * @param columnName The name of the column to be modified.
    * @param newDef The MyMoneyColumn object of the new column definition.
    *
    * @return QString containing DDL to change the column.
    */
  const QString modifyColumnString(const KSharedPtr<MyMoneyDbDriver>& driver, const QString& columnName, const MyMoneyDbColumn& newDef) const;

  /**
    * This method builds all of the SQL strings for common operations.
    */
  void buildSQLStrings();

  /**
    * This method generates the DDL required to create the table.
    *
    * @param dbType The driver type of the database.
    *
    * @return QString of the DDL.
    */
  const QString generateCreateSQL(const KSharedPtr<MyMoneyDbDriver>& driver, int version = std::numeric_limits<int>::max()) const;

  /**
    * This method creates a MyMoneyDbIndex object and adds it to the list of indices for the table.
    *
    * @param name The name of the index.
    * @param columns The list of the columns affected.
    * @param unique Whether or not this should be a unique index.
    */
  void addIndex(const QString& name, const QStringList& columns, bool unique = false);

  typedef QList<KSharedPtr <MyMoneyDbColumn> >::const_iterator field_iterator;
  inline field_iterator begin() const {
    return m_fields.constBegin();
  }
  inline field_iterator end() const {
    return m_fields.constEnd();
  }

  int fieldNumber(const QString& name) const;

  typedef QList<MyMoneyDbIndex>::const_iterator index_iterator;
  inline index_iterator indexBegin() const {
    return m_indices.constBegin();
  }
  inline index_iterator indexEnd() const {
    return m_indices.constEnd();
  }

  inline QString fullColumnString() const
  {
    QString colList = columnList();
    const QStringList columns = colList.remove(QChar(' ')).split(',');
    const int maxColumn = columns.count();
    QString qs;
    for (int i = 0; i < maxColumn; ++i) {
      qs += QString("%1.%2, ").arg(m_name, columns.at(i));
    }
    if (!qs.isEmpty())
      qs = qs.left(qs.length() - 2);
    return qs;
  }

private:
  QString m_name;
  QList<KSharedPtr <MyMoneyDbColumn> > m_fields;
  QHash<QString, int> m_fieldOrder;

  QList<MyMoneyDbIndex> m_indices;
  QString m_initVersion;
  QString m_insertString; // string to insert a record
  QString m_selectAllString; // to select all fields
  QString m_updateString;  // normal string for record update
  QString m_deleteString; // string to delete 1 record
};

/**
  * The MyMoneyDbView class is a representation of a db view.
  *
  * Views will be dropped and recreated on upgrade, so there is no need
  * to do anything more complex than storing the name of the view and
  * the CREATE VIEW string.
  */
class MyMoneyDbView
{
public:
  MyMoneyDbView(const QString& name,
                const QString& createString,
                const QString& initVersion = "0.1")
      : m_name(name), m_createString(createString), m_initVersion(initVersion) {}

  MyMoneyDbView() {}

  inline const QString& name() const {
    return (m_name);
  }
  inline const QString createString() const {
    return (m_createString);
  };

private:
  QString m_name;
  QString m_createString;
  QString m_initVersion;
};

/**
  * The MyMoneyDbDef class is
  */
class MyMoneyDbDef
{
  friend class MyMoneyStorageSql;
public:
  MyMoneyDbDef();
  ~MyMoneyDbDef() {}

  const QString generateSQL(const KSharedPtr<MyMoneyDbDriver>& driver) const;

  typedef QMap<QString, MyMoneyDbTable>::const_iterator table_iterator;
  inline table_iterator tableBegin() const {
    return m_tables.constBegin();
  }
  inline table_iterator tableEnd() const {
    return m_tables.constEnd();
  }

  typedef QMap<QString, MyMoneyDbView>::const_iterator view_iterator;
  inline view_iterator viewBegin() const {
    return m_views.constBegin();
  }
  inline view_iterator viewEnd() const {
    return m_views.constEnd();
  }

  inline unsigned int currentVersion() const {
    return (m_currentVersion);
  };

private:
  const QString enclose(const QString& text) const {
    return (QString("'" + text + "'"));
  }

  static unsigned int m_currentVersion; // The current version of the database layout

#define TABLE(name) void name();
#define VIEW(name) void name();
  TABLE(FileInfo)
  TABLE(Institutions)
  TABLE(Payees)
  TABLE(PayeesPayeeIdentifier)
  TABLE(Tags)
  TABLE(TagSplits)
  TABLE(Accounts)
  TABLE(AccountsPayeeIdentifier)
  TABLE(Transactions)
  TABLE(Splits)
  TABLE(KeyValuePairs)
  TABLE(Schedules)
  TABLE(SchedulePaymentHistory)
  TABLE(Securities)
  TABLE(Prices)
  TABLE(Currencies)
  TABLE(Reports)
  TABLE(Budgets)
  TABLE(OnlineJobs)
  TABLE(PayeeIdentifier)
  TABLE(PluginInfo)
  TABLE(CostCenter)

  VIEW(Balances)
protected:
  QMap<QString, MyMoneyDbTable> m_tables;
  QMap<QString, MyMoneyDbView> m_views;
};

#endif // MYMONEYDBDEF_H
