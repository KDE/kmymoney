/***************************************************************************
                          mymoneystoragesql.h
                          -------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
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

#ifndef MYMONEYSTORAGESQL_H
#define MYMONEYSTORAGESQL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QStack>

#include <QtDebug>

class QIODevice;
// ----------------------------------------------------------------------------
// KDE Includes

#include <kurl.h>
#include <ksharedptr.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneystorageformat.h"
#include "../mymoneyinstitution.h"
#include "../mymoneypayee.h"
#include "../mymoneyaccount.h"
#include "../mymoneytransaction.h"
#include "../mymoneysplit.h"
#include "../mymoneyscheduled.h"
#include "../mymoneysecurity.h"
#include "../mymoneyprice.h"
#include "../mymoneyreport.h"
#include "../mymoneybudget.h"
#include "../mymoneyfile.h"
#include "../mymoneykeyvaluecontainer.h"
#include "mymoneymap.h"
#include "../mymoneymoney.h"
#include "../mymoneytransactionfilter.h"

// This is a convenience functor to make it easier to use STL algorithms
// It will return false if the MyMoneyTransaction DOES match the filter.
// This functor may disappear when all filtering can be handled in SQL.
class FilterFail {
  public:
    FilterFail (const MyMoneyTransactionFilter& filter,
                   IMyMoneyStorage* storage)
      : m_filter (filter),
        m_storage (storage)
    {}

    inline bool operator() (const QPair<QString, MyMoneyTransaction>& transactionPair)
    { return (*this) (transactionPair.second); }

    inline bool operator() (const MyMoneyTransaction& transaction)
    {
      return (! m_filter.match(transaction)) && (m_filter.matchingSplits().count() == 0);
    }

  private:
    MyMoneyTransactionFilter m_filter;
    IMyMoneyStorage *m_storage;
};

/**
@author Tony Bloomfield
 */
typedef enum databaseTypeE { // database (driver) type
  Db2 = 0, //
  Interbase, //
  Mysql, //
  Oracle8, //
  ODBC3, //
  Postgresql, //
  Sqlite, //
  Sybase, //
  Sqlite3 //
} _databaseType;

class MyMoneyStorageSql;

/**
  * The MyMoneySqlQuery class is derived from QSqlQuery to provide
  * a way to adjust some queries based on databaseTypeE and make
  * debugging easier by providing a place to put debug statements.
  */
class MyMoneySqlQuery : public QSqlQuery {
  public:
    MyMoneySqlQuery (MyMoneyStorageSql* db = 0);
    virtual ~MyMoneySqlQuery();
    bool exec ();
    bool prepare ( const QString & query );
  private:
    const MyMoneyStorageSql* m_db;
};

/**
  * The MyMoneyDbDrivers class is a map from string to enum of db types.
  */
class MyMoneyDbDrivers {
  public:
    MyMoneyDbDrivers ();
    /**
      *  @return a list ofsupported Qt database driver types, their qt names and useful names
      **/
    const QMap<QString, QString> driverMap() const {return (m_driverMap);};
    databaseTypeE driverToType (const QString& driver) const;
    bool isTested (databaseTypeE dbType) const;
 private:
    QMap<QString, QString> m_driverMap;
};

/**
  * The MyMoneyDbColumn class is a base type for generic db columns.
  * Derived types exist for several common column types.
  */
class MyMoneyDbColumn : public KShared {
  public:
    MyMoneyDbColumn (const QString& iname,
             const QString& itype = QString::null,
             const bool iprimary = false,
             const bool inotnull = false,
             const QString &initVersion = "0.1"):
    m_name(iname),
    m_type(itype),
    m_isPrimary(iprimary),
    m_isNotNull(inotnull),
    m_initVersion(initVersion) {}
    MyMoneyDbColumn (void) {}
    virtual ~MyMoneyDbColumn () {}

    /**
      * This method is used to copy column objects. Because there are several derived types,
      * clone() is more appropriate than a copy ctor in most cases.
      */
    virtual MyMoneyDbColumn* clone () const;

    /**
      * This method generates the DDL (Database Design Language) string for the column.
      *
      * @param dbType Database driver type
      *
      * @return QString of the DDL for the column, tailored for what the driver supports.
      */
    virtual const QString generateDDL (databaseTypeE dbType) const;

    const QString& name(void) const {return (m_name);}
    const QString& type(void) const {return (m_type);}
    bool isPrimaryKey(void) const {return (m_isPrimary);}
    bool isNotNull(void) const {return (m_isNotNull);}
  private:
    QString m_name;
    QString m_type;
    bool m_isPrimary;
    bool m_isNotNull;
    QString m_initVersion;
};

/**
  * The MyMoneyDbDatetimeColumn class is a representation of datetime columns.
  */
class MyMoneyDbDatetimeColumn : public MyMoneyDbColumn {
  public:
    MyMoneyDbDatetimeColumn (const QString& iname,
                             const bool iprimary = false,
                             const bool inotnull = false,
                             const QString &initVersion = "0.1"):
            MyMoneyDbColumn (iname, "", iprimary, inotnull, initVersion)
            {}
    virtual ~MyMoneyDbDatetimeColumn() {}
    virtual const QString generateDDL (databaseTypeE dbType) const;
    virtual MyMoneyDbDatetimeColumn* clone () const;
  private:
    static const QString calcType(void);
};

/**
  * The MyMoneyDbColumn class is a representation of integer db columns.
  */
class MyMoneyDbIntColumn : public MyMoneyDbColumn {
  public:
    enum size {TINY, SMALL, MEDIUM, BIG};
    MyMoneyDbIntColumn (const QString& iname,
                        const size type = MEDIUM,
                        const bool isigned = true,
                        const bool iprimary = false,
                        const bool inotnull = false,
             const QString &initVersion = "0.1"):
        MyMoneyDbColumn (iname, "", iprimary, inotnull, initVersion),
    m_type  (type),
    m_isSigned (isigned) {}
    virtual ~MyMoneyDbIntColumn() {}
    virtual const QString generateDDL (databaseTypeE dbType) const;
    virtual MyMoneyDbIntColumn* clone () const;
  private:
    size m_type;
    bool m_isSigned;
};

/**
  * The MyMoneyDbTextColumn class is a representation of text db columns,
  * for drivers that support it.  If the driver does not support it, it is
  * usually some sort of really large varchar or varchar2.
  */
class MyMoneyDbTextColumn : public MyMoneyDbColumn {
  public:
    enum size {TINY, NORMAL, MEDIUM, LONG};
    MyMoneyDbTextColumn (const QString& iname,
                         const size type = MEDIUM,
                         const bool iprimary = false,
                         const bool inotnull = false,
             const QString &initVersion = "0.1"):
        MyMoneyDbColumn (iname, "", iprimary, inotnull, initVersion),
    m_type  (type) {}
    virtual ~MyMoneyDbTextColumn() {}
    virtual const QString generateDDL (databaseTypeE dbType) const;
    virtual MyMoneyDbTextColumn* clone () const;
  private:
    size m_type;
};

/**
  * The MyMoneyDbIndex class is a representation of a db index.
  * To provide generic support for most databases, the table name,
  * name of the index, and list of columns for the index are required.
  * Additionally, the user can specify whether the index is unique or not.
  *
  * At this time, different types of index are not supported, since the portability
  * is fairly limited.
  */
class MyMoneyDbIndex {
  public:
    MyMoneyDbIndex (const QString& table,
                    const QString& name,
                    const QStringList& columns,
                    bool unique = false):
      m_table(table),
      m_unique(unique),
      m_name(name),
      m_columns(columns)
      {}
    MyMoneyDbIndex () {}
    inline const QString table () const {return m_table;}
    inline bool isUnique () const {return m_unique;}
    inline const QString name () const {return m_name;}
    inline const QStringList columns () const {return m_columns;}
    const QString generateDDL (databaseTypeE dbType) const;
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
class MyMoneyDbTable {
  public:
    MyMoneyDbTable (const QString& iname,
             const QList<KSharedPtr <MyMoneyDbColumn> >& ifields,
             const QString& initVersion = "1.0"):
    m_name(iname),
    m_fields(ifields),
    m_initVersion(initVersion) {}
    MyMoneyDbTable (void) {}

    inline const QString& name(void) const {return (m_name);}
    inline const QString& insertString(void) const {return (m_insertString);};
    inline const QString selectAllString(bool terminate = true) const
      {return (terminate ? QString(m_selectAllString + ";") : m_selectAllString);};
    inline const QString& updateString(void) const {return (m_updateString);};
    inline const QString& deleteString(void) const {return (m_deleteString);};

    /**
      * This method determines the string required to drop the primary key for the table
      * based on the db specific syntax.
      *
      * @param dbType The driver type of the database.
      *
      * @return QString for the syntax to drop the primary key.
      */
    const QString dropPrimaryKeyString(databaseTypeE dbType) const;
    /**
      * This method returns a comma-separated list of all column names in the table
      *
      * @return QString column list.
      */
    const QString columnList() const;
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
    const QString modifyColumnString(databaseTypeE dbType, const QString& columnName, const MyMoneyDbColumn& newDef) const;

    /**
      * This method builds all of the SQL strings for common operations.
      */
    void buildSQLStrings(void);

    /**
      * This method generates the DDL required to create the table.
      *
      * @param dbType The driver type of the database.
      *
      * @return QString of the DDL.
      */
    const QString generateCreateSQL (databaseTypeE dbType) const;

    /**
      * This method creates a MyMoneyDbIndex object and adds it to the list of indices for the table.
      *
      * @param name The name of the index.
      * @param columns The list of the columns affected.
      * @param unique Whether or not this should be a unique index.
      */
    void addIndex(const QString& name, const QStringList& columns, bool unique = false);

    typedef QList<KSharedPtr <MyMoneyDbColumn> >::const_iterator field_iterator;
    inline field_iterator begin(void) const {return m_fields.constBegin();}
    inline field_iterator end(void) const {return m_fields.constEnd(); }
  private:
    QString m_name;
    QList<KSharedPtr <MyMoneyDbColumn> > m_fields;

    typedef QList<MyMoneyDbIndex>::const_iterator index_iterator;
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
class MyMoneyDbView {
  public:
    MyMoneyDbView (const QString& name,
                   const QString& createString,
                   const QString& initVersion = "0.1")
    : m_name (name), m_createString (createString), m_initVersion (initVersion)
    {}

    MyMoneyDbView (void) {}

    inline const QString& name(void) const {return (m_name);}
    inline const QString createString(void) const {return (m_createString);};

  private:
    QString m_name;
    QString m_createString;
    QString m_initVersion;
};

/**
  * The MyMoneyDbDef class is
  */
class MyMoneyDbDef  {
  friend class MyMoneyStorageSql;
  friend class MyMoneyDatabaseMgr;
public:
    MyMoneyDbDef();
    ~MyMoneyDbDef() {}

    const QString generateSQL (const QString& driver) const;

    typedef QMap<QString, MyMoneyDbTable>::const_iterator table_iterator;
    inline table_iterator tableBegin(void) const {return m_tables.constBegin();}
    inline table_iterator tableEnd(void) const {return m_tables.constEnd();}

    typedef QMap<QString, MyMoneyDbView>::const_iterator view_iterator;
    inline view_iterator viewBegin(void) const {return m_views.constBegin();}
    inline view_iterator viewEnd(void) const {return m_views.constEnd();}

    inline unsigned int currentVersion() const {return (m_currentVersion);};

private:
  static unsigned int m_currentVersion; // The current version of the database layout
  MyMoneyDbDrivers m_drivers;
#define TABLE(name) void name();
#define VIEW(name) void name();
  TABLE(FileInfo)
  TABLE(Institutions)
  TABLE(Payees)
  TABLE(Accounts)
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
  VIEW(Balances)
protected:
  QMap<QString, MyMoneyDbTable> m_tables;
  QMap<QString, MyMoneyDbView> m_views;
};

class IMyMoneySerialize;

/**
  * The MyMoneyDbColumn class is a base type for generic db columns.
  * Derived types exist for several common column types.
  */
class MyMoneyStorageSql : public IMyMoneyStorageFormat, public QSqlDatabase, public KShared {
public:

  MyMoneyStorageSql (IMyMoneySerialize *storage, const KUrl& = KUrl());
  virtual ~MyMoneyStorageSql() {close(true);}

  unsigned int currentVersion() const {return (m_db.currentVersion());};

    /**
   * MyMoneyStorageSql - open database file
   *
   * @param url pseudo-URL of database to be opened
   * @param openMode open mode, same as for QFile::open
   * @param clear whether existing data can be deleted

   * @return 0 - database successfully opened
   * @return 1 - database not opened, use lastError function for reason
   * @return -1 - output database not opened, contains data, clean not specified
   *
     */
  int open(const KUrl& url, int openMode, bool clear = false);
  /**
   * MyMoneyStorageSql close the database
   *
   * @return void
   *
   */
  void close(bool logoff = true);
  /**
   * MyMoneyStorageSql read all the database into storage
   *
   * @return void
   *
   */
  bool readFile(void);
  /**
   * MyMoneyStorageSql write/update the database from storage
   *
   * @return void
   *
   */
  bool writeFile(void);

  // check database type
  bool isDb2() const { return (m_dbType == Db2);};
  bool isInterbase() const { return (m_dbType == Interbase);};
  bool isMysql() const { return (m_dbType == Mysql);};
  bool isOracle8() const { return (m_dbType == Oracle8);};
  bool isODBC3() const { return (m_dbType == ODBC3);};
  bool isPostgresql() const { return (m_dbType == Postgresql);};
  bool isSybase() const { return (m_dbType == Sybase);};
  bool isSqlite3() const { return (m_dbType == Sqlite3);};

    /**
   * MyMoneyStorageSql generalized error routine
   *
   * @return : error message to be displayed
   *
     */
  const QString& lastError() const {return (m_error);};
  /**
   * This method is used when a database file is open, and the data is to
   * be saved in a different file or format. It will ensure that all data
   * from the database is available in memory to enable it to be written.
   */
  virtual void fillStorage();
  /**
    * The following functions correspond to the identically named (usually) functions
    * within the Storage Manager, and are called to update the database
    */
  void modifyUserInfo(const MyMoneyPayee& payee);
  void addInstitution(const MyMoneyInstitution& inst);
  void modifyInstitution(const MyMoneyInstitution& inst);
  void removeInstitution(const MyMoneyInstitution& inst);
  void addPayee(const MyMoneyPayee& payee);
  void modifyPayee(const MyMoneyPayee& payee);
  void removePayee(const MyMoneyPayee& payee);
  void addAccount(const MyMoneyAccount& acc);
  void modifyAccount(const MyMoneyAccount& acc);
  void removeAccount(const MyMoneyAccount& acc);
  void addTransaction(const MyMoneyTransaction& tx);
  void modifyTransaction(const MyMoneyTransaction& tx);
  void removeTransaction(const MyMoneyTransaction& tx);
  void addSchedule(const MyMoneySchedule& sch);
  void modifySchedule(const MyMoneySchedule& sch);
  void removeSchedule(const MyMoneySchedule& sch);
  void addSecurity(const MyMoneySecurity& sec);
  void modifySecurity(const MyMoneySecurity& sec);
  void removeSecurity(const MyMoneySecurity& sec);
  void addPrice(const MyMoneyPrice& p);
  void removePrice(const MyMoneyPrice& p);
  void addCurrency(const MyMoneySecurity& sec);
  void modifyCurrency(const MyMoneySecurity& sec);
  void removeCurrency(const MyMoneySecurity& sec);
  void addReport(const MyMoneyReport& rep);
  void modifyReport(const MyMoneyReport& rep);
  void removeReport(const MyMoneyReport& rep);
  void addBudget(const MyMoneyBudget& bud);
  void modifyBudget(const MyMoneyBudget& bud);
  void removeBudget(const MyMoneyBudget& bud);

  unsigned long transactionCount  (const QString& aid = QString()) const;
  inline const QMap<QString, unsigned long> transactionCountMap () const
      {return (m_transactionCountMap);};
  /**
    * the storage manager also needs the following read entry points
    */
  const QMap<QString, MyMoneyAccount> fetchAccounts (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneyMoney> fetchBalance(const QStringList& id, const QDate& date) const;
  const QMap<QString, MyMoneyBudget> fetchBudgets (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneySecurity> fetchCurrencies (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneyInstitution> fetchInstitutions (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneyPayee> fetchPayees (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const  MyMoneyPriceList fetchPrices (const QStringList& fromIdList = QStringList (), const QStringList& toIdList = QStringList(), bool forUpdate = false) const;
  const  MyMoneyPrice fetchSinglePrice (const QString& fromIdList, const QString& toIdList, const QDate& date, bool exactDate, bool forUpdate = false) const;
  const QMap<QString, MyMoneyReport> fetchReports (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneySchedule> fetchSchedules (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneySecurity> fetchSecurities (const QStringList& idList = QStringList (), bool forUpdate = false) const;
  const QMap<QString, MyMoneyTransaction> fetchTransactions (const QString& tidList = QString (), const QString& dateClause = QString(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyTransaction> fetchTransactions (const MyMoneyTransactionFilter& filter) const;
  bool isReferencedByTransaction(const QString& id) const;

  void readPayees(const QString&);
  void readPayees(const QList<QString> payeeList = QList<QString>());
  void readTransactions(const MyMoneyTransactionFilter& filter);
  void setProgressCallback(void(*callback)(int, int, const QString&));

  virtual void readFile(QIODevice* s, IMyMoneySerialize* storage) { Q_UNUSED(s); Q_UNUSED(storage) };
  virtual void writeFile(QIODevice* s, IMyMoneySerialize* storage){ Q_UNUSED(s); Q_UNUSED(storage) };

  void startCommitUnit (const QString& callingFunction);
  bool endCommitUnit (const QString& callingFunction);
  void cancelCommitUnit (const QString& callingFunction);

  long unsigned getRecCount(const QString& table) const;
  long unsigned getNextBudgetId() const;
  long unsigned getNextAccountId() const;
  long unsigned getNextInstitutionId() const;
  long unsigned getNextPayeeId() const;
  long unsigned getNextReportId() const;
  long unsigned getNextScheduleId() const;
  long unsigned getNextSecurityId() const;
  long unsigned getNextTransactionId() const;

  long unsigned incrementBudgetId();
  long unsigned incrementAccountId();
  long unsigned incrementInstitutionId();
  long unsigned incrementPayeeId();
  long unsigned incrementReportId();
  long unsigned incrementScheduleId();
  long unsigned incrementSecurityId();
  long unsigned incrementTransactionId();

  void loadAccountId(const unsigned long& id);
  void loadTransactionId(const unsigned long& id);
  void loadPayeeId(const unsigned long& id);
  void loadInstitutionId(const unsigned long& id);
  void loadScheduleId(const unsigned long& id);
  void loadSecurityId(const unsigned long& id);
  void loadReportId(const unsigned long& id);
  void loadBudgetId(const unsigned long& id);

private:
  void init(void);
  // a function to build a comprehensive error message
  QString& buildError (const QSqlQuery& q, const QString& function, const QString& message) const;
  QString& buildError (const QSqlQuery& q, const QString& function, const QString& message,
                       const QSqlDatabase*) const;
  // write routines
  void writeUserInformation(void);
  void writeInstitutions(void);
  void writePayees(void);
  void writeAccounts(void);
  void writeTransactions(void);
  void writeSchedules(void);
  void writeSecurities(void);
  void writePrices(void);
  void writeCurrencies(void);
  void writeFileInfo(void);
  void writeReports(void);
  void writeBudgets(void);

  void writeInstitution(const MyMoneyInstitution& i, MyMoneySqlQuery& q);
  void writePayee(const MyMoneyPayee& p, MyMoneySqlQuery& q, bool isUserInfo = false);
  void writeAccount (const MyMoneyAccount& a, MyMoneySqlQuery& q);
  void writeTransaction(const QString& txId, const MyMoneyTransaction& tx, MyMoneySqlQuery& q, const QString& type);
  void writeSplits(const QString& txId, const QString& type, const QList<MyMoneySplit>& splitList);
  void writeSplit(const QString& txId, const MyMoneySplit& split, const QString& type, const int splitId, MyMoneySqlQuery& q);
  void writeSchedule(const MyMoneySchedule& sch, MyMoneySqlQuery& q, bool insert);
  void writeSecurity(const MyMoneySecurity& security, MyMoneySqlQuery& q);
  void writePricePair ( const MyMoneyPriceEntries& p);
  void writePrice (const MyMoneyPrice& p);
  void writeCurrency(const MyMoneySecurity& currency, MyMoneySqlQuery& q);
  void writeReport (const MyMoneyReport& rep, MyMoneySqlQuery& q);
  void writeBudget (const MyMoneyBudget& bud, MyMoneySqlQuery& q);
  void writeKeyValuePairs(const QString& kvpType, const QString& kvpId, const QMap<QString, QString>& pairs);
  void writeKeyValuePair(const QString& kvpType, const QString& kvpId,
                         const QString& kvpKey, const QString& kvpData);
  // read routines
  void readFileInfo(void);
  void readLogonData(void);
  void readUserInformation(void);
  void readInstitutions(void);
  void readAccounts(void);
  void readTransaction(const QString id);
  void readTransactions(const QString& tidList = QString(), const QString& dateClause = QString());
  void readTransaction(MyMoneyTransaction &tx, const QString& tid);
  void readSplit (MyMoneySplit& s, const MyMoneySqlQuery& q, const MyMoneyDbTable& t) const;
  const MyMoneyKeyValueContainer readKeyValuePairs (const QString& kvpType, const QString& kvpId) const;
  const QMap<QString, MyMoneyKeyValueContainer> readKeyValuePairs (const QString& kvpType, const QStringList& kvpIdList) const;
  void readSchedules(void);
  void readSecurities(void);
  void readPrices(void);
  void readCurrencies(void);
  void readReports(void);
  void readBudgets(void);

  void deleteTransaction(const QString& id);
  void deleteSchedule(const QString& id);
  void deleteKeyValuePairs(const QString& kvpType, const QString& kvpId);
  long unsigned calcHighId (const long unsigned&, const QString&);

  void setVersion (const QString& version);

  void signalProgress(int current, int total, const QString& = "") const;
  void (*m_progressCallback)(int, int, const QString&);

  //void startCommitUnit (const QString& callingFunction);
  //void endCommitUnit (const QString& callingFunction);
  //void cancelCommitUnit (const QString& callingFunction);
  int splitState(const MyMoneyTransactionFilter::stateOptionE& state) const;

  inline const QDate getDate (const QString& date) const {
    return (date.isNull() ? QDate() : QDate::fromString(date, Qt::ISODate));
  }

  inline const QDateTime getDateTime (const QString& date) const {
    return (date.isNull() ? QDateTime() : QDateTime::fromString(date, Qt::ISODate));
  }

  // open routines
  /**
   * MyMoneyStorageSql create database
   *
   * @param url pseudo-URL of database to be opened
   *
   * @return true - creation successful
   * @return false - could not create
   *
   */
  bool createDatabase(const KUrl& url);
  int upgradeDb();
  int upgradeToV1();
  int upgradeToV2();
  int upgradeToV3();
  int upgradeToV4();
  int upgradeToV5();
  int upgradeToV6();
  bool sqliteAlterTable(const MyMoneyDbTable& t);
  bool addColumn(const MyMoneyDbTable& t, const MyMoneyDbColumn& c,
                 const QString& after = QString());
  bool addColumn(const QString& table,
                 const QString& column,
                 const QString& after = QString());
  bool dropColumn(const MyMoneyDbTable& t,
                  const QString& c);
  bool dropColumn(const QString& table,
                  const QString& column);

//  long long unsigned getRecCount(const QString& table);
  int createTables();
  void createTable(const MyMoneyDbTable& t);
  void clean ();
  int isEmpty();
  // data
  MyMoneyDbDrivers m_drivers;
  databaseTypeE m_dbType;

  MyMoneyDbDef m_db;
  unsigned int m_dbVersion;
  IMyMoneySerialize *m_storage;
  IMyMoneyStorage *m_storagePtr;
  // input options
  bool m_loadAll; // preload all data
  bool m_override; // override open if already in use
  // error message
  QString m_error;
  // record counts
  long unsigned m_institutions;
  long unsigned m_accounts;
  long unsigned m_payees;
  long unsigned m_transactions;
  long unsigned m_splits;
  long unsigned m_securities;
  long unsigned m_prices;
  long unsigned m_currencies;
  long unsigned m_schedules;
  long unsigned m_reports;
  long unsigned m_kvps;
  long unsigned m_budgets;
  // next id to use (for future archive)
  long unsigned m_hiIdInstitutions;
  long unsigned m_hiIdPayees;
  long unsigned m_hiIdAccounts;
  long unsigned m_hiIdTransactions;
  long unsigned m_hiIdSchedules;
  long unsigned m_hiIdSecurities;
  long unsigned m_hiIdReports;
  long unsigned m_hiIdBudgets;
  // encrypt option - usage TBD
  QString m_encryptData;

  /**
    * This variable is used to suppress status messages except during
   * initial data load and final write

  */
  bool m_displayStatus;
  /**
   * On occasions, e.g. after a complex transaction search, or for populating a
   * payee popup list, it becomes necessary to load all data into memory. The
   * following flags will be set after such a load, to indicate that further
   * retrievals are not needed.
   */
//  bool m_transactionListRead;
//  bool m_payeeListRead;
  /**
   * This member variable holds a list of those accounts for which all
   * transactions are in memory, thus saving reading them again
   */
//  QList<QString> m_accountsLoaded;
  /**
    * This member variable is used when loading transactions to list all
    * referenced payees, which can then be read into memory (if not already there)
    */
//  QList<QString> m_payeeList;

  void alert(QString s) const {qDebug() << s;}; // FIXME: remove...
  /** The following keeps track of commitment units (known as transactions in SQL
    * though it would be confusing to use that term within KMM). It is implemented
    * as a stack for debug purposes. Long term, probably a count would suffice
    */
  QStack<QString> m_commitUnitStack;
  /**
    * This member variable is used to preload transactions for preferred accounts
    */
  MyMoneyTransactionFilter m_preferred;
  /**
    * This member variable is used because reading prices from a file uses the 'add...' function rather than a
    * 'load...' function which other objects use. Having this variable allows us to avoid needing to check the
    * database to see if this really is a new or modified price
    */
  bool m_readingPrices;
  /**
    * This member variable holds a map of transaction counts per account, indexed by
    * the account id. It is used
    * to avoid having to scan all transactions whenever a count is needed. It should
    * probably be moved into the MyMoneyAccount object; maybe we will do that once
    * the database code has been properly checked out
    */
  QMap<QString, unsigned long> m_transactionCountMap;
  /**
    * These member variables hold the user name and date/time of logon
    */
  QString m_logonUser;
  QDateTime m_logonAt;
  QDate m_txPostDate; // FIXME: remove when Tom puts date into split object

  //Disable copying
  MyMoneyStorageSql (const MyMoneyStorageSql& rhs);
  MyMoneyStorageSql& operator= (const MyMoneyStorageSql& rhs);
  bool m_newDatabase;
};
#endif // MYMONEYSTORAGESQL_H
