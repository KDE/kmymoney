/***************************************************************************
                          mymoneydatabasemgr.cpp
                             -------------------
    begin                : June 5 2007
    copyright            : (C) 2007 by Fernando Vilas
    email                : Fernando Vilas <fvilas@iname.com>
                           2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneydatabasemgr_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
#include "mymoneyprice.h"
#include "onlinejob.h"
#include "mymoneystoragenames.h"

using namespace eStorage;
using namespace MyMoneyStandardAccounts;

const int INSTITUTION_ID_SIZE = 6;
const int ACCOUNT_ID_SIZE = 6;
const int TRANSACTION_ID_SIZE = 18;
const int PAYEE_ID_SIZE = 6;
const int TAG_ID_SIZE = 6;
const int SCHEDULE_ID_SIZE = 6;
const int SECURITY_ID_SIZE = 6;
const int REPORT_ID_SIZE = 6;
const int BUDGET_ID_SIZE = 6;
const int ONLINEJOB_ID_SIZE = 8;
const int PAYEEIDENTIFIER_ID_SIZE = 6;
const int COSTCENTER_ID_SIZE = 6;

// Increment this to force an update in KMMView.
// This is different from the db schema version stored in
// MMStorageSql::m_majorVersion
const int CURRENT_FIX_VERSION = 4;

MyMoneyDatabaseMgr::MyMoneyDatabaseMgr() :
  d_ptr(new MyMoneyDatabaseMgrPrivate(this))
{
}

MyMoneyDatabaseMgr::~MyMoneyDatabaseMgr()
{
  Q_D(MyMoneyDatabaseMgr);
  delete d;
}

// general get functions
MyMoneyPayee MyMoneyDatabaseMgr::user() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_user;
}

QDate MyMoneyDatabaseMgr::creationDate() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_creationDate;
}

QDate MyMoneyDatabaseMgr::lastModificationDate() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_lastModificationDate;
}

uint MyMoneyDatabaseMgr::currentFixVersion() const
{
  return CURRENT_FIX_VERSION;
}

uint MyMoneyDatabaseMgr::fileFixVersion() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_fileFixVersion;
}

// general set functions
void MyMoneyDatabaseMgr::setUser(const MyMoneyPayee& user)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_user = user;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    d->m_sql->modifyUserInfo(user);
  }
}

void MyMoneyDatabaseMgr::setFileFixVersion(uint v)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_fileFixVersion = v;
}

// methods provided by MyMoneyKeyValueContainer
QString MyMoneyDatabaseMgr::value(const QString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneyDatabaseMgr::setValue(const QString& key, const QString& val)
{
  MyMoneyKeyValueContainer::setValue(key, val);
}

void MyMoneyDatabaseMgr::deletePair(const QString& key)
{
  MyMoneyKeyValueContainer::deletePair(key);
}

QMap<QString, QString> MyMoneyDatabaseMgr::pairs() const
{
  return MyMoneyKeyValueContainer::pairs();
}

void MyMoneyDatabaseMgr::setPairs(const QMap<QString, QString>& list)
{
  MyMoneyKeyValueContainer::setPairs(list);
}

//MyMoneyDatabaseMgr const * MyMoneyDatabaseMgr::duplicate()
//{
//  MyMoneyDatabaseMgr* that = new MyMoneyDatabaseMgr();
//  *that = *this;
//  return that;
//}

void MyMoneyDatabaseMgr::addAccount(MyMoneyAccount& account)
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();

    // create the account.
    MyMoneyAccount newAccount(nextAccountID(), account);

    d->m_sql->addAccount(newAccount);
    account = newAccount;
  }
}

void MyMoneyDatabaseMgr::addAccount(MyMoneyAccount& parent, MyMoneyAccount& account)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyAccount> accountList;
  QStringList accountIdList;
  QMap<QString, MyMoneyAccount>::ConstIterator theParent;
  QMap<QString, MyMoneyAccount>::ConstIterator theChild;

  accountIdList << parent.id() << account.id();
  startTransaction();
  accountList = d->m_sql->fetchAccounts(accountIdList, true);

  theParent = accountList.constFind(parent.id());
  if (theParent == accountList.constEnd()) {
    QString msg = "Unknown parent account '";
    msg += parent.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  theChild = accountList.constFind(account.id());
  if (theChild == accountList.constEnd()) {
    QString msg = "Unknown child account '";
    msg += account.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  MyMoneyAccount acc = *theParent;
  acc.addAccountId(account.id());
  parent = acc;

  acc = *theChild;
  acc.setParentAccountId(parent.id());
  account = acc;

//FIXME:  MyMoneyBalanceCacheItem balance;
//FIXME:  m_balanceCache[account.id()] = balance;

  QList<MyMoneyAccount> aList;
  aList << parent << account;
  d->m_sql->modifyAccountList(aList);
  commitTransaction();
}

void MyMoneyDatabaseMgr::addPayee(MyMoneyPayee& payee)
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())
      static_cast<QSqlDatabase*>(d->m_sql.data())->open();
    // create the payee
    MyMoneyPayee newPayee(nextPayeeID(), payee);

    d->m_sql->addPayee(newPayee);
    payee = newPayee;
  }
}

MyMoneyPayee MyMoneyDatabaseMgr::payee(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyPayee>::ConstIterator it;
  QMap<QString, MyMoneyPayee> payeeList = d->m_sql->fetchPayees(QStringList(id));
  it = payeeList.constFind(id);
  if (it == payeeList.constEnd())
    throw MYMONEYEXCEPTION("Unknown payee '" + id + '\'');

  return *it;
}

MyMoneyPayee MyMoneyDatabaseMgr::payeeByName(const QString& payee) const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (payee.isEmpty())
    return MyMoneyPayee::null;

  QMap<QString, MyMoneyPayee> payeeList;

  try {
    payeeList = d->m_sql->fetchPayees();
  } catch (const MyMoneyException &) {
    throw;
  }

  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  for (it_p = payeeList.constBegin(); it_p != payeeList.constEnd(); ++it_p) {
    if ((*it_p).name() == payee) {
      return *it_p;
    }
  }

  throw MYMONEYEXCEPTION("Unknown payee '" + payee + '\'');
}

void MyMoneyDatabaseMgr::modifyPayee(const MyMoneyPayee& payee)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyPayee> payeeList = d->m_sql->fetchPayees(QStringList(payee.id()), true);
  QMap<QString, MyMoneyPayee>::ConstIterator it;

  it = payeeList.constFind(payee.id());
  if (it == payeeList.constEnd()) {
    QString msg = "Unknown payee '" + payee.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->modifyPayee(payee);
}

void MyMoneyDatabaseMgr::removePayee(const MyMoneyPayee& payee)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneySchedule>::ConstIterator it_s;
  QMap<QString, MyMoneyPayee> payeeList = d->m_sql->fetchPayees(QStringList(payee.id()));
  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  it_p = payeeList.constFind(payee.id());
  if (it_p == payeeList.constEnd()) {
    QString msg = "Unknown payee '" + payee.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the payee is still referenced

  QMap<QString, MyMoneyTransaction> transactionList = d->m_sql->fetchTransactions(); // make sure they're all here
  for (it_t = transactionList.constBegin(); it_t != transactionList.constEnd(); ++it_t) {
    if ((*it_t).hasReferenceTo(payee.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove payee that is still referenced to a %1").arg("transaction"));
    }
  }

  // check referential integrity in schedules
  QMap<QString, MyMoneySchedule> scheduleList = d->m_sql->fetchSchedules(); // make sure they're all here
  for (it_s = scheduleList.constBegin(); it_s != scheduleList.constEnd(); ++it_s) {
    if ((*it_s).hasReferenceTo(payee.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove payee that is still referenced to a %1").arg("schedule"));
    }
  }
  // remove any reference to report and/or budget
  d->removeReferences(payee.id());

  d->m_sql->removePayee(payee);
}

QList<MyMoneyPayee> MyMoneyDatabaseMgr::payeeList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchPayees().values();
  } else
    return QList<MyMoneyPayee> ();
}

void MyMoneyDatabaseMgr::addTag(MyMoneyTag& tag)
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    // create the tag
    MyMoneyTag newTag(nextTagID(), tag);

    d->m_sql->addTag(newTag);
    tag = newTag;
  }
}

MyMoneyTag MyMoneyDatabaseMgr::tag(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyTag>::ConstIterator it;
  QMap<QString, MyMoneyTag> tagList = d->m_sql->fetchTags(QStringList(id));
  it = tagList.constFind(id);
  if (it == tagList.constEnd())
    throw MYMONEYEXCEPTION("Unknown tag '" + id + '\'');

  return *it;
}

MyMoneyTag MyMoneyDatabaseMgr::tagByName(const QString& tag) const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (tag.isEmpty())
    return MyMoneyTag::null;

  QMap<QString, MyMoneyTag> tagList;

  try {
    tagList = d->m_sql->fetchTags();
  } catch (const MyMoneyException &) {
    throw;
  }

  QMap<QString, MyMoneyTag>::ConstIterator it_ta;

  for (it_ta = tagList.constBegin(); it_ta != tagList.constEnd(); ++it_ta) {
    if ((*it_ta).name() == tag) {
      return *it_ta;
    }
  }

  throw MYMONEYEXCEPTION("Unknown tag '" + tag + '\'');
}

void MyMoneyDatabaseMgr::modifyTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyTag> tagList = d->m_sql->fetchTags(QStringList(tag.id()), true);
  QMap<QString, MyMoneyTag>::ConstIterator it;

  it = tagList.constFind(tag.id());
  if (it == tagList.constEnd()) {
    QString msg = "Unknown tag '" + tag.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->modifyTag(tag);
}

void MyMoneyDatabaseMgr::removeTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneySchedule>::ConstIterator it_s;
  QMap<QString, MyMoneyTag> tagList = d->m_sql->fetchTags(QStringList(tag.id()));
  QMap<QString, MyMoneyTag>::ConstIterator it_ta;

  it_ta = tagList.constFind(tag.id());
  if (it_ta == tagList.constEnd()) {
    QString msg = "Unknown tag '" + tag.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the tag is still referenced

  QMap<QString, MyMoneyTransaction> transactionList = d->m_sql->fetchTransactions(); // make sure they're all here
  for (it_t = transactionList.constBegin(); it_t != transactionList.constEnd(); ++it_t) {
    if ((*it_t).hasReferenceTo(tag.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove tag that is still referenced to a %1").arg("transaction"));
    }
  }

  // check referential integrity in schedules
  QMap<QString, MyMoneySchedule> scheduleList = d->m_sql->fetchSchedules(); // make sure they're all here
  for (it_s = scheduleList.constBegin(); it_s != scheduleList.constEnd(); ++it_s) {
    if ((*it_s).hasReferenceTo(tag.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove tag that is still referenced to a %1").arg("schedule"));
    }
  }
  // remove any reference to report and/or budget
  d->removeReferences(tag.id());

  d->m_sql->removeTag(tag);
}

QList<MyMoneyTag> MyMoneyDatabaseMgr::tagList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchTags().values();
  } else {
    return QList<MyMoneyTag> ();
  }
}

void MyMoneyDatabaseMgr::modifyOnlineJob(const onlineJob& job)
{
  Q_D(MyMoneyDatabaseMgr);
  if (job.id().isEmpty())
    throw MYMONEYEXCEPTION("empty online job id");
  d->m_sql->modifyOnlineJob(job);
}

void MyMoneyDatabaseMgr::addOnlineJob(onlineJob& job)
{
  Q_D(MyMoneyDatabaseMgr);
  job = onlineJob(nextOnlineJobID(), job);
  d->m_sql->addOnlineJob(job);
}

onlineJob MyMoneyDatabaseMgr::getOnlineJob(const QString &jobId) const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (jobId.isEmpty())
    throw MYMONEYEXCEPTION("empty online job id");

  if (d->m_sql) {
    if (! d->m_sql->isOpen())
      ((QSqlDatabase*)(d->m_sql.data()))->open();

    QMap <QString, onlineJob> jobList = d->m_sql->fetchOnlineJobs(QStringList(jobId));
    QMap <QString, onlineJob>::ConstIterator pos = jobList.constFind(jobId);

    // locate the account and if present, return it's data
    if (pos != jobList.constEnd())
      return *pos;
  } else {
    throw MYMONEYEXCEPTION("No database connected");
  }

  // throw an exception, if it does not exist
  throw MYMONEYEXCEPTION(QLatin1String("Unknown online job id '") + jobId + QLatin1Char('\''));
}

QList<onlineJob> MyMoneyDatabaseMgr::onlineJobList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (!d->m_sql->isOpen())
      ((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchOnlineJobs().values();
  }
  return QList<onlineJob>();
}

void MyMoneyDatabaseMgr::removeOnlineJob(const onlineJob& job)
{
  Q_D(MyMoneyDatabaseMgr);
  if (job.id().isEmpty())
    throw MYMONEYEXCEPTION("Empty online job id during remove.");

  d->m_sql->removeOnlineJob(job);
}

MyMoneyAccount MyMoneyDatabaseMgr::account(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (id.isEmpty()) {
    throw MYMONEYEXCEPTION("empty account id");
  }

  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    QMap <QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts(QStringList(id));
    QMap <QString, MyMoneyAccount>::ConstIterator pos = accountList.constFind(id);

    // locate the account and if present, return it's data
    if (pos != accountList.constEnd())
      return *pos;
  } else {
    throw MYMONEYEXCEPTION("No database connected");
  }

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + '\'';
  throw MYMONEYEXCEPTION(msg);
}

bool MyMoneyDatabaseMgr::isStandardAccount(const QString& id) const
{
  return id == stdAccNames[stdAccLiability]
         || id == stdAccNames[stdAccAsset]
         || id == stdAccNames[stdAccExpense]
         || id == stdAccNames[stdAccIncome]
         || id == stdAccNames[stdAccEquity];
}

void MyMoneyDatabaseMgr::setAccountName(const QString& id, const QString& name)
{
  Q_D(MyMoneyDatabaseMgr);
  if (!isStandardAccount(id))
    throw MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    startTransaction();
    MyMoneyAccount acc = d->m_sql->fetchAccounts(QStringList(id), true)[id];
    acc.setName(name);
    d->m_sql->modifyAccount(acc);
    commitTransaction();
  }
}

void MyMoneyDatabaseMgr::addInstitution(MyMoneyInstitution& institution)
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    MyMoneyInstitution newInstitution(nextInstitutionID(), institution);

    // mark file as changed
    d->m_sql->addInstitution(newInstitution);

    // return new data
    institution = newInstitution;
  }
}

QString MyMoneyDatabaseMgr::nextPayeeID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementPayeeId()));
    id = 'P' + id.rightJustified(PAYEE_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextTagID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementTagId()));
    id = 'G' + id.rightJustified(TAG_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextInstitutionID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementInstitutionId()));
    id = 'I' + id.rightJustified(INSTITUTION_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextAccountID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementAccountId()));
    id = 'A' + id.rightJustified(ACCOUNT_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextBudgetID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementBudgetId()));
    id = 'B' + id.rightJustified(BUDGET_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextReportID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementReportId()));
    id = 'R' + id.rightJustified(REPORT_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextTransactionID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementTransactionId()));
    id = 'T' + id.rightJustified(TRANSACTION_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextScheduleID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementScheduleId()));
    id = "SCH" + id.rightJustified(SCHEDULE_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextSecurityID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(ulong(d->m_sql->incrementSecurityId()));
    id = 'E' + id.rightJustified(SECURITY_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextOnlineJobID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(d->m_sql->incrementOnlineJobId());
    id = QLatin1Char('O') + id.rightJustified(ONLINEJOB_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextPayeeIdentifierID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(d->m_sql->incrementPayeeIdentfierId());
    id = QLatin1String("IDENT") + id.rightJustified(PAYEEIDENTIFIER_ID_SIZE, '0');
  }
  return id;
}

QString MyMoneyDatabaseMgr::nextCostCenterID()
{
  Q_D(MyMoneyDatabaseMgr);
  QString id;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    id.setNum(d->m_sql->incrementCostCenterId());
    id = QLatin1Char('C') + id.rightJustified(COSTCENTER_ID_SIZE, '0');
  }
  return id;
}

void MyMoneyDatabaseMgr::addTransaction(MyMoneyTransaction& transaction, bool skipAccountUpdate)
{
  Q_D(MyMoneyDatabaseMgr);
  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if (!transaction.id().isEmpty())
    throw MYMONEYEXCEPTION("transaction already contains an id");
  if (!transaction.postDate().isValid())
    throw MYMONEYEXCEPTION("invalid post date");

  // now check the splits
  foreach (const MyMoneySplit& it_s, transaction.splits()) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account(it_s.accountId());
    if (!it_s.payeeId().isEmpty())
      payee(it_s.payeeId());
  }

  MyMoneyTransaction newTransaction(nextTransactionID(), transaction);
  QString key = newTransaction.uniqueSortKey();

  d->m_sql->addTransaction(newTransaction);

  transaction = newTransaction;

  QList<MyMoneyAccount> aList;
  // adjust the balance of all affected accounts
  foreach (const MyMoneySplit& it_s, transaction.splits()) {
    MyMoneyAccount acc = account(it_s.accountId());
    acc.adjustBalance(it_s);
    if (!skipAccountUpdate) {
      acc.touch();
//FIXME:      invalidateBalanceCache(acc.id());
    }
    aList << acc;
  }
  d->m_sql->modifyAccountList(aList);
}

bool MyMoneyDatabaseMgr::hasActiveSplits(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyTransaction>::ConstIterator it;

  MyMoneyTransactionFilter f(id);
  QMap<QString, MyMoneyTransaction> transactionList = d->m_sql->fetchTransactions(f);

  for (it = transactionList.constBegin(); it != transactionList.constEnd(); ++it) {
    if ((*it).accountReferenced(id)) {
      return true;
    }
  }
  return false;
}

/**
  * This method is used to return the actual balance of an account
  * without it's sub-ordinate accounts. If a @p date is presented,
  * the balance at the beginning of this date (not including any
  * transaction on this date) is returned. Otherwise all recorded
  * transactions are included in the balance.
  *
  * @param id id of the account in question
  * @param date return balance for specific date
  * @return balance of the account as MyMoneyMoney object
  */
//MyMoneyMoney MyMoneyDatabaseMgr::balance(const QString& id, const QDate& date);

MyMoneyMoney MyMoneyDatabaseMgr::totalBalance(const QString& id, const QDate& date) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QStringList accounts;

  MyMoneyMoney result; //(balance(id, date));

  accounts = MyMoneyFile::instance()->account(id).accountList();
  foreach (const QString& it_a, accounts) {
    if (!it_a.isEmpty()) {
      accounts << MyMoneyFile::instance()->account(it_a).accountList();
    }
  }

  // convert into a sorted list with each account only once
  QMap<QString, bool> tempList;
  foreach (const QString& it_a, accounts) {
    tempList[it_a] = true;
  }
  accounts = tempList.uniqueKeys();

  QMap<QString, MyMoneyMoney> balanceMap = d->m_sql->fetchBalance(accounts, date);
  for (QMap<QString, MyMoneyMoney>::ConstIterator it_b = balanceMap.constBegin(); it_b != balanceMap.constEnd(); ++it_b) {
    result += it_b.value();
  }

  return result;
}

MyMoneyInstitution MyMoneyDatabaseMgr::institution(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;
  QMap<QString, MyMoneyInstitution> institutionList = d->m_sql->fetchInstitutions(QStringList(id));

  pos = institutionList.constFind(id);
  if (pos != institutionList.constEnd())
    return *pos;
  throw MYMONEYEXCEPTION("unknown institution");
}

bool MyMoneyDatabaseMgr::dirty() const
{
  return false;
}

void MyMoneyDatabaseMgr::setDirty()
{}

uint MyMoneyDatabaseMgr::accountCount() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getRecCount("kmmAccounts");
}

QList<MyMoneyInstitution> MyMoneyDatabaseMgr::institutionList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchInstitutions().values();
  } else {
    return QList<MyMoneyInstitution> ();
  }
}

void MyMoneyDatabaseMgr::modifyAccount(const MyMoneyAccount& account, bool skipCheck)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyAccount>::ConstIterator pos;

  // locate the account in the file global pool
  startTransaction();
  QMap<QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts(QStringList(account.id()), true);
  pos = accountList.constFind(account.id());
  if (pos != accountList.constEnd()) {
    // check if the new info is based on the old one.
    // this is the case, when the file and the id
    // as well as the type are equal.
    if (((*pos).parentAccountId() == account.parentAccountId()
         && ((*pos).accountType() == account.accountType()
             || ((*pos).isLiquidAsset() && account.isLiquidAsset())))
        || skipCheck == true) {
      // make sure that all the referenced objects exist
      if (!account.institutionId().isEmpty())
        institution(account.institutionId());

      //FIXME: fetch the whole list at once
      foreach (const QString& it_a, account.accountList()) {
        this->account(it_a);
      }

      // update information in account list
      //m_accountList.modify(account.id(), account);

      // invalidate cached balance
//FIXME:      invalidateBalanceCache(account.id());

      // mark file as changed
      d->m_sql->modifyAccount(account);
      commitTransaction();
    } else {
      rollbackTransaction();
      throw MYMONEYEXCEPTION("Invalid information for update");
    }

  } else {
    rollbackTransaction();
    throw MYMONEYEXCEPTION("Unknown account id");
  }
}

void MyMoneyDatabaseMgr::modifyInstitution(const MyMoneyInstitution& institution)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyInstitution> institutionList = d->m_sql->fetchInstitutions(QStringList(institution.id()));
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;

  // locate the institution in the file global pool
  pos = institutionList.constFind(institution.id());
  if (pos != institutionList.constEnd()) {
    d->m_sql->modifyInstitution(institution);
  } else
    throw MYMONEYEXCEPTION("unknown institution");
}

/**
  * This method is used to update a specific transaction in the
  * transaction pool of the MyMoneyFile object
  *
  * An exception will be thrown upon error conditions.
  *
  * @param transaction reference to transaction to be changed
  */
void MyMoneyDatabaseMgr::modifyTransaction(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, bool> modifiedAccounts;

  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * ids are assigned
  // * the pointer to the MyMoneyFile object is not 0
  // * the date valid (must not be empty)
  // * the splits must have valid account ids

  // first perform all the checks
  if (transaction.id().isEmpty()
//  || transaction.file() != this
      || !transaction.postDate().isValid())
    throw MYMONEYEXCEPTION("invalid transaction to be modified");

  // now check the splits
  foreach (const MyMoneySplit& it_s, transaction.splits()) {
    // the following lines will throw an exception if the
    // account, payee or tags do not exist
    account(it_s.accountId());
    if (!it_s.payeeId().isEmpty())
      payee(it_s.payeeId());
    foreach (const QString& tagId, it_s.tagIdList()) {
      if (!tagId.isEmpty())
        tag(tagId);
    }
  }

  // new data seems to be ok. find old version of transaction
  // in our pool. Throw exception if unknown.
//  if(!m_transactionKeys.contains(transaction.id()))
//    throw MYMONEYEXCEPTION("invalid transaction id");

//  QString oldKey = m_transactionKeys[transaction.id()];
  QMap <QString, MyMoneyTransaction> transactionList = d->m_sql->fetchTransactions("('" + transaction.id() + "')");
//  if(transactionList.size() != 1)
//    throw MYMONEYEXCEPTION("invalid transaction key");

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

//  it_t = transactionList.find(oldKey);
  it_t = transactionList.constBegin();
  if (it_t == transactionList.constEnd())
    throw MYMONEYEXCEPTION("invalid transaction key");

  d->m_sql->modifyTransaction(transaction);

  // mark all accounts referenced in old and new transaction data
  // as modified
  QMap<QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts();
  QList<MyMoneyAccount> aList;
  foreach (const MyMoneySplit& it_s, (*it_t).splits()) {
    MyMoneyAccount acc = accountList[it_s.accountId()];
    acc.adjustBalance(it_s, true);
    acc.touch();
//FIXME:    invalidateBalanceCache(acc.id());
    //m_accountList.modify(acc.id(), acc);
    aList << acc;
    //modifiedAccounts[(*it_s).accountId()] = true;
  }
  d->m_sql->modifyAccountList(aList);
  aList.clear();
  foreach (const MyMoneySplit& it_s, transaction.splits()) {
    MyMoneyAccount acc = accountList[it_s.accountId()];
    acc.adjustBalance(it_s);
    acc.touch();
//FIXME:    invalidateBalanceCache(acc.id());
    //m_accountList.modify(acc.id(), acc);
    aList << acc;
    //modifiedAccounts[(*it_s).accountId()] = true;
  }
  d->m_sql->modifyAccountList(aList);

  // remove old transaction from lists
//  m_sql->removeTransaction(oldKey);

  // add new transaction to lists
// QString newKey = transaction.uniqueSortKey();
//  m_sql->insertTransaction(newKey, transaction);
  //m_transactionKeys.modify(transaction.id(), newKey);
}

void MyMoneyDatabaseMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  Q_D(MyMoneyDatabaseMgr);
  if (account.accountType() == eMyMoney::Account::Type::Stock && parent.accountType() != eMyMoney::Account::Type::Investment)
    throw MYMONEYEXCEPTION("Cannot move a stock acocunt into a non-investment account");

  QStringList accountIdList;
  QMap<QString, MyMoneyAccount>::ConstIterator oldParent;
  QMap<QString, MyMoneyAccount>::ConstIterator newParent;
  QMap<QString, MyMoneyAccount>::ConstIterator childAccount;

  // verify that accounts exist. If one does not,
  // an exception is thrown
  accountIdList << account.id() << parent.id();
  MyMoneyDatabaseMgr::account(account.id());
  MyMoneyDatabaseMgr::account(parent.id());

  if (!account.parentAccountId().isEmpty()) {
    accountIdList << account.parentAccountId();
  }

  startTransaction();
  QMap<QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts(accountIdList, true);

  if (!account.parentAccountId().isEmpty()) {
    MyMoneyDatabaseMgr::account(account.parentAccountId());
    oldParent = accountList.constFind(account.parentAccountId());
  }

  newParent = accountList.constFind(parent.id());
  childAccount = accountList.constFind(account.id());

  MyMoneyAccount acc;
  QList<MyMoneyAccount> aList;
  if (!account.parentAccountId().isEmpty()) {
    acc = (*oldParent);
    acc.removeAccountId(account.id());
    aList << acc;
  }

  parent = (*newParent);
  parent.addAccountId(account.id());

  account = (*childAccount);
  account.setParentAccountId(parent.id());

  aList << parent << account;
  d->m_sql->modifyAccountList(aList);
  commitTransaction();
}

void MyMoneyDatabaseMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, bool> modifiedAccounts;

  // first perform all the checks
  if (transaction.id().isEmpty())
    throw MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap<QString, QString>::ConstIterator it_k;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

//  it_k = m_transactionKeys.find(transaction.id());
//  if(it_k == m_transactionKeys.end())
//    throw MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap <QString, MyMoneyTransaction> transactionList = d->m_sql->fetchTransactions("('" + QString(transaction.id()) + "')");
//  it_t = transactionList.find(*it_k);
  it_t = transactionList.constBegin();
  if (it_t == transactionList.constEnd())
    throw MYMONEYEXCEPTION("invalid transaction key");

  // scan the splits and collect all accounts that need
  // to be updated after the removal of this transaction
  QMap<QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts();
  QList<MyMoneyAccount> aList;
  foreach (const MyMoneySplit& it_s, (*it_t).splits()) {
    MyMoneyAccount acc = accountList[it_s.accountId()];
//    modifiedAccounts[(*it_s).accountId()] = true;
    acc.adjustBalance(it_s, true);
    acc.touch();
    aList << acc;
//FIXME:    invalidateBalanceCache(acc.id());
  }
  d->m_sql->modifyAccountList(aList);

  // FIXME: check if any split is frozen and throw exception

  // remove the transaction from the two lists
  //m_transactionList.remove(*it_k);
//  m_transactionKeys.remove(transaction.id());

  // mark file as changed
  d->m_sql->removeTransaction(transaction);
}

uint MyMoneyDatabaseMgr::transactionCount(const QString& account) const
{
  Q_D(const MyMoneyDatabaseMgr);
  return (d->m_sql->transactionCount(account));
}

QMap<QString, ulong> MyMoneyDatabaseMgr::transactionCountMap() const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, ulong> retval;
  QHash<QString, ulong> hash = d->m_sql->transactionCountMap();

  for (QHash<QString, ulong>::ConstIterator i = hash.constBegin();
       i != hash.constEnd(); ++i) {
    retval[i.key()] = i.value();
  }
  return retval;
}

QList<MyMoneyTransaction> MyMoneyDatabaseMgr::transactionList(MyMoneyTransactionFilter& filter) const
{
  QList<MyMoneyTransaction> list;
  transactionList(list, filter);
  return list;
}

void MyMoneyDatabaseMgr::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  Q_D(const MyMoneyDatabaseMgr);
  list.clear();

  try {
    if (d->m_sql) {
      if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
      list = d->m_sql->fetchTransactions(filter).values();
    }
  } catch (const MyMoneyException &) {
    throw;
  }
}

void MyMoneyDatabaseMgr::transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  Q_D(const MyMoneyDatabaseMgr);
  list.clear();
  MyMoneyMap<QString, MyMoneyTransaction> transactionList;
  try {
    if (d->m_sql) {
      if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
      transactionList = d->m_sql->fetchTransactions(filter);
    }
  } catch (const MyMoneyException &) {
    throw;
  }

  for (const auto& transaction : transactionList)
    for (const auto& split : filter.matchingSplits(transaction))
      list.append(qMakePair(transaction, split));
}

void MyMoneyDatabaseMgr::removeAccount(const MyMoneyAccount& account)
{
  Q_D(MyMoneyDatabaseMgr);
  MyMoneyAccount parent;

  // check that the account and it's parent exist
  // this will throw an exception if the id is unknown
  MyMoneyDatabaseMgr::account(account.id());
  parent = MyMoneyDatabaseMgr::account(account.parentAccountId());

  // check that it's not one of the standard account groups
  if (isStandardAccount(account.id()))
    throw MYMONEYEXCEPTION("Unable to remove the standard account groups");

  if (hasActiveSplits(account.id())) {
    throw MYMONEYEXCEPTION("Unable to remove account with active splits");
  }

  // re-parent all sub-ordinate accounts to the parent of the account
  // to be deleted. First round check that all accounts exist, second
  // round do the re-parenting.
  foreach (const QString& it, account.accountList()) {
    MyMoneyDatabaseMgr::account(it);
  }

  // if one of the accounts did not exist, an exception had been
  // thrown and we would not make it until here.

  QStringList accountIdList;
  accountIdList << parent.id() << account.id();
  startTransaction();
  QMap<QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts(accountIdList, true);

  QMap<QString, MyMoneyAccount>::ConstIterator it_a;
  QMap<QString, MyMoneyAccount>::ConstIterator it_p;

  // locate the account in the file global pool

  it_a = accountList.constFind(account.id());
  if (it_a == accountList.constEnd())
    throw MYMONEYEXCEPTION("Internal error: account not found in list");

  it_p = accountList.constFind(parent.id());
  if (it_p == accountList.constEnd())
    throw MYMONEYEXCEPTION("Internal error: parent account not found in list");

  if (!account.institutionId().isEmpty())
    throw MYMONEYEXCEPTION("Cannot remove account still attached to an institution");

  // FIXME: check referential integrity for the account to be removed

  // check if the new info is based on the old one.
  // this is the case, when the file and the id
  // as well as the type are equal.
  if ((*it_a).id() == account.id()
      && (*it_a).accountType() == account.accountType()) {

    // second round over sub-ordinate accounts: do re-parenting
    // but only if the list contains at least one entry
    // FIXME: move this logic to MyMoneyFile
    if ((*it_a).accountList().count() > 0) {
      foreach (const QString& it, (*it_a).accountList()) {
        MyMoneyAccount acc(MyMoneyDatabaseMgr::account(it));
        reparentAccount(acc, parent);//, false);
      }
    }
    // remove account from parent's list
    parent.removeAccountId(account.id());
    d->m_sql->modifyAccount(parent);

    // remove account from the global account pool
    //m_accountList.remove(account.id());

    // remove from balance list
//FIXME:    m_balanceCache.remove(account.id());
//FIXME:    invalidateBalanceCache(parent.id());

    d->m_sql->removeAccount(account);
  }
  commitTransaction();
}

void MyMoneyDatabaseMgr::removeInstitution(const MyMoneyInstitution& institution)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyInstitution> institutionList = d->m_sql->fetchInstitutions(QStringList(institution.id()));
  QMap<QString, MyMoneyInstitution>::ConstIterator it_i;

  it_i = institutionList.constFind(institution.id());
  if (it_i != institutionList.constEnd()) {
    // mark file as changed
    d->m_sql->removeInstitution(institution);
  } else
    throw MYMONEYEXCEPTION("invalid institution");
}

MyMoneyTransaction MyMoneyDatabaseMgr::transaction(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  // get the full key of this transaction, throw exception
  // if it's invalid (unknown)
  //if(!m_transactionKeys.contains(id))
  //  throw MYMONEYEXCEPTION("invalid transaction id");

  // check if this key is in the list, throw exception if not
  //QString key = m_transactionKeys[id];
  QMap <QString, MyMoneyTransaction> transactionList = d->m_sql->fetchTransactions("('" + QString(id) + "')");

  //there should only be one transaction in the map, if it was found, so check the size of the map
  //return the first element.
  //if(!transactionList.contains(key))
  if (!transactionList.size())
    throw MYMONEYEXCEPTION("invalid transaction key");

  return transactionList.begin().value();
}

MyMoneyMoney MyMoneyDatabaseMgr::balance(const QString& id, const QDate& date) const
{
  Q_D(const MyMoneyDatabaseMgr);
  MyMoneyMoney result;
  QStringList idList;
  idList.append(id);
  QMap<QString, MyMoneyMoney> tempMap = d->m_sql->fetchBalance(idList, date);

  QMap<QString, MyMoneyMoney>::ConstIterator returnValue = tempMap.constFind(id);
  if (returnValue != tempMap.constEnd()) {
    result = returnValue.value();
    return result;
  }

//DEBUG
  QDate date_(date);
  //if (date_ == QDate()) date_ = QDate::currentDate();
// END DEBUG

//  MyMoneyAccount acc;
  QMap<QString, MyMoneyAccount> accountList = d->m_sql->fetchAccounts(/*QString(id)*/);
  //QMap<QString, MyMoneyAccount>::const_iterator accpos = accountList.find(id);
  if (date_ != QDate()) qDebug("request balance for %s at %s", qPrintable(id), qPrintable(date_.toString(Qt::ISODate)));
//  if(!date_.isValid() && MyMoneyFile::instance()->account(id).accountType() != eMyMoney::Account::Type::Stock) {
//    if(accountList.find(id) != accountList.end())
//      return accountList[id].balance();
//    return MyMoneyMoney(0);
//  }
  if (/*m_balanceCache[id].valid == false || date != m_balanceCacheDate) || */ d->m_sql) {
    QMap<QString, MyMoneyMoney> balances;
    QMap<QString, MyMoneyMoney>::ConstIterator it_b;
//FIXME:    if (date != m_balanceCacheDate) {
//FIXME:      m_balanceCache.clear();
//FIXME:      m_balanceCacheDate = date;
//FIXME:    }

    MyMoneyTransactionFilter filter;
    filter.addAccount(id);
    filter.setDateFilter(QDate(), date_);
    filter.setReportAllSplits(false);
    QList<MyMoneyTransaction> list = transactionList(filter);

    foreach (const MyMoneyTransaction& it_t, list) {
      foreach (const MyMoneySplit& it_s, it_t.splits()) {
        const QString aid = it_s.accountId();
        if (it_s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
          balances[aid] *= it_s.shares();
        } else {
          balances[aid] += it_s.value(it_t.commodity(), accountList[aid].currencyId());
        }
      }
    }

    // fill the found balances into the cache
//FIXME:    for(it_b = balances.begin(); it_b != balances.end(); ++it_b) {
//FIXME:      MyMoneyBalanceCacheItem balance(*it_b);
//FIXME:      m_balanceCache[it_b.key()] = balance;
//FIXME:    }

    // fill all accounts w/o transactions to zero
//    if (m_sql != 0) {
//      QMap<QString, MyMoneyAccount>::ConstIterator it_a;
//      for(it_a = m_accountList.begin(); it_a != m_accountList.end(); ++it_a) {
//FIXME:        if(m_balanceCache[(*it_a).id()].valid == false) {
//FIXME:          MyMoneyBalanceCacheItem balance(MyMoneyMoney(0,1));
//FIXME:          m_balanceCache[(*it_a).id()] = balance;
//FIXME:        }
//      }
//    }

    result = balances[id];

  }

//FIXME:  if(m_balanceCache[id].valid == true)
//FIXME:    result = m_balanceCache[id].balance;
//FIXME:  else
//FIXME:    qDebug("Cache mishit should never happen at this point");

  return result;
}

MyMoneyTransaction MyMoneyDatabaseMgr::transaction(const QString& account, const int idx) const
{
  Q_D(const MyMoneyDatabaseMgr);
  /* removed with MyMoneyAccount::Transaction
    QMap<QString, MyMoneyAccount>::ConstIterator acc;

    // find account object in list, throw exception if unknown
    acc = m_accountList.find(account);
    if(acc == m_accountList.end())
      throw MYMONEYEXCEPTION("unknown account id");

    // get the transaction info from the account
    MyMoneyAccount::Transaction t = (*acc).transaction(idx);

    // return the transaction, throw exception if not found
    return transaction(t.transactionID());
  */

  // new implementation if the above code does not work anymore
  QList<MyMoneyTransaction> list;
  //MyMoneyAccount acc = m_accountList[account];
  MyMoneyAccount acc = d->m_sql->fetchAccounts(QStringList(account))[account];
  MyMoneyTransactionFilter filter;

  if (acc.accountGroup() == eMyMoney::Account::Type::Income
      || acc.accountGroup() == eMyMoney::Account::Type::Expense)
    filter.addCategory(account);
  else
    filter.addAccount(account);

  transactionList(list, filter);
  if (idx < 0 || idx >= static_cast<int>(list.count()))
    throw MYMONEYEXCEPTION("Unknown idx for transaction");

  return transaction(list[idx].id());
}

uint MyMoneyDatabaseMgr::institutionCount() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getRecCount("kmmInstitutions");
}

void MyMoneyDatabaseMgr::accountList(QList<MyMoneyAccount>& list) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap <QString, MyMoneyAccount> accountList;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    accountList  = d->m_sql->fetchAccounts();
  }
  QMap<QString, MyMoneyAccount>::ConstIterator it;
  QMap<QString, MyMoneyAccount>::ConstIterator accEnd = accountList.constEnd();
  for (it = accountList.constBegin(); it != accEnd; ++it) {
    if (!isStandardAccount((*it).id())) {
      list.append(*it);
    }
  }
}

MyMoneyAccount MyMoneyDatabaseMgr::liability() const
{
  return MyMoneyFile::instance()->account(stdAccNames[stdAccLiability]);
}

MyMoneyAccount MyMoneyDatabaseMgr::asset() const
{
  return MyMoneyFile::instance()->account(stdAccNames[stdAccAsset]);
}

MyMoneyAccount MyMoneyDatabaseMgr::expense() const
{
  return MyMoneyFile::instance()->account(stdAccNames[stdAccExpense]);
}

MyMoneyAccount MyMoneyDatabaseMgr::income() const
{
  return MyMoneyFile::instance()->account(stdAccNames[stdAccIncome]);
}

MyMoneyAccount MyMoneyDatabaseMgr::equity() const
{
  return MyMoneyFile::instance()->account(stdAccNames[stdAccEquity]);
}

void MyMoneyDatabaseMgr::addSecurity(MyMoneySecurity& security)
{
  Q_D(MyMoneyDatabaseMgr);
  // create the account
  try {
    startTransaction();
    MyMoneySecurity newSecurity(nextSecurityID(), security);

    d->m_sql->addSecurity(newSecurity);
    security = newSecurity;
    commitTransaction();
  } catch (...) {
    rollbackTransaction();
    throw;
  }
}

void MyMoneyDatabaseMgr::modifySecurity(const MyMoneySecurity& security)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySecurity> securitiesList = d->m_sql->fetchSecurities(QStringList(security.id()), true);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = securitiesList.constFind(security.id());
  if (it == securitiesList.constEnd()) {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during modifySecurity()";
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->modifySecurity(security);
}

void MyMoneyDatabaseMgr::removeSecurity(const MyMoneySecurity& security)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySecurity> securitiesList = d->m_sql->fetchSecurities(QStringList(security.id()));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = securitiesList.constFind(security.id());
  if (it == securitiesList.constEnd()) {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during removeSecurity()";
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->removeSecurity(security);
}

MyMoneySecurity MyMoneyDatabaseMgr::security(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySecurity> securitiesList = d->m_sql->fetchSecurities(QStringList(id));
  QMap<QString, MyMoneySecurity>::ConstIterator it = securitiesList.constFind(id);
  if (it != securitiesList.constEnd()) {
    return it.value();
  }

  return MyMoneySecurity();
}

QList<MyMoneySecurity> MyMoneyDatabaseMgr::securityList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->fetchSecurities().values();
}

void MyMoneyDatabaseMgr::addPrice(const MyMoneyPrice& price)
{
  Q_D(MyMoneyDatabaseMgr);
  MyMoneyPriceEntries::ConstIterator it;
  MyMoneyPriceList priceList = d->m_sql->fetchPrices();
  it = priceList[MyMoneySecurityPair(price.from(), price.to())].constFind(price.date());
  // do not replace, if the information did not change.
  if (it != priceList[MyMoneySecurityPair(price.from(), price.to())].constEnd()) {
    if ((*it).rate((*it).to()) == price.rate(price.to())
        && (*it).source() == price.source())
      return;
  }

  d->m_sql->addPrice(price);
}

void MyMoneyDatabaseMgr::removePrice(const MyMoneyPrice& price)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->removePrice(price);
}

MyMoneyPrice MyMoneyDatabaseMgr::price(const QString& fromId, const QString& toId, const QDate& _date, bool exactDate) const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->fetchSinglePrice(fromId, toId, _date, exactDate);
}

MyMoneyPriceList MyMoneyDatabaseMgr::priceList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->fetchPrices();
}

void MyMoneyDatabaseMgr::addSchedule(MyMoneySchedule& sched)
{
  Q_D(MyMoneyDatabaseMgr);
  // first perform all the checks
  if (!sched.id().isEmpty())
    throw MYMONEYEXCEPTION("schedule already contains an id");

  // The following will throw an exception when it fails
  sched.validate(false);

  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    try {
      startTransaction();
      sched = MyMoneySchedule(nextScheduleID(), sched);

      d->m_sql->addSchedule(sched);
      commitTransaction();
    } catch (...) {
      rollbackTransaction();
      throw;
    }
  }
}

void MyMoneyDatabaseMgr::modifySchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySchedule> scheduleList = d->m_sql->fetchSchedules(QStringList(sched.id()));
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = scheduleList.constFind(sched.id());
  if (it == scheduleList.constEnd()) {
    QString msg = "Unknown schedule '" + sched.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->modifySchedule(sched);
}

void MyMoneyDatabaseMgr::removeSchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySchedule> scheduleList = d->m_sql->fetchSchedules(QStringList(sched.id()));
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = scheduleList.constFind(sched.id());
  if (it == scheduleList.constEnd()) {
    QString msg = "Unknown schedule '" + sched.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  // FIXME: check referential integrity for loan accounts

  d->m_sql->removeSchedule(sched);
}

MyMoneySchedule MyMoneyDatabaseMgr::schedule(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySchedule> scheduleList = d->m_sql->fetchSchedules(QStringList(id));
  QMap<QString, MyMoneySchedule>::ConstIterator pos;

  // locate the schedule and if present, return it's data
  pos = scheduleList.constFind(id);
  if (pos != scheduleList.constEnd())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown schedule id '" + id + '\'';
  throw MYMONEYEXCEPTION(msg);
}

QList<MyMoneySchedule> MyMoneyDatabaseMgr::scheduleList(const QString& accountId,
    eMyMoney::Schedule::Type type,
    eMyMoney::Schedule::Occurrence occurrence,
    eMyMoney::Schedule::PaymentType paymentType,
    const QDate& startDate,
    const QDate& endDate,
    bool overdue) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySchedule> scheduleList;
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    scheduleList = d->m_sql->fetchSchedules();
  }
  QMap<QString, MyMoneySchedule>::ConstIterator pos;
  QList<MyMoneySchedule> list;

  // qDebug("scheduleList()");

  for (pos = scheduleList.constBegin(); pos != scheduleList.constEnd(); ++pos) {
    // qDebug("  '%s'", (*pos).id().data());

    if (type != eMyMoney::Schedule::Type::Any) {
      if (type != (*pos).type()) {
        continue;
      }
    }

    if (occurrence != eMyMoney::Schedule::Occurrence::Any) {
      if (occurrence != (*pos).occurrence()) {
        continue;
      }
    }

    if (paymentType != eMyMoney::Schedule::PaymentType::Any) {
      if (paymentType != (*pos).paymentType()) {
        continue;
      }
    }

    if (!accountId.isEmpty()) {
      MyMoneyTransaction t = (*pos).transaction();
      QList<MyMoneySplit>::ConstIterator it;
      QList<MyMoneySplit> splits;
      splits = t.splits();
      for (it = splits.constBegin(); it != splits.constEnd(); ++it) {
        if ((*it).accountId() == accountId)
          break;
      }
      if (it == splits.constEnd()) {
        continue;
      }
    }

    if (startDate.isValid() && endDate.isValid()) {
      if ((*pos).paymentDates(startDate, endDate).count() == 0) {
        continue;
      }
    }

    if (startDate.isValid() && !endDate.isValid()) {
      if (!(*pos).nextPayment(startDate.addDays(-1)).isValid()) {
        continue;
      }
    }

    if (!startDate.isValid() && endDate.isValid()) {
      if ((*pos).startDate() > endDate) {
        continue;
      }
    }

    if (overdue) {
      if (!(*pos).isOverdue())
        continue;
      /*
            QDate nextPayment = (*pos).nextPayment((*pos).lastPayment());
            if(!nextPayment.isValid())
              continue;
            if(nextPayment >= QDate::currentDate())
              continue;
      */
    }

    // qDebug("Adding '%s'", (*pos).name().latin1());
    list << *pos;
  }
  return list;
}

QList<MyMoneySchedule> MyMoneyDatabaseMgr::scheduleListEx(int scheduleTypes,
    int scheduleOcurrences,
    int schedulePaymentTypes,
    QDate startDate,
    const QStringList& accounts) const
{
  Q_D(const MyMoneyDatabaseMgr);
//  qDebug("scheduleListEx");
  QMap<QString, MyMoneySchedule> scheduleList = d->m_sql->fetchSchedules();
  QMap<QString, MyMoneySchedule>::ConstIterator pos;
  QList<MyMoneySchedule> list;

  if (!startDate.isValid())
    return list;

  for (pos = scheduleList.constBegin(); pos != scheduleList.constEnd(); ++pos) {
    if (scheduleTypes && !(scheduleTypes & (int)(*pos).type()))
      continue;

    if (scheduleOcurrences && !(scheduleOcurrences & (int)(*pos).occurrence()))
      continue;

    if (schedulePaymentTypes && !(schedulePaymentTypes & (int)(*pos).paymentType()))
      continue;

    if ((*pos).paymentDates(startDate, startDate).count() == 0)
      continue;

    if ((*pos).isFinished())
      continue;

    if ((*pos).hasRecordedPayment(startDate))
      continue;

    if (accounts.count() > 0) {
      if (accounts.contains((*pos).account().id()))
        continue;
    }

//    qDebug("\tAdding '%s'", (*pos).name().latin1());
    list << *pos;
  }

  return list;
}

void MyMoneyDatabaseMgr::addCurrency(const MyMoneySecurity& currency)
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    QMap<QString, MyMoneySecurity> currencyList = d->m_sql->fetchCurrencies(QStringList(currency.id()));
    QMap<QString, MyMoneySecurity>::ConstIterator it;

    it = currencyList.constFind(currency.id());
    if (it != currencyList.constEnd()) {
      throw MYMONEYEXCEPTION(i18n("Cannot add currency with existing id %1", currency.id()));
    }

    d->m_sql->addCurrency(currency);
  }
}

void MyMoneyDatabaseMgr::modifyCurrency(const MyMoneySecurity& currency)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySecurity> currencyList = d->m_sql->fetchCurrencies(QStringList(currency.id()));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.constFind(currency.id());
  if (it == currencyList.constEnd()) {
    throw MYMONEYEXCEPTION(i18n("Cannot modify currency with unknown id %1", currency.id()));
  }

  d->m_sql->modifyCurrency(currency);
}

void MyMoneyDatabaseMgr::removeCurrency(const MyMoneySecurity& currency)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneySecurity> currencyList = d->m_sql->fetchCurrencies(QStringList(currency.id()));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = currencyList.constFind(currency.id());
  if (it == currencyList.constEnd()) {
    throw MYMONEYEXCEPTION(i18n("Cannot remove currency with unknown id %1", currency.id()));
  }

  d->m_sql->removeCurrency(currency);
}

MyMoneySecurity MyMoneyDatabaseMgr::currency(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (id.isEmpty()) {

  }
  QMap<QString, MyMoneySecurity> currencyList = d->m_sql->fetchCurrencies(QStringList(id));
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = currencyList.constFind(id);
  if (it == currencyList.constEnd()) {
    throw MYMONEYEXCEPTION(i18n("Cannot retrieve currency with unknown id '%1'", id));
  }

  return *it;
}

QList<MyMoneySecurity> MyMoneyDatabaseMgr::currencyList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchCurrencies().values();
  } else {
    return QList<MyMoneySecurity> ();
  }
}

QList<MyMoneyReport> MyMoneyDatabaseMgr::reportList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchReports().values();
  } else {
    return QList<MyMoneyReport> ();
  }
}

void MyMoneyDatabaseMgr::addReport(MyMoneyReport& report)
{
  Q_D(MyMoneyDatabaseMgr);
  if (!report.id().isEmpty())
    throw MYMONEYEXCEPTION("transaction already contains an id");

  d->m_sql->addReport(MyMoneyReport(nextReportID(), report));
}

void MyMoneyDatabaseMgr::modifyReport(const MyMoneyReport& report)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyReport> reportList = d->m_sql->fetchReports(QStringList(report.id()));
  QMap<QString, MyMoneyReport>::ConstIterator it;

  it = reportList.constFind(report.id());
  if (it == reportList.constEnd()) {
    QString msg = "Unknown report '" + report.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->modifyReport(report);
}

uint MyMoneyDatabaseMgr::countReports() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getRecCount("kmmReports");
}

MyMoneyReport MyMoneyDatabaseMgr::report(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->fetchReports(QStringList(id))[id];
}

void MyMoneyDatabaseMgr::removeReport(const MyMoneyReport& report)
{
  Q_D(MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyReport> reportList = d->m_sql->fetchReports(QStringList(report.id()));
  QMap<QString, MyMoneyReport>::ConstIterator it;

  it = reportList.constFind(report.id());
  if (it == reportList.constEnd()) {
    QString msg = "Unknown report '" + report.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_sql->removeReport(report);
}

QList<MyMoneyBudget> MyMoneyDatabaseMgr::budgetList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->fetchBudgets().values();
}

void MyMoneyDatabaseMgr::addBudget(MyMoneyBudget& budget)
{
  Q_D(MyMoneyDatabaseMgr);
  MyMoneyBudget newBudget(nextBudgetID(), budget);
  d->m_sql->addBudget(newBudget);
}

MyMoneyBudget MyMoneyDatabaseMgr::budgetByName(const QString& budget) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyBudget> budgets = d->m_sql->fetchBudgets();
  QMap<QString, MyMoneyBudget>::ConstIterator it_p;

  for (it_p = budgets.constBegin(); it_p != budgets.constEnd(); ++it_p) {
    if ((*it_p).name() == budget) {
      return *it_p;
    }
  }

  throw MYMONEYEXCEPTION("Unknown budget '" + budget + '\'');
}

void MyMoneyDatabaseMgr::modifyBudget(const MyMoneyBudget& budget)
{
  Q_D(MyMoneyDatabaseMgr);
  //QMap<QString, MyMoneyBudget>::ConstIterator it;

  //it = m_budgetList.find(budget.id());
  //if(it == m_budgetList.end()) {
  //  QString msg = "Unknown budget '" + budget.id() + '\'';
  //  throw MYMONEYEXCEPTION(msg);
  //}
  //m_budgetList.modify(budget.id(), budget);

  startTransaction();
  if (d->m_sql->fetchBudgets(QStringList(budget.id()), true).empty()) {
    QString msg = "Unknown budget '" + budget.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }
  d->m_sql->modifyBudget(budget);
  commitTransaction();
}

uint MyMoneyDatabaseMgr::countBudgets() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getRecCount("kmmBudgetConfig");
}

MyMoneyBudget MyMoneyDatabaseMgr::budget(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->fetchBudgets(QStringList(id))[id];
}

void MyMoneyDatabaseMgr::removeBudget(const MyMoneyBudget& budget)
{
  Q_D(MyMoneyDatabaseMgr);
//  QMap<QString, MyMoneyBudget>::ConstIterator it;
//
//  it = m_budgetList.find(budget.id());
//  if(it == m_budgetList.end()) {
//    QString msg = "Unknown budget '" + budget.id() + '\'';
//    throw MYMONEYEXCEPTION(msg);
//  }
//
  d->m_sql->removeBudget(budget);
}

void MyMoneyDatabaseMgr::clearCache()
{
  //m_balanceCache.clear();
}

class isReferencedHelper
{
public:
  isReferencedHelper(const QString& id)
      : m_id(id) {}

  inline bool operator()(const MyMoneyObject& obj) const {
    return obj.hasReferenceTo(m_id);
  }

private:
  QString m_id;
};

bool MyMoneyDatabaseMgr::isReferenced(const MyMoneyObject& obj, const QBitArray& skipCheck) const
{
  Q_D(const MyMoneyDatabaseMgr);
  Q_ASSERT(skipCheck.count() == (int)Reference::Count);

  const auto& id = obj.id();

  // FIXME optimize the list of objects we have to checks
  //       with a bit of knowledge of the internal structure, we
  //       could optimize the number of objects we check for references

  // Scan all engine objects for a reference
  if (!skipCheck.testBit((int)Reference::Transaction)) {
    auto skipTransactions = false;
    MyMoneyTransactionFilter f;
    if (typeid(obj) == typeid(MyMoneyAccount)) {
      f.addAccount(id);
    } else if (typeid(obj) == typeid(MyMoneyCategory)) {
      f.addCategory(id);
    } else if (typeid(obj) == typeid(MyMoneyPayee)) {
      f.addPayee(id);
    } // if it's anything else, I guess we just read everything
    //FIXME: correction, transactions can only have a reference to an account or payee,
    //             so, read nothing.
    else {
      skipTransactions = true;
    }
    if (! skipTransactions) {
      //QMap <QString, MyMoneyTransaction> transactionList = m_sql->fetchTransactions(f);
      //rc = (transactionList.end() != std::find_if(transactionList.begin(), transactionList.end(),  isReferencedHelper(id)));
      //if (rc != m_sql->isReferencedByTransaction(obj.id()))
      //  qDebug ("Transaction match inconsistency.");
      if (d->m_sql->isReferencedByTransaction(id))
        return true;
    }
  }

  if (!skipCheck.testBit((int)Reference::Account)) {
    QList<MyMoneyAccount> accountList;
    MyMoneyFile::instance()->accountList(accountList);
    foreach (const auto it, accountList)
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Institution)) {
    QList<MyMoneyInstitution> institutionList;
    MyMoneyFile::instance()->institutionList(institutionList);
    foreach (const auto it, institutionList)
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Payee)) {
    foreach (const auto it, MyMoneyFile::instance()->payeeList())
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Tag)) {
    foreach (const auto it, MyMoneyFile::instance()->tagList())
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Report)) {
    foreach (const auto it, d->m_sql->fetchReports())
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Budget)) {
    foreach (const auto it, d->m_sql->fetchBudgets())
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Schedule)) {
    foreach (const auto it, d->m_sql->fetchSchedules())
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Security)) {
    foreach (const auto it, MyMoneyFile::instance()->securityList())
      if (it.hasReferenceTo(id))
        return true;
  }
  if (!skipCheck.testBit((int)Reference::Currency)) {
    const auto currencyList = d->m_sql->fetchCurrencies().values();
    // above line cannot go directly here because m_sql->fetchCurrencies() will return temporary object which will get destructed before .values()
    foreach (const auto it, currencyList)
      if (it.hasReferenceTo(id))
        return true;
  }
  // within the pricelist we don't have to scan each entry. Checking the QPair
  // members of the MyMoneySecurityPair is enough as they are identical to the
  // two security ids
  if (!skipCheck.testBit((int)Reference::Price)) {
    const auto priceList = d->m_sql->fetchPrices();
    for (auto it_pr = priceList.begin(); it_pr != priceList.end(); ++it_pr) {
      if ((it_pr.key().first == id) || (it_pr.key().second == id))
        return true;
    }
  }
  return false;
}

void MyMoneyDatabaseMgr::close()
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    d->m_sql->close(true);
    d->m_sql = 0;
  }
}

void MyMoneyDatabaseMgr::startTransaction()
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    d->m_sql->startCommitUnit("databasetransaction");
  }
}

bool MyMoneyDatabaseMgr::commitTransaction()
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->endCommitUnit("databasetransaction");
  }
  return false;
}

void MyMoneyDatabaseMgr::rollbackTransaction()
{
  Q_D(MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    d->m_sql->cancelCommitUnit("databasetransaction");
  }
}

void MyMoneyDatabaseMgr::setCreationDate(const QDate& val)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_creationDate = val;
}

QExplicitlySharedDataPointer <MyMoneyStorageSql> MyMoneyDatabaseMgr::connectToDatabase(const QUrl &url)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql = new MyMoneyStorageSql(this, url);
  return d->m_sql;
}

void MyMoneyDatabaseMgr::fillStorage()
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->fillStorage();
}

void MyMoneyDatabaseMgr::setLastModificationDate(const QDate& val)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_lastModificationDate = val;
}

bool MyMoneyDatabaseMgr::isDuplicateTransaction(const QString& /*id*/) const
{
  //FIXME: figure out the real id from the key and check the DB.
//return m_transactionKeys.contains(id);
  return false;
}

void MyMoneyDatabaseMgr::loadAccounts(const QMap<QString, MyMoneyAccount>& /*map*/)
{
//  m_accountList = map;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmAccounts
// for each account in the map
//    m_sql->addAccount(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadTransactions(const QMap<QString, MyMoneyTransaction>& /*map*/)
{
//  m_transactionList = map;
//FIXME: update the database.

//  // now fill the key map
//  QMap<QString, QString> keys;
//  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
//  for(it_t = map.begin(); it_t != map.end(); ++it_t) {
//    keys[(*it_t).id()] = it_t.key();
//  }
//  m_transactionKeys = keys;
}

void MyMoneyDatabaseMgr::loadInstitutions(const QMap<QString, MyMoneyInstitution>& /*map*/)
{
//  m_institutionList = map;
//FIXME: update the database.

//  // now fill the key map
//  QMap<QString, QString> keys;
//  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
//  for(it_t = map.begin(); it_t != map.end(); ++it_t) {
//    keys[(*it_t).id()] = it_t.key();
//  }
//  m_transactionKeys = keys;
}

void MyMoneyDatabaseMgr::loadPayees(const QMap<QString, MyMoneyPayee>& /*map*/)
{
//  m_payeeList = map;
}

void MyMoneyDatabaseMgr::loadTags(const QMap<QString, MyMoneyTag>& /*map*/)
{
//  m_tagList = map;
}

void MyMoneyDatabaseMgr::loadSchedules(const QMap<QString, MyMoneySchedule>& /*map*/)
{
//  m_scheduleList = map;
}

void MyMoneyDatabaseMgr::loadSecurities(const QMap<QString, MyMoneySecurity>& /*map*/)
{
//  m_securitiesList = map;
}

void MyMoneyDatabaseMgr::loadCurrencies(const QMap<QString, MyMoneySecurity>& /*map*/)
{
//  m_currencyList = map;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmBudgetConfig
// for each budget in the map
//    m_sql->addBudget(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadReports(const QMap<QString, MyMoneyReport>& /*reports*/)
{
//  m_reportList = reports;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmBudgetConfig
// for each budget in the map
//    m_sql->addBudget(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadBudgets(const QMap<QString, MyMoneyBudget>& /*budgets*/)
{
//  m_budgetList = budgets;
//FIXME: update the database.
// startTransaction
// DELETE FROM kmmBudgetConfig
// for each budget in the map
//    m_sql->addBudget(...)
// commitTransaction
// on error, rollbackTransaction
}

void MyMoneyDatabaseMgr::loadPrices(const MyMoneyPriceList& list)
{
  Q_UNUSED(list);
}

void MyMoneyDatabaseMgr::loadOnlineJobs(const QMap< QString, onlineJob >& onlineJobs)
{
  Q_UNUSED(onlineJobs);
}

ulong MyMoneyDatabaseMgr::accountId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextAccountId() - 1;
}

ulong MyMoneyDatabaseMgr::transactionId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextTransactionId() - 1;
}

ulong MyMoneyDatabaseMgr::payeeId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextPayeeId() - 1;
}

ulong MyMoneyDatabaseMgr::tagId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextTagId() - 1;
}

ulong MyMoneyDatabaseMgr::institutionId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextInstitutionId() - 1;
}

ulong MyMoneyDatabaseMgr::scheduleId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextScheduleId() - 1;
}

ulong MyMoneyDatabaseMgr::securityId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextSecurityId() - 1;
}

ulong MyMoneyDatabaseMgr::reportId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextReportId() - 1;
}

ulong MyMoneyDatabaseMgr::budgetId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextBudgetId() - 1;
}

ulong MyMoneyDatabaseMgr::onlineJobId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextOnlineJobId() - 1;
}

ulong MyMoneyDatabaseMgr::payeeIdentifierId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextPayeeIdentifierId() - 1;
}

void MyMoneyDatabaseMgr::loadAccountId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadAccountId(id);
}

void MyMoneyDatabaseMgr::loadTransactionId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadTransactionId(id);
}

void MyMoneyDatabaseMgr::loadPayeeId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadPayeeId(id);
}

void MyMoneyDatabaseMgr::loadTagId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadTagId(id);
}

void MyMoneyDatabaseMgr::loadInstitutionId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadInstitutionId(id);
}

void MyMoneyDatabaseMgr::loadScheduleId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadScheduleId(id);
}

void MyMoneyDatabaseMgr::loadSecurityId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadSecurityId(id);
}

void MyMoneyDatabaseMgr::loadReportId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadReportId(id);
}

void MyMoneyDatabaseMgr::loadBudgetId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadBudgetId(id);
}

void MyMoneyDatabaseMgr::loadOnlineJobId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadOnlineJobId(id);
}

void MyMoneyDatabaseMgr::loadPayeeIdentifierId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadPayeeIdentifierId(id);
}

void MyMoneyDatabaseMgr::loadCostCenterId(ulong id)
{
  Q_D(MyMoneyDatabaseMgr);
  d->m_sql->loadAccountId(id);
}

void MyMoneyDatabaseMgr::rebuildAccountBalances()
{
  Q_D(MyMoneyDatabaseMgr);
  startTransaction();
  QMap<QString, MyMoneyAccount> accountMap = d->m_sql->fetchAccounts(QStringList(), true);

  QMap<QString, MyMoneyMoney> balanceMap = d->m_sql->fetchBalance(accountMap.keys(), QDate());

  for (QMap<QString, MyMoneyMoney>::const_iterator it_b = balanceMap.constBegin();
       it_b != balanceMap.constEnd(); ++it_b) {
    accountMap[it_b.key()].setBalance(it_b.value());
  }

  QList<MyMoneyAccount> aList;
  for (QMap<QString, MyMoneyAccount>::const_iterator it_a = accountMap.constBegin();
       it_a != accountMap.constEnd(); ++it_a) {
    aList << it_a.value();
  }
  d->m_sql->modifyAccountList(aList);

  commitTransaction();
}

QList< MyMoneyCostCenter > MyMoneyDatabaseMgr::costCenterList() const
{
  Q_D(const MyMoneyDatabaseMgr);
  if (d->m_sql) {
    if (! d->m_sql->isOpen())((QSqlDatabase*)(d->m_sql.data()))->open();
    return d->m_sql->fetchCostCenters().values();
  }
  return QList<MyMoneyCostCenter> ();
}

void MyMoneyDatabaseMgr::loadCostCenters(const QMap< QString, MyMoneyCostCenter >& costCenters)
{
  Q_UNUSED(costCenters);
}

ulong MyMoneyDatabaseMgr::costCenterId() const
{
  Q_D(const MyMoneyDatabaseMgr);
  return d->m_sql->getNextCostCenterId() - 1;
}

MyMoneyCostCenter MyMoneyDatabaseMgr::costCenter(const QString& id) const
{
  Q_D(const MyMoneyDatabaseMgr);
  QMap<QString, MyMoneyCostCenter>::ConstIterator it;
  QMap<QString, MyMoneyCostCenter> costCenterList = d->m_sql->fetchCostCenters(QStringList(id));
  it = costCenterList.constFind(id);
  if (it == costCenterList.constEnd())
    throw MYMONEYEXCEPTION("Unknown costcenter '" + id + '\'');

  return *it;
}
