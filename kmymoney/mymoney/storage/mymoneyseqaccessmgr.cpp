/***************************************************************************
                          mymoneyseqaccessmgr.cpp
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                               2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "mymoneyseqaccessmgr_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyreport.h"
#include "mymoneyprice.h"
#include "onlinejob.h"
#include "mymoneystoragenames.h"

#define TRY try {
#define CATCH } catch (const MyMoneyException &e) {
#define PASS } catch (const MyMoneyException &e) { throw; }

const int INSTITUTION_ID_SIZE = 6;
const int ACCOUNT_ID_SIZE = 6;
const int TRANSACTION_ID_SIZE = 18;
const int PAYEE_ID_SIZE = 6;
const int TAG_ID_SIZE = 6;
const int SCHEDULE_ID_SIZE = 6;
const int SECURITY_ID_SIZE = 6;
const int REPORT_ID_SIZE = 6;
const int BUDGET_ID_SIZE = 6;
const int ONLINE_JOB_ID_SIZE = 6;
const int COSTCENTER_ID_SIZE = 6;

using namespace MyMoneyStandardAccounts;
#include <QHash>
MyMoneySeqAccessMgr::MyMoneySeqAccessMgr() :
  d_ptr(new MyMoneySeqAccessMgrPrivate(this))
{
  Q_D(MyMoneySeqAccessMgr);
  // setup standard accounts
  MyMoneyAccount acc_l;
  acc_l.setAccountType(eMyMoney::Account::Type::Liability);
  acc_l.setName("Liability");
  MyMoneyAccount liability(stdAccNames[stdAccLiability], acc_l);

  MyMoneyAccount acc_a;
  acc_a.setAccountType(eMyMoney::Account::Type::Asset);
  acc_a.setName("Asset");
  MyMoneyAccount asset(stdAccNames[stdAccAsset], acc_a);

  MyMoneyAccount acc_e;
  acc_e.setAccountType(eMyMoney::Account::Type::Expense);
  acc_e.setName("Expense");
  MyMoneyAccount expense(stdAccNames[stdAccExpense], acc_e);

  MyMoneyAccount acc_i;
  acc_i.setAccountType(eMyMoney::Account::Type::Income);
  acc_i.setName("Income");
  MyMoneyAccount income(stdAccNames[stdAccIncome], acc_i);

  MyMoneyAccount acc_q;
  acc_q.setAccountType(eMyMoney::Account::Type::Equity);
  acc_q.setName("Equity");
  MyMoneyAccount equity(stdAccNames[stdAccEquity], acc_q);

  QMap<QString, MyMoneyAccount> map;
  map[stdAccNames[stdAccAsset]] = asset;
  map[stdAccNames[stdAccLiability]] = liability;
  map[stdAccNames[stdAccIncome]] = income;
  map[stdAccNames[stdAccExpense]] = expense;
  map[stdAccNames[stdAccEquity]] = equity;

  // load account list with initial accounts
  d->m_accountList = map;
}

MyMoneySeqAccessMgr::~MyMoneySeqAccessMgr()
{
  Q_D(MyMoneySeqAccessMgr);
  delete d;
}

MyMoneyPayee MyMoneySeqAccessMgr::user() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_user;
}

QDate MyMoneySeqAccessMgr::creationDate() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_creationDate;
}

QDate MyMoneySeqAccessMgr::lastModificationDate() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_lastModificationDate;
}

uint MyMoneySeqAccessMgr::currentFixVersion() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_currentFixVersion;
}

uint MyMoneySeqAccessMgr::fileFixVersion() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_fileFixVersion;
}

void MyMoneySeqAccessMgr::setUser(const MyMoneyPayee& user)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_user = user;
  d->touch();
}

void MyMoneySeqAccessMgr::setCreationDate(const QDate& val)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_creationDate = val; d->touch();
}

void MyMoneySeqAccessMgr::setLastModificationDate(const QDate& val)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_lastModificationDate = val; d->m_dirty = false;
}

void MyMoneySeqAccessMgr::setFileFixVersion(uint v)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_fileFixVersion = v;
}

//MyMoneySeqAccessMgr const * MyMoneySeqAccessMgr::duplicate()
//{
//  auto that = new MyMoneySeqAccessMgr();
//  *that = *this;
//  return that;
//}

/**
* This method is used to get a SQL reader for subsequent database access
 */
QExplicitlySharedDataPointer <MyMoneyStorageSql> MyMoneySeqAccessMgr::connectToDatabase
(const QUrl& /*url*/)
{
  return QExplicitlySharedDataPointer <MyMoneyStorageSql>();
}

void MyMoneySeqAccessMgr::fillStorage()
{

}

bool MyMoneySeqAccessMgr::isStandardAccount(const QString& id) const
{
  return id == stdAccNames[stdAccLiability]
         || id == stdAccNames[stdAccAsset]
         || id == stdAccNames[stdAccExpense]
         || id == stdAccNames[stdAccIncome]
         || id == stdAccNames[stdAccEquity];
}

void MyMoneySeqAccessMgr::setAccountName(const QString& id, const QString& name)
{
  Q_D(MyMoneySeqAccessMgr);
  if (!isStandardAccount(id))
    throw MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  auto acc = d->m_accountList[id];
  acc.setName(name);
  d->m_accountList.modify(acc.id(), acc);
}

MyMoneyAccount MyMoneySeqAccessMgr::account(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  // locate the account and if present, return it's data
  if (d->m_accountList.find(id) != d->m_accountList.end())
    return d->m_accountList[id];

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + '\'';
  throw MYMONEYEXCEPTION(msg);
}

void MyMoneySeqAccessMgr::accountList(QList<MyMoneyAccount>& list) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyAccount>::ConstIterator it;
  for (it = d->m_accountList.begin(); it != d->m_accountList.end(); ++it) {
    if (!isStandardAccount((*it).id())) {
      list.append(*it);
    }
  }
}

void MyMoneySeqAccessMgr::addAccount(MyMoneyAccount& account)
{
  Q_D(MyMoneySeqAccessMgr);
  // create the account.
  MyMoneyAccount newAccount(nextAccountID(), account);
  d->m_accountList.insert(newAccount.id(), newAccount);

  account = newAccount;
}

void MyMoneySeqAccessMgr::addPayee(MyMoneyPayee& payee)
{
  Q_D(MyMoneySeqAccessMgr);
  // create the payee
  MyMoneyPayee newPayee(nextPayeeID(), payee);
  d->m_payeeList.insert(newPayee.id(), newPayee);
  payee = newPayee;
}

/**
 * @brief Add onlineJob to storage
 * @param job caller stays owner of the object, but id will be set
 */
void MyMoneySeqAccessMgr::addOnlineJob(onlineJob &job)
{
  Q_D(MyMoneySeqAccessMgr);
  onlineJob newJob = onlineJob(nextOnlineJobID(), job);
  d->m_onlineJobList.insert(newJob.id(), newJob);
  job = newJob;
}

void MyMoneySeqAccessMgr::removeOnlineJob(const onlineJob& job)
{
  Q_D(MyMoneySeqAccessMgr);
  if (!d->m_onlineJobList.contains(job.id())) {
    throw MYMONEYEXCEPTION("Unknown onlineJob '" + job.id() + "' should be removed.");
  }
  d->m_onlineJobList.remove(job.id());
}

void MyMoneySeqAccessMgr::modifyOnlineJob(const onlineJob &job)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, onlineJob>::ConstIterator iter = d->m_onlineJobList.find(job.id());
  if (iter == d->m_onlineJobList.end()) {
    throw MYMONEYEXCEPTION("Got unknown onlineJob '" + job.id() + "' for modifying");
  }
  onlineJob oldJob = iter.value();
  d->m_onlineJobList.modify((*iter).id(), job);
}

onlineJob MyMoneySeqAccessMgr::getOnlineJob(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  if (d->m_onlineJobList.contains(id)) {
    return d->m_onlineJobList[id];
  }
  throw MYMONEYEXCEPTION("Unknown online Job '" + id + '\'');
}

ulong MyMoneySeqAccessMgr::onlineJobId() const
{
  return 1;
}

MyMoneyPayee MyMoneySeqAccessMgr::payee(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyPayee>::ConstIterator it;
  it = d->m_payeeList.find(id);
  if (it == d->m_payeeList.end())
    throw MYMONEYEXCEPTION("Unknown payee '" + id + '\'');

  return *it;
}

MyMoneyPayee MyMoneySeqAccessMgr::payeeByName(const QString& payee) const
{
  Q_D(const MyMoneySeqAccessMgr);
  if (payee.isEmpty())
    return MyMoneyPayee::null;

  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  for (it_p = d->m_payeeList.begin(); it_p != d->m_payeeList.end(); ++it_p) {
    if ((*it_p).name() == payee) {
      return *it_p;
    }
  }

  throw MYMONEYEXCEPTION("Unknown payee '" + payee + '\'');
}

void MyMoneySeqAccessMgr::modifyPayee(const MyMoneyPayee& payee)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyPayee>::ConstIterator it;

  it = d->m_payeeList.find(payee.id());
  if (it == d->m_payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }
  d->m_payeeList.modify((*it).id(), payee);
}

void MyMoneySeqAccessMgr::removePayee(const MyMoneyPayee& payee)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneySchedule>::ConstIterator it_s;
  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  it_p = d->m_payeeList.find(payee.id());
  if (it_p == d->m_payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the payee is still referenced
  for (it_t = d->m_transactionList.begin(); it_t != d->m_transactionList.end(); ++it_t) {
    if ((*it_t).hasReferenceTo(payee.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove payee that is still referenced to a %1").arg("transaction"));
    }
  }

  // check referential integrity in schedules
  for (it_s = d->m_scheduleList.begin(); it_s != d->m_scheduleList.end(); ++it_s) {
    if ((*it_s).hasReferenceTo(payee.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove payee that is still referenced to a %1").arg("schedule"));
    }
  }

  // remove any reference to report and/or budget
  d->removeReferences(payee.id());

  d->m_payeeList.remove((*it_p).id());
}

QList<MyMoneyPayee> MyMoneySeqAccessMgr::payeeList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_payeeList.values();
}

void MyMoneySeqAccessMgr::addTag(MyMoneyTag& tag)
{
  Q_D(MyMoneySeqAccessMgr);
  // create the tag
  MyMoneyTag newTag(nextTagID(), tag);
  d->m_tagList.insert(newTag.id(), newTag);
  tag = newTag;
}

MyMoneyTag MyMoneySeqAccessMgr::tag(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyTag>::ConstIterator it;
  it = d->m_tagList.find(id);
  if (it == d->m_tagList.end())
    throw MYMONEYEXCEPTION("Unknown tag '" + id + '\'');

  return *it;
}

MyMoneyTag MyMoneySeqAccessMgr::tagByName(const QString& tag) const
{
  Q_D(const MyMoneySeqAccessMgr);
  if (tag.isEmpty())
    return MyMoneyTag::null;

  QMap<QString, MyMoneyTag>::ConstIterator it_ta;

  for (it_ta = d->m_tagList.begin(); it_ta != d->m_tagList.end(); ++it_ta) {
    if ((*it_ta).name() == tag) {
      return *it_ta;
    }
  }

  throw MYMONEYEXCEPTION("Unknown tag '" + tag + '\'');
}

void MyMoneySeqAccessMgr::modifyTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyTag>::ConstIterator it;

  it = d->m_tagList.find(tag.id());
  if (it == d->m_tagList.end()) {
    QString msg = "Unknown tag '" + tag.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }
  d->m_tagList.modify((*it).id(), tag);
}

void MyMoneySeqAccessMgr::removeTag(const MyMoneyTag& tag)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneySchedule>::ConstIterator it_s;
  QMap<QString, MyMoneyTag>::ConstIterator it_ta;

  it_ta = d->m_tagList.find(tag.id());
  if (it_ta == d->m_tagList.end()) {
    QString msg = "Unknown tag '" + tag.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the tag is still referenced
  for (it_t = d->m_transactionList.begin(); it_t != d->m_transactionList.end(); ++it_t) {
    if ((*it_t).hasReferenceTo(tag.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove tag that is still referenced to a %1").arg("transaction"));
    }
  }

  // check referential integrity in schedules
  for (it_s = d->m_scheduleList.begin(); it_s != d->m_scheduleList.end(); ++it_s) {
    if ((*it_s).hasReferenceTo(tag.id())) {
      throw MYMONEYEXCEPTION(QString("Cannot remove tag that is still referenced to a %1").arg("schedule"));
    }
  }

  // remove any reference to report and/or budget
  d->removeReferences(tag.id());

  d->m_tagList.remove((*it_ta).id());
}

QList<MyMoneyTag> MyMoneySeqAccessMgr::tagList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_tagList.values();
}

void MyMoneySeqAccessMgr::addAccount(MyMoneyAccount& parent, MyMoneyAccount& account)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyAccount>::ConstIterator theParent;
  QMap<QString, MyMoneyAccount>::ConstIterator theChild;

  theParent = d->m_accountList.find(parent.id());
  if (theParent == d->m_accountList.end()) {
    QString msg = "Unknown parent account '";
    msg += parent.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  theChild = d->m_accountList.find(account.id());
  if (theChild == d->m_accountList.end()) {
    QString msg = "Unknown child account '";
    msg += account.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  auto acc = *theParent;
  acc.addAccountId(account.id());
  d->m_accountList.modify(acc.id(), acc);
  parent = acc;

  acc = *theChild;
  acc.setParentAccountId(parent.id());
  d->m_accountList.modify(acc.id(), acc);
  account = acc;

}

void MyMoneySeqAccessMgr::addInstitution(MyMoneyInstitution& institution)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneyInstitution newInstitution(nextInstitutionID(), institution);

  d->m_institutionList.insert(newInstitution.id(), newInstitution);

  // return new data
  institution = newInstitution;
}

uint MyMoneySeqAccessMgr::transactionCount(const QString& account) const
{
  Q_D(const MyMoneySeqAccessMgr);
  uint cnt = 0;

  if (account.length() == 0) {
    cnt = d->m_transactionList.count();

  } else {
    // scan all transactions
    foreach (const auto transaction, d->m_transactionList) {
      // scan all splits of this transaction
      auto found = false;
      foreach (const auto split, transaction.splits()) {
        // is it a split in our account?
        if (split.accountId() == account) {
          // since a transaction can only have one split referencing
          // each account, we're done with the splits here!
          found = true;
          break;
        }
      }
      // if no split contains the account id, continue with the
      // next transaction
      if (!found)
        continue;

      // otherwise count it
      ++cnt;
    }
  }
  return cnt;
}

QMap<QString, ulong> MyMoneySeqAccessMgr::transactionCountMap() const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, ulong> map;

  // scan all transactions
  foreach (const auto transaction, d->m_transactionList) {
    // scan all splits of this transaction
    foreach (const auto split, transaction.splits()) {
      map[split.accountId()]++;
    }
  }
  return map;
}

uint MyMoneySeqAccessMgr::institutionCount() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_institutionList.count();
}

uint MyMoneySeqAccessMgr::accountCount() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_accountList.count();
}

void MyMoneySeqAccessMgr::addTransaction(MyMoneyTransaction& transaction, bool skipAccountUpdate)
{
  Q_D(MyMoneySeqAccessMgr);
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
  foreach (const auto split, transaction.splits()) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account(split.accountId());
    if (!split.payeeId().isEmpty())
      payee(split.payeeId());
  }

  MyMoneyTransaction newTransaction(nextTransactionID(), transaction);
  QString key = newTransaction.uniqueSortKey();

  d->m_transactionList.insert(key, newTransaction);
  d->m_transactionKeys.insert(newTransaction.id(), key);

  transaction = newTransaction;

  // adjust the balance of all affected accounts
  foreach (const auto split, transaction.splits()) {
    auto acc = d->m_accountList[split.accountId()];
    d->adjustBalance(acc, split, false);
    if (!skipAccountUpdate) {
      acc.touch();
    }
    d->m_accountList.modify(acc.id(), acc);
  }
}

bool MyMoneySeqAccessMgr::hasActiveSplits(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyTransaction>::ConstIterator it;

  for (it = d->m_transactionList.begin(); it != d->m_transactionList.end(); ++it) {
    if ((*it).accountReferenced(id)) {
      return true;
    }
  }
  return false;
}

MyMoneyInstitution MyMoneySeqAccessMgr::institution(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;

  pos = d->m_institutionList.find(id);
  if (pos != d->m_institutionList.end())
    return *pos;
  throw MYMONEYEXCEPTION("unknown institution");
}

bool MyMoneySeqAccessMgr::dirty() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_dirty;
}

void MyMoneySeqAccessMgr::setDirty()
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_dirty = true;
}

QList<MyMoneyInstitution> MyMoneySeqAccessMgr::institutionList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_institutionList.values();
}

void MyMoneySeqAccessMgr::modifyAccount(const MyMoneyAccount& account, bool skipCheck)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyAccount>::ConstIterator pos;

  // locate the account in the file global pool
  pos = d->m_accountList.find(account.id());
  if (pos != d->m_accountList.end()) {
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

      foreach (const auto sAccount, account.accountList())
        this->account(sAccount);

      // update information in account list
      d->m_accountList.modify(account.id(), account);

    } else
      throw MYMONEYEXCEPTION("Invalid information for update");

  } else
    throw MYMONEYEXCEPTION("Unknown account id");
}

void MyMoneySeqAccessMgr::modifyInstitution(const MyMoneyInstitution& institution)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;

  // locate the institution in the file global pool
  pos = d->m_institutionList.find(institution.id());
  if (pos != d->m_institutionList.end()) {
    d->m_institutionList.modify(institution.id(), institution);

  } else
    throw MYMONEYEXCEPTION("unknown institution");
}

void MyMoneySeqAccessMgr::modifyTransaction(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneySeqAccessMgr);
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
  foreach (const auto split, transaction.splits()) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account(split.accountId());
    if (!split.payeeId().isEmpty())
      payee(split.payeeId());
    foreach (const auto tagId, split.tagIdList()) {
      if (!tagId.isEmpty())
        tag(tagId);
    }
  }

  // new data seems to be ok. find old version of transaction
  // in our pool. Throw exception if unknown.
  if (!d->m_transactionKeys.contains(transaction.id()))
    throw MYMONEYEXCEPTION("invalid transaction id");

  QString oldKey = d->m_transactionKeys[transaction.id()];
  if (!d->m_transactionList.contains(oldKey))
    throw MYMONEYEXCEPTION("invalid transaction key");

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

  it_t = d->m_transactionList.find(oldKey);
  if (it_t == d->m_transactionList.end())
    throw MYMONEYEXCEPTION("invalid transaction key");

  foreach (const auto split, (*it_t).splits()) {
    auto acc = d->m_accountList[split.accountId()];
    // we only need to adjust non-investment accounts here
    // as for investment accounts the balance will be recalculated
    // after the transaction has been added.
    if (!acc.isInvest()) {
      d->adjustBalance(acc, split, true);
      acc.touch();
      d->m_accountList.modify(acc.id(), acc);
    }
  }

  // remove old transaction from lists
  d->m_transactionList.remove(oldKey);

  // add new transaction to lists
  QString newKey = transaction.uniqueSortKey();
  d->m_transactionList.insert(newKey, transaction);
  d->m_transactionKeys.modify(transaction.id(), newKey);

  // adjust account balances
  foreach (const auto split, transaction.splits()) {
    auto acc = d->m_accountList[split.accountId()];
    d->adjustBalance(acc, split, false);
    acc.touch();
    d->m_accountList.modify(acc.id(), acc);
  }
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  Q_D(MyMoneySeqAccessMgr);
  d->reparentAccount(account, parent, true);
}

void MyMoneySeqAccessMgr::close()
{
}

void MyMoneySeqAccessMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  Q_D(MyMoneySeqAccessMgr);
  // first perform all the checks
  if (transaction.id().isEmpty())
    throw MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap<QString, QString>::ConstIterator it_k;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

  it_k = d->m_transactionKeys.find(transaction.id());
  if (it_k == d->m_transactionKeys.end())
    throw MYMONEYEXCEPTION("invalid transaction to be deleted");

  it_t = d->m_transactionList.find(*it_k);
  if (it_t == d->m_transactionList.end())
    throw MYMONEYEXCEPTION("invalid transaction key");

  // keep a copy so that we still have the data after removal
  MyMoneyTransaction t(*it_t);

  // FIXME: check if any split is frozen and throw exception

  // remove the transaction from the two lists
  d->m_transactionList.remove(*it_k);
  d->m_transactionKeys.remove(transaction.id());

  // scan the splits and collect all accounts that need
  // to be updated after the removal of this transaction
  foreach (const auto split, t.splits()) {
    auto acc = d->m_accountList[split.accountId()];
    d->adjustBalance(acc, split, true);
    acc.touch();
    d->m_accountList.modify(acc.id(), acc);
  }

}

void MyMoneySeqAccessMgr::removeAccount(const MyMoneyAccount& account)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneyAccount parent;

  // check that the account and it's parent exist
  // this will throw an exception if the id is unknown
  MyMoneySeqAccessMgr::account(account.id());
  parent = MyMoneySeqAccessMgr::account(account.parentAccountId());

  // check that it's not one of the standard account groups
  if (isStandardAccount(account.id()))
    throw MYMONEYEXCEPTION("Unable to remove the standard account groups");

  if (hasActiveSplits(account.id())) {
    throw MYMONEYEXCEPTION("Unable to remove account with active splits");
  }

  // re-parent all sub-ordinate accounts to the parent of the account
  // to be deleted. First round check that all accounts exist, second
  // round do the re-parenting.

  foreach (const auto accountID, account.accountList())
    MyMoneySeqAccessMgr::account(accountID);

  // if one of the accounts did not exist, an exception had been
  // thrown and we would not make it until here.

  QMap<QString, MyMoneyAccount>::ConstIterator it_a;
  QMap<QString, MyMoneyAccount>::ConstIterator it_p;

  // locate the account in the file global pool

  it_a = d->m_accountList.find(account.id());
  if (it_a == d->m_accountList.end())
    throw MYMONEYEXCEPTION("Internal error: account not found in list");

  it_p = d->m_accountList.find(parent.id());
  if (it_p == d->m_accountList.end())
    throw MYMONEYEXCEPTION("Internal error: parent account not found in list");

  if (!account.institutionId().isEmpty())
    throw MYMONEYEXCEPTION("Cannot remove account still attached to an institution");

  d->removeReferences(account.id());

  // FIXME: check referential integrity for the account to be removed

  // check if the new info is based on the old one.
  // this is the case, when the file and the id
  // as well as the type are equal.
  if ((*it_a).id() == account.id()
      && (*it_a).accountType() == account.accountType()) {

    // second round over sub-ordinate accounts: do re-parenting
    // but only if the list contains at least one entry
    // FIXME: move this logic to MyMoneyFile

    foreach (const auto accountID, (*it_a).accountList()) {
      MyMoneyAccount acc(MyMoneySeqAccessMgr::account(accountID));
      d->reparentAccount(acc, parent, false);
    }

    // remove account from parent's list
    parent.removeAccountId(account.id());
    d->m_accountList.modify(parent.id(), parent);

    // remove account from the global account pool
    d->m_accountList.remove(account.id());
  }
}

void MyMoneySeqAccessMgr::removeInstitution(const MyMoneyInstitution& institution)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyInstitution>::ConstIterator it_i;

  it_i = d->m_institutionList.find(institution.id());
  if (it_i != d->m_institutionList.end()) {
    d->m_institutionList.remove(institution.id());

  } else
    throw MYMONEYEXCEPTION("invalid institution");
}

void MyMoneySeqAccessMgr::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  Q_D(const MyMoneySeqAccessMgr);
  list.clear();

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t_end = d->m_transactionList.end();

  for (it_t = d->m_transactionList.begin(); it_t != it_t_end; ++it_t) {
    // This code is used now. It adds the transaction to the list for
    // each matching split exactly once. This allows to show information
    // about different splits in the same register view (e.g. search result)
    //
    // I have no idea, if this has some impact on the functionality. So far,
    // I could not see it.  (ipwizard 9/5/2003)
    if (filter.match(*it_t)) {
      uint cnt = filter.matchingSplits().count();
      if (cnt > 1) {
        for (uint i = 0; i < cnt; ++i)
          list.append(*it_t);
      } else {
        list.append(*it_t);
      }
    }
  }
}

void MyMoneySeqAccessMgr::transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  Q_D(const MyMoneySeqAccessMgr);
  list.clear();

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t_end = d->m_transactionList.end();

  for (it_t = d->m_transactionList.begin(); it_t != it_t_end; ++it_t) {
    if (filter.match(*it_t)) {
      foreach (const auto split, filter.matchingSplits())
        list.append(qMakePair(*it_t, split));
    }
  }
}

QList<MyMoneyTransaction> MyMoneySeqAccessMgr::transactionList(MyMoneyTransactionFilter& filter) const
{
  QList<MyMoneyTransaction> list;
  transactionList(list, filter);
  return list;
}

QList<onlineJob> MyMoneySeqAccessMgr::onlineJobList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_onlineJobList.values();
}

QList< MyMoneyCostCenter > MyMoneySeqAccessMgr::costCenterList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_costCenterList.values();
}

MyMoneyCostCenter MyMoneySeqAccessMgr::costCenter(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  if (!d->m_costCenterList.contains(id)) {
    QString msg = QString("Invalid cost center id '%1'").arg(id);
    throw MYMONEYEXCEPTION(msg);
  }
  return d->m_costCenterList[id];
}

bool MyMoneySeqAccessMgr::isDuplicateTransaction(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_transactionKeys.contains(id);
}

MyMoneyTransaction MyMoneySeqAccessMgr::transaction(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  // get the full key of this transaction, throw exception
  // if it's invalid (unknown)
  if (!d->m_transactionKeys.contains(id)) {
    QString msg = QString("Invalid transaction id '%1'").arg(id);
    throw MYMONEYEXCEPTION(msg);
  }

  // check if this key is in the list, throw exception if not
  QString key = d->m_transactionKeys[id];
  if (!d->m_transactionList.contains(key)) {
    QString msg = QString("Invalid transaction key '%1'").arg(key);
    throw MYMONEYEXCEPTION(msg);
  }

  return d->m_transactionList[key];
}

MyMoneyTransaction MyMoneySeqAccessMgr::transaction(const QString& account, const int idx) const
{
  Q_D(const MyMoneySeqAccessMgr);
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
  auto acc = d->m_accountList[account];
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

MyMoneyMoney MyMoneySeqAccessMgr::balance(const QString& id, const QDate& date) const
{
  Q_D(const MyMoneySeqAccessMgr);
  if (!d->m_accountList.contains(id))
    throw MYMONEYEXCEPTION(QString("Unknown account id '%1'").arg(id));

  if (!date.isValid()) {
    // the balance of all transactions for this account has
    // been requested. no need to calculate anything as we
    // have this number with the account object already.
    if (d->m_accountList.find(id) != d->m_accountList.end()) {
      return d->m_accountList[id].balance();
    }
    return MyMoneyMoney();
  }

  return d->calculateBalance(id, date);
}


MyMoneyMoney MyMoneySeqAccessMgr::totalBalance(const QString& id, const QDate& date) const
{
  MyMoneyMoney result(balance(id, date));

  foreach (const auto sAccount, account(id).accountList())
    result += totalBalance(sAccount, date);

  return result;
}

MyMoneyAccount MyMoneySeqAccessMgr::liability() const {
  return account(stdAccNames[stdAccLiability]);
}

MyMoneyAccount MyMoneySeqAccessMgr::asset() const {
  return account(stdAccNames[stdAccAsset]);
}

MyMoneyAccount MyMoneySeqAccessMgr::expense() const {
  return account(stdAccNames[stdAccExpense]);
}

MyMoneyAccount MyMoneySeqAccessMgr::income() const {
  return account(stdAccNames[stdAccIncome]);
}

MyMoneyAccount MyMoneySeqAccessMgr::equity() const {
  return account(stdAccNames[stdAccEquity]);
}

void MyMoneySeqAccessMgr::loadAccounts(const QMap<QString, MyMoneyAccount>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_accountList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyAccount>::const_iterator it_a;
  QString lastId("");
  for (it_a = map.begin(); it_a != map.end(); ++it_a) {
    if (!isStandardAccount((*it_a).id()) && ((*it_a).id() > lastId))
      lastId = (*it_a).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextAccountID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadTransactions(const QMap<QString, MyMoneyTransaction>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_transactionList = map;

  // now fill the key map and
  // identify the last used id
  QString lastId("");
  QMap<QString, QString> keys;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

  for (it_t = map.constBegin(); it_t != map.constEnd(); ++it_t) {
    keys[(*it_t).id()] = it_t.key();
    if ((*it_t).id() > lastId)
      lastId = (*it_t).id();
  }
  d->m_transactionKeys = keys;

  // determine highest used ID so far
  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextTransactionID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadInstitutions(const QMap<QString, MyMoneyInstitution>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_institutionList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyInstitution>::const_iterator it_i;
  QString lastId("");
  for (it_i = map.begin(); it_i != map.end(); ++it_i) {
    if ((*it_i).id() > lastId)
      lastId = (*it_i).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextInstitutionID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadPayees(const QMap<QString, MyMoneyPayee>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_payeeList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyPayee>::const_iterator it_p;
  QString lastId("");
  for (it_p = map.begin(); it_p != map.end(); ++it_p) {
    if ((*it_p).id().length() <= PAYEE_ID_SIZE + 1) {
      if ((*it_p).id() > lastId)
        lastId = (*it_p).id();
    } else {
    }
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextPayeeID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadTags(const QMap<QString, MyMoneyTag>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_tagList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyTag>::const_iterator it_ta;
  QString lastId("");
  for (it_ta = map.begin(); it_ta != map.end(); ++it_ta) {
    if ((*it_ta).id().length() <= TAG_ID_SIZE + 1) {
      if ((*it_ta).id() > lastId)
        lastId = (*it_ta).id();
    } else {
    }
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextTagID = lastId.mid(pos).toUInt();
  }
}

void MyMoneySeqAccessMgr::loadSecurities(const QMap<QString, MyMoneySecurity>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_securitiesList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneySecurity>::const_iterator it_s;
  QString lastId("");
  for (it_s = map.begin(); it_s != map.end(); ++it_s) {
    if ((*it_s).id() > lastId)
      lastId = (*it_s).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextSecurityID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadCurrencies(const QMap<QString, MyMoneySecurity>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_currencyList = map;
}

void MyMoneySeqAccessMgr::loadPrices(const MyMoneyPriceList& list)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_priceList = list;
}

void MyMoneySeqAccessMgr::loadOnlineJobs(const QMap< QString, onlineJob >& onlineJobs)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_onlineJobList = onlineJobs;
  QString lastId("");
  const QMap< QString, onlineJob >::const_iterator end = onlineJobs.constEnd();
  for (QMap< QString, onlineJob >::const_iterator iter = onlineJobs.constBegin(); iter != end; ++iter) {
    if ((*iter).id() > lastId)
      lastId = (*iter).id();
  }

  const int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextOnlineJobID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadCostCenters(const QMap< QString, MyMoneyCostCenter >& costCenters)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_costCenterList = costCenters;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyCostCenter>::const_iterator it_s;
  QString lastId;
  for (it_s = costCenters.constBegin(); it_s != costCenters.constEnd(); ++it_s) {
    if ((*it_s).id() > lastId)
      lastId = (*it_s).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextCostCenterID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadAccountId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextAccountID = id;
}

void MyMoneySeqAccessMgr::loadTransactionId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextTransactionID = id;
}

void MyMoneySeqAccessMgr::loadPayeeId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextPayeeID = id;
}

void MyMoneySeqAccessMgr::loadTagId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextTagID = id;
}

void MyMoneySeqAccessMgr::loadInstitutionId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextInstitutionID = id;
}

void MyMoneySeqAccessMgr::loadSecurityId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextSecurityID = id;
}

void MyMoneySeqAccessMgr::loadReportId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextReportID = id;
}

void MyMoneySeqAccessMgr::loadBudgetId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextBudgetID = id;
}

void MyMoneySeqAccessMgr::loadOnlineJobId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextOnlineJobID = id;
}

void MyMoneySeqAccessMgr::loadCostCenterId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextCostCenterID = id;
}

ulong MyMoneySeqAccessMgr::accountId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextAccountID;
}

ulong MyMoneySeqAccessMgr::transactionId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextTransactionID;
}

ulong MyMoneySeqAccessMgr::payeeId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextPayeeID;
}

ulong MyMoneySeqAccessMgr::tagId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextTagID;
}

ulong MyMoneySeqAccessMgr::institutionId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextInstitutionID;
}

ulong MyMoneySeqAccessMgr::scheduleId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextScheduleID;
}

ulong MyMoneySeqAccessMgr::securityId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextSecurityID;
}

ulong MyMoneySeqAccessMgr::reportId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextReportID;
}

ulong MyMoneySeqAccessMgr::budgetId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextBudgetID;
}

ulong MyMoneySeqAccessMgr::costCenterId() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_nextCostCenterID;
}

QString MyMoneySeqAccessMgr::value(const QString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneySeqAccessMgr::setValue(const QString& key, const QString& val)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneyKeyValueContainer::setValue(key, val);
  d->touch();
}

void MyMoneySeqAccessMgr::deletePair(const QString& key)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneyKeyValueContainer::deletePair(key);
  d->touch();
}

QMap<QString, QString> MyMoneySeqAccessMgr::pairs() const
{
  return MyMoneyKeyValueContainer::pairs();
}

void MyMoneySeqAccessMgr::setPairs(const QMap<QString, QString>& list)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneyKeyValueContainer::setPairs(list);
  d->touch();
}

void MyMoneySeqAccessMgr::addSchedule(MyMoneySchedule& sched)
{
  Q_D(MyMoneySeqAccessMgr);
  // first perform all the checks
  if (!sched.id().isEmpty())
    throw MYMONEYEXCEPTION("schedule already contains an id");

  // The following will throw an exception when it fails
  sched.validate(false);

  MyMoneySchedule newSched(nextScheduleID(), sched);
  d->m_scheduleList.insert(newSched.id(), newSched);
  sched = newSched;
}

void MyMoneySeqAccessMgr::modifySchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = d->m_scheduleList.find(sched.id());
  if (it == d->m_scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_scheduleList.modify(sched.id(), sched);
}

void MyMoneySeqAccessMgr::removeSchedule(const MyMoneySchedule& sched)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = d->m_scheduleList.find(sched.id());
  if (it == d->m_scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  // FIXME: check referential integrity for loan accounts
  d->m_scheduleList.remove(sched.id());
}

MyMoneySchedule MyMoneySeqAccessMgr::schedule(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySchedule>::ConstIterator pos;

  // locate the schedule and if present, return it's data
  pos = d->m_scheduleList.find(id);
  if (pos != d->m_scheduleList.end())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown schedule id '" + id + '\'';
  throw MYMONEYEXCEPTION(msg);
}

QList<MyMoneySchedule> MyMoneySeqAccessMgr::scheduleList(const QString& accountId,
  eMyMoney::Schedule::Type type,
  eMyMoney::Schedule::Occurrence occurrence,
  eMyMoney::Schedule::PaymentType paymentType,
  const QDate& startDate,
  const QDate& endDate,
  bool overdue) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySchedule>::ConstIterator pos;
  QList<MyMoneySchedule> list;

  // qDebug("scheduleList()");

  for (pos = d->m_scheduleList.begin(); pos != d->m_scheduleList.end(); ++pos) {
    // qDebug("  '%s'", qPrintable((*pos).id()));

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
    }

    // qDebug("Adding '%s'", (*pos).name().toLatin1());
    list << *pos;
  }
  return list;
}

void MyMoneySeqAccessMgr::loadSchedules(const QMap<QString, MyMoneySchedule>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_scheduleList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneySchedule>::const_iterator it_s;
  QString lastId("");
  for (it_s = map.begin(); it_s != map.end(); ++it_s) {
    if ((*it_s).id() > lastId)
      lastId = (*it_s).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextScheduleID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadScheduleId(ulong id)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_nextScheduleID = id;
}

QList<MyMoneySchedule> MyMoneySeqAccessMgr::scheduleListEx(int scheduleTypes,
    int scheduleOcurrences,
    int schedulePaymentTypes,
    QDate date,
    const QStringList& accounts) const
{
  Q_D(const MyMoneySeqAccessMgr);
//  qDebug("scheduleListEx");

  QMap<QString, MyMoneySchedule>::ConstIterator pos;
  QList<MyMoneySchedule> list;

  if (!date.isValid())
    return list;

  for (pos = d->m_scheduleList.begin(); pos != d->m_scheduleList.end(); ++pos) {
    if (scheduleTypes && !(scheduleTypes & (int)(*pos).type()))
      continue;

    if (scheduleOcurrences && !(scheduleOcurrences & (int)(*pos).occurrence()))
      continue;

    if (schedulePaymentTypes && !(schedulePaymentTypes & (int)(*pos).paymentType()))
      continue;

    if ((*pos).paymentDates(date, date).count() == 0)
      continue;

    if ((*pos).isFinished())
      continue;

    if ((*pos).hasRecordedPayment(date))
      continue;

    if (accounts.count() > 0) {
      if (accounts.contains((*pos).account().id()))
        continue;
    }

//    qDebug("\tAdding '%s'", (*pos).name().toLatin1());
    list << *pos;
  }

  return list;
}

void MyMoneySeqAccessMgr::addSecurity(MyMoneySecurity& security)
{
  Q_D(MyMoneySeqAccessMgr);
  // create the account
  MyMoneySecurity newSecurity(nextSecurityID(), security);

  d->m_securitiesList.insert(newSecurity.id(), newSecurity);

  security = newSecurity;
}

void MyMoneySeqAccessMgr::modifySecurity(const MyMoneySecurity& security)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = d->m_securitiesList.find(security.id());
  if (it == d->m_securitiesList.end()) {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during modifySecurity()";
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_securitiesList.modify(security.id(), security);
}

void MyMoneySeqAccessMgr::removeSecurity(const MyMoneySecurity& security)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = d->m_securitiesList.find(security.id());
  if (it == d->m_securitiesList.end()) {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during removeSecurity()";
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_securitiesList.remove(security.id());
}

MyMoneySecurity MyMoneySeqAccessMgr::security(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySecurity>::ConstIterator it = d->m_securitiesList.find(id);
  if (it != d->m_securitiesList.end()) {
    return it.value();
  }

  return MyMoneySecurity();
}

QList<MyMoneySecurity> MyMoneySeqAccessMgr::securityList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  //qDebug("securityList: Security list size is %d, this=%8p", m_equitiesList.size(), (void*)this);
  return d->m_securitiesList.values();
}

void MyMoneySeqAccessMgr::addCurrency(const MyMoneySecurity& currency)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = d->m_currencyList.find(currency.id());
  if (it != d->m_currencyList.end()) {
    throw MYMONEYEXCEPTION(i18n("Cannot add currency with existing id %1", currency.id()));
  }

  d->m_currencyList.insert(currency.id(), currency);
}

void MyMoneySeqAccessMgr::modifyCurrency(const MyMoneySecurity& currency)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = d->m_currencyList.find(currency.id());
  if (it == d->m_currencyList.end()) {
    throw MYMONEYEXCEPTION(i18n("Cannot modify currency with unknown id %1", currency.id()));
  }

  d->m_currencyList.modify(currency.id(), currency);
}

void MyMoneySeqAccessMgr::removeCurrency(const MyMoneySecurity& currency)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = d->m_currencyList.find(currency.id());
  if (it == d->m_currencyList.end()) {
    throw MYMONEYEXCEPTION(i18n("Cannot remove currency with unknown id %1", currency.id()));
  }

  d->m_currencyList.remove(currency.id());
}

MyMoneySecurity MyMoneySeqAccessMgr::currency(const QString& id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  if (id.isEmpty()) {

  }
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = d->m_currencyList.find(id);
  if (it == d->m_currencyList.end()) {
    throw MYMONEYEXCEPTION(i18n("Cannot retrieve currency with unknown id '%1'", id));
  }

  return *it;
}

QList<MyMoneySecurity> MyMoneySeqAccessMgr::currencyList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_currencyList.values();
}

QList<MyMoneyReport> MyMoneySeqAccessMgr::reportList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_reportList.values();
}

void MyMoneySeqAccessMgr::addReport(MyMoneyReport& report)
{
  Q_D(MyMoneySeqAccessMgr);
  if (!report.id().isEmpty())
    throw MYMONEYEXCEPTION("report already contains an id");

  MyMoneyReport newReport(nextReportID(), report);
  d->m_reportList.insert(newReport.id(), newReport);
  report = newReport;
}

void MyMoneySeqAccessMgr::loadReports(const QMap<QString, MyMoneyReport>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_reportList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyReport>::const_iterator it_r;
  QString lastId("");
  for (it_r = map.begin(); it_r != map.end(); ++it_r) {
    if ((*it_r).id() > lastId)
      lastId = (*it_r).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextReportID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::modifyReport(const MyMoneyReport& report)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyReport>::ConstIterator it;

  it = d->m_reportList.find(report.id());
  if (it == d->m_reportList.end()) {
    QString msg = "Unknown report '" + report.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }
  d->m_reportList.modify(report.id(), report);
}

uint MyMoneySeqAccessMgr::countReports() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_reportList.count();
}

MyMoneyReport MyMoneySeqAccessMgr::report(const QString& _id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_reportList[_id];
}

void MyMoneySeqAccessMgr::removeReport(const MyMoneyReport& report)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyReport>::ConstIterator it;

  it = d->m_reportList.find(report.id());
  if (it == d->m_reportList.end()) {
    QString msg = "Unknown report '" + report.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_reportList.remove(report.id());
}

QList<MyMoneyBudget> MyMoneySeqAccessMgr::budgetList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_budgetList.values();
}


void MyMoneySeqAccessMgr::addBudget(MyMoneyBudget& budget)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneyBudget newBudget(nextBudgetID(), budget);
  d->m_budgetList.insert(newBudget.id(), newBudget);
  budget = newBudget;
}

void MyMoneySeqAccessMgr::loadBudgets(const QMap<QString, MyMoneyBudget>& map)
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_budgetList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyBudget>::const_iterator it_b;
  QString lastId("");
  for (it_b = map.begin(); it_b != map.end(); ++it_b) {
    if ((*it_b).id() > lastId)
      lastId = (*it_b).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    d->m_nextBudgetID = lastId.mid(pos).toInt();
  }
}

MyMoneyBudget MyMoneySeqAccessMgr::budgetByName(const QString& budget) const
{
  Q_D(const MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyBudget>::ConstIterator it_p;

  for (it_p = d->m_budgetList.begin(); it_p != d->m_budgetList.end(); ++it_p) {
    if ((*it_p).name() == budget) {
      return *it_p;
    }
  }

  throw MYMONEYEXCEPTION("Unknown budget '" + budget + '\'');
}

void MyMoneySeqAccessMgr::modifyBudget(const MyMoneyBudget& budget)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyBudget>::ConstIterator it;

  it = d->m_budgetList.find(budget.id());
  if (it == d->m_budgetList.end()) {
    QString msg = "Unknown budget '" + budget.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }
  d->m_budgetList.modify(budget.id(), budget);
}

uint MyMoneySeqAccessMgr::countBudgets() const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_budgetList.count();
}

MyMoneyBudget MyMoneySeqAccessMgr::budget(const QString& _id) const
{
  Q_D(const MyMoneySeqAccessMgr);
  return d->m_budgetList[_id];
}

void MyMoneySeqAccessMgr::removeBudget(const MyMoneyBudget& budget)
{
  Q_D(MyMoneySeqAccessMgr);
  QMap<QString, MyMoneyBudget>::ConstIterator it;

  it = d->m_budgetList.find(budget.id());
  if (it == d->m_budgetList.end()) {
    QString msg = "Unknown budget '" + budget.id() + '\'';
    throw MYMONEYEXCEPTION(msg);
  }

  d->m_budgetList.remove(budget.id());
}

void MyMoneySeqAccessMgr::addPrice(const MyMoneyPrice& price)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneySecurityPair pricePair(price.from(), price.to());
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::ConstIterator it_m;
  it_m = d->m_priceList.find(pricePair);

  MyMoneyPriceEntries entries;
  if (it_m != d->m_priceList.end()) {
    entries = (*it_m);
  }
  // entries contains the current entries for this security pair
  // in case it_m points to m_priceList.end() we need to create a
  // new entry in the priceList, otherwise we need to modify
  // an existing one.

  MyMoneyPriceEntries::ConstIterator it;
  it = entries.constFind(price.date());
  if (it != entries.constEnd()) {
    if ((*it).rate(QString()) == price.rate(QString())
        && (*it).source() == price.source())
      // in case the information did not change, we don't do anything
      return;
  }

  // store new value in local copy
  entries[price.date()] = price;

  if (it_m != d->m_priceList.end()) {
    d->m_priceList.modify(pricePair, entries);
  } else {
    d->m_priceList.insert(pricePair, entries);
  }
}

void MyMoneySeqAccessMgr::removePrice(const MyMoneyPrice& price)
{
  Q_D(MyMoneySeqAccessMgr);
  MyMoneySecurityPair pricePair(price.from(), price.to());
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::ConstIterator it_m;
  it_m = d->m_priceList.find(pricePair);

  MyMoneyPriceEntries entries;
  if (it_m != d->m_priceList.end()) {
    entries = (*it_m);
  }

  // store new value in local copy
  entries.remove(price.date());

  if (entries.count() != 0) {
    d->m_priceList.modify(pricePair, entries);
  } else {
    d->m_priceList.remove(pricePair);
  }
}

MyMoneyPriceList MyMoneySeqAccessMgr::priceList() const
{
  Q_D(const MyMoneySeqAccessMgr);
  MyMoneyPriceList list;
  d->m_priceList.map(list);
  return list;
}

MyMoneyPrice MyMoneySeqAccessMgr::price(const QString& fromId, const QString& toId, const QDate& _date, bool exactDate) const
{
  Q_D(const MyMoneySeqAccessMgr);
  // if the caller selected an exact entry, we can search for it using the date as the key
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::const_iterator itm = d->m_priceList.find(qMakePair(fromId, toId));
  if (itm != d->m_priceList.end()) {
    // if no valid date is passed, we use today's date.
    const QDate &date = _date.isValid() ? _date : QDate::currentDate();
    const MyMoneyPriceEntries &entries = itm.value();
    // regardless of the exactDate flag if the exact date is present return it's value since it's the correct value
    MyMoneyPriceEntries::const_iterator it = entries.find(date);
    if (it != entries.end())
      return it.value();

    // the exact date was not found look for the latest date before the requested date if the flag allows it
    if (!exactDate && !entries.empty()) {
      // if there are entries get the lower bound of the date
      it = entries.lowerBound(date);
      // since lower bound returns the first item with a larger key (we already know that key is not present)
      // if it's not the first item then we need to return the previous item (the map is not empty so there is one)
      if (it != entries.begin()) {
        return (--it).value();
      }
    }
  }
  return MyMoneyPrice();
}

void MyMoneySeqAccessMgr::rebuildAccountBalances()
{
  Q_D(MyMoneySeqAccessMgr);
  // reset the balance of all accounts to 0
  QMap<QString, MyMoneyAccount> map;
  d->m_accountList.map(map);

  QMap<QString, MyMoneyAccount>::iterator it_a;
  for (it_a = map.begin(); it_a != map.end(); ++it_a) {
    (*it_a).setBalance(MyMoneyMoney());
  }

  // now scan over all transactions and all splits and setup the balances
  foreach (const auto transaction, d->m_transactionList) {
    foreach (const auto split, transaction.splits()) {
      if (!split.shares().isZero()) {
        const QString& id = split.accountId();
        // locate the account and if present, update data
        if (map.find(id) != map.end()) {
          map[id].adjustBalance(split);
        }
      }
    }
  }

  d->m_accountList = map;
}

bool MyMoneySeqAccessMgr::isReferenced(const MyMoneyObject& obj, const QBitArray& skipCheck) const
{
  Q_D(const MyMoneySeqAccessMgr);
  Q_ASSERT(skipCheck.count() == (int)Reference::Count);

  // We delete all references in reports when an object
  // is deleted, so we don't need to check here. See
  // MyMoneySeqAccessMgr::removeReferences(). In case
  // you miss the report checks in the following lines ;)

  const auto& id = obj.id();

  // FIXME optimize the list of objects we have to checks
  //       with a bit of knowledge of the internal structure, we
  //       could optimize the number of objects we check for references

  // Scan all engine objects for a reference
  if (!skipCheck.testBit((int)Reference::Transaction))
    foreach (const auto it, d->m_transactionList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Account))
    foreach (const auto it, d->m_accountList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Institution))
    foreach (const auto it, d->m_institutionList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Payee))
    foreach (const auto it, d->m_payeeList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Tag))
    foreach (const auto it, d->m_tagList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Budget))
    foreach (const auto it, d->m_budgetList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Schedule))
    foreach (const auto it, d->m_scheduleList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Security))
    foreach (const auto it, d->m_securitiesList)
      if (it.hasReferenceTo(id))
        return true;

  if (!skipCheck.testBit((int)Reference::Currency))
    foreach (const auto it, d->m_currencyList)
      if (it.hasReferenceTo(id))
        return true;

  // within the pricelist we don't have to scan each entry. Checking the QPair
  // members of the MyMoneySecurityPair is enough as they are identical to the
  // two security ids
  if (!skipCheck.testBit((int)Reference::Price)) {
    for (auto it_pr = d->m_priceList.begin(); it_pr != d->m_priceList.end(); ++it_pr) {
      if ((it_pr.key().first == id) || (it_pr.key().second == id))
        return true;
    }
  }

  return false;
}

void MyMoneySeqAccessMgr::startTransaction()
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_payeeList.startTransaction(&d->m_nextPayeeID);
  d->m_tagList.startTransaction(&d->m_nextTagID);
  d->m_institutionList.startTransaction(&d->m_nextInstitutionID);
  d->m_accountList.startTransaction(&d->m_nextPayeeID);
  d->m_transactionList.startTransaction(&d->m_nextTransactionID);
  d->m_transactionKeys.startTransaction();
  d->m_scheduleList.startTransaction(&d->m_nextScheduleID);
  d->m_securitiesList.startTransaction(&d->m_nextSecurityID);
  d->m_currencyList.startTransaction();
  d->m_reportList.startTransaction(&d->m_nextReportID);
  d->m_budgetList.startTransaction(&d->m_nextBudgetID);
  d->m_priceList.startTransaction();
  d->m_onlineJobList.startTransaction(&d->m_nextOnlineJobID);
}

bool MyMoneySeqAccessMgr::commitTransaction()
{
  Q_D(MyMoneySeqAccessMgr);
  bool rc = false;
  rc |= d->m_payeeList.commitTransaction();
  rc |= d->m_tagList.commitTransaction();
  rc |= d->m_institutionList.commitTransaction();
  rc |= d->m_accountList.commitTransaction();
  rc |= d->m_transactionList.commitTransaction();
  rc |= d->m_transactionKeys.commitTransaction();
  rc |= d->m_scheduleList.commitTransaction();
  rc |= d->m_securitiesList.commitTransaction();
  rc |= d->m_currencyList.commitTransaction();
  rc |= d->m_reportList.commitTransaction();
  rc |= d->m_budgetList.commitTransaction();
  rc |= d->m_priceList.commitTransaction();
  rc |= d->m_onlineJobList.commitTransaction();

  // if there was a change, touch the whole storage object
  if (rc)
    d->touch();

  return rc;
}

void MyMoneySeqAccessMgr::rollbackTransaction()
{
  Q_D(MyMoneySeqAccessMgr);
  d->m_payeeList.rollbackTransaction();
  d->m_tagList.rollbackTransaction();
  d->m_institutionList.rollbackTransaction();
  d->m_accountList.rollbackTransaction();
  d->m_transactionList.rollbackTransaction();
  d->m_transactionKeys.rollbackTransaction();
  d->m_scheduleList.rollbackTransaction();
  d->m_securitiesList.rollbackTransaction();
  d->m_currencyList.rollbackTransaction();
  d->m_reportList.rollbackTransaction();
  d->m_budgetList.rollbackTransaction();
  d->m_priceList.rollbackTransaction();
  d->m_onlineJobList.rollbackTransaction();
}

QString MyMoneySeqAccessMgr::nextAccountID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextAccountID);
  id = 'A' + id.rightJustified(ACCOUNT_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextTransactionID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextTransactionID);
  id = 'T' + id.rightJustified(TRANSACTION_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextPayeeID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextPayeeID);
  id = 'P' + id.rightJustified(PAYEE_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextTagID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextTagID);
  id = 'G' + id.rightJustified(TAG_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextInstitutionID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextInstitutionID);
  id = 'I' + id.rightJustified(INSTITUTION_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextScheduleID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextScheduleID);
  id = "SCH" + id.rightJustified(SCHEDULE_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextSecurityID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextSecurityID);
  id = 'E' + id.rightJustified(SECURITY_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextReportID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextReportID);
  id = 'R' + id.rightJustified(REPORT_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextBudgetID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextBudgetID);
  id = 'B' + id.rightJustified(BUDGET_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextOnlineJobID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextOnlineJobID);
  id = 'O' + id.rightJustified(ONLINE_JOB_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextCostCenterID()
{
  Q_D(MyMoneySeqAccessMgr);
  QString id;
  id.setNum(++d->m_nextCostCenterID);
  id = 'C' + id.rightJustified(COSTCENTER_ID_SIZE, '0');
  return id;
}

#undef TRY
#undef CATCH
#undef PASS
