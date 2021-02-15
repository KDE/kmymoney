/*
    SPDX-FileCopyrightText: 2005 Tony Bloomfield <tonybloom@users.sourceforge.net>
    Copyright (C)      Fernando Vilas <fvilas@iname.com>
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSTORAGESQL_H
#define MYMONEYSTORAGESQL_H

#include <QSqlDatabase>
#include <QSharedData>

#include "imymoneystorageformat.h"
#include "mymoneyunittestable.h"

// This is a convenience functor to make it easier to use STL algorithms
// It will return false if the MyMoneyTransaction DOES match the filter.
// This functor may disappear when all filtering can be handled in SQL.

class QUrl;
class QDate;
class QIODevice;

class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneyCostCenter;
class MyMoneyMoney;
class MyMoneySchedule;
class MyMoneyPayee;
class MyMoneyTag;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneyTransactionFilter;
class MyMoneyBudget;
class MyMoneyReport;
class MyMoneyPrice;
class payeeIdentifier;
class onlineJob;
class MyMoneyStorageSql;
class MyMoneyStorageMgr;

template <class T1, class T2> struct QPair;
template <class Key, class Value> class QMap;

typedef QPair<QString, QString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;

namespace eMyMoney { namespace TransactionFilter { enum class State; } }

/**
  * The MyMoneyDbColumn class is a base type for generic db columns.
  * Derived types exist for several common column types.
  *
  * @todo Remove unneeded columns which store the row count of tables from kmmFileInfo
  */

class MyMoneyStorageSqlPrivate;
class MyMoneyStorageSql : public IMyMoneyOperationsFormat, public QSqlDatabase, public QSharedData
{
  Q_DISABLE_COPY(MyMoneyStorageSql)
  friend class MyMoneyDbDef;
  KMM_MYMONEY_UNIT_TESTABLE

public:
  explicit MyMoneyStorageSql(MyMoneyStorageMgr *storage, const QUrl&);
  ~MyMoneyStorageSql() override;

  uint currentVersion() const;

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
  QString lastError() const;

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

  ulong transactionCount(const QString& aid) const;
  QHash<QString, ulong> transactionCountMap() const;

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
  QMap<QString, MyMoneyAccount> fetchAccounts(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyAccount> fetchAccounts() const;

  QMap<QString, MyMoneyMoney> fetchBalance(const QStringList& id, const QDate& date) const;

  QMap<QString, MyMoneyBudget> fetchBudgets(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyBudget> fetchBudgets() const;

  QMap<QString, MyMoneySecurity> fetchCurrencies(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneySecurity> fetchCurrencies() const;

  QMap<QString, MyMoneyInstitution> fetchInstitutions(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyInstitution> fetchInstitutions() const;

  QMap<QString, MyMoneyPayee> fetchPayees(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyPayee> fetchPayees() const;

  QMap<QString, MyMoneyTag> fetchTags(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyTag> fetchTags() const;

  QMap<QString, onlineJob> fetchOnlineJobs(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, onlineJob> fetchOnlineJobs() const;

  QMap<QString, MyMoneyCostCenter> fetchCostCenters(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyCostCenter> fetchCostCenters() const;

  MyMoneyPriceList fetchPrices(const QStringList& fromIdList, const QStringList& toIdList, bool forUpdate = false) const;
  MyMoneyPriceList fetchPrices() const;

  MyMoneyPrice fetchSinglePrice(const QString& fromId, const QString& toId, const QDate& date_, bool exactDate, bool = false) const;

  QMap<QString, MyMoneyReport> fetchReports(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneyReport> fetchReports() const;

  QMap<QString, MyMoneySchedule> fetchSchedules(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneySchedule> fetchSchedules() const;

  QMap<QString, MyMoneySecurity> fetchSecurities(const QStringList& idList, bool forUpdate = false) const;
  QMap<QString, MyMoneySecurity> fetchSecurities() const;

  QMap<QString, MyMoneyTransaction> fetchTransactions(const QString& tidList, const QString& dateClause, bool forUpdate = false) const;
  QMap<QString, MyMoneyTransaction> fetchTransactions(const QString& tidList) const;
  QMap<QString, MyMoneyTransaction> fetchTransactions() const;

  QMap<QString, MyMoneyTransaction> fetchTransactions(const MyMoneyTransactionFilter& filter) const;
  payeeIdentifier fetchPayeeIdentifier(const QString& id) const;

  QMap<QString, payeeIdentifier> fetchPayeeIdentifiers(const QStringList& idList) const;
  QMap<QString, payeeIdentifier> fetchPayeeIdentifiers() const;

  bool isReferencedByTransaction(const QString& id) const;

  void readPayees(const QString&);
  void readPayees(const QList<QString>& payeeList);
  void readPayees();

  void readTags(const QString&);
  void readTags(const QList<QString>& tagList);
  void readTags();

  void readTransactions(const MyMoneyTransactionFilter& filter);
  void setProgressCallback(void(*callback)(int, int, const QString&)) override;

  void readFile(QIODevice* s, MyMoneyStorageMgr* storage) override;
  void writeFile(QIODevice* s, MyMoneyStorageMgr* storage) override;

  void startCommitUnit(const QString& callingFunction);
  bool endCommitUnit(const QString& callingFunction);
  void cancelCommitUnit(const QString& callingFunction);

  ulong getRecCount(const QString& table) const;
  ulong getNextBudgetId() const;
  ulong getNextAccountId() const;
  ulong getNextInstitutionId() const;
  ulong getNextPayeeId() const;
  ulong getNextTagId() const;
  ulong getNextOnlineJobId() const;
  ulong getNextPayeeIdentifierId() const;
  ulong getNextReportId() const;
  ulong getNextScheduleId() const;
  ulong getNextSecurityId() const;
  ulong getNextTransactionId() const;
  ulong getNextCostCenterId() const;

  ulong incrementBudgetId();
  ulong incrementAccountId();
  ulong incrementInstitutionId();
  ulong incrementPayeeId();
  ulong incrementTagId();
  ulong incrementReportId();
  ulong incrementScheduleId();
  ulong incrementSecurityId();
  ulong incrementTransactionId();
  ulong incrementOnlineJobId();
  ulong incrementPayeeIdentfierId();
  ulong incrementCostCenterId();

  void loadAccountId(ulong id);
  void loadTransactionId(ulong id);
  void loadPayeeId(ulong id);
  void loadTagId(ulong id);
  void loadInstitutionId(ulong id);
  void loadScheduleId(ulong id);
  void loadSecurityId(ulong id);
  void loadReportId(ulong id);
  void loadBudgetId(ulong id);
  void loadOnlineJobId(ulong id);
  void loadPayeeIdentifierId(ulong id);
  void loadCostCenterId(ulong id);

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
  MyMoneyStorageSqlPrivate* const d_ptr;
  Q_DECLARE_PRIVATE(MyMoneyStorageSql)
};
#endif // MYMONEYSTORAGESQL_H
