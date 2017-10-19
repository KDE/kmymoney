/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2005 Tony Bloomfield <tonybloom@users.sourceforge.net>
 * Copyright (C)      Fernando Vilas <fvilas@iname.com>
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

#ifndef MYMONEYSTORAGESQL_H
#define MYMONEYSTORAGESQL_H

#include <limits>

#include <QPair>
#include <QString>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QStack>
#include <QUrl>
#include <QExplicitlySharedDataPointer>

class QIODevice;

#include "imymoneystorageformat.h"
#include "mymoneyinstitution.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneybudget.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneydbdef.h"
#include "mymoneydbdriver.h"
#include "onlinejob.h"
#include "payeeidentifier/payeeidentifier.h"

// This is a convenience functor to make it easier to use STL algorithms
// It will return false if the MyMoneyTransaction DOES match the filter.
// This functor may disappear when all filtering can be handled in SQL.

template <class T1, class T2> struct QPair;

class IMyMoneyStorage;
class MyMoneyCostCenter;
class MyMoneyMoney;
class MyMoneySchedule;
class MyMoneySplit;
class MyMoneyTransaction;
class databaseStoreableObject;
class FilterFail
{
public:
  FilterFail(const MyMoneyTransactionFilter& filter) : m_filter(filter) {}

  inline bool operator()(const QPair<QString, MyMoneyTransaction>& transactionPair) {
    return (*this)(transactionPair.second);
  }

  inline bool operator()(const MyMoneyTransaction& transaction) {
    return (! m_filter.match(transaction)) && (m_filter.matchingSplits().count() == 0);
  }

private:
  MyMoneyTransactionFilter m_filter;
};

class MyMoneyStorageSql;

//*****************************************************************************
// Create a class to handle db transactions using scope
//
// Don't let the database object get destroyed while this object exists,
// that would result in undefined behavior.
class MyMoneyDbTransaction
{
public:
  MyMoneyDbTransaction(MyMoneyStorageSql& db, const QString& name);

  ~MyMoneyDbTransaction();
private:
  MyMoneyStorageSql& m_db;
  QString m_name;
};

/**
  * The MyMoneySqlQuery class is derived from QSqlQuery to provide
  * a way to adjust some queries based on database type and make
  * debugging easier by providing a place to put debug statements.
  */
class MyMoneySqlQuery : public QSqlQuery
{
public:
  MyMoneySqlQuery(MyMoneyStorageSql* db = 0);
  virtual ~MyMoneySqlQuery();
  bool exec();
  bool exec(const QString & query);
  bool prepare(const QString & query);
};

class IMyMoneySerialize;

/**
  * The MyMoneyDbColumn class is a base type for generic db columns.
  * Derived types exist for several common column types.
  *
  * @todo Remove unneeded columns which store the row count of tables from kmmFileInfo
  */
class MyMoneyStorageSql : public IMyMoneyStorageFormat, public QSqlDatabase, public QSharedData
{
  friend class MyMoneyDbDef;
  KMM_MYMONEY_UNIT_TESTABLE

public:
  explicit MyMoneyStorageSql(IMyMoneySerialize *storage, const QUrl& = QUrl());
  virtual ~MyMoneyStorageSql();

  unsigned int currentVersion() const {
    return (m_db.currentVersion());
  };

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
  int open(const QUrl &url, int openMode, bool clear = false);
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
  bool readFile();
  /**
   * MyMoneyStorageSql write/update the database from storage
   *
   * @return void
   *
   */
  bool writeFile();

  /**
   * MyMoneyStorageSql generalized error routine
   *
   * @return : error message to be displayed
   *
   */
  const QString& lastError() const {
    return (m_error);
  };

  /**
   * MyMoneyStorageSql get highest ID number from the database
   *
   * @return : highest ID number
   */
  long unsigned highestNumberFromIdString(QString tableName, QString tableField, int prefixLength);

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
  void modifyPayee(MyMoneyPayee payee);
  void removePayee(const MyMoneyPayee& payee);
  void addTag(const MyMoneyTag& tag);
  void modifyTag(const MyMoneyTag& tag);
  void removeTag(const MyMoneyTag& tag);
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
  void addOnlineJob(const onlineJob& job);
  void modifyOnlineJob(const onlineJob& job);
  void removeOnlineJob(const onlineJob& job);
  void addPayeeIdentifier(payeeIdentifier& ident);
  void modifyPayeeIdentifier(const payeeIdentifier& ident);
  void removePayeeIdentifier(const payeeIdentifier& ident);

  unsigned long transactionCount(const QString& aid = QString()) const;
  inline const QHash<QString, unsigned long> transactionCountMap() const {
    return (m_transactionCountMap);
  };

  /**
   * The following functions are perform the same operations as the
   * above functions, but on a QList of the items.
   * This reduces db round-trips, so should be the preferred method when
   * such a function exists.
   */
  void modifyAccountList(const QList<MyMoneyAccount>& acc);

  /**
    * the storage manager also needs the following read entry points
    */
  const QMap<QString, MyMoneyAccount> fetchAccounts(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyMoney> fetchBalance(const QStringList& id, const QDate& date) const;
  const QMap<QString, MyMoneyBudget> fetchBudgets(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneySecurity> fetchCurrencies(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyInstitution> fetchInstitutions(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyPayee> fetchPayees(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyTag> fetchTags(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, onlineJob> fetchOnlineJobs(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyCostCenter> fetchCostCenters(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const MyMoneyPriceList fetchPrices(const QStringList& fromIdList = QStringList(), const QStringList& toIdList = QStringList(), bool forUpdate = false) const;
  MyMoneyPrice fetchSinglePrice(const QString& fromId, const QString& toId, const QDate& date_, bool exactDate, bool = false) const;
  const QMap<QString, MyMoneyReport> fetchReports(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneySchedule> fetchSchedules(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneySecurity> fetchSecurities(const QStringList& idList = QStringList(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyTransaction> fetchTransactions(const QString& tidList = QString(), const QString& dateClause = QString(), bool forUpdate = false) const;
  const QMap<QString, MyMoneyTransaction> fetchTransactions(const MyMoneyTransactionFilter& filter) const;
  payeeIdentifier fetchPayeeIdentifier(const QString& id) const;
  const QMap<QString, payeeIdentifier> fetchPayeeIdentifiers(const QStringList& idList = QStringList()) const;
  bool isReferencedByTransaction(const QString& id) const;

  void readPayees(const QString&);
  void readPayees(const QList<QString>& payeeList = QList<QString>());
  void readTags(const QString&);
  void readTags(const QList<QString>& tagList = QList<QString>());

  void readTransactions(const MyMoneyTransactionFilter& filter);
  void setProgressCallback(void(*callback)(int, int, const QString&));

  virtual void readFile(QIODevice* s, IMyMoneySerialize* storage) {
    Q_UNUSED(s); Q_UNUSED(storage)
  };
  virtual void writeFile(QIODevice* s, IMyMoneySerialize* storage) {
    Q_UNUSED(s); Q_UNUSED(storage)
  };

  void startCommitUnit(const QString& callingFunction);
  bool endCommitUnit(const QString& callingFunction);
  void cancelCommitUnit(const QString& callingFunction);

  long unsigned getRecCount(const QString& table) const;
  long unsigned getNextBudgetId() const;
  long unsigned getNextAccountId() const;
  long unsigned getNextInstitutionId() const;
  long unsigned getNextPayeeId() const;
  long unsigned getNextTagId() const;
  long unsigned getNextOnlineJobId() const;
  long unsigned getNextPayeeIdentifierId() const;
  long unsigned getNextReportId() const;
  long unsigned getNextScheduleId() const;
  long unsigned getNextSecurityId() const;
  long unsigned getNextTransactionId() const;
  long unsigned getNextCostCenterId() const;

  long unsigned incrementBudgetId();
  long unsigned incrementAccountId();
  long unsigned incrementInstitutionId();
  long unsigned incrementPayeeId();
  long unsigned incrementTagId();
  long unsigned incrementReportId();
  long unsigned incrementScheduleId();
  long unsigned incrementSecurityId();
  long unsigned incrementTransactionId();
  long unsigned incrementOnlineJobId();
  long unsigned incrementPayeeIdentfierId();
  long unsigned incrementCostCenterId();

  void loadAccountId(const unsigned long& id);
  void loadTransactionId(const unsigned long& id);
  void loadPayeeId(const unsigned long& id);
  void loadTagId(const unsigned long& id);
  void loadInstitutionId(const unsigned long& id);
  void loadScheduleId(const unsigned long& id);
  void loadSecurityId(const unsigned long& id);
  void loadReportId(const unsigned long& id);
  void loadBudgetId(const unsigned long& id);
  void loadOnlineJobId(const unsigned long& id);
  void loadPayeeIdentifierId(const unsigned long& id);
  void loadCostCenterId(const unsigned long& id);

  /**
    * This method allows to modify the precision with which prices
    * are handled within the object. The default of the precision is 4.
    */
  static void setPrecision(int prec);

  /**
    * This method allows to modify the start date for transaction retrieval
    * The default of the precision is Jan 1st, 1900.
    */
  static void setStartDate(const QDate &startDate);

private:
  bool fileExists(const QString& dbName);
  /** @brief a function to build a comprehensive error message for an SQL error */
  QString& buildError(const QSqlQuery& q, const QString& function, const QString& message) const;

  /** @copydoc buildError */
  QString& buildError(const QSqlQuery& q, const QString& function, const QString& message,
                      const QSqlDatabase*) const;

  /**
   * @name writeFromStorageMethods
   * @{
   * These method write all data from m_storage to the database. Data which is
   * stored in the database is deleted.
   */
  void writeUserInformation();
  void writeInstitutions();
  void writePayees();
  void writeTags();
  void writeAccounts();
  void writeTransactions();
  void writeSchedules();
  void writeSecurities();
  void writePrices();
  void writeCurrencies();
  void writeFileInfo();
  void writeReports();
  void writeBudgets();
  void writeOnlineJobs();
  /** @} */

  /**
   * @name writeMethods
   * @{
   * These methods bind the data fields of MyMoneyObjects to a given query and execute the query.
   * This is helpfull as the query has usually an update and a insert format.
   */
  void writeInstitutionList(const QList<MyMoneyInstitution>& iList, QSqlQuery& q);
  void writePayee(const MyMoneyPayee& p, QSqlQuery& q, bool isUserInfo = false);
  void writeTag(const MyMoneyTag& p, QSqlQuery& q);
  void writeAccountList(const QList<MyMoneyAccount>& accList, QSqlQuery& q);
  void writeTransaction(const QString& txId, const MyMoneyTransaction& tx, QSqlQuery& q, const QString& type);
  void writeSplits(const QString& txId, const QString& type, const QList<MyMoneySplit>& splitList);
  void writeTagSplitsList(const QString& txId, const QList<MyMoneySplit>& splitList, const QList<int>& splitIdList);
  void writeSplitList(const QString& txId, const QList<MyMoneySplit>& splitList, const QString& type, const QList<int>& splitIdList, QSqlQuery& q);
  void writeSchedule(const MyMoneySchedule& sch, QSqlQuery& q, bool insert);
  void writeSecurity(const MyMoneySecurity& security, QSqlQuery& q);
  void writePricePair(const MyMoneyPriceEntries& p);
  void writePrice(const MyMoneyPrice& p);
  void writeCurrency(const MyMoneySecurity& currency, QSqlQuery& q);
  void writeReport(const MyMoneyReport& rep, QSqlQuery& q);
  void writeBudget(const MyMoneyBudget& bud, QSqlQuery& q);
  void writeKeyValuePairs(const QString& kvpType, const QVariantList& kvpId, const QList<QMap<QString, QString> >& pairs);
  void writeOnlineJob(const onlineJob& job, QSqlQuery& query);
  void writePayeeIdentifier(const payeeIdentifier& pid, QSqlQuery& query);
  /** @} */

  /**
   * @name readMethods
   * @{
   */
  void readFileInfo();
  void readLogonData();
  void readUserInformation();
  void readInstitutions();
  void readAccounts();
  void readTransactions(const QString& tidList = QString(), const QString& dateClause = QString());
  void readSplit(MyMoneySplit& s, const QSqlQuery& q) const;
  const MyMoneyKeyValueContainer readKeyValuePairs(const QString& kvpType, const QString& kvpId) const;
  const QHash<QString, MyMoneyKeyValueContainer> readKeyValuePairs(const QString& kvpType, const QStringList& kvpIdList) const;
  void readSchedules();
  void readSecurities();
  void readPrices();
  void readCurrencies();
  void readReports();
  void readBudgets();
  /** @} */

  template<long unsigned MyMoneyStorageSql::* cache>
  long unsigned int getNextId(const QString& table, const QString& id, const int prefixLength) const;

  void deleteTransaction(const QString& id);
  void deleteTagSplitsList(const QString& txId, const QList<int>& splitIdList);
  void deleteSchedule(const QString& id);
  void deleteKeyValuePairs(const QString& kvpType, const QVariantList& kvpId);
  long unsigned calcHighId(const long unsigned&, const QString&);

  void setVersion(const QString& version);

  void signalProgress(int current, int total, const QString& = "") const;
  void (*m_progressCallback)(int, int, const QString&);

  //void startCommitUnit (const QString& callingFunction);
  //void endCommitUnit (const QString& callingFunction);
  //void cancelCommitUnit (const QString& callingFunction);
  int splitState(const MyMoneyTransactionFilter::stateOptionE& state) const;

  inline const QDate getDate(const QString& date) const {
    return (date.isNull() ? QDate() : QDate::fromString(date, Qt::ISODate));
  }

  inline const QDateTime getDateTime(const QString& date) const {
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
  bool createDatabase(const QUrl &url);
  int upgradeDb();
  int upgradeToV1();
  int upgradeToV2();
  int upgradeToV3();
  int upgradeToV4();
  int upgradeToV5();
  int upgradeToV6();
  int upgradeToV7();
  int upgradeToV8();
  int upgradeToV9();
  int upgradeToV10();
  int upgradeToV11();

  int createTables();
  void createTable(const MyMoneyDbTable& t, int version = std::numeric_limits<int>::max());
  bool alterTable(const MyMoneyDbTable& t, int fromVersion);
  void clean();
  int isEmpty();
  // for bug 252841
  const QStringList tables(QSql::TableType tt) {
    return (m_driver->tables(tt, static_cast<const QSqlDatabase&>(*this)));
  };

  /**
   * @brief Ensure the storagePlugin with iid was setup
   *
   * @throws MyMoneyException in case of an error which makes the use
   * of the plugin unavailable.
   */
  bool setupStoragePlugin(QString iid);

  void insertStorableObject(const databaseStoreableObject& obj, const QString& id);
  void updateStorableObject(const databaseStoreableObject& obj, const QString& id);
  void deleteStorableObject(const databaseStoreableObject& obj, const QString& id);

  // data
  QExplicitlySharedDataPointer<MyMoneyDbDriver> m_driver;

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
  long unsigned m_tags;
  long unsigned m_transactions;
  long unsigned m_splits;
  long unsigned m_securities;
  long unsigned m_prices;
  long unsigned m_currencies;
  long unsigned m_schedules;
  long unsigned m_reports;
  long unsigned m_kvps;
  long unsigned m_budgets;
  long unsigned m_onlineJobs;
  long unsigned m_payeeIdentifier;

  // Cache for next id to use
  // value 0 means data is not available and has to be loaded from the database
  long unsigned m_hiIdInstitutions;
  long unsigned m_hiIdPayees;
  long unsigned m_hiIdTags;
  long unsigned m_hiIdAccounts;
  long unsigned m_hiIdTransactions;
  long unsigned m_hiIdSchedules;
  long unsigned m_hiIdSecurities;
  long unsigned m_hiIdReports;
  long unsigned m_hiIdBudgets;
  long unsigned m_hiIdOnlineJobs;
  long unsigned m_hiIdPayeeIdentifier;
  long unsigned m_hiIdCostCenter;

  // encrypt option - usage TBD
  QString m_encryptData;

  /**
    * This variable is used to suppress status messages except during
   * initial data load and final write

  */
  bool m_displayStatus;

  void alert(QString s) const {
    qDebug() << s;
  }; // FIXME: remove...
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
  QHash<QString, unsigned long> m_transactionCountMap;
  /**
    * These member variables hold the user name and date/time of logon
    */
  QString m_logonUser;
  QDateTime m_logonAt;
  QDate m_txPostDate; // FIXME: remove when Tom puts date into split object

  //Disable copying
  MyMoneyStorageSql(const MyMoneyStorageSql& rhs);
  MyMoneyStorageSql& operator= (const MyMoneyStorageSql& rhs);
  bool m_newDatabase;

  /**
    * This member keeps the current precision to be used fro prices.
    * @sa setPrecision()
    */
  static int m_precision;

  /**
    * This member keeps the current start date used for transaction retrieval.
    * @sa setStartDate()
    */
  static QDate m_startDate;

  /**
    *
    */
  QSet<QString> m_loadedStoragePlugins;
};
#endif // MYMONEYSTORAGESQL_H
