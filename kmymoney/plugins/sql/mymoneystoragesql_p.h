/***************************************************************************
                          mymoneystoragesql.cpp
                          ---------------------
    begin                : 11 November 2005
    copyright            : (C) 2005 by Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
                         : Fernando Vilas <fvilas@iname.com>
                         : Christian Dávid <christian-david@web.de>
                         (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYSTORAGESQL_P_H
#define MYMONEYSTORAGESQL_P_H

#include "mymoneystoragesql.h"

// ----------------------------------------------------------------------------
// System Includes
#include <algorithm>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDateTime>
#include <QStringList>
#include <QIODevice>
#include <QUrlQuery>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QSqlRecord>
#include <QMap>
#include <QFile>
#include <QVariant>
#include <QColor>
#include <QDebug>
#include <QStack>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KServiceTypeTrader>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragemgr.h"
#include "kmymoneystorageplugin.h"
#include "onlinejobadministration.h"
#include "payeeidentifier/payeeidentifierloader.h"
#include "onlinetasks/interfaces/tasks/onlinetask.h"
#include "mymoneycostcenter.h"
#include "mymoneyexception.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneybudget.h"
#include "mymoneyreport.h"
#include "mymoneyprice.h"
#include "mymoneyutils.h"
#include "mymoneydbdef.h"
#include "mymoneydbdriver.h"
#include "payeeidentifierdata.h"
#include "payeeidentifier.h"
#include "payeeidentifiertyped.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "onlinetasks/sepa/sepaonlinetransferimpl.h"
#include "mymoneyenums.h"
#include "mymoneystoragenames.h"

using namespace eMyMoney;
using namespace MyMoneyStandardAccounts;

class FilterFail
{
public:
  explicit FilterFail(const MyMoneyTransactionFilter& filter) : m_filter(filter) {}

  inline bool operator()(const QPair<QString, MyMoneyTransaction>& transactionPair) {
    return (*this)(transactionPair.second);
  }

  inline bool operator()(const MyMoneyTransaction& transaction) {
    return !m_filter.match(transaction);
  }

private:
  MyMoneyTransactionFilter m_filter;
};

//*****************************************************************************
// Create a class to handle db transactions using scope
//
// Don't let the database object get destroyed while this object exists,
// that would result in undefined behavior.
class MyMoneyDbTransaction
{
public:
  explicit MyMoneyDbTransaction(MyMoneyStorageSql& db, const QString& name) :
    m_db(db), m_name(name)
  {
    db.startCommitUnit(name);
  }

  ~MyMoneyDbTransaction()
  {
    if (std::uncaught_exception()) {
      m_db.cancelCommitUnit(m_name);
    } else {
      try{
        m_db.endCommitUnit(m_name);
      } catch (const MyMoneyException &) {
        try {
          m_db.cancelCommitUnit(m_name);
        } catch (const MyMoneyException &e) {
          qDebug() << e.what();
        }
      }
    }
  }
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
  explicit MyMoneySqlQuery(MyMoneyStorageSql* db = 0) :
    QSqlQuery(*db)
  {
  }

  virtual ~MyMoneySqlQuery()
  {
  }

  bool exec()
  {
    qDebug() << "start sql:" << lastQuery();
    bool rc = QSqlQuery::exec();
    qDebug() << "end sql:" << QSqlQuery::executedQuery();
    qDebug() << "***Query returned:" << rc << ", row count:" << numRowsAffected();
    return (rc);
  }

  bool exec(const QString & query)
  {
    qDebug() << "start sql:" << query;
    bool rc = QSqlQuery::exec(query);
    qDebug() << "end sql:" << QSqlQuery::executedQuery();
    qDebug() << "***Query returned:" << rc << ", row count:" << numRowsAffected();
    return rc;
  }

  bool prepare(const QString & query)
  {
    return (QSqlQuery::prepare(query));
  }
};

#define GETSTRING(a) query.value(a).toString()
#define GETDATE(a) getDate(GETSTRING(a))
#define GETDATE_D(a) d->getDate(GETSTRING(a))
#define GETDATETIME(a) getDateTime(GETSTRING(a))
#define GETINT(a) query.value(a).toInt()
#define GETULL(a) query.value(a).toULongLong()
#define MYMONEYEXCEPTIONSQL(exceptionMessage) MYMONEYEXCEPTION(buildError(query, Q_FUNC_INFO, exceptionMessage))
#define MYMONEYEXCEPTIONSQL_D(exceptionMessage) MYMONEYEXCEPTION(d->buildError(query, Q_FUNC_INFO, exceptionMessage))

class MyMoneyStorageSqlPrivate
{
  Q_DISABLE_COPY(MyMoneyStorageSqlPrivate)
  Q_DECLARE_PUBLIC(MyMoneyStorageSql)

public:
  explicit MyMoneyStorageSqlPrivate(MyMoneyStorageSql* qq) :
    q_ptr(qq),
    m_dbVersion(0),
    m_storage(nullptr),
    m_loadAll(false),
    m_override(false),
    m_institutions(0),
    m_accounts(0),
    m_payees(0),
    m_tags(0),
    m_transactions(0),
    m_splits(0),
    m_securities(0),
    m_prices(0),
    m_currencies(0),
    m_schedules(0),
    m_reports(0),
    m_kvps(0),
    m_budgets(0),
    m_onlineJobs(0),
    m_payeeIdentifier(0),
    m_hiIdInstitutions(0),
    m_hiIdPayees(0),
    m_hiIdTags(0),
    m_hiIdAccounts(0),
    m_hiIdTransactions(0),
    m_hiIdSchedules(0),
    m_hiIdSecurities(0),
    m_hiIdReports(0),
    m_hiIdBudgets(0),
    m_hiIdOnlineJobs(0),
    m_hiIdPayeeIdentifier(0),
    m_hiIdCostCenter(0),
    m_displayStatus(false),
    m_readingPrices(false),
    m_newDatabase(false),
    m_progressCallback(nullptr)
  {
    m_preferred.setReportAllSplits(false);
  }

  ~MyMoneyStorageSqlPrivate()
  {
  }

  enum class SQLAction {
    Save,
    Modify,
    Remove
  };

  /**
   * MyMoneyStorageSql get highest ID number from the database
   *
   * @return : highest ID number
   */
  ulong highestNumberFromIdString(QString tableName, QString tableField, int prefixLength)
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);

    if (!query.exec(m_driver->highestNumberFromIdString(tableName, tableField, prefixLength)) || !query.next())
      throw MYMONEYEXCEPTIONSQL("retrieving highest ID number");

    return query.value(0).toULongLong();
  }

  /**
   * @name writeFromStorageMethods
   * @{
   * These method write all data from m_storage to the database. Data which is
   * stored in the database is deleted.
   */
  void writeUserInformation();

  void writeInstitutions()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database
    // anything not in the list needs to be inserted
    // anything which is will be updated and removed from the list
    // anything left over at the end will need to be deleted
    // this is an expensive and inconvenient way to do things; find a better way
    // one way would be to build the lists when reading the db
    // unfortunately this object does not persist between read and write
    // it would also be nice if we could tell which objects had been updated since we read them in
    QList<QString> dbList;
    QSqlQuery query(*q);
    query.prepare("SELECT id FROM kmmInstitutions;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Institution list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    const QList<MyMoneyInstitution> list = m_storage->institutionList();
    QList<MyMoneyInstitution> insertList;
    QList<MyMoneyInstitution> updateList;
    QSqlQuery query2(*q);
    query.prepare(m_db.m_tables["kmmInstitutions"].updateString());
    query2.prepare(m_db.m_tables["kmmInstitutions"].insertString());
    signalProgress(0, list.count(), "Writing Institutions...");
    foreach (const MyMoneyInstitution& i, list) {
      if (dbList.contains(i.id())) {
        dbList.removeAll(i.id());
        updateList << i;
      } else {
        insertList << i;
      }
      signalProgress(++m_institutions, 0);
    }
    if (!insertList.isEmpty())
      writeInstitutionList(insertList, query2);

    if (!updateList.isEmpty())
      writeInstitutionList(updateList, query);

    if (!dbList.isEmpty()) {
      QVariantList deleteList;
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (const QString& it, dbList) {
        deleteList << it;
      }
      query.prepare("DELETE FROM kmmInstitutions WHERE id = :id");
      query.bindValue(":id", deleteList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Institution");

      deleteKeyValuePairs("OFXSETTINGS", deleteList);
    }
  }

  void writePayees()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)

    QSqlQuery query(*q);
    query.prepare("SELECT id FROM kmmPayees;");
    if (!query.exec())
      throw MYMONEYEXCEPTIONSQL("building Payee list"); // krazy:exclude=crashy

    QList<QString> dbList;
    dbList.reserve(query.numRowsAffected());
    while (query.next())
      dbList.append(query.value(0).toString());

    QList<MyMoneyPayee> list = m_storage->payeeList();
    MyMoneyPayee user(QString("USER"), m_storage->user());
    list.prepend(user);
    signalProgress(0, list.count(), "Writing Payees...");

    Q_FOREACH(const MyMoneyPayee& it, list) {
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        q->modifyPayee(it);
      } else {
        q->addPayee(it);
      }
      signalProgress(++m_payees, 0);
    }

    if (!dbList.isEmpty()) {
      QMap<QString, MyMoneyPayee> payeesToDelete = q->fetchPayees(dbList, true);
      Q_FOREACH(const MyMoneyPayee& payee, payeesToDelete) {
        q->removePayee(payee);
      }
    }
  }

  void writeTags()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    query.prepare("SELECT id FROM kmmTags;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Tag list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    QList<MyMoneyTag> list = m_storage->tagList();
    signalProgress(0, list.count(), "Writing Tags...");
    QSqlQuery query2(*q);
    query.prepare(m_db.m_tables["kmmTags"].updateString());
    query2.prepare(m_db.m_tables["kmmTags"].insertString());
    foreach (const MyMoneyTag& it, list) {
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        writeTag(it, query);
      } else {
        writeTag(it, query2);
      }
      signalProgress(++m_tags, 0);
    }

    if (!dbList.isEmpty()) {
      QVariantList deleteList;
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (const QString& it, dbList) {
        deleteList << it;
      }
      query.prepare(m_db.m_tables["kmmTags"].deleteString());
      query.bindValue(":id", deleteList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Tag");
      m_tags -= query.numRowsAffected();
    }
  }

  void writeAccounts()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    query.prepare("SELECT id FROM kmmAccounts;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Account list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    QList<MyMoneyAccount> list;
    m_storage->accountList(list);
    unsigned progress = 0;
    signalProgress(0, list.count(), "Writing Accounts...");
    if (dbList.isEmpty()) { // new table, insert standard accounts
      query.prepare(m_db.m_tables["kmmAccounts"].insertString());
    } else {
      query.prepare(m_db.m_tables["kmmAccounts"].updateString());
    }
    // Attempt to write the standard accounts. For an empty db, this will fail.
    try {
      QList<MyMoneyAccount> stdList;
      stdList << m_storage->asset();
      stdList << m_storage->liability();
      stdList << m_storage->expense();
      stdList << m_storage->income();
      stdList << m_storage->equity();
      writeAccountList(stdList, query);
      m_accounts += stdList.size();
    } catch (const MyMoneyException &) {
      // If the above failed, assume that the database is empty and create
      // the standard accounts by hand before writing them.
      MyMoneyAccount acc_l;
      acc_l.setAccountType(Account::Type::Liability);
      acc_l.setName("Liability");
      MyMoneyAccount liability(stdAccNames[stdAccLiability], acc_l);

      MyMoneyAccount acc_a;
      acc_a.setAccountType(Account::Type::Asset);
      acc_a.setName("Asset");
      MyMoneyAccount asset(stdAccNames[stdAccAsset], acc_a);

      MyMoneyAccount acc_e;
      acc_e.setAccountType(Account::Type::Expense);
      acc_e.setName("Expense");
      MyMoneyAccount expense(stdAccNames[stdAccExpense], acc_e);

      MyMoneyAccount acc_i;
      acc_i.setAccountType(Account::Type::Income);
      acc_i.setName("Income");
      MyMoneyAccount income(stdAccNames[stdAccIncome], acc_i);

      MyMoneyAccount acc_q;
      acc_q.setAccountType(Account::Type::Equity);
      acc_q.setName("Equity");
      MyMoneyAccount equity(stdAccNames[stdAccEquity], acc_q);

      QList<MyMoneyAccount> stdList;
      stdList << asset;
      stdList << liability;
      stdList << expense;
      stdList << income;
      stdList << equity;
      writeAccountList(stdList, query);
      m_accounts += stdList.size();
    }

    QSqlQuery query2(*q);
    query.prepare(m_db.m_tables["kmmAccounts"].updateString());
    query2.prepare(m_db.m_tables["kmmAccounts"].insertString());
    QList<MyMoneyAccount> updateList;
    QList<MyMoneyAccount> insertList;
    // Update the accounts that exist; insert the ones that do not.
    foreach (const MyMoneyAccount& it, list) {
      m_transactionCountMap[it.id()] = m_storage->transactionCount(it.id());
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        updateList << it;
      } else {
        insertList << it;
      }
      signalProgress(++progress, 0);
      ++m_accounts;
    }

    writeAccountList(updateList, query);
    writeAccountList(insertList, query2);

    // Delete the accounts that are in the db but no longer in memory.
    if (!dbList.isEmpty()) {
      QVariantList kvpList;

      query.prepare("DELETE FROM kmmAccounts WHERE id = :id");
      foreach (const QString& it, dbList) {
        if (!m_storage->isStandardAccount(it)) {
          kvpList << it;
        }
      }
      query.bindValue(":id", kvpList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Account");

      deleteKeyValuePairs("ACCOUNT", kvpList);
      deleteKeyValuePairs("ONLINEBANKING", kvpList);
    }
  }

  void writeTransactions()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    query.prepare("SELECT id FROM kmmTransactions WHERE txType = 'N';");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Transaction list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    MyMoneyTransactionFilter filter;
    filter.setReportAllSplits(false);
    QList<MyMoneyTransaction> list;
    m_storage->transactionList(list, filter);
    signalProgress(0, list.count(), "Writing Transactions...");
    QSqlQuery q2(*q);
    query.prepare(m_db.m_tables["kmmTransactions"].updateString());
    q2.prepare(m_db.m_tables["kmmTransactions"].insertString());
    foreach (const MyMoneyTransaction& it, list) {
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        writeTransaction(it.id(), it, query, "N");
      } else {
        writeTransaction(it.id(), it, q2, "N");
      }
      signalProgress(++m_transactions, 0);
    }

    if (!dbList.isEmpty()) {
      foreach (const QString& it, dbList) {
        deleteTransaction(it);
      }
    }
  }

  void writeSchedules()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    query.prepare("SELECT id FROM kmmSchedules;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Schedule list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    const auto list = m_storage->scheduleList(QString(), Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                                              QDate(), QDate(), false);
    QSqlQuery query2(*q);
    //TODO: find a way to prepare the queries outside of the loop.  writeSchedule()
    // modifies the query passed to it, so they have to be re-prepared every pass.
    signalProgress(0, list.count(), "Writing Schedules...");
    foreach (const MyMoneySchedule& it, list) {
      query.prepare(m_db.m_tables["kmmSchedules"].updateString());
      query2.prepare(m_db.m_tables["kmmSchedules"].insertString());
      bool insert = true;
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        insert = false;
        writeSchedule(it, query, insert);
      } else {
        writeSchedule(it, query2, insert);
      }
      signalProgress(++m_schedules, 0);
    }

    if (!dbList.isEmpty()) {
      foreach (const QString& it, dbList) {
        deleteSchedule(it);
      }
    }
  }

  void writeSecurities()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    QSqlQuery query2(*q);
    query.prepare("SELECT id FROM kmmSecurities;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building security list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    const QList<MyMoneySecurity> securityList = m_storage->securityList();
    signalProgress(0, securityList.count(), "Writing Securities...");
    query.prepare(m_db.m_tables["kmmSecurities"].updateString());
    query2.prepare(m_db.m_tables["kmmSecurities"].insertString());
    foreach (const MyMoneySecurity& it, securityList) {
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        writeSecurity(it, query);
      } else {
        writeSecurity(it, query2);
      }
      signalProgress(++m_securities, 0);
    }

    if (!dbList.isEmpty()) {
      QVariantList idList;
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (const QString& it, dbList) {
        idList << it;
      }

      query.prepare("DELETE FROM kmmSecurities WHERE id = :id");
      query2.prepare("DELETE FROM kmmPrices WHERE fromId = :id OR toId = :id");
      query.bindValue(":id", idList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Security");

      query2.bindValue(":fromId", idList);
      query2.bindValue(":toId", idList);
      if (!query2.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Security");

      deleteKeyValuePairs("SECURITY", idList);
    }
  }

  void writePrices()
  {
    Q_Q(MyMoneyStorageSql);
    // due to difficulties in matching and determining deletes
    // easiest way is to delete all and re-insert
    QSqlQuery query(*q);
    query.prepare("DELETE FROM kmmPrices");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("deleting Prices"); // krazy:exclude=crashy
    m_prices = 0;

    const MyMoneyPriceList list = m_storage->priceList();
    signalProgress(0, list.count(), "Writing Prices...");
    MyMoneyPriceList::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it)   {
      writePricePair(*it);
    }
  }

  void writeCurrencies()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    QSqlQuery query2(*q);
    query.prepare("SELECT ISOCode FROM kmmCurrencies;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Currency list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    const QList<MyMoneySecurity> currencyList = m_storage->currencyList();
    signalProgress(0, currencyList.count(), "Writing Currencies...");
    query.prepare(m_db.m_tables["kmmCurrencies"].updateString());
    query2.prepare(m_db.m_tables["kmmCurrencies"].insertString());
    foreach (const MyMoneySecurity& it, currencyList) {
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        writeCurrency(it, query);
      } else {
        writeCurrency(it, query2);
      }
      signalProgress(++m_currencies, 0);
    }

    if (!dbList.isEmpty()) {
      QVariantList isoCodeList;
      query.prepare("DELETE FROM kmmCurrencies WHERE ISOCode = :ISOCode");
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (const QString& it, dbList) {
        isoCodeList << it;
      }

      query.bindValue(":ISOCode", isoCodeList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Currency");
    }
  }

  void writeFileInfo()
  {
    Q_Q(MyMoneyStorageSql);
    // we have no real way of knowing when these change, so re-write them every time
    QVariantList kvpList;
    kvpList << "";
    QList<QMap<QString, QString> > pairs;
    pairs << m_storage->pairs();
    deleteKeyValuePairs("STORAGE", kvpList);
    writeKeyValuePairs("STORAGE", kvpList, pairs);

    QSqlQuery query(*q);
    query.prepare("SELECT count(*) FROM kmmFileInfo;");
    if (!query.exec() || !query.next())
      throw MYMONEYEXCEPTIONSQL("checking fileinfo"); // krazy:exclude=crashy

    if (query.value(0).toInt() == 0) {
      // Cannot use "INSERT INTO kmmFileInfo DEFAULT VALUES;" because it is not supported by MySQL
      query.prepare(QLatin1String("INSERT INTO kmmFileInfo (version) VALUES (null);"));
      if (!query.exec()) throw MYMONEYEXCEPTIONSQL("inserting fileinfo"); // krazy:exclude=crashy
    }

    query.prepare(QLatin1String(
      "UPDATE kmmFileInfo SET "
        "version = :version, "
        "fixLevel = :fixLevel, "
        "created = :created, "
        "lastModified = :lastModified, "
        "baseCurrency = :baseCurrency, "
        "dateRangeStart = :dateRangeStart, "
        "dateRangeEnd = :dateRangeEnd, "
        "hiInstitutionId = :hiInstitutionId, "
        "hiPayeeId = :hiPayeeId, "
        "hiTagId = :hiTagId, "
        "hiAccountId = :hiAccountId, "
        "hiTransactionId = :hiTransactionId, "
        "hiScheduleId = :hiScheduleId, "
        "hiSecurityId = :hiSecurityId, "
        "hiReportId = :hiReportId, "
        "hiBudgetId = :hiBudgetId, "
        "hiOnlineJobId = :hiOnlineJobId, "
        "hiPayeeIdentifierId = :hiPayeeIdentifierId, "
        "encryptData = :encryptData, "
        "updateInProgress = :updateInProgress, "
        "logonUser = :logonUser, "
        "logonAt = :logonAt, "
        //! @todo The following updates are for backwards compatibility only
        //! remove backwards compatibility in a later version
        "institutions = :institutions, "
        "accounts = :accounts, "
        "payees = :payees, "
        "tags = :tags, "
        "transactions = :transactions, "
        "splits = :splits, "
        "securities = :securities, "
        "prices = :prices, "
        "currencies = :currencies, "
        "schedules = :schedules, "
        "reports = :reports, "
        "kvps = :kvps, "
        "budgets = :budgets; "
      )
    );

    query.bindValue(":version", m_dbVersion);
    query.bindValue(":fixLevel", m_storage->fileFixVersion());
    query.bindValue(":created", m_storage->creationDate().toString(Qt::ISODate));
    //q.bindValue(":lastModified", m_storage->lastModificationDate().toString(Qt::ISODate));
    query.bindValue(":lastModified", QDate::currentDate().toString(Qt::ISODate));
    query.bindValue(":baseCurrency", m_storage->pairs()["kmm-baseCurrency"]);
    query.bindValue(":dateRangeStart", QDate());
    query.bindValue(":dateRangeEnd", QDate());

    //FIXME: This modifies all m_<variable> used in this function.
    // Sometimes the memory has been updated.

    // Should most of these be tracked in a view?
    // Variables actually needed are: version, fileFixVersion, creationDate,
    // baseCurrency, encryption, update info, and logon info.
    //try {
    //readFileInfo();
    //} catch (...) {
    //q->startCommitUnit(Q_FUNC_INFO);
    //}

    //! @todo The following bindings are for backwards compatibility only
    //! remove backwards compatibility in a later version
    query.bindValue(":hiInstitutionId", QVariant::fromValue(q->getNextInstitutionId()));
    query.bindValue(":hiPayeeId", QVariant::fromValue(q->getNextPayeeId()));
    query.bindValue(":hiTagId", QVariant::fromValue(q->getNextTagId()));
    query.bindValue(":hiAccountId", QVariant::fromValue(q->getNextAccountId()));
    query.bindValue(":hiTransactionId", QVariant::fromValue(q->getNextTransactionId()));
    query.bindValue(":hiScheduleId", QVariant::fromValue(q->getNextScheduleId()));
    query.bindValue(":hiSecurityId", QVariant::fromValue(q->getNextSecurityId()));
    query.bindValue(":hiReportId", QVariant::fromValue(q->getNextReportId()));
    query.bindValue(":hiBudgetId", QVariant::fromValue(q->getNextBudgetId()));
    query.bindValue(":hiOnlineJobId", QVariant::fromValue(q->getNextOnlineJobId()));
    query.bindValue(":hiPayeeIdentifierId", QVariant::fromValue(q->getNextPayeeIdentifierId()));

    query.bindValue(":encryptData", m_encryptData);
    query.bindValue(":updateInProgress", "N");
    query.bindValue(":logonUser", m_logonUser);
    query.bindValue(":logonAt", m_logonAt.toString(Qt::ISODate));

    //! @todo The following bindings are for backwards compatibility only
    //! remove backwards compatibility in a later version
    query.bindValue(":institutions", (unsigned long long) m_institutions);
    query.bindValue(":accounts", (unsigned long long) m_accounts);
    query.bindValue(":payees", (unsigned long long) m_payees);
    query.bindValue(":tags", (unsigned long long) m_tags);
    query.bindValue(":transactions", (unsigned long long) m_transactions);
    query.bindValue(":splits", (unsigned long long) m_splits);
    query.bindValue(":securities", (unsigned long long) m_securities);
    query.bindValue(":prices", (unsigned long long) m_prices);
    query.bindValue(":currencies", (unsigned long long) m_currencies);
    query.bindValue(":schedules", (unsigned long long) m_schedules);
    query.bindValue(":reports", (unsigned long long) m_reports);
    query.bindValue(":kvps", (unsigned long long) m_kvps);
    query.bindValue(":budgets", (unsigned long long) m_budgets);

    if (!query.exec())
      throw MYMONEYEXCEPTIONSQL("writing FileInfo"); // krazy:exclude=crashy
  }

  void writeReports()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    QSqlQuery query2(*q);
    query.prepare("SELECT id FROM kmmReportConfig;");
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Report list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toString());

    QList<MyMoneyReport> list = m_storage->reportList();
    signalProgress(0, list.count(), "Writing Reports...");
    query.prepare(m_db.m_tables["kmmReportConfig"].updateString());
    query2.prepare(m_db.m_tables["kmmReportConfig"].insertString());
    foreach (const MyMoneyReport& it, list) {
      if (dbList.contains(it.id())) {
        dbList.removeAll(it.id());
        writeReport(it, query);
      } else {
        writeReport(it, query2);
      }
      signalProgress(++m_reports, 0);
    }

    if (!dbList.isEmpty()) {
      QVariantList idList;
      query.prepare("DELETE FROM kmmReportConfig WHERE id = :id");
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (const QString& it, dbList) {
        idList << it;
      }

      query.bindValue(":id", idList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Report");
    }
  }

  void writeBudgets()
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<QString> dbList;
    QSqlQuery query(*q);
    QSqlQuery query2(*q);
    query.prepare("SELECT name FROM kmmBudgetConfig;");
    if (!query.exec())
      throw MYMONEYEXCEPTIONSQL("building Budget list"); // krazy:exclude=crashy
    while (query.next())
      dbList.append(query.value(0).toString());

    QList<MyMoneyBudget> list = m_storage->budgetList();
    signalProgress(0, list.count(), "Writing Budgets...");
    query.prepare(m_db.m_tables["kmmBudgetConfig"].updateString());
    query2.prepare(m_db.m_tables["kmmBudgetConfig"].insertString());
    foreach (const MyMoneyBudget& it, list) {
      if (dbList.contains(it.name())) {
        dbList.removeAll(it.name());
        writeBudget(it, query);
      } else {
        writeBudget(it, query2);
      }
      signalProgress(++m_budgets, 0);
    }

    if (!dbList.isEmpty()) {
      QVariantList idList;
      query.prepare("DELETE FROM kmmBudgetConfig WHERE id = :id");
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (const QString& it, dbList) {
        idList << it;
      }

      query.bindValue(":name", idList);
      if (!query.execBatch())
        throw MYMONEYEXCEPTIONSQL("deleting Budget");
    }
  }

  void writeOnlineJobs()
  {
    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);
    if (!query.exec("DELETE FROM kmmOnlineJobs;"))
      throw MYMONEYEXCEPTIONSQL("Clean kmmOnlineJobs table");

    const QList<onlineJob> jobs(m_storage->onlineJobList());
    signalProgress(0, jobs.count(), i18n("Inserting online jobs."));
    // Create list for onlineJobs which failed and the reason therefor
    QList<QPair<onlineJob, QString> > failedJobs;
    int jobCount = 0;
    foreach (const onlineJob& job, jobs) {
      try {
        q->addOnlineJob(job);
      } catch (const MyMoneyException &e) {
        // Do not save e as this may point to an inherited class
        failedJobs.append(QPair<onlineJob, QString>(job, e.what()));
        qDebug() << "Failed to save onlineJob" << job.id() << "Reson:" << e.what();
      }

      signalProgress(++jobCount, 0);
    }

    if (!failedJobs.isEmpty()) {
      /** @todo Improve error message */
      throw MYMONEYEXCEPTION_CSTRING("Could not save onlineJob.");
    }
  }
  /** @} */

  /**
   * @name writeMethods
   * @{
   * These methods bind the data fields of MyMoneyObjects to a given query and execute the query.
   * This is helpfull as the query has usually an update and a insert format.
   */
  void writeInstitutionList(const QList<MyMoneyInstitution>& iList, QSqlQuery& query)
  {
    QVariantList idList;
    QVariantList nameList;
    QVariantList managerList;
    QVariantList routingCodeList;
    QVariantList addressStreetList;
    QVariantList addressCityList;
    QVariantList addressZipcodeList;
    QVariantList telephoneList;
    QList<QMap<QString, QString> > kvpPairsList;

    foreach (const MyMoneyInstitution& i, iList) {
      idList << i.id();
      nameList << i.name();
      managerList << i.manager();
      routingCodeList << i.sortcode();
      addressStreetList << i.street();
      addressCityList << i.city();
      addressZipcodeList << i.postcode();
      telephoneList << i.telephone();
      kvpPairsList << i.pairs();
    }

    query.bindValue(":id", idList);
    query.bindValue(":name", nameList);
    query.bindValue(":manager", managerList);
    query.bindValue(":routingCode", routingCodeList);
    query.bindValue(":addressStreet", addressStreetList);
    query.bindValue(":addressCity", addressCityList);
    query.bindValue(":addressZipcode", addressZipcodeList);
    query.bindValue(":telephone", telephoneList);

    if (!query.execBatch())
      throw MYMONEYEXCEPTIONSQL("writing Institution");
    writeKeyValuePairs("OFXSETTINGS", idList, kvpPairsList);
    // Set m_hiIdInstitutions to 0 to force recalculation the next time it is requested
    m_hiIdInstitutions = 0;
  }

  void writePayee(const MyMoneyPayee& p, QSqlQuery& query, bool isUserInfo = false)
  {
    if (isUserInfo) {
      query.bindValue(":id", "USER");
    } else {
      query.bindValue(":id", p.id());
    }
    query.bindValue(":name", p.name());
    query.bindValue(":reference", p.reference());
    query.bindValue(":email", p.email());
    query.bindValue(":addressStreet", p.address());
    query.bindValue(":addressCity", p.city());
    query.bindValue(":addressZipcode", p.postcode());
    query.bindValue(":addressState", p.state());
    query.bindValue(":telephone", p.telephone());
    query.bindValue(":notes", p.notes());
    query.bindValue(":defaultAccountId", p.defaultAccountId());
    bool ignoreCase;
    QString matchKeys;
    auto type = p.matchData(ignoreCase, matchKeys);
    query.bindValue(":matchData", static_cast<uint>(type));

    if (ignoreCase)
      query.bindValue(":matchIgnoreCase", "Y");
    else
      query.bindValue(":matchIgnoreCase", "N");

    query.bindValue(":matchKeys", matchKeys);
    if (!query.exec()) // krazy:exclude=crashy
      throw MYMONEYEXCEPTIONSQL("writing Payee"); // krazy:exclude=crashy

    if (!isUserInfo)
      m_hiIdPayees = 0;
  }

  void writeTag(const MyMoneyTag& ta, QSqlQuery& query)
  {
    query.bindValue(":id", ta.id());
    query.bindValue(":name", ta.name());
    query.bindValue(":tagColor", ta.tagColor().name());
    if (ta.isClosed()) query.bindValue(":closed", "Y");
    else query.bindValue(":closed", "N");
    query.bindValue(":notes", ta.notes());
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Tag"); // krazy:exclude=crashy
    m_hiIdTags = 0;
  }

  void writeAccountList(const QList<MyMoneyAccount>& accList, QSqlQuery& query)
  {
    //MyMoneyMoney balance = m_storagePtr->balance(acc.id(), QDate());

    QVariantList idList;
    QVariantList institutionIdList;
    QVariantList parentIdList;
    QVariantList lastReconciledList;
    QVariantList lastModifiedList;
    QVariantList openingDateList;
    QVariantList accountNumberList;
    QVariantList accountTypeList;
    QVariantList accountTypeStringList;
    QVariantList isStockAccountList;
    QVariantList accountNameList;
    QVariantList descriptionList;
    QVariantList currencyIdList;
    QVariantList balanceList;
    QVariantList balanceFormattedList;
    QVariantList transactionCountList;

    QList<QMap<QString, QString> > pairs;
    QList<QMap<QString, QString> > onlineBankingPairs;

    foreach (const MyMoneyAccount& a, accList) {
      idList << a.id();
      institutionIdList << a.institutionId();
      parentIdList << a.parentAccountId();
      if (a.lastReconciliationDate() == QDate())
        lastReconciledList << a.lastReconciliationDate();
      else
        lastReconciledList << a.lastReconciliationDate().toString(Qt::ISODate);
      lastModifiedList << a.lastModified();
      if (a.openingDate() == QDate())
        openingDateList << a.openingDate();
      else
        openingDateList << a.openingDate().toString(Qt::ISODate);
      accountNumberList << a.number();
      accountTypeList << (int)a.accountType();
      accountTypeStringList << MyMoneyAccount::accountTypeToString(a.accountType());
      if (a.accountType() == Account::Type::Stock)
        isStockAccountList << "Y";
      else
        isStockAccountList << "N";
      accountNameList << a.name();
      descriptionList << a.description();
      currencyIdList << a.currencyId();
      // This section attempts to get the balance from the database, if possible
      // That way, the balance fields are kept in sync. If that fails, then
      // It is assumed that the account actually knows its correct balance.

      //FIXME: Using exceptions for branching always feels like a kludge.
      //       Look for a better way.
      try {
        MyMoneyMoney bal = m_storage->balance(a.id(), QDate());
        balanceList << bal.toString();
        balanceFormattedList << bal.formatMoney("", -1, false);
      } catch (const MyMoneyException &) {
        balanceList << a.balance().toString();
        balanceFormattedList << a.balance().formatMoney("", -1, false);
      }
      transactionCountList << quint64(m_transactionCountMap[a.id()]);

      //MMAccount inherits from KVPContainer AND has a KVPContainer member
      //so handle both
      pairs << a.pairs();
      onlineBankingPairs << a.onlineBankingSettings().pairs();
    }

    query.bindValue(":id", idList);
    query.bindValue(":institutionId", institutionIdList);
    query.bindValue(":parentId", parentIdList);
    query.bindValue(":lastReconciled", lastReconciledList);
    query.bindValue(":lastModified", lastModifiedList);
    query.bindValue(":openingDate", openingDateList);
    query.bindValue(":accountNumber", accountNumberList);
    query.bindValue(":accountType", accountTypeList);
    query.bindValue(":accountTypeString", accountTypeStringList);
    query.bindValue(":isStockAccount", isStockAccountList);
    query.bindValue(":accountName", accountNameList);
    query.bindValue(":description", descriptionList);
    query.bindValue(":currencyId", currencyIdList);
    query.bindValue(":balance", balanceList);
    query.bindValue(":balanceFormatted", balanceFormattedList);
    query.bindValue(":transactionCount", transactionCountList);

    if (!query.execBatch())
      throw MYMONEYEXCEPTIONSQL("writing Account");

    //Add in Key-Value Pairs for accounts.
    writeKeyValuePairs("ACCOUNT", idList, pairs);
    writeKeyValuePairs("ONLINEBANKING", idList, onlineBankingPairs);
    m_hiIdAccounts = 0;
  }

  void writeTransaction(const QString& txId, const MyMoneyTransaction& tx, QSqlQuery& query, const QString& type)
  {
    query.bindValue(":id", txId);
    query.bindValue(":txType", type);
    query.bindValue(":postDate", tx.postDate().toString(Qt::ISODate));
    query.bindValue(":memo", tx.memo());
    query.bindValue(":entryDate", tx.entryDate().toString(Qt::ISODate));
    query.bindValue(":currencyId", tx.commodity());
    query.bindValue(":bankId", tx.bankID());

    if (!query.exec()) // krazy:exclude=crashy
      throw MYMONEYEXCEPTIONSQL("writing Transaction"); // krazy:exclude=crashy

    m_txPostDate = tx.postDate(); // FIXME: TEMP till Tom puts date in split object
    QList<MyMoneySplit> splitList = tx.splits();
    writeSplits(txId, type, splitList);

    //Add in Key-Value Pairs for transactions.
    QVariantList idList;
    idList << txId;
    deleteKeyValuePairs("TRANSACTION", idList);
    QList<QMap<QString, QString> > pairs;
    pairs << tx.pairs();
    writeKeyValuePairs("TRANSACTION", idList, pairs);
    m_hiIdTransactions = 0;
  }

  void writeSplits(const QString& txId, const QString& type, const QList<MyMoneySplit>& splitList)
  {
    Q_Q(MyMoneyStorageSql);
    // first, get a list of what's on the database (see writeInstitutions)
    QList<uint> dbList;
    QList<MyMoneySplit> insertList;
    QList<MyMoneySplit> updateList;
    QList<int> insertIdList;
    QList<int> updateIdList;
    QSqlQuery query(*q);
    query.prepare("SELECT splitId FROM kmmSplits where transactionId = :id;");
    query.bindValue(":id", txId);
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("building Split list"); // krazy:exclude=crashy
    while (query.next()) dbList.append(query.value(0).toUInt());

    QSqlQuery query2(*q);
    query.prepare(m_db.m_tables["kmmSplits"].updateString());
    query2.prepare(m_db.m_tables["kmmSplits"].insertString());
    auto i = 0;
    for (auto it = splitList.constBegin(); it != splitList.constEnd(); ++it) {
      if (dbList.contains(i)) {
        dbList.removeAll(i);
        updateList << *it;
        updateIdList << i;
      } else {
        ++m_splits;
        insertList << *it;
        insertIdList << i;
      }
      ++i;
    }

    if (!insertList.isEmpty()) {
      writeSplitList(txId, insertList, type, insertIdList, query2);
      writeTagSplitsList(txId, insertList, insertIdList);
    }

    if (!updateList.isEmpty()) {
      writeSplitList(txId, updateList, type, updateIdList, query);
      deleteTagSplitsList(txId, updateIdList);
      writeTagSplitsList(txId, updateList, updateIdList);
    }

    if (!dbList.isEmpty()) {
      QVector<QVariant> txIdList(dbList.count(), txId);
      QVariantList splitIdList;
      query.prepare("DELETE FROM kmmSplits WHERE transactionId = :txId AND splitId = :splitId");
      // qCopy segfaults here, so do it with a hand-rolled loop
      foreach (int it, dbList) {
        splitIdList << it;
      }
      query.bindValue(":txId", txIdList.toList());
      query.bindValue(":splitId", splitIdList);
      if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Splits");
    }
  }

  void writeTagSplitsList
  (const QString& txId,
   const QList<MyMoneySplit>& splitList,
   const QList<int>& splitIdList)
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QVariantList tagIdList;
    QVariantList txIdList;
    QVariantList splitIdList_TagSplits;
    QVariantList tagSplitsIdList;

    int i = 0, l = 0;
    foreach (const MyMoneySplit& s, splitList) {
      for (l = 0; l < s.tagIdList().size(); ++l) {
        tagIdList << s.tagIdList()[l];
        splitIdList_TagSplits << splitIdList[i];
        txIdList << txId;
      }
      i++;
    }
    QSqlQuery query(*q);
    query.prepare(m_db.m_tables["kmmTagSplits"].insertString());
    query.bindValue(":tagId", tagIdList);
    query.bindValue(":splitId", splitIdList_TagSplits);
    query.bindValue(":transactionId", txIdList);
    if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("writing tagSplits");
  }

  void writeSplitList
  (const QString& txId,
   const QList<MyMoneySplit>& splitList,
   const QString& type,
   const QList<int>& splitIdList,
   QSqlQuery& query)
  {
    QVariantList txIdList;
    QVariantList typeList;
    QVariantList payeeIdList;
    QVariantList reconcileDateList;
    QVariantList actionList;
    QVariantList reconcileFlagList;
    QVariantList valueList;
    QVariantList valueFormattedList;
    QVariantList sharesList;
    QVariantList sharesFormattedList;
    QVariantList priceList;
    QVariantList priceFormattedList;
    QVariantList memoList;
    QVariantList accountIdList;
    QVariantList costCenterIdList;
    QVariantList checkNumberList;
    QVariantList postDateList;
    QVariantList bankIdList;
    QVariantList kvpIdList;
    QList<QMap<QString, QString> > kvpPairsList;

    int i = 0;
    foreach (const MyMoneySplit& s, splitList) {
      txIdList << txId;
      typeList << type;
      payeeIdList << s.payeeId();
      if (s.reconcileDate() == QDate())
        reconcileDateList << s.reconcileDate();
      else
        reconcileDateList << s.reconcileDate().toString(Qt::ISODate);
      actionList << s.action();
      reconcileFlagList << (int)s.reconcileFlag();
      valueList << s.value().toString();
      valueFormattedList << s.value().formatMoney("", -1, false).replace(QChar(','), QChar('.'));
      sharesList << s.shares().toString();
      MyMoneyAccount acc = m_storage->account(s.accountId());
      MyMoneySecurity sec = m_storage->security(acc.currencyId());
      sharesFormattedList << s.price().
      formatMoney("", MyMoneyMoney::denomToPrec(sec.smallestAccountFraction()), false).
      replace(QChar(','), QChar('.'));
      MyMoneyMoney price = s.actualPrice();
      if (!price.isZero()) {
        priceList << price.toString();
        priceFormattedList << price.formatMoney
        ("", sec.pricePrecision(), false)
        .replace(QChar(','), QChar('.'));
      } else {
        priceList << QString();
        priceFormattedList << QString();
      }
      memoList << s.memo();
      accountIdList << s.accountId();
      costCenterIdList << s.costCenterId();
      checkNumberList << s.number();
      postDateList << m_txPostDate.toString(Qt::ISODate); // FIXME: when Tom puts date into split object
      bankIdList << s.bankID();

      kvpIdList << QString(txId + QString::number(splitIdList[i]));
      kvpPairsList << s.pairs();
      ++i;
    }

    query.bindValue(":transactionId", txIdList);
    query.bindValue(":txType", typeList);
    QVariantList iList;
    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (int it_s, splitIdList) {
      iList << it_s;
    }

    query.bindValue(":splitId", iList);
    query.bindValue(":payeeId", payeeIdList);
    query.bindValue(":reconcileDate", reconcileDateList);
    query.bindValue(":action", actionList);
    query.bindValue(":reconcileFlag", reconcileFlagList);
    query.bindValue(":value", valueList);
    query.bindValue(":valueFormatted", valueFormattedList);
    query.bindValue(":shares", sharesList);
    query.bindValue(":sharesFormatted", sharesFormattedList);
    query.bindValue(":price", priceList);
    query.bindValue(":priceFormatted", priceFormattedList);
    query.bindValue(":memo", memoList);
    query.bindValue(":accountId", accountIdList);
    query.bindValue(":costCenterId", costCenterIdList);
    query.bindValue(":checkNumber", checkNumberList);
    query.bindValue(":postDate", postDateList);
    query.bindValue(":bankId", bankIdList);
    if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("writing Split");
    deleteKeyValuePairs("SPLIT", kvpIdList);
    writeKeyValuePairs("SPLIT", kvpIdList, kvpPairsList);
  }

  void writeSchedule(const MyMoneySchedule& sch, QSqlQuery& query, bool insert)
  {
    query.bindValue(":id", sch.id());
    query.bindValue(":name", sch.name());
    query.bindValue(":type", (int)sch.type());
    query.bindValue(":typeString", MyMoneySchedule::scheduleTypeToString(sch.type()));
    query.bindValue(":occurence", (int)sch.occurrencePeriod()); // krazy:exclude=spelling
    query.bindValue(":occurenceMultiplier", sch.occurrenceMultiplier()); // krazy:exclude=spelling
    query.bindValue(":occurenceString", sch.occurrenceToString()); // krazy:exclude=spelling
    query.bindValue(":paymentType", (int)sch.paymentType());
    query.bindValue(":paymentTypeString", MyMoneySchedule::paymentMethodToString(sch.paymentType()));
    query.bindValue(":startDate", sch.startDate().toString(Qt::ISODate));
    query.bindValue(":endDate", sch.endDate().toString(Qt::ISODate));
    if (sch.isFixed()) {
      query.bindValue(":fixed", "Y");
    } else {
      query.bindValue(":fixed", "N");
    }
    if (sch.lastDayInMonth()) {
      query.bindValue(":lastDayInMonth", "Y");
    } else {
      query.bindValue(":lastDayInMonth", "N");
    }
    if (sch.autoEnter()) {
      query.bindValue(":autoEnter", "Y");
    } else {
      query.bindValue(":autoEnter", "N");
    }
    query.bindValue(":lastPayment", sch.lastPayment());
    query.bindValue(":nextPaymentDue", sch.nextDueDate().toString(Qt::ISODate));
    query.bindValue(":weekendOption", (int)sch.weekendOption());
    query.bindValue(":weekendOptionString", MyMoneySchedule::weekendOptionToString(sch.weekendOption()));
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Schedules"); // krazy:exclude=crashy

    //store the payment history for this scheduled task.
    //easiest way is to delete all and re-insert; it's not a high use table
    query.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id;");
    query.bindValue(":id", sch.id());
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("deleting  Schedule Payment History"); // krazy:exclude=crashy

    query.prepare(m_db.m_tables["kmmSchedulePaymentHistory"].insertString());
    foreach (const QDate& it, sch.recordedPayments()) {
      query.bindValue(":schedId", sch.id());
      query.bindValue(":payDate", it.toString(Qt::ISODate));
      if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Schedule Payment History"); // krazy:exclude=crashy
    }

    //store the transaction data for this task.
    if (!insert) {
      query.prepare(m_db.m_tables["kmmTransactions"].updateString());
    } else {
      query.prepare(m_db.m_tables["kmmTransactions"].insertString());
    }
    writeTransaction(sch.id(), sch.transaction(), query, "S");

    //FIXME: enable when schedules have KVPs.

    //Add in Key-Value Pairs for transactions.
    //deleteKeyValuePairs("SCHEDULE", sch.id());
    //writeKeyValuePairs("SCHEDULE", sch.id(), sch.pairs());
  }

  void writeSecurity(const MyMoneySecurity& security, QSqlQuery& query)
  {
    query.bindValue(":id", security.id());
    query.bindValue(":name", security.name());
    query.bindValue(":symbol", security.tradingSymbol());
    query.bindValue(":type", static_cast<int>(security.securityType()));
    query.bindValue(":typeString", MyMoneySecurity::securityTypeToString(security.securityType()));
    query.bindValue(":roundingMethod", static_cast<int>(security.roundingMethod()));
    query.bindValue(":smallestAccountFraction", security.smallestAccountFraction());
    query.bindValue(":pricePrecision", security.pricePrecision());
    query.bindValue(":tradingCurrency", security.tradingCurrency());
    query.bindValue(":tradingMarket", security.tradingMarket());
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Securities"); // krazy:exclude=crashy

    //Add in Key-Value Pairs for security
    QVariantList idList;
    idList << security.id();
    QList<QMap<QString, QString> > pairs;
    pairs << security.pairs();
    writeKeyValuePairs("SECURITY", idList, pairs);
    m_hiIdSecurities = 0;
  }

  void writePricePair(const MyMoneyPriceEntries& p)
  {
    MyMoneyPriceEntries::ConstIterator it;
    for (it = p.constBegin(); it != p.constEnd(); ++it) {
      writePrice(*it);
      signalProgress(++m_prices, 0);
    }
  }

  void writePrice(const MyMoneyPrice& p)
  {
    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);
    query.prepare(m_db.m_tables["kmmPrices"].insertString());
    query.bindValue(":fromId", p.from());
    query.bindValue(":toId", p.to());
    query.bindValue(":priceDate", p.date().toString(Qt::ISODate));
    query.bindValue(":price", p.rate(QString()).toString());
    query.bindValue(":priceFormatted", p.rate(QString()).formatMoney("", 2));
    query.bindValue(":priceSource", p.source());
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Prices"); // krazy:exclude=crashy
  }

  void writeCurrency(const MyMoneySecurity& currency, QSqlQuery& query)
  {
    query.bindValue(":ISOcode", currency.id());
    query.bindValue(":name", currency.name());
    query.bindValue(":type", static_cast<int>(currency.securityType()));
    query.bindValue(":typeString", MyMoneySecurity::securityTypeToString(currency.securityType()));
    // writing the symbol as three short ints is a PITA, but the
    // problem is that database drivers have incompatible ways of declaring UTF8
    QString symbol = currency.tradingSymbol() + "   ";
    const ushort* symutf = symbol.utf16();
    //int ix = 0;
    //while (x[ix] != '\0') qDebug() << "symbol" << symbol << "char" << ix << "=" << x[ix++];
    //q.bindValue(":symbol1", symbol.mid(0,1).unicode()->unicode());
    //q.bindValue(":symbol2", symbol.mid(1,1).unicode()->unicode());
    //q.bindValue(":symbol3", symbol.mid(2,1).unicode()->unicode());
    query.bindValue(":symbol1", symutf[0]);
    query.bindValue(":symbol2", symutf[1]);
    query.bindValue(":symbol3", symutf[2]);
    query.bindValue(":symbolString", symbol);
    query.bindValue(":smallestCashFraction", currency.smallestCashFraction());
    query.bindValue(":smallestAccountFraction", currency.smallestAccountFraction());
    query.bindValue(":pricePrecision", currency.pricePrecision());
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Currencies"); // krazy:exclude=crashy
  }

  void writeReport(const MyMoneyReport& rep, QSqlQuery& query)
  {
    QDomDocument d; // create a dummy XML document
    QDomElement e = d.createElement("REPORTS");
    d.appendChild(e);
    rep.writeXML(d, e); // write the XML to document
    query.bindValue(":id", rep.id());
    query.bindValue(":name", rep.name());
    query.bindValue(":XML", d.toString());
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("writing Reports"); // krazy:exclude=crashy
  }

  void writeBudget(const MyMoneyBudget& bud, QSqlQuery& query)
  {
    QDomDocument d; // create a dummy XML document
    QDomElement e = d.createElement("BUDGETS");
    d.appendChild(e);
    bud.writeXML(d, e); // write the XML to document
    query.bindValue(":id", bud.id());
    query.bindValue(":name", bud.name());
    query.bindValue(":start", bud.budgetStart());
    query.bindValue(":XML", d.toString());
    if (!query.exec()) // krazy:exclude=crashy
      throw MYMONEYEXCEPTIONSQL("writing Budgets"); // krazy:exclude=crashy
  }

  void writeKeyValuePairs(const QString& kvpType, const QVariantList& kvpId, const QList<QMap<QString, QString> >& pairs)
  {
    Q_Q(MyMoneyStorageSql);
    if (pairs.empty())
      return;

    QVariantList type;
    QVariantList id;
    QVariantList key;
    QVariantList value;
    int pairCount = 0;

    for (int i = 0; i < kvpId.size(); ++i) {
      QMap<QString, QString>::ConstIterator it;
      for (it = pairs[i].constBegin(); it != pairs[i].constEnd(); ++it) {
        type << kvpType;
        id << kvpId[i];
        key << it.key();
        value << it.value();
      }
      pairCount += pairs[i].size();
    }

    QSqlQuery query(*q);
    query.prepare(m_db.m_tables["kmmKeyValuePairs"].insertString());
    query.bindValue(":kvpType", type);
    query.bindValue(":kvpId", id);
    query.bindValue(":kvpKey", key);
    query.bindValue(":kvpData", value);
    if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("writing KVP");
    m_kvps += pairCount;
  }

  void writeOnlineJob(const onlineJob& job, QSqlQuery& query)
  {
    Q_ASSERT(job.id().startsWith('O'));

    query.bindValue(":id", job.id());
    query.bindValue(":type", job.taskIid());
    query.bindValue(":jobSend", job.sendDate());
    query.bindValue(":bankAnswerDate", job.bankAnswerDate());
    switch (job.bankAnswerState()) {
      case eMyMoney::OnlineJob::sendingState::acceptedByBank: query.bindValue(":state", QLatin1String("acceptedByBank")); break;
      case eMyMoney::OnlineJob::sendingState::rejectedByBank: query.bindValue(":state", QLatin1String("rejectedByBank")); break;
      case eMyMoney::OnlineJob::sendingState::abortedByUser: query.bindValue(":state", QLatin1String("abortedByUser")); break;
      case eMyMoney::OnlineJob::sendingState::sendingError: query.bindValue(":state", QLatin1String("sendingError")); break;
      case eMyMoney::OnlineJob::sendingState::noBankAnswer:
      default: query.bindValue(":state", QLatin1String("noBankAnswer"));
    }
    query.bindValue(":locked", QVariant::fromValue<QString>(job.isLocked() ? QLatin1String("Y") : QLatin1String("N")));
  }

  void writePayeeIdentifier(const payeeIdentifier& pid, QSqlQuery& query)
  {
    query.bindValue(":id", pid.idString());
    query.bindValue(":type", pid.iid());
    if (!query.exec()) { // krazy:exclude=crashy
      qWarning() << buildError(query, Q_FUNC_INFO, QString("modifying payeeIdentifier"));
      throw MYMONEYEXCEPTIONSQL("modifying payeeIdentifier"); // krazy:exclude=crashy
    }
  }
    /** @} */

  /**
   * @name readMethods
   * @{
   */
  void readFileInfo()
  {
    Q_Q(MyMoneyStorageSql);
    signalProgress(0, 1, QObject::tr("Loading file information..."));

    QSqlQuery query(*q);

    query.prepare(
      "SELECT "
      "  created, lastModified, "
      "  encryptData, logonUser, logonAt, "
      "  (SELECT count(*) FROM kmmInstitutions) AS institutions, "
      "  (SELECT count(*) from kmmAccounts) AS accounts, "
      "  (SELECT count(*) FROM kmmCurrencies) AS currencies, "
      "  (SELECT count(*) FROM kmmPayees) AS payees, "
      "  (SELECT count(*) FROM kmmTags) AS tags, "
      "  (SELECT count(*) FROM kmmTransactions) AS transactions, "
      "  (SELECT count(*) FROM kmmSplits) AS splits, "
      "  (SELECT count(*) FROM kmmSecurities) AS securities, "
      "  (SELECT count(*) FROM kmmCurrencies) AS currencies, "
      "  (SELECT count(*) FROM kmmSchedules) AS schedules, "
      "  (SELECT count(*) FROM kmmPrices) AS prices, "
      "  (SELECT count(*) FROM kmmKeyValuePairs) AS kvps, "
      "  (SELECT count(*) FROM kmmReportConfig) AS reports, "
      "  (SELECT count(*) FROM kmmBudgetConfig) AS budgets, "
      "  (SELECT count(*) FROM kmmOnlineJobs) AS onlineJobs, "
      "  (SELECT count(*) FROM kmmPayeeIdentifier) AS payeeIdentifier "
      "FROM kmmFileInfo;"
    );

    if (!query.exec())
      throw MYMONEYEXCEPTIONSQL("reading FileInfo"); // krazy:exclude=crashy
    if (!query.next())
      throw MYMONEYEXCEPTIONSQL("retrieving FileInfo");

    QSqlRecord rec = query.record();
    m_storage->setCreationDate(GETDATE(rec.indexOf("created")));
    m_storage->setLastModificationDate(GETDATE(rec.indexOf("lastModified")));

    m_institutions = (ulong) GETULL(rec.indexOf("institutions"));
    m_accounts = (ulong) GETULL(rec.indexOf("accounts"));
    m_payees = (ulong) GETULL(rec.indexOf("payees"));
    m_tags = (ulong) GETULL(rec.indexOf("tags"));
    m_transactions = (ulong) GETULL(rec.indexOf("transactions"));
    m_splits = (ulong) GETULL(rec.indexOf("splits"));
    m_securities = (ulong) GETULL(rec.indexOf("securities"));
    m_currencies = (ulong) GETULL(rec.indexOf("currencies"));
    m_schedules = (ulong) GETULL(rec.indexOf("schedules"));
    m_prices = (ulong) GETULL(rec.indexOf("prices"));
    m_kvps = (ulong) GETULL(rec.indexOf("kvps"));
    m_reports = (ulong) GETULL(rec.indexOf("reports"));
    m_budgets = (ulong) GETULL(rec.indexOf("budgets"));
    m_onlineJobs = (ulong) GETULL(rec.indexOf("onlineJobs"));
    m_payeeIdentifier = (ulong) GETULL(rec.indexOf("payeeIdentifier"));

    m_encryptData = GETSTRING(rec.indexOf("encryptData"));
    m_logonUser = GETSTRING(rec.indexOf("logonUser"));
    m_logonAt = GETDATETIME(rec.indexOf("logonAt"));

    signalProgress(1, 0);
    m_storage->setPairs(readKeyValuePairs("STORAGE", QString("")).pairs());
  }

  void readLogonData();
  void readUserInformation();

  void readInstitutions()
  {
    Q_Q(MyMoneyStorageSql);
    try {
      QMap<QString, MyMoneyInstitution> iList = q->fetchInstitutions();
      m_storage->loadInstitutions(iList);
      readFileInfo();
    } catch (const MyMoneyException &) {
      throw;
    }
  }

  void readAccounts()
  {
    Q_Q(MyMoneyStorageSql);
    m_storage->loadAccounts(q->fetchAccounts());
  }

  void readTransactions(const QString& tidList, const QString& dateClause)
  {
    Q_Q(MyMoneyStorageSql);
    try {
      m_storage->loadTransactions(q->fetchTransactions(tidList, dateClause));
    } catch (const MyMoneyException &) {
      throw;
    }
  }

  void readTransactions()
  {
    readTransactions(QString(), QString());
  }

  void readSplit(MyMoneySplit& s, const QSqlQuery& query) const
  {
    Q_Q(const MyMoneyStorageSql);
    // Set these up as statics, since the field numbers should not change
    // during execution.
    static const MyMoneyDbTable& t = m_db.m_tables["kmmSplits"];
    static const int splitIdCol = t.fieldNumber("splitId");
    static const int transactionIdCol = t.fieldNumber("transactionId");
    static const int payeeIdCol = t.fieldNumber("payeeId");
    static const int reconcileDateCol = t.fieldNumber("reconcileDate");
    static const int actionCol = t.fieldNumber("action");
    static const int reconcileFlagCol = t.fieldNumber("reconcileFlag");
    static const int valueCol = t.fieldNumber("value");
    static const int sharesCol = t.fieldNumber("shares");
    static const int priceCol = t.fieldNumber("price");
    static const int memoCol = t.fieldNumber("memo");
    static const int accountIdCol = t.fieldNumber("accountId");
    static const int costCenterIdCol = t.fieldNumber("costCenterId");
    static const int checkNumberCol = t.fieldNumber("checkNumber");
  //  static const int postDateCol = t.fieldNumber("postDate"); // FIXME - when Tom puts date into split object
    static const int bankIdCol = t.fieldNumber("bankId");

    s.clearId();

    QList<QString> tagIdList;
    QSqlQuery query1(*const_cast <MyMoneyStorageSql*>(q));
    query1.prepare("SELECT tagId from kmmTagSplits where splitId = :id and transactionId = :transactionId");
    query1.bindValue(":id", GETSTRING(splitIdCol));
    query1.bindValue(":transactionId", GETSTRING(transactionIdCol));
    if (!query1.exec()) throw MYMONEYEXCEPTIONSQL("reading tagId in Split"); // krazy:exclude=crashy
    while (query1.next())
      tagIdList << query1.value(0).toString();

    s.setTagIdList(tagIdList);
    s.setPayeeId(GETSTRING(payeeIdCol));
    s.setReconcileDate(GETDATE(reconcileDateCol));
    s.setAction(GETSTRING(actionCol));
    s.setReconcileFlag(static_cast<Split::State>(GETINT(reconcileFlagCol)));
    s.setValue(MyMoneyMoney(MyMoneyUtils::QStringEmpty(GETSTRING(valueCol))));
    s.setShares(MyMoneyMoney(MyMoneyUtils::QStringEmpty(GETSTRING(sharesCol))));
    s.setPrice(MyMoneyMoney(MyMoneyUtils::QStringEmpty(GETSTRING(priceCol))));
    s.setMemo(GETSTRING(memoCol));
    s.setAccountId(GETSTRING(accountIdCol));
    s.setCostCenterId(GETSTRING(costCenterIdCol));
    s.setNumber(GETSTRING(checkNumberCol));
    //s.setPostDate(GETDATETIME(postDateCol)); // FIXME - when Tom puts date into split object
    s.setBankID(GETSTRING(bankIdCol));

    return;
  }

  const MyMoneyKeyValueContainer readKeyValuePairs(const QString& kvpType, const QString& kvpId) const
  {
    Q_Q(const MyMoneyStorageSql);
    MyMoneyKeyValueContainer list;
    QSqlQuery query(*const_cast <MyMoneyStorageSql*>(q));
    query.prepare("SELECT kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type and kvpId = :id;");
    query.bindValue(":type", kvpType);
    query.bindValue(":id", kvpId);
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("reading Kvp for %1 %2").arg(kvpType) // krazy:exclude=crashy
                                            .arg(kvpId));
    while (query.next()) list.setValue(query.value(0).toString(), query.value(1).toString());
    return (list);
  }

  const QHash<QString, MyMoneyKeyValueContainer> readKeyValuePairs(const QString& kvpType, const QStringList& kvpIdList) const
  {
    Q_Q(const MyMoneyStorageSql);
    QHash<QString, MyMoneyKeyValueContainer> retval;

    QSqlQuery query(*const_cast <MyMoneyStorageSql*>(q));

    QString idList;
    if (!kvpIdList.empty()) {
      idList = QString(" and kvpId IN ('%1')").arg(kvpIdList.join("', '"));
    }

    QString sQuery = QString("SELECT kvpId, kvpKey, kvpData from kmmKeyValuePairs where kvpType = :type %1 order by kvpId;").arg(idList);

    query.prepare(sQuery);
    query.bindValue(":type", kvpType);
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("reading Kvp List for %1").arg(kvpType)); // krazy:exclude=crashy

    // Reserve enough space for all values.
    retval.reserve(kvpIdList.size());

    // The loop below is designed to limit the number of calls to
    // QHash::operator[] in order to speed up calls to this function. This
    // assumes that QString::operator== is faster.
    /*
      if (q.next()) {
        QString oldkey = q.value(0).toString();
        MyMoneyKeyValueContainer& kvpc = retval[oldkey];

        kvpc.setValue(q.value(1).toString(), q.value(2).toString());

        while (q.next()) {
          if (q.value(0).toString() != oldkey) {
            oldkey = q.value(0).toString();
            kvpc = retval[oldkey];
          }
          kvpc.setValue(q.value(1).toString(), q.value(2).toString());
        }
      }
    */
    while (query.next()) {
      retval[query.value(0).toString()].setValue(query.value(1).toString(), query.value(2).toString());
    }
    return (retval);
  }

  void readSchedules()
  {
    Q_Q(MyMoneyStorageSql);
    try {
      m_storage->loadSchedules(q->fetchSchedules());
    } catch (const MyMoneyException &) {
      throw;
    }

  }

  void readSecurities()
  {
    Q_Q(MyMoneyStorageSql);
    try {
      m_storage->loadSecurities(q->fetchSecurities());
    } catch (const MyMoneyException &) {
      throw;
    }

  }

  void readPrices()
  {
  //  try {
  //    m_storage->addPrice(MyMoneyPrice(from, to,  date, rate, source));
  //  } catch (const MyMoneyException &) {
  //    throw;
  //  }
  }

  void readCurrencies()
  {
    Q_Q(MyMoneyStorageSql);
    try {
      m_storage->loadCurrencies(q->fetchCurrencies());
    } catch (const MyMoneyException &) {
      throw;
    }
  }

  void readReports()
  {
    Q_Q(MyMoneyStorageSql);
    try {
      m_storage->loadReports(q->fetchReports());
    } catch (const MyMoneyException &) {
      throw;
    }
  }

  void readBudgets()
  {
    Q_Q(MyMoneyStorageSql);
    m_storage->loadBudgets(q->fetchBudgets());
  }

  void readOnlineJobs()
  {
    Q_Q(MyMoneyStorageSql);
    m_storage->loadOnlineJobs(q->fetchOnlineJobs());
  }

  /** @} */

  void deleteTransaction(const QString& id)
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);
    QVariantList idList;
    idList << id;
    query.prepare("DELETE FROM kmmSplits WHERE transactionId = :transactionId;");
    query.bindValue(":transactionId", idList);
    if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Splits");

    query.prepare("DELETE FROM kmmKeyValuePairs WHERE kvpType = 'SPLIT' "
              "AND kvpId LIKE '?%'");
    query.bindValue(1, idList);
    if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting Splits KVP");

    m_splits -= query.numRowsAffected();
    deleteKeyValuePairs("TRANSACTION", idList);
    query.prepare(m_db.m_tables["kmmTransactions"].deleteString());
    query.bindValue(":id", idList);
    if (!query.execBatch())
      throw MYMONEYEXCEPTIONSQL("deleting Transaction");
  }

  void deleteTagSplitsList(const QString& txId, const QList<int>& splitIdList)
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QVariantList iList;
    QVariantList transactionIdList;

    // qCopy segfaults here, so do it with a hand-rolled loop
    foreach (int it_s, splitIdList) {
      iList << it_s;
      transactionIdList << txId;
    }
    QSqlQuery query(*q);
    query.prepare("DELETE FROM kmmTagSplits WHERE transactionId = :transactionId AND splitId = :splitId");
    query.bindValue(":splitId", iList);
    query.bindValue(":transactionId", transactionIdList);
    if (!query.execBatch()) throw MYMONEYEXCEPTIONSQL("deleting tagSplits");
  }

  void deleteSchedule(const QString& id)
  {
    Q_Q(MyMoneyStorageSql);
    deleteTransaction(id);
    QSqlQuery query(*q);
    query.prepare("DELETE FROM kmmSchedulePaymentHistory WHERE schedId = :id");
    query.bindValue(":id", id);
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("deleting Schedule Payment History"); // krazy:exclude=crashy
    query.prepare(m_db.m_tables["kmmSchedules"].deleteString());
    query.bindValue(":id", id);
    if (!query.exec()) throw MYMONEYEXCEPTIONSQL("deleting Schedule"); // krazy:exclude=crashy
    //FIXME: enable when schedules have KVPs.
    //deleteKeyValuePairs("SCHEDULE", id);
  }

  void deleteKeyValuePairs(const QString& kvpType, const QVariantList& idList)
  {
    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);
    query.prepare("DELETE FROM kmmKeyValuePairs WHERE kvpType = :kvpType AND kvpId = :kvpId;");
    QVariantList typeList;
    for (int i = 0; i < idList.size(); ++i) {
      typeList << kvpType;
    }
    query.bindValue(":kvpType", typeList);
    query.bindValue(":kvpId", idList);
    if (!query.execBatch()) {
      QString idString;
      for (int i = 0; i < idList.size(); ++i) {
        idString.append(idList[i].toString() + ' ');
      }
      throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("deleting kvp for %1 %2").arg(kvpType).arg(idString));
    }
    m_kvps -= query.numRowsAffected();
  }

  ulong calcHighId(ulong i, const QString& id)
  {
    QString nid = id;
    ulong high = (ulong) nid.remove(QRegExp("[A-Z]*")).toULongLong();
    return std::max(high, i);
  }

  void setVersion(const QString& version);

  int splitState(const TransactionFilter::State& state) const
  {
    auto rc = (int)Split::State::NotReconciled;

    switch (state) {
      default:
      case TransactionFilter::State::NotReconciled:
        break;

      case TransactionFilter::State::Cleared:
        rc = (int)Split::State::Cleared;
        break;

      case TransactionFilter::State::Reconciled:
        rc = (int)Split::State::Reconciled;
        break;

      case TransactionFilter::State::Frozen:
        rc = (int)Split::State::Frozen;
        break;
    }
    return rc;
  }

  QDate getDate(const QString& date) const
  {
    return (date.isNull() ? QDate() : QDate::fromString(date, Qt::ISODate));
  }

  QDateTime getDateTime(const QString& date) const
  {
    return (date.isNull() ? QDateTime() : QDateTime::fromString(date, Qt::ISODate));
  }

  bool fileExists(const QString& dbName)
  {
    QFile f(dbName);
    if (!f.exists()) {
      m_error = i18n("SQLite file %1 does not exist", dbName);
      return (false);
    }
    return (true);
  }

  /** @brief a function to build a comprehensive error message for an SQL error */
  QString& buildError(const QSqlQuery& query, const QString& function,
                                         const QString& messageb) const
  {
    Q_Q(const MyMoneyStorageSql);
    return (buildError(query, function, messageb, q));
  }

  QString& buildError(const QSqlQuery& query, const QString& function,
                                         const QString& message, const QSqlDatabase* db) const
  {
    Q_Q(const MyMoneyStorageSql);
    QString s = QString("Error in function %1 : %2").arg(function).arg(message);
    s += QString("\nDriver = %1, Host = %2, User = %3, Database = %4")
         .arg(db->driverName()).arg(db->hostName()).arg(db->userName()).arg(db->databaseName());
    QSqlError e = db->lastError();
    s += QString("\nDriver Error: %1").arg(e.driverText());
    s += QString("\nDatabase Error No %1: %2").arg(e.number()).arg(e.databaseText());
    s += QString("\nText: %1").arg(e.text());
    s += QString("\nError type %1").arg(e.type());
    e = query.lastError();
    s += QString("\nExecuted: %1").arg(query.executedQuery());
    s += QString("\nQuery error No %1: %2").arg(e.number()).arg(e.text());
    s += QString("\nError type %1").arg(e.type());

    const_cast <MyMoneyStorageSql*>(q)->d_func()->m_error = s;
    qDebug("%s", qPrintable(s));
    const_cast <MyMoneyStorageSql*>(q)->cancelCommitUnit(function);
    return (const_cast <MyMoneyStorageSql*>(q)->d_func()->m_error);
  }

  /**
   * MyMoneyStorageSql create database
   *
   * @param url pseudo-URL of database to be opened
   *
   * @return true - creation successful
   * @return false - could not create
   *
   */

  bool createDatabase(const QUrl &url)
  {
    Q_Q(MyMoneyStorageSql);
    int rc = true;
    if (!m_driver->requiresCreation()) return(true); // not needed for sqlite
    QString dbName = url.path().right(url.path().length() - 1); // remove separator slash
    if (!m_driver->canAutocreate()) {
      m_error = i18n("Automatic database creation for type %1 is not currently implemented.\n"
                     "Please create database %2 manually", q->driverName(), dbName);
      return (false);
    }
    // create the database (only works for mysql and postgre at present)
    { // for this code block, see QSqlDatabase API re removeDatabase
      QSqlDatabase maindb = QSqlDatabase::addDatabase(q->driverName(), "main");
      maindb.setDatabaseName(m_driver->defaultDbName());
      maindb.setHostName(url.host());
      maindb.setUserName(url.userName());
      maindb.setPassword(url.password());
      if (!maindb.open()) {
        throw MYMONEYEXCEPTION(QString::fromLatin1("opening database %1 in function %2")
                                  .arg(maindb.databaseName()).arg(Q_FUNC_INFO));
      } else {
        QSqlQuery qm(maindb);
        qm.exec(QString::fromLatin1("PRAGMA key = '%1'").arg(q->password()));
        QString qs = m_driver->createDbString(dbName) + ';';
        if (!qm.exec(qs)) { // krazy:exclude=crashy
          buildError(qm, Q_FUNC_INFO,
                     i18n("Error in create database %1; do you have create permissions?", dbName), &maindb);
          rc = false;
        }
        maindb.close();
      }
    }
    QSqlDatabase::removeDatabase("main");
    return (rc);
  }


  int upgradeDb()
  {
    Q_Q(MyMoneyStorageSql);
    //signalProgress(0, 1, QObject::tr("Upgrading database..."));
    QSqlQuery query(*q);
    query.prepare("SELECT version FROM kmmFileInfo;");
    if (!query.exec() || !query.next()) { // krazy:exclude=crashy
      if (!m_newDatabase) {
        buildError(query, Q_FUNC_INFO, "Error retrieving file info (version)");
        return(1);
      } else {
        m_dbVersion = m_db.currentVersion();
        m_storage->setFileFixVersion(m_storage->currentFixVersion());
        QSqlQuery query2(*q);
        query2.prepare("UPDATE kmmFileInfo SET version = :version, \
                  fixLevel = :fixLevel;");
        query2.bindValue(":version", m_dbVersion);
        query2.bindValue(":fixLevel", m_storage->currentFixVersion());
        if (!query2.exec()) { // krazy:exclude=crashy
          buildError(query2, Q_FUNC_INFO, "Error updating file info(version)");
          return(1);
        }
        return (0);
      }
    }
    // prior to dbv6, 'version' format was 'dbversion.fixLevel+1'
    // as of dbv6, these are separate fields
    QString version = query.value(0).toString();
    if (version.contains('.')) {
      m_dbVersion = query.value(0).toString().section('.', 0, 0).toUInt();
      m_storage->setFileFixVersion(query.value(0).toString().section('.', 1, 1).toUInt() - 1);
    } else {
      m_dbVersion = version.toUInt();
      query.prepare("SELECT fixLevel FROM kmmFileInfo;");
      if (!query.exec() || !query.next()) { // krazy:exclude=crashy
        buildError(query, Q_FUNC_INFO, "Error retrieving file info (fixLevel)");
        return(1);
      }
      m_storage->setFileFixVersion(query.value(0).toUInt());
    }

    if (m_dbVersion == m_db.currentVersion())
      return 0;

    int rc = 0;

    // Drop VIEWs
    QStringList lowerTables = tables(QSql::AllTables);
    for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
      (*i) = (*i).toLower();
    }

    for (QMap<QString, MyMoneyDbView>::ConstIterator tt = m_db.viewBegin(); tt != m_db.viewEnd(); ++tt) {
      if (lowerTables.contains(tt.key().toLower())) {
        if (!query.exec("DROP VIEW " + tt.value().name() + ';')) // krazy:exclude=crashy
          throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("dropping view %1").arg(tt.key()));
      }
    }

    while ((m_dbVersion < m_db.currentVersion()) && (rc == 0)) {
      switch (m_dbVersion) {
        case 0:
          if ((rc = upgradeToV1()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 1:
          if ((rc = upgradeToV2()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 2:
          if ((rc = upgradeToV3()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 3:
          if ((rc = upgradeToV4()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 4:
          if ((rc = upgradeToV5()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 5:
          if ((rc = upgradeToV6()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 6:
          if ((rc = upgradeToV7()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 7:
          if ((rc = upgradeToV8()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 8:
          if ((rc = upgradeToV9()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 9:
          if ((rc = upgradeToV10()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 10:
          if ((rc = upgradeToV11()) != 0) return (1);
          ++m_dbVersion;
          break;
        case 11:
          if ((rc = upgradeToV12()) != 0) return (1);
          ++m_dbVersion;
          break;
        default:
          qWarning("Unknown version number in database - %d", m_dbVersion);
      }
    }

    // restore VIEWs
    lowerTables = tables(QSql::AllTables);
    for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
      (*i) = (*i).toLower();
    }

    for (QMap<QString, MyMoneyDbView>::ConstIterator tt = m_db.viewBegin(); tt != m_db.viewEnd(); ++tt) {
      if (!lowerTables.contains(tt.key().toLower())) {
        if (!query.exec(tt.value().createString())) // krazy:exclude=crashy
          throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("creating view %1").arg(tt.key()));
      }
    }

    // write updated version to DB
    //setVersion(QString("%1.%2").arg(m_dbVersion).arg(m_minorVersion))
    query.prepare(QString("UPDATE kmmFileInfo SET version = :version;"));
    query.bindValue(":version", m_dbVersion);
    if (!query.exec()) { // krazy:exclude=crashy
      buildError(query, Q_FUNC_INFO, "Error updating db version");
      return (1);
    }
    //signalProgress(-1,-1);
    return (0);
  }

  int upgradeToV1()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);
    // change kmmSplits pkey to (transactionId, splitId)
    if (!query.exec("ALTER TABLE kmmSplits ADD PRIMARY KEY (transactionId, splitId);")) { // krazy:exclude=crashy
      buildError(query, Q_FUNC_INFO, "Error updating kmmSplits pkey");
      return (1);
    }
    // change kmmSplits alter checkNumber varchar(32)
    if (!query.exec(m_db.m_tables["kmmSplits"].modifyColumnString(m_driver, "checkNumber", // krazy:exclude=crashy
                MyMoneyDbColumn("checkNumber", "varchar(32)")))) {
      buildError(query, Q_FUNC_INFO, "Error expanding kmmSplits.checkNumber");
      return (1);
    }
    // change kmmSplits add postDate datetime
    if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
      return (1);
    // initialize it to same value as transaction (do it the long way round)
    query.prepare("SELECT id, postDate FROM kmmTransactions WHERE txType = 'N';");
    if (!query.exec()) { // krazy:exclude=crashy
      buildError(query, Q_FUNC_INFO, "Error priming kmmSplits.postDate");
      return (1);
    }
    QMap<QString, QDateTime> tids;
    while (query.next()) tids[query.value(0).toString()] = query.value(1).toDateTime();
    QMap<QString, QDateTime>::ConstIterator it;
    for (it = tids.constBegin(); it != tids.constEnd(); ++it) {
      query.prepare("UPDATE kmmSplits SET postDate=:postDate WHERE transactionId = :id;");
      query.bindValue(":postDate", it.value().toString(Qt::ISODate));
      query.bindValue(":id", it.key());
      if (!query.exec()) { // krazy:exclude=crashy
        buildError(query, Q_FUNC_INFO, "priming kmmSplits.postDate");
        return(1);
      }
    }
    // add index to kmmKeyValuePairs to (kvpType,kvpId)
    QStringList list;
    list << "kvpType" << "kvpId";
    if (!query.exec(MyMoneyDbIndex("kmmKeyValuePairs", "kmmKVPtype_id", list, false).generateDDL(m_driver) + ';')) {
      buildError(query, Q_FUNC_INFO, "Error adding kmmKeyValuePairs index");
      return (1);
    }
    // add index to kmmSplits to (accountId, txType)
    list.clear();
    list << "accountId" << "txType";
    if (!query.exec(MyMoneyDbIndex("kmmSplits", "kmmSplitsaccount_type", list, false).generateDDL(m_driver) + ';')) {
      buildError(query, Q_FUNC_INFO, "Error adding kmmSplits index");
      return (1);
    }
    // change kmmSchedulePaymentHistory pkey to (schedId, payDate)
    if (!query.exec("ALTER TABLE kmmSchedulePaymentHistory ADD PRIMARY KEY (schedId, payDate);")) {
      buildError(query, Q_FUNC_INFO, "Error updating kmmSchedulePaymentHistory pkey");
      return (1);
    }
    // change kmmPrices pkey to (fromId, toId, priceDate)
    if (!query.exec("ALTER TABLE kmmPrices ADD PRIMARY KEY (fromId, toId, priceDate);")) {
      buildError(query, Q_FUNC_INFO, "Error updating kmmPrices pkey");
      return (1);
    }
    // change kmmReportConfig pkey to (name)
    // There wasn't one previously, so no need to drop it.
    if (!query.exec("ALTER TABLE kmmReportConfig ADD PRIMARY KEY (name);")) {
      buildError(query, Q_FUNC_INFO, "Error updating kmmReportConfig pkey");
      return (1);
    }
    // change kmmFileInfo add budgets, hiBudgetId unsigned bigint
    // change kmmFileInfo add logonUser
    // change kmmFileInfo add logonAt datetime
    if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
      return (1);
    // change kmmAccounts add transactionCount unsigned bigint as last field
    if (!alterTable(m_db.m_tables["kmmAccounts"], m_dbVersion))
      return (1);
    // calculate the transaction counts. the application logic defines an account's tx count
    // in such a way as to count multiple splits in a tx which reference the same account as one.
    // this is the only way I can think of to do this which will work in sqlite too.
    // inefficient, but it only gets done once...
    // get a list of all accounts so we'll get a zero value for those without txs
    query.prepare("SELECT id FROM kmmAccounts");
    if (!query.exec()) { // krazy:exclude=crashy
      buildError(query, Q_FUNC_INFO, "Error retrieving accounts for transaction counting");
      return(1);
    }
    while (query.next()) {
      m_transactionCountMap[query.value(0).toString()] = 0;
    }
    query.prepare("SELECT accountId, transactionId FROM kmmSplits WHERE txType = 'N' ORDER BY 1, 2");
    if (!query.exec()) { // krazy:exclude=crashy
      buildError(query, Q_FUNC_INFO, "Error retrieving splits for transaction counting");
      return(1);
    }
    QString lastAcc, lastTx;
    while (query.next()) {
      QString thisAcc = query.value(0).toString();
      QString thisTx = query.value(1).toString();
      if ((thisAcc != lastAcc) || (thisTx != lastTx)) ++m_transactionCountMap[thisAcc];
      lastAcc = thisAcc;
      lastTx = thisTx;
    }
    QHash<QString, ulong>::ConstIterator itm;
    query.prepare("UPDATE kmmAccounts SET transactionCount = :txCount WHERE id = :id;");
    for (itm = m_transactionCountMap.constBegin(); itm != m_transactionCountMap.constEnd(); ++itm) {
      query.bindValue(":txCount", QString::number(itm.value()));
      query.bindValue(":id", itm.key());
      if (!query.exec()) { // krazy:exclude=crashy
        buildError(query, Q_FUNC_INFO, "Error updating transaction count");
        return (1);
      }
    }
    m_transactionCountMap.clear();
    return (0);
  }

  int upgradeToV2()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);
    // change kmmSplits add price, priceFormatted fields
    if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
      return (1);
    return (0);
  }

  int upgradeToV3()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);
    // kmmSchedules - add occurrenceMultiplier
    // The default value is given here to populate the column.
    if (!query.exec("ALTER TABLE kmmSchedules ADD COLUMN " +
                MyMoneyDbIntColumn("occurenceMultiplier",
                                   MyMoneyDbIntColumn::SMALL, false, false, true)
                .generateDDL(m_driver) + " DEFAULT 0;")) {
      buildError(query, Q_FUNC_INFO, "Error adding kmmSchedules.occurenceMultiplier");
      return (1);
    }
    //The default is less than any useful value, so as each schedule is hit, it will update
    //itself to the appropriate value.
    return 0;
  }

  int upgradeToV4()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);
    // kmmSplits - add index on transactionId + splitId
    QStringList list;
    list << "transactionId" << "splitId";
    if (!query.exec(MyMoneyDbIndex("kmmSplits", "kmmTx_Split", list, false).generateDDL(m_driver) + ';')) {
      buildError(query, Q_FUNC_INFO, "Error adding kmmSplits index on (transactionId, splitId)");
      return (1);
    }
    return 0;
  }

  int upgradeToV5()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);
    // kmmSplits - add bankId
    if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
      return (1);
    //kmmPayees - add columns "notes" "defaultAccountId" "matchData" "matchIgnoreCase" "matchKeys";
    if (!alterTable(m_db.m_tables["kmmPayees"], m_dbVersion))
      return (1);
    // kmmReportConfig - drop primary key on name since duplicate names are allowed
    if (!alterTable(m_db.m_tables["kmmReportConfig"], m_dbVersion))
      return (1);
    //}
    return 0;
  }

  int upgradeToV6()
  {
    Q_Q(MyMoneyStorageSql);
    q->startCommitUnit(Q_FUNC_INFO);
    QSqlQuery query(*q);
    // kmmFileInfo - add fixLevel
    if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
      return (1);
    // upgrade Mysql to InnoDB transaction-safe engine
    // the following is not a good way to test for mysql - think of a better way
    if (!m_driver->tableOptionString().isEmpty()) {
      for (QMap<QString, MyMoneyDbTable>::ConstIterator tt = m_db.tableBegin(); tt != m_db.tableEnd(); ++tt) {
        if (!query.exec(QString("ALTER TABLE %1 ENGINE = InnoDB;").arg(tt.value().name()))) {
          buildError(query, Q_FUNC_INFO, "Error updating to InnoDB");
          return (1);
        }
      }
    }

    // the alterTable function really doesn't work too well
    // with adding a new column which is also to be primary key
    // so add the column first
    if (!query.exec("ALTER TABLE kmmReportConfig ADD COLUMN " +
                MyMoneyDbColumn("id", "varchar(32)").generateDDL(m_driver) + ';')) {
      buildError(query, Q_FUNC_INFO, "adding id to report table");
      return(1);
    }
    QMap<QString, MyMoneyReport> reportList = q->fetchReports();
    // the V5 database allowed lots of duplicate reports with no
    // way to distinguish between them. The fetchReports call
    // will have effectively removed all duplicates
    // so we now delete from the db and re-write them
    if (!query.exec("DELETE FROM kmmReportConfig;")) {
      buildError(query, Q_FUNC_INFO, "Error deleting reports");
      return (1);
    }
    // add unique id to reports table
    if (!alterTable(m_db.m_tables["kmmReportConfig"], m_dbVersion))
      return(1);

    QMap<QString, MyMoneyReport>::const_iterator it_r;
    for (it_r = reportList.constBegin(); it_r != reportList.constEnd(); ++it_r) {
      MyMoneyReport r = *it_r;
      query.prepare(m_db.m_tables["kmmReportConfig"].insertString());
      writeReport(*it_r, query);
    }

    q->endCommitUnit(Q_FUNC_INFO);
    return 0;
  }

  int upgradeToV7()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);
    QSqlQuery query(*q);

    // add tags support
    // kmmFileInfo - add tags and hiTagId
    if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
      return (1);

    m_tags = 0;
    return 0;
  }

  int upgradeToV8()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);

    // Added onlineJobs and payeeIdentifier
    if (!alterTable(m_db.m_tables["kmmFileInfo"], m_dbVersion))
      return (1);

    return 0;
  }

  int upgradeToV9()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);

    QSqlQuery query(*q);
    // kmmSplits - add bankId
    if (!alterTable(m_db.m_tables["kmmSplits"], m_dbVersion))
      return (1);

    return 0;
  }

  int upgradeToV10()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);

    QSqlQuery query(*q);
    if (!alterTable(m_db.m_tables["kmmPayeesPayeeIdentifier"], m_dbVersion))
      return (1);
    if (!alterTable(m_db.m_tables["kmmAccountsPayeeIdentifier"], m_dbVersion))
      return (1);

    return 0;
  }

  int upgradeToV11()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);

    QSqlQuery query(*q);

    // add column roundingMethodCol to kmmSecurities
    if (!alterTable(m_db.m_tables["kmmSecurities"], m_dbVersion))
      return 1;
    // add column pricePrecision to kmmCurrencies
    if (!alterTable(m_db.m_tables["kmmCurrencies"], m_dbVersion))
      return 1;

    return 0;
  }

  int upgradeToV12()
  {
    Q_Q(MyMoneyStorageSql);
    MyMoneyDbTransaction dbtrans(*q, Q_FUNC_INFO);

    switch(haveColumnInTable(QLatin1String("kmmSchedules"), QLatin1String("lastDayInMonth"))) {
      case -1:
        return 1;
      case 1:         // column exists, nothing to do
        break;
      case 0:         // need update of kmmSchedules
        // add column lastDayInMonth. Simply redo the update for 10 .. 11
        if (!alterTable(m_db.m_tables["kmmSchedules"], m_dbVersion-1))
          return 1;
        break;
    }

    switch(haveColumnInTable(QLatin1String("kmmSecurities"), QLatin1String("roundingMethod"))) {
      case -1:
        return 1;
      case 1:         // column exists, nothing to do
        break;
      case 0:         // need update of kmmSecurities and kmmCurrencies
        // add column roundingMethodCol to kmmSecurities. Simply redo the update for 10 .. 11
        if (!alterTable(m_db.m_tables["kmmSecurities"], m_dbVersion-1))
          return 1;
        // add column pricePrecision to kmmCurrencies. Simply redo the update for 10 .. 11
        if (!alterTable(m_db.m_tables["kmmCurrencies"], m_dbVersion-1))
          return 1;
        break;
    }
    return 0;
  }

  int createTables()
  {
    Q_Q(MyMoneyStorageSql);
    // check tables, create if required
    // convert everything to lower case, since SQL standard is case insensitive
    // table and column names (when not delimited), but some DBMSs disagree.
    QStringList lowerTables = tables(QSql::AllTables);
    for (QStringList::iterator i = lowerTables.begin(); i != lowerTables.end(); ++i) {
      (*i) = (*i).toLower();
    }

    for (QMap<QString, MyMoneyDbTable>::ConstIterator tt = m_db.tableBegin(); tt != m_db.tableEnd(); ++tt) {
      if (!lowerTables.contains(tt.key().toLower())) {
        createTable(tt.value());
      }
    }

    QSqlQuery query(*q);
    for (QMap<QString, MyMoneyDbView>::ConstIterator tt = m_db.viewBegin(); tt != m_db.viewEnd(); ++tt) {
      if (!lowerTables.contains(tt.key().toLower())) {
        if (!query.exec(tt.value().createString())) throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("creating view %1").arg(tt.key()));
      }
    }

    // The columns to store version info changed with version 6. Prior versions are not supported here but an error is prevented and
    // an old behaviour is used: call upgradeDb().
    m_dbVersion = m_db.currentVersion();
    if (m_dbVersion >= 6) {
      query.prepare(QLatin1String("INSERT INTO kmmFileInfo (version, fixLevel) VALUES(?,?);"));
      query.bindValue(0, m_dbVersion);
      query.bindValue(1, m_storage->fileFixVersion());
      if (!query.exec())
        throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("Saving database version"));
    }

    return upgradeDb();
  }

  void createTable(const MyMoneyDbTable& t, int version = std::numeric_limits<int>::max())
  {
    Q_Q(MyMoneyStorageSql);
  // create the tables
    QStringList ql = t.generateCreateSQL(m_driver, version).split('\n', QString::SkipEmptyParts);
    QSqlQuery query(*q);
    foreach (const QString& i, ql) {
      if (!query.exec(i)) throw MYMONEYEXCEPTIONSQL(QString::fromLatin1("creating table/index %1").arg(t.name()));
    }
  }

  bool alterTable(const MyMoneyDbTable& t, int fromVersion)
  {
    Q_Q(MyMoneyStorageSql);
    const int toVersion = fromVersion + 1;

    QString tempTableName = t.name();
    tempTableName.replace("kmm", "kmmtmp");
    QSqlQuery query(*q);
    // drop primary key if it has one (and driver supports it)
    if (t.hasPrimaryKey(fromVersion)) {
      QString dropString = m_driver->dropPrimaryKeyString(t.name());
      if (!dropString.isEmpty()) {
        if (!query.exec(dropString)) {
          buildError(query, Q_FUNC_INFO, QString("Error dropping old primary key from %1").arg(t.name()));
          return false;
        }
      }
    }
    for (MyMoneyDbTable::index_iterator i = t.indexBegin(); i != t.indexEnd(); ++i) {
      QString indexName = t.name() + '_' + i->name() + "_idx";
      if (!query.exec(m_driver->dropIndexString(t.name(), indexName))) {
        buildError(query, Q_FUNC_INFO, QString("Error dropping index from %1").arg(t.name()));
        return false;
      }
    }
    if (!query.exec(QString("ALTER TABLE " + t.name() + " RENAME TO " + tempTableName + ';'))) {
      buildError(query, Q_FUNC_INFO, QString("Error renaming table %1").arg(t.name()));
      return false;
    }
    createTable(t, toVersion);
    if (q->getRecCount(tempTableName) > 0) {
      query.prepare(QString("INSERT INTO " + t.name() + " (" + t.columnList(fromVersion) +
                          ") SELECT " + t.columnList(fromVersion) + " FROM " + tempTableName + ';'));
      if (!query.exec()) { // krazy:exclude=crashy
          buildError(query, Q_FUNC_INFO, QString("Error inserting into new table %1").arg(t.name()));
          return false;
      }
    }
    if (!query.exec(QString("DROP TABLE " + tempTableName + ';'))) {
      buildError(query, Q_FUNC_INFO, QString("Error dropping old table %1").arg(t.name()));
      return false;
    }
    return true;
  }

  void clean()
  {
    Q_Q(MyMoneyStorageSql);
  // delete all existing records
    QMap<QString, MyMoneyDbTable>::ConstIterator it = m_db.tableBegin();
    QSqlQuery query(*q);
    while (it != m_db.tableEnd()) {
      query.prepare(QString("DELETE from %1;").arg(it.key()));
      if (!query.exec()) throw MYMONEYEXCEPTIONSQL("cleaning database"); // krazy:exclude=crashy
      ++it;
    }
  }

  int isEmpty()
  {
    Q_Q(MyMoneyStorageSql);
    // check all tables are empty
    QMap<QString, MyMoneyDbTable>::ConstIterator tt = m_db.tableBegin();
    int recordCount = 0;
    QSqlQuery query(*q);
    while ((tt != m_db.tableEnd()) && (recordCount == 0)) {
      query.prepare(QString("select count(*) from %1;").arg((*tt).name()));
      if (!query.exec()) throw MYMONEYEXCEPTIONSQL("getting record count"); // krazy:exclude=crashy
      if (!query.next()) throw MYMONEYEXCEPTIONSQL("retrieving record count");
      recordCount += query.value(0).toInt();
      ++tt;
    }

    // a fresh created database contains at least one record (see createTables()) in
    // the kmmFileInfo table providing file and fix version. So we report empty
    // even if there is a recordCount of 1
    if (recordCount > 1) {
      return -1;    // not empty
    } else {
      return 0;
    }
  }

  // for bug 252841
  QStringList tables(QSql::TableType tt)
  {
    Q_Q(MyMoneyStorageSql);
    return (m_driver->tables(tt, static_cast<const QSqlDatabase&>(*q)));
  }

  //! Returns 1 in case the @a column exists in @a table, 0 if not. In case of error, -1 is returned.
  int haveColumnInTable(const QString& table, const QString& column)
  {
    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);
    QString cmd = QString("SELECT * FROM %1 LIMIT 1").arg(table);
    if(!query.exec(cmd)) {
      buildError(query, Q_FUNC_INFO, QString("Error detecting if %1 exists in %2").arg(column).arg(table));
      return -1;
    }
    QSqlRecord rec = query.record();
    return (rec.indexOf(column) != -1) ? 1 : 0;
  }

  /**
   * @brief Ensure the storagePlugin with iid was setup
   *
   * @throws MyMoneyException in case of an error which makes the use
   * of the plugin unavailable.
   */
  bool setupStoragePlugin(QString iid)
  {
    Q_Q(MyMoneyStorageSql);
    // setupDatabase has to be called every time because this simple technique to check if was updated already
    // does not work if a user opens another file
    // also the setup is removed if the current database transaction is rolled back
    if (iid.isEmpty() /*|| m_loadedStoragePlugins.contains(iid)*/)
      return false;
    QString sqlIID;

    if (iid == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid())
      sqlIID = QString::fromLatin1("org.kmymoney.payeeIdentifier.ibanbic.sqlStoragePlugin");
    else if (iid == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid())
      sqlIID = QLatin1String("org.kmymoney.payeeIdentifier.nationalAccount.sqlStoragePlugin");
    else if (iid == sepaOnlineTransferImpl::name())
      sqlIID = QLatin1String("org.kmymoney.creditTransfer.sepa.sqlStoragePlugin");
    else
      return false;

    QString errorMsg;
    KMyMoneyPlugin::storagePlugin* plugin = KServiceTypeTrader::createInstanceFromQuery<KMyMoneyPlugin::storagePlugin>(
      QLatin1String("KMyMoney/sqlStoragePlugin"),
      QString("'%1' ~in [X-KMyMoney-PluginIid]").arg(sqlIID.replace(QLatin1Char('\''), QLatin1String("\\'"))),
      0,
      QVariantList(),
      &errorMsg
    );

    if (plugin == 0)
      throw MYMONEYEXCEPTION(QString::fromLatin1("Could not load sqlStoragePlugin '%1', (error: %2)").arg(sqlIID, errorMsg));

    MyMoneyDbTransaction t(*q, Q_FUNC_INFO);
    if (plugin->setupDatabase(*q)) {
      m_loadedStoragePlugins.insert(sqlIID);
      return true;
    }

    throw MYMONEYEXCEPTION(QString::fromLatin1("Could not install sqlStoragePlugin '%1' in database.").arg(sqlIID));
  }

  bool actOnIBANBICObjectInSQL(SQLAction action, const payeeIdentifier &obj)
  {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> payeeIdentifier = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(obj);

    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);

    auto writeQuery = [&]() {
      query.bindValue(":id", obj.idString());
      query.bindValue(":iban", payeeIdentifier->electronicIban());
      const auto bic = payeeIdentifier->fullStoredBic();
      query.bindValue(":bic", (bic.isEmpty()) ? QVariant(QVariant::String) : bic);
      query.bindValue(":name", payeeIdentifier->ownerName());
      if (!query.exec()) { // krazy:exclude=crashy
        qWarning("Error while saving ibanbic data for '%s': %s", qPrintable(obj.idString()), qPrintable(query.lastError().text()));
        return false;
      }
      return true;
    };

    switch(action) {
      case SQLAction::Save:
        query.prepare("INSERT INTO kmmIbanBic "
                      " ( id, iban, bic, name )"
                      " VALUES( :id, :iban, :bic, :name ) "
                     );
        return writeQuery();

      case SQLAction::Modify:
        query.prepare("UPDATE kmmIbanBic SET iban = :iban, bic = :bic, name = :name WHERE id = :id;");
        return writeQuery();

      case SQLAction::Remove:
        query.prepare("DELETE FROM kmmIbanBic WHERE id = ?;");
        query.bindValue(0, obj.idString());
        if (!query.exec()) {
          qWarning("Error while deleting ibanbic data '%s': %s", qPrintable(obj.idString()), qPrintable(query.lastError().text()));
          return false;
        }
        return true;
    }
    return false;
  }

  bool actOnNationalAccountObjectInSQL(SQLAction action, const payeeIdentifier &obj)
  {
    payeeIdentifierTyped<payeeIdentifiers::nationalAccount> payeeIdentifier = payeeIdentifierTyped<payeeIdentifiers::nationalAccount>(obj);

    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);

    auto writeQuery = [&]() {
      query.bindValue(":id", obj.idString());
      query.bindValue(":countryCode", payeeIdentifier->country());
      query.bindValue(":accountNumber", payeeIdentifier->accountNumber());
      query.bindValue(":bankCode", (payeeIdentifier->bankCode().isEmpty()) ? QVariant(QVariant::String) : payeeIdentifier->bankCode());
      query.bindValue(":name", payeeIdentifier->ownerName());
      if (!query.exec()) { // krazy:exclude=crashy
        qWarning("Error while saving national account number for '%s': %s", qPrintable(obj.idString()), qPrintable(query.lastError().text()));
        return false;
      }
      return true;

    };

    switch(action) {
      case SQLAction::Save:
        query.prepare("INSERT INTO kmmNationalAccountNumber "
                      " ( id, countryCode, accountNumber, bankCode, name )"
                      " VALUES( :id, :countryCode, :accountNumber, :bankCode, :name ) "
                     );
        return writeQuery();

      case SQLAction::Modify:
        query.prepare("UPDATE kmmNationalAccountNumber SET countryCode = :countryCode, accountNumber = :accountNumber, bankCode = :bankCode, name = :name WHERE id = :id;");
        return writeQuery();

      case SQLAction::Remove:
        query.prepare("DELETE FROM kmmNationalAccountNumber WHERE id = ?;");
        query.bindValue(0, obj.idString());
        if (!query.exec()) {
          qWarning("Error while deleting national account number '%s': %s", qPrintable(obj.idString()), qPrintable(query.lastError().text()));
          return false;
        }
        return true;
    }
    return false;
  }

  bool actOnSepaOnlineTransferObjectInSQL(SQLAction action, const onlineTask &obj, const QString& id)
  {
    Q_Q(MyMoneyStorageSql);
    QSqlQuery query(*q);
    const auto& task = dynamic_cast<const sepaOnlineTransferImpl &>(obj);

    auto bindValuesToQuery = [&]() {
      query.bindValue(":id", id);
      query.bindValue(":originAccount", task.responsibleAccount());
      query.bindValue(":value", task.value().toString());
      query.bindValue(":purpose", task.purpose());
      query.bindValue(":endToEndReference", (task.endToEndReference().isEmpty()) ? QVariant() : QVariant::fromValue(task.endToEndReference()));
      query.bindValue(":beneficiaryName", task.beneficiaryTyped().ownerName());
      query.bindValue(":beneficiaryIban", task.beneficiaryTyped().electronicIban());
      query.bindValue(":beneficiaryBic", (task.beneficiaryTyped().storedBic().isEmpty()) ? QVariant() : QVariant::fromValue(task.beneficiaryTyped().storedBic()));
      query.bindValue(":textKey", task.textKey());
      query.bindValue(":subTextKey", task.subTextKey());
    };

    switch(action) {
      case SQLAction::Save:
        query.prepare("INSERT INTO kmmSepaOrders ("
                      " id, originAccount, value, purpose, endToEndReference, beneficiaryName, beneficiaryIban, "
                      " beneficiaryBic, textKey, subTextKey) "
                      " VALUES( :id, :originAccount, :value, :purpose, :endToEndReference, :beneficiaryName, :beneficiaryIban, "
                      "         :beneficiaryBic, :textKey, :subTextKey ) "
                     );
        bindValuesToQuery();
        if (!query.exec()) {
          qWarning("Error while saving sepa order '%s': %s", qPrintable(id), qPrintable(query.lastError().text()));
          return false;
        }
        return true;

      case SQLAction::Modify:
        query.prepare(
          "UPDATE kmmSepaOrders SET"
          " originAccount = :originAccount,"
          " value = :value,"
          " purpose = :purpose,"
          " endToEndReference = :endToEndReference,"
          " beneficiaryName = :beneficiaryName,"
          " beneficiaryIban = :beneficiaryIban,"
          " beneficiaryBic = :beneficiaryBic,"
          " textKey = :textKey,"
          " subTextKey = :subTextKey "
          " WHERE id = :id");
        bindValuesToQuery();
        if (!query.exec()) {
          qWarning("Could not modify sepaOnlineTransfer '%s': %s", qPrintable(id), qPrintable(query.lastError().text()));
          return false;
        }
        return true;

      case SQLAction::Remove:
        query.prepare("DELETE FROM kmmSepaOrders WHERE id = ?");
        query.bindValue(0, id);
        return query.exec();
    }
    return false;
  }

  void actOnPayeeIdentifierObjectInSQL(SQLAction action, const payeeIdentifier& obj)
  {
    setupStoragePlugin(obj->payeeIdentifierId());
    auto isSuccessfull = false;

    if (obj->payeeIdentifierId() == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid())
      isSuccessfull = actOnIBANBICObjectInSQL(action, obj);
    else if (obj->payeeIdentifierId() == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid())
      isSuccessfull = actOnNationalAccountObjectInSQL(action, obj);

    if (!isSuccessfull) {
      switch (action) {
        case SQLAction::Save:
          throw MYMONEYEXCEPTION(QString::fromLatin1("Could not save object with id '%1' in database (plugin failed).").arg(obj.idString()));
        case SQLAction::Modify:
          throw MYMONEYEXCEPTION(QString::fromLatin1("Could not modify object with id '%1' in database (plugin failed).").arg(obj.idString()));
        case SQLAction::Remove:
          throw MYMONEYEXCEPTION(QString::fromLatin1("Could not remove object with id '%1' from database (plugin failed).").arg(obj.idString()));
      }
    }
  }

  void actOnOnlineJobInSQL(SQLAction action, const onlineTask& obj, const QString& id)
  {
    setupStoragePlugin(obj.taskName());
    auto isSuccessfull = false;

    if (obj.taskName() == sepaOnlineTransferImpl::name())
      isSuccessfull = actOnSepaOnlineTransferObjectInSQL(action, obj, id);

    if (!isSuccessfull) {
      switch (action) {
        case SQLAction::Save:
          throw MYMONEYEXCEPTION(QString::fromLatin1("Could not save object with id '%1' in database (plugin failed).").arg(id));
        case SQLAction::Modify:
          throw MYMONEYEXCEPTION(QString::fromLatin1("Could not modify object with id '%1' in database (plugin failed).").arg(id));
        case SQLAction::Remove:
          throw MYMONEYEXCEPTION(QString::fromLatin1("Could not remove object with id '%1' from database (plugin failed).").arg(id));
      }
    }
  }

  payeeIdentifierData* createIBANBICObject(QSqlDatabase db, const QString& identId) const
  {
    QSqlQuery query(db);
    query.prepare("SELECT iban, bic, name FROM kmmIbanBic WHERE id = ?;");
    query.bindValue(0, identId);
    if (!query.exec() || !query.next()) {
      qWarning("Could load iban bic identifier from database");
      return nullptr;
    }

    payeeIdentifiers::ibanBic *const ident = new payeeIdentifiers::ibanBic;
    ident->setIban(query.value(0).toString());
    ident->setBic(query.value(1).toString());
    ident->setOwnerName(query.value(2).toString());
    return ident;
  }

  payeeIdentifierData* createNationalAccountObject(QSqlDatabase db, const QString& identId) const
  {
    QSqlQuery query(db);
    query.prepare("SELECT countryCode, accountNumber, bankCode, name FROM kmmNationalAccountNumber WHERE id = ?;");
    query.bindValue(0, identId);
    if (!query.exec() || !query.next()) {
      qWarning("Could load national account number from database");
      return nullptr;
    }

    payeeIdentifiers::nationalAccount *const ident = new payeeIdentifiers::nationalAccount;
    ident->setCountry(query.value(0).toString());
    ident->setAccountNumber(query.value(1).toString());
    ident->setBankCode(query.value(2).toString());
    ident->setOwnerName(query.value(3).toString());
    return ident;
  }

  payeeIdentifier createPayeeIdentifierObject(QSqlDatabase db, const QString& identifierType, const QString& identifierId) const
  {
    payeeIdentifierData* identData = nullptr;
    if (identifierType == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid())
      identData = createIBANBICObject(db, identifierId);
    else if (identifierType == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid())
      identData = createNationalAccountObject(db, identifierId);

    return payeeIdentifier(identifierId, identData);
  }

  onlineTask* createSepaOnlineTransferObject(QSqlDatabase connection, const QString& onlineJobId) const
  {
    Q_ASSERT(!onlineJobId.isEmpty());
    Q_ASSERT(connection.isOpen());

    QSqlQuery query = QSqlQuery(
                        "SELECT originAccount, value, purpose, endToEndReference, beneficiaryName, beneficiaryIban, "
                        " beneficiaryBic, textKey, subTextKey FROM kmmSepaOrders WHERE id = ?",
                        connection
                      );
    query.bindValue(0, onlineJobId);
    if (query.exec() && query.next()) {
      sepaOnlineTransferImpl* task = new sepaOnlineTransferImpl();
      task->setOriginAccount(query.value(0).toString());
      task->setValue(MyMoneyMoney(query.value(1).toString()));
      task->setPurpose(query.value(2).toString());
      task->setEndToEndReference(query.value(3).toString());
      task->setTextKey(query.value(7).toUInt());
      task->setSubTextKey(query.value(8).toUInt());

      payeeIdentifiers::ibanBic beneficiary;
      beneficiary.setOwnerName(query.value(4).toString());
      beneficiary.setIban(query.value(5).toString());
      beneficiary.setBic(query.value(6).toString());
      task->setBeneficiary(beneficiary);
      return task;
    }

    return nullptr;
  }

  onlineTask* createOnlineTaskObject(const QString& iid, const QString& onlineTaskId, QSqlDatabase connection) const
  {
    onlineTask* taskOnline = nullptr;
    if (iid == sepaOnlineTransferImpl::name()) {
      // @todo This is probably memory leak but for now it works alike to original code
      onlineJobAdministration::instance()->registerOnlineTask(new sepaOnlineTransferImpl);
      taskOnline = createSepaOnlineTransferObject(connection, onlineTaskId);
    }
    if (!taskOnline)
      qWarning("In the file is a onlineTask for which I could not find the plugin ('%s')", qPrintable(iid));

    return taskOnline;
  }

  void alert(QString s) const // FIXME: remove...
  {
    qDebug() << s;
  }

  void signalProgress(qint64 current, qint64 total, const QString& msg) const
  {
    if (m_progressCallback != 0)
      (*m_progressCallback)(current, total, msg);
  }

  void signalProgress(qint64 current, qint64 total) const
  {
    signalProgress(current, total, QString());
  }

  template<ulong MyMoneyStorageSqlPrivate::* cache>
  ulong getNextId(const QString& table, const QString& id, const int prefixLength) const
  {
    Q_CHECK_PTR(cache);
    if (this->*cache == 0) {
      MyMoneyStorageSqlPrivate* nonConstThis = const_cast<MyMoneyStorageSqlPrivate*>(this);
      nonConstThis->*cache = 1 + nonConstThis->highestNumberFromIdString(table, id, prefixLength);
    }
    Q_ASSERT(this->*cache > 0); // everything else is never a valid id
    return this->*cache;
  }

  //void startCommitUnit (const QString& callingFunction);
  //void endCommitUnit (const QString& callingFunction);
  //void cancelCommitUnit (const QString& callingFunction);

  MyMoneyStorageSql *q_ptr;

  // data
  QExplicitlySharedDataPointer<MyMoneyDbDriver> m_driver;

  MyMoneyDbDef m_db;
  uint m_dbVersion;
  MyMoneyStorageMgr *m_storage;
  // input options
  bool m_loadAll; // preload all data
  bool m_override; // override open if already in use
  // error message
  QString m_error;
  // record counts
  ulong m_institutions;
  ulong m_accounts;
  ulong m_payees;
  ulong m_tags;
  ulong m_transactions;
  ulong m_splits;
  ulong m_securities;
  ulong m_prices;
  ulong m_currencies;
  ulong m_schedules;
  ulong m_reports;
  ulong m_kvps;
  ulong m_budgets;
  ulong m_onlineJobs;
  ulong m_payeeIdentifier;

  // Cache for next id to use
  // value 0 means data is not available and has to be loaded from the database
  ulong m_hiIdInstitutions;
  ulong m_hiIdPayees;
  ulong m_hiIdTags;
  ulong m_hiIdAccounts;
  ulong m_hiIdTransactions;
  ulong m_hiIdSchedules;
  ulong m_hiIdSecurities;
  ulong m_hiIdReports;
  ulong m_hiIdBudgets;
  ulong m_hiIdOnlineJobs;
  ulong m_hiIdPayeeIdentifier;
  ulong m_hiIdCostCenter;

  // encrypt option - usage TBD
  QString m_encryptData;

  /**
    * This variable is used to suppress status messages except during
   * initial data load and final write

  */
  bool m_displayStatus;

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
  QHash<QString, ulong> m_transactionCountMap;
  /**
    * These member variables hold the user name and date/time of logon
    */
  QString m_logonUser;
  QDateTime m_logonAt;
  QDate m_txPostDate; // FIXME: remove when Tom puts date into split object

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

  void (*m_progressCallback)(int, int, const QString&);
};
#endif
