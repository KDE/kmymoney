/***************************************************************************
                          mymoneyseqaccessmgr.cpp
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                               2002 Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyseqaccessmgr.h"

#include <typeinfo>
#include "mymoneytransactionfilter.h"
#include "mymoneycategory.h"
#include <QList>

#define TRY try {
#define CATCH } catch (MyMoneyException *e) {
#define PASS } catch (MyMoneyException *e) { throw; }

bool MyMoneyBalanceCacheItem::operator ==(const MyMoneyBalanceCacheItem & right) const
{
  return ((balance == right.balance)
          && (valid == right.valid));
}

MyMoneySeqAccessMgr::MyMoneySeqAccessMgr()
{
  m_nextAccountID = 0;
  m_nextInstitutionID = 0;
  m_nextTransactionID = 0;
  m_nextPayeeID = 0;
  m_nextScheduleID = 0;
  m_nextSecurityID = 0;
  m_nextReportID = 0;
  m_nextBudgetID = 0;
  m_user = MyMoneyPayee();
  m_dirty = false;
  m_creationDate = QDate::currentDate();

  // setup standard accounts
  MyMoneyAccount acc_l;
  acc_l.setAccountType(MyMoneyAccount::Liability);
  acc_l.setName("Liability");
  MyMoneyAccount liability(STD_ACC_LIABILITY, acc_l);

  MyMoneyAccount acc_a;
  acc_a.setAccountType(MyMoneyAccount::Asset);
  acc_a.setName("Asset");
  MyMoneyAccount asset(STD_ACC_ASSET, acc_a);

  MyMoneyAccount acc_e;
  acc_e.setAccountType(MyMoneyAccount::Expense);
  acc_e.setName("Expense");
  MyMoneyAccount expense(STD_ACC_EXPENSE, acc_e);

  MyMoneyAccount acc_i;
  acc_i.setAccountType(MyMoneyAccount::Income);
  acc_i.setName("Income");
  MyMoneyAccount income(STD_ACC_INCOME, acc_i);

  MyMoneyAccount acc_q;
  acc_q.setAccountType(MyMoneyAccount::Equity);
  acc_q.setName("Equity");
  MyMoneyAccount equity(STD_ACC_EQUITY, acc_q);

  QMap<QString, MyMoneyAccount> map;
  map[STD_ACC_ASSET] = asset;
  map[STD_ACC_LIABILITY] = liability;
  map[STD_ACC_INCOME] = income;
  map[STD_ACC_EXPENSE] = expense;
  map[STD_ACC_EQUITY] = equity;

  // load account list with initial accounts
  m_accountList = map;

  MyMoneyBalanceCacheItem balance;

  m_balanceCache.clear();
  m_balanceCache[STD_ACC_LIABILITY] = balance;
  m_balanceCache[STD_ACC_ASSET] = balance;
  m_balanceCache[STD_ACC_EXPENSE] = balance;
  m_balanceCache[STD_ACC_INCOME] = balance;
  m_balanceCache[STD_ACC_EQUITY] = balance;

  // initialize for file fixes (see kmymoneyview.cpp)
  m_currentFixVersion = 2;
  m_fileFixVersion = 0; // default value if no fix-version in file
  m_transactionListFull = false;
}

MyMoneySeqAccessMgr::~MyMoneySeqAccessMgr()
{
}

MyMoneySeqAccessMgr const * MyMoneySeqAccessMgr::duplicate(void)
{
  MyMoneySeqAccessMgr* that = new MyMoneySeqAccessMgr();
  *that = *this;
  return that;
}
/**
* This method is used to get a SQL reader for subsequent database access
 */
KSharedPtr <MyMoneyStorageSql> MyMoneySeqAccessMgr::connectToDatabase
(const KUrl& /*url*/)
{
  return KSharedPtr <MyMoneyStorageSql>();
}

bool MyMoneySeqAccessMgr::isStandardAccount(const QString& id) const
{
  return id == STD_ACC_LIABILITY
         || id == STD_ACC_ASSET
         || id == STD_ACC_EXPENSE
         || id == STD_ACC_INCOME
         || id == STD_ACC_EQUITY;
}

void MyMoneySeqAccessMgr::setAccountName(const QString& id, const QString& name)
{
  if (!isStandardAccount(id))
    throw new MYMONEYEXCEPTION("Only standard accounts can be modified using setAccountName()");

  MyMoneyAccount acc = m_accountList[id];
  acc.setName(name);
  m_accountList.modify(acc.id(), acc);
}

const MyMoneyAccount MyMoneySeqAccessMgr::account(const QString& id) const
{
  // locate the account and if present, return it's data
  if (m_accountList.find(id) != m_accountList.end())
    return m_accountList[id];

  // throw an exception, if it does not exist
  QString msg = "Unknown account id '" + id + '\'';
  throw new MYMONEYEXCEPTION(msg);
}

void MyMoneySeqAccessMgr::accountList(QList<MyMoneyAccount>& list) const
{
  QMap<QString, MyMoneyAccount>::ConstIterator it;
  for (it = m_accountList.begin(); it != m_accountList.end(); ++it) {
    if (!isStandardAccount((*it).id())) {
      list.append(*it);
    }
  }
}

void MyMoneySeqAccessMgr::addAccount(MyMoneyAccount& account)
{
  // create the account.
  MyMoneyAccount newAccount(nextAccountID(), account);
  m_accountList.insert(newAccount.id(), newAccount);

  account = newAccount;
}

void MyMoneySeqAccessMgr::addPayee(MyMoneyPayee& payee)
{
  // create the payee
  MyMoneyPayee newPayee(nextPayeeID(), payee);
  m_payeeList.insert(newPayee.id(), newPayee);
  payee = newPayee;
}

const MyMoneyPayee MyMoneySeqAccessMgr::payee(const QString& id) const
{
  QMap<QString, MyMoneyPayee>::ConstIterator it;
  it = m_payeeList.find(id);
  if (it == m_payeeList.end())
    throw new MYMONEYEXCEPTION("Unknown payee '" + id + '\'');

  return *it;
}

const MyMoneyPayee MyMoneySeqAccessMgr::payeeByName(const QString& payee) const
{
  if (payee.isEmpty())
    return MyMoneyPayee::null;

  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  for (it_p = m_payeeList.begin(); it_p != m_payeeList.end(); ++it_p) {
    if ((*it_p).name() == payee) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown payee '" + payee + '\'');
}

void MyMoneySeqAccessMgr::modifyPayee(const MyMoneyPayee& payee)
{
  QMap<QString, MyMoneyPayee>::ConstIterator it;

  it = m_payeeList.find(payee.id());
  if (it == m_payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }
  m_payeeList.modify((*it).id(), payee);
}

void MyMoneySeqAccessMgr::removePayee(const MyMoneyPayee& payee)
{
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneySchedule>::ConstIterator it_s;
  QMap<QString, MyMoneyPayee>::ConstIterator it_p;

  it_p = m_payeeList.find(payee.id());
  if (it_p == m_payeeList.end()) {
    QString msg = "Unknown payee '" + payee.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  // scan all transactions to check if the payee is still referenced
  for (it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    if ((*it_t).hasReferenceTo(payee.id())) {
      throw new MYMONEYEXCEPTION(QString("Cannot remove payee that is still referenced to a %1").arg("transaction"));
    }
  }

  // check referential integrity in schedules
  for (it_s = m_scheduleList.begin(); it_s != m_scheduleList.end(); ++it_s) {
    if ((*it_s).hasReferenceTo(payee.id())) {
      throw new MYMONEYEXCEPTION(QString("Cannot remove payee that is still referenced to a %1").arg("schedule"));
    }
  }

  // remove any reference to report and/or budget
  removeReferences(payee.id());

  m_payeeList.remove((*it_p).id());
}

const QList<MyMoneyPayee> MyMoneySeqAccessMgr::payeeList(void) const
{
  return m_payeeList.values();
}


void MyMoneySeqAccessMgr::addAccount(MyMoneyAccount& parent, MyMoneyAccount& account)
{
  QMap<QString, MyMoneyAccount>::ConstIterator theParent;
  QMap<QString, MyMoneyAccount>::ConstIterator theChild;

  theParent = m_accountList.find(parent.id());
  if (theParent == m_accountList.end()) {
    QString msg = "Unknown parent account '";
    msg += parent.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  theChild = m_accountList.find(account.id());
  if (theChild == m_accountList.end()) {
    QString msg = "Unknown child account '";
    msg += account.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  MyMoneyAccount acc = *theParent;
  acc.addAccountId(account.id());
  m_accountList.modify(acc.id(), acc);
  parent = acc;

  acc = *theChild;
  acc.setParentAccountId(parent.id());
  m_accountList.modify(acc.id(), acc);
  account = acc;

  MyMoneyBalanceCacheItem balance;
  m_balanceCache[account.id()] = balance;
}

void MyMoneySeqAccessMgr::addInstitution(MyMoneyInstitution& institution)
{
  MyMoneyInstitution newInstitution(nextInstitutionID(), institution);

  m_institutionList.insert(newInstitution.id(), newInstitution);

  // return new data
  institution = newInstitution;
}

unsigned int MyMoneySeqAccessMgr::transactionCount(const QString& account) const
{
  unsigned int cnt = 0;

  if (account.length() == 0) {
    cnt = m_transactionList.count();

  } else {
    QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
    QList<MyMoneySplit>::ConstIterator it_s;

    // scan all transactions
    for (it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {

      // scan all splits of this transaction
      for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
        // is it a split in our account?
        if ((*it_s).accountId() == account) {
          // since a transaction can only have one split referencing
          // each account, we're done with the splits here!
          break;
        }
      }
      // if no split contains the account id, continue with the
      // next transaction
      if (it_s == (*it_t).splits().end())
        continue;

      // otherwise count it
      ++cnt;
    }
  }
  return cnt;
}

const QMap<QString, unsigned long> MyMoneySeqAccessMgr::transactionCountMap(void) const
{
  QMap<QString, unsigned long> map;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QList<MyMoneySplit>::ConstIterator it_s;

  // scan all transactions
  for (it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      map[(*it_s).accountId()]++;
    }
  }
  return map;
}

unsigned int MyMoneySeqAccessMgr::institutionCount(void) const
{
  return m_institutionList.count();
}

unsigned int MyMoneySeqAccessMgr::accountCount(void) const
{
  return m_accountList.count();
}

QString MyMoneySeqAccessMgr::nextPayeeID(void)
{
  QString id;
  id.setNum(++m_nextPayeeID);
  id = 'P' + id.rightJustified(PAYEE_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextInstitutionID(void)
{
  QString id;
  id.setNum(++m_nextInstitutionID);
  id = 'I' + id.rightJustified(INSTITUTION_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextAccountID(void)
{
  QString id;
  id.setNum(++m_nextAccountID);
  id = 'A' + id.rightJustified(ACCOUNT_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextTransactionID(void)
{
  QString id;
  id.setNum(++m_nextTransactionID);
  id = 'T' + id.rightJustified(TRANSACTION_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextScheduleID(void)
{
  QString id;
  id.setNum(++m_nextScheduleID);
  id = "SCH" + id.rightJustified(SCHEDULE_ID_SIZE, '0');
  return id;
}

QString MyMoneySeqAccessMgr::nextSecurityID(void)
{
  QString id;
  id.setNum(++m_nextSecurityID);
  id = 'E' + id.rightJustified(SECURITY_ID_SIZE, '0');
  return id;
}


void MyMoneySeqAccessMgr::addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate)
{
  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if (!transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("transaction already contains an id");
  if (!transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("invalid post date");

  // now check the splits
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account((*it_s).accountId());
    if (!(*it_s).payeeId().isEmpty())
      payee((*it_s).payeeId());
  }

  MyMoneyTransaction newTransaction(nextTransactionID(), transaction);
  QString key = newTransaction.uniqueSortKey();

  m_transactionList.insert(key, newTransaction);
  m_transactionKeys.insert(newTransaction.id(), key);

  transaction = newTransaction;

  // adjust the balance of all affected accounts
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    MyMoneyAccount acc = m_accountList[(*it_s).accountId()];
    acc.adjustBalance(*it_s);
    if (!skipAccountUpdate) {
      acc.touch();
      invalidateBalanceCache(acc.id());
    }
    m_accountList.modify(acc.id(), acc);
  }
}

void MyMoneySeqAccessMgr::touch(void)
{
  m_dirty = true;
  m_lastModificationDate = QDate::currentDate();
}

bool MyMoneySeqAccessMgr::hasActiveSplits(const QString& id) const
{
  QMap<QString, MyMoneyTransaction>::ConstIterator it;

  for (it = m_transactionList.begin(); it != m_transactionList.end(); ++it) {
    if ((*it).accountReferenced(id)) {
      return true;
    }
  }
  return false;
}

const MyMoneyInstitution MyMoneySeqAccessMgr::institution(const QString& id) const
{
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;

  pos = m_institutionList.find(id);
  if (pos != m_institutionList.end())
    return *pos;
  throw new MYMONEYEXCEPTION("unknown institution");
}

const QList<MyMoneyInstitution> MyMoneySeqAccessMgr::institutionList(void) const
{
  return m_institutionList.values();
}

void MyMoneySeqAccessMgr::modifyAccount(const MyMoneyAccount& account, const bool skipCheck)
{
  QMap<QString, MyMoneyAccount>::ConstIterator pos;

  // locate the account in the file global pool
  pos = m_accountList.find(account.id());
  if (pos != m_accountList.end()) {
    // check if the new info is based on the old one.
    // this is the case, when the file and the id
    // as well as the type are equal.
    if ((((*pos).parentAccountId() == account.parentAccountId())
         && ((*pos).accountType() == account.accountType()))
        || (skipCheck == true)) {
      // make sure that all the referenced objects exist
      if (!account.institutionId().isEmpty())
        institution(account.institutionId());

      QList<QString>::ConstIterator it_a;
      for (it_a = account.accountList().constBegin(); it_a != account.accountList().constEnd(); ++it_a) {
        this->account(*it_a);
      }
      // update information in account list
      m_accountList.modify(account.id(), account);

      // invalidate cached balance
      invalidateBalanceCache(account.id());

    } else
      throw new MYMONEYEXCEPTION("Invalid information for update");

  } else
    throw new MYMONEYEXCEPTION("Unknown account id");
}

void MyMoneySeqAccessMgr::modifyInstitution(const MyMoneyInstitution& institution)
{
  QMap<QString, MyMoneyInstitution>::ConstIterator pos;

  // locate the institution in the file global pool
  pos = m_institutionList.find(institution.id());
  if (pos != m_institutionList.end()) {
    m_institutionList.modify(institution.id(), institution);

  } else
    throw new MYMONEYEXCEPTION("unknown institution");
}

void MyMoneySeqAccessMgr::modifyTransaction(const MyMoneyTransaction& transaction)
{
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
    throw new MYMONEYEXCEPTION("invalid transaction to be modified");

  // now check the splits
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    // the following lines will throw an exception if the
    // account or payee do not exist
    account((*it_s).accountId());
    if (!(*it_s).payeeId().isEmpty())
      payee((*it_s).payeeId());
  }

  // new data seems to be ok. find old version of transaction
  // in our pool. Throw exception if unknown.
  if (!m_transactionKeys.contains(transaction.id()))
    throw new MYMONEYEXCEPTION("invalid transaction id");

  QString oldKey = m_transactionKeys[transaction.id()];
  if (!m_transactionList.contains(oldKey))
    throw new MYMONEYEXCEPTION("invalid transaction key");

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

  it_t = m_transactionList.find(oldKey);
  if (it_t == m_transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  // adjust account balances
  for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
    MyMoneyAccount acc = m_accountList[(*it_s).accountId()];
    acc.adjustBalance(*it_s, true);   // reverse the adjust operation (reverse = true)
    acc.touch();
    invalidateBalanceCache(acc.id());
    m_accountList.modify(acc.id(), acc);
  }
  for (it_s = transaction.splits().begin(); it_s != transaction.splits().end(); ++it_s) {
    MyMoneyAccount acc = m_accountList[(*it_s).accountId()];
    acc.adjustBalance(*it_s);
    acc.touch();
    invalidateBalanceCache(acc.id());
    m_accountList.modify(acc.id(), acc);
  }

  // remove old transaction from lists
  m_transactionList.remove(oldKey);

  // add new transaction to lists
  QString newKey = transaction.uniqueSortKey();
  m_transactionList.insert(newKey, transaction);
  m_transactionKeys.modify(transaction.id(), newKey);
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  reparentAccount(account, parent, true);
}

void MyMoneySeqAccessMgr::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent, const bool /* sendNotification */)
{
  QMap<QString, MyMoneyAccount>::ConstIterator oldParent;
  QMap<QString, MyMoneyAccount>::ConstIterator newParent;
  QMap<QString, MyMoneyAccount>::ConstIterator childAccount;

  // verify that accounts exist. If one does not,
  // an exception is thrown
  MyMoneySeqAccessMgr::account(account.id());
  MyMoneySeqAccessMgr::account(parent.id());
  if (!account.parentAccountId().isEmpty()) {
    MyMoneySeqAccessMgr::account(account.parentAccountId());
    oldParent = m_accountList.find(account.parentAccountId());
  }

  if (account.accountType() == MyMoneyAccount::Stock && parent.accountType() != MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Cannot move a stock acocunt into a non-investment account");

  newParent = m_accountList.find(parent.id());
  childAccount = m_accountList.find(account.id());

  MyMoneyAccount acc;
  if (!account.parentAccountId().isEmpty()) {
    acc = (*oldParent);
    acc.removeAccountId(account.id());
    m_accountList.modify(acc.id(), acc);
  }

  parent = (*newParent);
  parent.addAccountId(account.id());
  m_accountList.modify(parent.id(), parent);

  account = (*childAccount);
  account.setParentAccountId(parent.id());
  m_accountList.modify(account.id(), account);

#if 0
  // make sure the type is the same as the new parent. This does not work for stock and investment
  if (account.accountType() != MyMoneyAccount::Stock && account.accountType() != MyMoneyAccount::Investment)
    (*childAccount).setAccountType((*newParent).accountType());
#endif
}

void MyMoneySeqAccessMgr::removeTransaction(const MyMoneyTransaction& transaction)
{
  // first perform all the checks
  if (transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  QMap<QString, QString>::ConstIterator it_k;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;

  it_k = m_transactionKeys.find(transaction.id());
  if (it_k == m_transactionKeys.end())
    throw new MYMONEYEXCEPTION("invalid transaction to be deleted");

  it_t = m_transactionList.find(*it_k);
  if (it_t == m_transactionList.end())
    throw new MYMONEYEXCEPTION("invalid transaction key");

  QList<MyMoneySplit>::ConstIterator it_s;

  // scan the splits and collect all accounts that need
  // to be updated after the removal of this transaction
  for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
    MyMoneyAccount acc = m_accountList[(*it_s).accountId()];
    acc.adjustBalance(*it_s, true);  // reverse = true
    acc.touch();
    m_accountList.modify(acc.id(), acc);
    invalidateBalanceCache(acc.id());
  }

  // FIXME: check if any split is frozen and throw exception

  // remove the transaction from the two lists
  m_transactionList.remove(*it_k);
  m_transactionKeys.remove(transaction.id());
}

void MyMoneySeqAccessMgr::removeAccount(const MyMoneyAccount& account)
{
  MyMoneyAccount parent;

  // check that the account and it's parent exist
  // this will throw an exception if the id is unknown
  MyMoneySeqAccessMgr::account(account.id());
  parent = MyMoneySeqAccessMgr::account(account.parentAccountId());

  // check that it's not one of the standard account groups
  if (isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to remove the standard account groups");

  if (hasActiveSplits(account.id())) {
    throw new MYMONEYEXCEPTION("Unable to remove account with active splits");
  }

  // re-parent all sub-ordinate accounts to the parent of the account
  // to be deleted. First round check that all accounts exist, second
  // round do the re-parenting.
  QStringList::ConstIterator it;
  for (it = account.accountList().begin(); it != account.accountList().end(); ++it) {
    MyMoneySeqAccessMgr::account(*it);
  }

  // if one of the accounts did not exist, an exception had been
  // thrown and we would not make it until here.

  QMap<QString, MyMoneyAccount>::ConstIterator it_a;
  QMap<QString, MyMoneyAccount>::ConstIterator it_p;

  // locate the account in the file global pool

  it_a = m_accountList.find(account.id());
  if (it_a == m_accountList.end())
    throw new MYMONEYEXCEPTION("Internal error: account not found in list");

  it_p = m_accountList.find(parent.id());
  if (it_p == m_accountList.end())
    throw new MYMONEYEXCEPTION("Internal error: parent account not found in list");

  if (!account.institutionId().isEmpty())
    throw new MYMONEYEXCEPTION("Cannot remove account still attached to an institution");

  removeReferences(account.id());

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
      while ((*it_a).accountList().count() > 0) {
        it = (*it_a).accountList().begin();
        MyMoneyAccount acc(MyMoneySeqAccessMgr::account(*it));
        reparentAccount(acc, parent, false);
      }
    }
    // remove account from parent's list
    parent.removeAccountId(account.id());
    m_accountList.modify(parent.id(), parent);

    // remove account from the global account pool
    m_accountList.remove(account.id());

    // remove from balance list
    m_balanceCache.remove(account.id());
    invalidateBalanceCache(parent.id());
  }
}

void MyMoneySeqAccessMgr::removeInstitution(const MyMoneyInstitution& institution)
{
  QMap<QString, MyMoneyInstitution>::ConstIterator it_i;

  it_i = m_institutionList.find(institution.id());
  if (it_i != m_institutionList.end()) {
    m_institutionList.remove(institution.id());

  } else
    throw new MYMONEYEXCEPTION("invalid institution");
}

void MyMoneySeqAccessMgr::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t_end = m_transactionList.end();

  for (it_t = m_transactionList.begin(); it_t != it_t_end; ++it_t) {
    // This code is used now. It adds the transaction to the list for
    // each matching split exactly once. This allows to show information
    // about different splits in the same register view (e.g. search result)
    //
    // I have no idea, if this has some impact on the functionality. So far,
    // I could not see it.  (ipwizard 9/5/2003)
    if (filter.match(*it_t)) {
      unsigned int cnt = filter.matchingSplits().count();
      if (cnt > 1) {
        for (unsigned i = 0; i < cnt; ++i)
          list.append(*it_t);
      } else {
        list.append(*it_t);
      }
    }
  }
}

void MyMoneySeqAccessMgr::transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  list.clear();

  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t_end = m_transactionList.end();

  for (it_t = m_transactionList.begin(); it_t != it_t_end; ++it_t) {
    if (filter.match(*it_t)) {
      const QList<MyMoneySplit>& splits = filter.matchingSplits();
      QList<MyMoneySplit>::ConstIterator it_s;
      QList<MyMoneySplit>::ConstIterator it_s_end = splits.end();
      for (it_s = splits.constBegin(); it_s != it_s_end; ++it_s) {
        list.append(qMakePair(*it_t, *it_s));
      }
    }
  }
}

const QList<MyMoneyTransaction> MyMoneySeqAccessMgr::transactionList(MyMoneyTransactionFilter& filter) const
{
  QList<MyMoneyTransaction> list;
  transactionList(list, filter);
  return list;
}

const MyMoneyTransaction MyMoneySeqAccessMgr::transaction(const QString& id) const
{
  // get the full key of this transaction, throw exception
  // if it's invalid (unknown)
  if (!m_transactionKeys.contains(id)) {
    QString msg = QString("Invalid transaction id '%1'").arg(id);
    throw new MYMONEYEXCEPTION(msg);
  }

  // check if this key is in the list, throw exception if not
  QString key = m_transactionKeys[id];
  if (!m_transactionList.contains(key)) {
    QString msg = QString("Invalid transaction key '%1'").arg(key);
    throw new MYMONEYEXCEPTION(msg);
  }

  return m_transactionList[key];
}

const MyMoneyTransaction MyMoneySeqAccessMgr::transaction(const QString& account, const int idx) const
{
  /* removed with MyMoneyAccount::Transaction
    QMap<QString, MyMoneyAccount>::ConstIterator acc;

    // find account object in list, throw exception if unknown
    acc = m_accountList.find(account);
    if(acc == m_accountList.end())
      throw new MYMONEYEXCEPTION("unknown account id");

    // get the transaction info from the account
    MyMoneyAccount::Transaction t = (*acc).transaction(idx);

    // return the transaction, throw exception if not found
    return transaction(t.transactionID());
  */

  // new implementation if the above code does not work anymore
  QList<MyMoneyTransaction> list;
  MyMoneyAccount acc = m_accountList[account];
  MyMoneyTransactionFilter filter;

  if (acc.accountGroup() == MyMoneyAccount::Income
      || acc.accountGroup() == MyMoneyAccount::Expense)
    filter.addCategory(account);
  else
    filter.addAccount(account);

  transactionList(list, filter);
  if (idx < 0 || idx >= static_cast<int>(list.count()))
    throw new MYMONEYEXCEPTION("Unknown idx for transaction");

  return transaction(list[idx].id());
}

const MyMoneyMoney MyMoneySeqAccessMgr::balance(const QString& id, const QDate& date) const
{
  MyMoneyMoney result(0);
  MyMoneyAccount acc;
  // if (date != QDate()) qDebug ("request balance for %s at %s", id.data(), date.toString(Qt::ISODate).toLatin1());
  if (!date.isValid() && account(id).accountType() != MyMoneyAccount::Stock) {
    if (m_accountList.find(id) != m_accountList.end())
      return m_accountList[id].balance();
    return MyMoneyMoney(0);
  }
  if (m_balanceCache[id].valid == false || date != m_balanceCacheDate) {
    QMap<QString, MyMoneyMoney> balances;
    QMap<QString, MyMoneyMoney>::ConstIterator it_b;
    if (date != m_balanceCacheDate) {
      m_balanceCache.clear();
      m_balanceCacheDate = date;
    }

    QList<MyMoneyTransaction> list;
    QList<MyMoneyTransaction>::ConstIterator it_t;
    QList<MyMoneySplit>::ConstIterator it_s;

    MyMoneyTransactionFilter filter;
    filter.setDateFilter(QDate(), date);
    filter.setReportAllSplits(false);
    transactionList(list, filter);

    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      for (it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
        const QString& aid = (*it_s).accountId();
        if ((*it_s).action() == MyMoneySplit::ActionSplitShares) {
          balances[aid] = balances[aid] * (*it_s).shares();
        } else {
          balances[aid] += (*it_s).shares();
        }
      }
    }

    // fill the found balances into the cache
    for (it_b = balances.constBegin(); it_b != balances.constEnd(); ++it_b) {
      MyMoneyBalanceCacheItem balance(*it_b);
      m_balanceCache[it_b.key()] = balance;
    }

    // fill all accounts w/o transactions to zero
    QMap<QString, MyMoneyAccount>::ConstIterator it_a;
    for (it_a = m_accountList.begin(); it_a != m_accountList.end(); ++it_a) {
      if (m_balanceCache[(*it_a).id()].valid == false) {
        MyMoneyBalanceCacheItem balance(MyMoneyMoney(0, 1));
        m_balanceCache[(*it_a).id()] = balance;
      }
    }
  }

  if (m_balanceCache[id].valid == true)
    result = m_balanceCache[id].balance;
  else
    qDebug("Cache mishit should never happen at this point");

  return result;
}

const MyMoneyMoney MyMoneySeqAccessMgr::totalBalance(const QString& id, const QDate& date) const
{
  QStringList accounts;
  QStringList::ConstIterator it_a;

  MyMoneyMoney result(balance(id, date));

  accounts = account(id).accountList();

  for (it_a = accounts.constBegin(); it_a != accounts.constEnd(); ++it_a) {
    result += totalBalance(*it_a, date);
  }

  return result;
}

/**
  * this was intended to move all splits from one account
  * to another. This somehow is strange to undo because many
  * changes to different objects are made within one single call.
  * I kept the source here but commented it out. If we ever need
  * the functionality, we can turn it back on. BTW: the stuff is untested ;-)
  */
/*
const unsigned int MyMoneyFile::moveSplits(const QString& oldAccount, const QString& newAccount)
{
  QMap<QString, MyMoneyTransaction>::Iterator it_t;
  QValueList<MyMoneySplit>::ConstIterator it_s;
  unsigned int cnt = 0;

  // scan all transactions
  for(it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    // scan all splits of this transaction
    for(it_s = (*it_t).splits().begin(); it_s != (*it_t).splits().end(); ++it_s) {
      // is it a split in our account?
      if((*it_s).account() == oldAccount) {
        MyMoneySplit s = *it_s;
        s.setAccount(newAccount);
        (*it_t).modifySplit(s);
        ++cnt;
      }
    }
  }

  if(cnt != 0) {
    // now update all the accounts that were referenced
    QMap<QString, MyMoneyAccount>::Iterator acc;
    acc = m_accountList.find(oldAccount);
    if(acc != m_accountList.end()) {
      (*acc).touch();
      refreshAccountTransactionList(acc);
    }
    acc = m_accountList.find(newAccount);
    if(acc != m_accountList.end()) {
      (*acc).touch();
      refreshAccountTransactionList(acc);
    }

    // mark file as changed
    m_dirty = true;
  }
  return cnt;
}
*/

void MyMoneySeqAccessMgr::invalidateBalanceCache(const QString& id)
{
  if (!id.isEmpty()) {
    try {
      m_balanceCache[id].valid = false;
      if (!isStandardAccount(id)) {
        invalidateBalanceCache(account(id).parentAccountId());
      }
    } catch (MyMoneyException *e) {
      delete e;
    }
  }
}

void MyMoneySeqAccessMgr::loadAccounts(const QMap<QString, MyMoneyAccount>& map)
{
  m_accountList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyAccount>::const_iterator it_a;
  QString lastId("");
  for (it_a = map.begin(); it_a != map.end(); ++it_a) {
    if (!isStandardAccount((*it_a).id()) && ((*it_a).id() > lastId))
      lastId = (*it_a).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextAccountID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadTransactions(const QMap<QString, MyMoneyTransaction>& map)
{
  m_transactionList = map;

  // now fill the key map and
  // identify the last used id
  QString lastId("");
  QMap<QString, QString> keys;
  QMap<QString, MyMoneyTransaction>::ConstIterator it_t;
  for (it_t = map.begin(); it_t != map.end(); ++it_t) {
    keys[(*it_t).id()] = it_t.key();
    if ((*it_t).id() > lastId)
      lastId = (*it_t).id();
  }
  m_transactionKeys = keys;


  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextTransactionID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadInstitutions(const QMap<QString, MyMoneyInstitution>& map)
{
  m_institutionList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyInstitution>::const_iterator it_i;
  QString lastId("");
  for (it_i = map.begin(); it_i != map.end(); ++it_i) {
    if ((*it_i).id() > lastId)
      lastId = (*it_i).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextInstitutionID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadPayees(const QMap<QString, MyMoneyPayee>& map)
{
  m_payeeList = map;

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
    m_nextPayeeID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadSecurities(const QMap<QString, MyMoneySecurity>& map)
{
  m_securitiesList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneySecurity>::const_iterator it_s;
  QString lastId("");
  for (it_s = map.begin(); it_s != map.end(); ++it_s) {
    if ((*it_s).id() > lastId)
      lastId = (*it_s).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextSecurityID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadCurrencies(const QMap<QString, MyMoneySecurity>& map)
{
  m_currencyList = map;
}

void MyMoneySeqAccessMgr::loadPrices(const MyMoneyPriceList& list)
{
  m_priceList = list;
}

void MyMoneySeqAccessMgr::loadAccountId(const unsigned long id)
{
  m_nextAccountID = id;
}

void MyMoneySeqAccessMgr::loadTransactionId(const unsigned long id)
{
  m_nextTransactionID = id;
}

void MyMoneySeqAccessMgr::loadPayeeId(const unsigned long id)
{
  m_nextPayeeID = id;
}

void MyMoneySeqAccessMgr::loadInstitutionId(const unsigned long id)
{
  m_nextInstitutionID = id;
}

void MyMoneySeqAccessMgr::loadSecurityId(const unsigned long id)
{
  m_nextSecurityID = id;
}

void MyMoneySeqAccessMgr::loadReportId(const unsigned long id)
{
  m_nextReportID = id;
}

void MyMoneySeqAccessMgr::loadBudgetId(const unsigned long id)
{
  m_nextBudgetID = id;
}

const QString MyMoneySeqAccessMgr::value(const QString& key) const
{
  return MyMoneyKeyValueContainer::value(key);
}

void MyMoneySeqAccessMgr::setValue(const QString& key, const QString& val)
{
  MyMoneyKeyValueContainer::setValue(key, val);
  touch();
}

void MyMoneySeqAccessMgr::deletePair(const QString& key)
{
  MyMoneyKeyValueContainer::deletePair(key);
  touch();
}

const QMap<QString, QString> MyMoneySeqAccessMgr::pairs(void) const
{
  return MyMoneyKeyValueContainer::pairs();
}

void MyMoneySeqAccessMgr::setPairs(const QMap<QString, QString>& list)
{
  MyMoneyKeyValueContainer::setPairs(list);
  touch();
}

void MyMoneySeqAccessMgr::addSchedule(MyMoneySchedule& sched)
{
  // first perform all the checks
  if (!sched.id().isEmpty())
    throw new MYMONEYEXCEPTION("schedule already contains an id");

  // The following will throw an exception when it fails
  sched.validate(false);

  MyMoneySchedule newSched(nextScheduleID(), sched);
  m_scheduleList.insert(newSched.id(), newSched);
  sched = newSched;
}

void MyMoneySeqAccessMgr::modifySchedule(const MyMoneySchedule& sched)
{
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = m_scheduleList.find(sched.id());
  if (it == m_scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  m_scheduleList.modify(sched.id(), sched);
}

void MyMoneySeqAccessMgr::removeSchedule(const MyMoneySchedule& sched)
{
  QMap<QString, MyMoneySchedule>::ConstIterator it;

  it = m_scheduleList.find(sched.id());
  if (it == m_scheduleList.end()) {
    QString msg = "Unknown schedule '" + sched.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  // FIXME: check referential integrity for loan accounts
  m_scheduleList.remove(sched.id());
}

const MyMoneySchedule MyMoneySeqAccessMgr::schedule(const QString& id) const
{
  QMap<QString, MyMoneySchedule>::ConstIterator pos;

  // locate the schedule and if present, return it's data
  pos = m_scheduleList.find(id);
  if (pos != m_scheduleList.end())
    return (*pos);

  // throw an exception, if it does not exist
  QString msg = "Unknown schedule id '" + id + '\'';
  throw new MYMONEYEXCEPTION(msg);
}

const QList<MyMoneySchedule> MyMoneySeqAccessMgr::scheduleList(
  const QString& accountId,
  const MyMoneySchedule::typeE type,
  const MyMoneySchedule::occurrenceE occurrence,
  const MyMoneySchedule::paymentTypeE paymentType,
  const QDate& startDate,
  const QDate& endDate,
  const bool overdue) const
{
  QMap<QString, MyMoneySchedule>::ConstIterator pos;
  QList<MyMoneySchedule> list;

  // qDebug("scheduleList()");

  for (pos = m_scheduleList.begin(); pos != m_scheduleList.end(); ++pos) {
    // qDebug("  '%s'", (*pos).id().data());

    if (type != MyMoneySchedule::TYPE_ANY) {
      if (type != (*pos).type()) {
        continue;
      }
    }

    if (occurrence != MyMoneySchedule::OCCUR_ANY) {
      if (occurrence != (*pos).occurrence()) {
        continue;
      }
    }

    if (paymentType != MyMoneySchedule::STYPE_ANY) {
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
  m_scheduleList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneySchedule>::const_iterator it_s;
  QString lastId("");
  for (it_s = map.begin(); it_s != map.end(); ++it_s) {
    if ((*it_s).id() > lastId)
      lastId = (*it_s).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextScheduleID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::loadScheduleId(const unsigned long id)
{
  m_nextScheduleID = id;
}

const QList<MyMoneySchedule> MyMoneySeqAccessMgr::scheduleListEx(int scheduleTypes,
    int scheduleOcurrences,
    int schedulePaymentTypes,
    QDate date,
    const QStringList& accounts) const
{
//  qDebug("scheduleListEx");

  QMap<QString, MyMoneySchedule>::ConstIterator pos;
  QList<MyMoneySchedule> list;

  if (!date.isValid())
    return list;

  for (pos = m_scheduleList.begin(); pos != m_scheduleList.end(); ++pos) {
    if (scheduleTypes && !(scheduleTypes & (*pos).type()))
      continue;

    if (scheduleOcurrences && !(scheduleOcurrences & (*pos).occurrence()))
      continue;

    if (schedulePaymentTypes && !(schedulePaymentTypes & (*pos).paymentType()))
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
  // create the account
  MyMoneySecurity newSecurity(nextSecurityID(), security);

  m_securitiesList.insert(newSecurity.id(), newSecurity);

  security = newSecurity;
}

void MyMoneySeqAccessMgr::modifySecurity(const MyMoneySecurity& security)
{
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = m_securitiesList.find(security.id());
  if (it == m_securitiesList.end()) {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during modifySecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_securitiesList.modify(security.id(), security);
}

void MyMoneySeqAccessMgr::removeSecurity(const MyMoneySecurity& security)
{
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = m_securitiesList.find(security.id());
  if (it == m_securitiesList.end()) {
    QString msg = "Unknown security  '";
    msg += security.id() + "' during removeSecurity()";
    throw new MYMONEYEXCEPTION(msg);
  }

  m_securitiesList.remove(security.id());
}

const MyMoneySecurity MyMoneySeqAccessMgr::security(const QString& id) const
{
  QMap<QString, MyMoneySecurity>::ConstIterator it = m_securitiesList.find(id);
  if (it != m_securitiesList.end()) {
    return it.value();
  }

  return MyMoneySecurity();
}

const QList<MyMoneySecurity> MyMoneySeqAccessMgr::securityList(void) const
{
  //qDebug("securityList: Security list size is %d, this=%8p", m_equitiesList.size(), (void*)this);
  return m_securitiesList.values();
}

void MyMoneySeqAccessMgr::addCurrency(const MyMoneySecurity& currency)
{
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = m_currencyList.find(currency.id());
  if (it != m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot add currency with existing id %1").arg(currency.id()));
  }

  m_currencyList.insert(currency.id(), currency);
}

void MyMoneySeqAccessMgr::modifyCurrency(const MyMoneySecurity& currency)
{
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = m_currencyList.find(currency.id());
  if (it == m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot modify currency with unknown id %1").arg(currency.id()));
  }

  m_currencyList.modify(currency.id(), currency);
}

void MyMoneySeqAccessMgr::removeCurrency(const MyMoneySecurity& currency)
{
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  // FIXME: check referential integrity

  it = m_currencyList.find(currency.id());
  if (it == m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot remove currency with unknown id %1").arg(currency.id()));
  }

  m_currencyList.remove(currency.id());
}

const MyMoneySecurity MyMoneySeqAccessMgr::currency(const QString& id) const
{
  if (id.isEmpty()) {

  }
  QMap<QString, MyMoneySecurity>::ConstIterator it;

  it = m_currencyList.find(id);
  if (it == m_currencyList.end()) {
    throw new MYMONEYEXCEPTION(QString("Cannot retrieve currency with unknown id '%1'").arg(id));
  }

  return *it;
}

const QList<MyMoneySecurity> MyMoneySeqAccessMgr::currencyList(void) const
{
  return m_currencyList.values();
}

const QList<MyMoneyReport> MyMoneySeqAccessMgr::reportList(void) const
{
  return m_reportList.values();
}

void MyMoneySeqAccessMgr::addReport(MyMoneyReport& report)
{
  if (!report.id().isEmpty())
    throw new MYMONEYEXCEPTION("report already contains an id");

  MyMoneyReport newReport(nextReportID(), report);
  m_reportList.insert(newReport.id(), newReport);
  report = newReport;
}

void MyMoneySeqAccessMgr::loadReports(const QMap<QString, MyMoneyReport>& map)
{
  m_reportList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyReport>::const_iterator it_r;
  QString lastId("");
  for (it_r = map.begin(); it_r != map.end(); ++it_r) {
    if ((*it_r).id() > lastId)
      lastId = (*it_r).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextReportID = lastId.mid(pos).toInt();
  }
}

void MyMoneySeqAccessMgr::modifyReport(const MyMoneyReport& report)
{
  QMap<QString, MyMoneyReport>::ConstIterator it;

  it = m_reportList.find(report.id());
  if (it == m_reportList.end()) {
    QString msg = "Unknown report '" + report.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }
  m_reportList.modify(report.id(), report);
}

QString MyMoneySeqAccessMgr::nextReportID(void)
{
  QString id;
  id.setNum(++m_nextReportID);
  id = 'R' + id.rightJustified(REPORT_ID_SIZE, '0');
  return id;
}

unsigned MyMoneySeqAccessMgr::countReports(void) const
{
  return m_reportList.count();
}

const MyMoneyReport MyMoneySeqAccessMgr::report(const QString& _id) const
{
  return m_reportList[_id];
}

void MyMoneySeqAccessMgr::removeReport(const MyMoneyReport& report)
{
  QMap<QString, MyMoneyReport>::ConstIterator it;

  it = m_reportList.find(report.id());
  if (it == m_reportList.end()) {
    QString msg = "Unknown report '" + report.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  m_reportList.remove(report.id());
}

const QList<MyMoneyBudget> MyMoneySeqAccessMgr::budgetList(void) const
{
  return m_budgetList.values();
}


void MyMoneySeqAccessMgr::addBudget(MyMoneyBudget& budget)
{
  MyMoneyBudget newBudget(nextBudgetID(), budget);
  m_budgetList.insert(newBudget.id(), newBudget);
  budget = newBudget;
}

void MyMoneySeqAccessMgr::loadBudgets(const QMap<QString, MyMoneyBudget>& map)
{
  m_budgetList = map;

  // scan the map to identify the last used id
  QMap<QString, MyMoneyBudget>::const_iterator it_b;
  QString lastId("");
  for (it_b = map.begin(); it_b != map.end(); ++it_b) {
    if ((*it_b).id() > lastId)
      lastId = (*it_b).id();
  }

  int pos = lastId.indexOf(QRegExp("\\d+"), 0);
  if (pos != -1) {
    m_nextBudgetID = lastId.mid(pos).toInt();
  }
}

const MyMoneyBudget MyMoneySeqAccessMgr::budgetByName(const QString& budget) const
{
  QMap<QString, MyMoneyBudget>::ConstIterator it_p;

  for (it_p = m_budgetList.begin(); it_p != m_budgetList.end(); ++it_p) {
    if ((*it_p).name() == budget) {
      return *it_p;
    }
  }

  throw new MYMONEYEXCEPTION("Unknown budget '" + budget + '\'');
}

void MyMoneySeqAccessMgr::modifyBudget(const MyMoneyBudget& budget)
{
  QMap<QString, MyMoneyBudget>::ConstIterator it;

  it = m_budgetList.find(budget.id());
  if (it == m_budgetList.end()) {
    QString msg = "Unknown budget '" + budget.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }
  m_budgetList.modify(budget.id(), budget);
}

QString MyMoneySeqAccessMgr::nextBudgetID(void)
{
  QString id;
  id.setNum(++m_nextBudgetID);
  id = 'B' + id.rightJustified(BUDGET_ID_SIZE, '0');
  return id;
}

unsigned MyMoneySeqAccessMgr::countBudgets(void) const
{
  return m_budgetList.count();
}

MyMoneyBudget MyMoneySeqAccessMgr::budget(const QString& _id) const
{
  return m_budgetList[_id];
}

void MyMoneySeqAccessMgr::removeBudget(const MyMoneyBudget& budget)
{
  QMap<QString, MyMoneyBudget>::ConstIterator it;

  it = m_budgetList.find(budget.id());
  if (it == m_budgetList.end()) {
    QString msg = "Unknown budget '" + budget.id() + '\'';
    throw new MYMONEYEXCEPTION(msg);
  }

  m_budgetList.remove(budget.id());
}

void MyMoneySeqAccessMgr::addPrice(const MyMoneyPrice& price)
{
  MyMoneySecurityPair pricePair(price.from(), price.to());
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::ConstIterator it_m;
  it_m = m_priceList.find(pricePair);

  MyMoneyPriceEntries entries;
  if (it_m != m_priceList.end()) {
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

  if (it_m != m_priceList.end()) {
    m_priceList.modify(pricePair, entries);
  } else {
    m_priceList.insert(pricePair, entries);
  }
}

void MyMoneySeqAccessMgr::removePrice(const MyMoneyPrice& price)
{
  MyMoneySecurityPair pricePair(price.from(), price.to());
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::ConstIterator it_m;
  it_m = m_priceList.find(pricePair);

  MyMoneyPriceEntries entries;
  if (it_m != m_priceList.end()) {
    entries = (*it_m);
  }

  // store new value in local copy
  entries.remove(price.date());

  if (entries.count() != 0) {
    m_priceList.modify(pricePair, entries);
  } else {
    m_priceList.remove(pricePair);
  }
}

const MyMoneyPriceList MyMoneySeqAccessMgr::priceList(void) const
{
  MyMoneyPriceList list;
  m_priceList.map(list);
  return list;
}

const MyMoneyPrice MyMoneySeqAccessMgr::price(const QString& fromId, const QString& toId, const QDate& _date, const bool exactDate) const
{
  MyMoneyPrice rc;
  MyMoneyPriceEntries::ConstIterator it;
  QDate date(_date);
  MyMoneySecurityPair pricePair(fromId, toId);

  // If no valid date is passed, we use today's date.
  if (!date.isValid())
    date = QDate::currentDate();

  // If the caller selected an exact entry, we can search for
  // it using the date as the key
  QMap<MyMoneySecurityPair, MyMoneyPriceEntries>::const_iterator itm;
  itm = m_priceList.find(pricePair);
  if (itm != m_priceList.end()) {
    if (exactDate) {
      it = itm.value().find(date);
      if (it != itm.value().end())
        rc = *it;

    } else {
      // otherwise, we must scan the map for the previous price entry
      for (it = itm.value().begin(); it != itm.value().end(); ++it) {
        if (date < it.key())
          break;

        if (date >= it.key())
          rc = *it;
      }
    }
  }
  return rc;
}

void MyMoneySeqAccessMgr::clearCache(void)
{
  m_balanceCache.clear();
}

void MyMoneySeqAccessMgr::rebuildAccountBalances(void)
{
  // reset the balance of all accounts to 0
  QMap<QString, MyMoneyAccount> map;
  m_accountList.map(map);

  QMap<QString, MyMoneyAccount>::iterator it_a;
  for (it_a = map.begin(); it_a != map.end(); ++it_a) {
    (*it_a).setBalance(MyMoneyMoney(0));
  }

  // now scan over all transactions and all splits and setup the balances
  QMap<QString, MyMoneyTransaction>::const_iterator it_t;
  for (it_t = m_transactionList.begin(); it_t != m_transactionList.end(); ++it_t) {
    const QList<MyMoneySplit>& splits = (*it_t).splits();
    QList<MyMoneySplit>::const_iterator it_s = splits.begin();
    for (; it_s != splits.end(); ++it_s) {
      if (!(*it_s).shares().isZero()) {
        const QString& id = (*it_s).accountId();
        // locate the account and if present, update data
        if (map.find(id) != map.end()) {
          map[id].adjustBalance(*it_s);
        }
      }
    }
  }

  m_accountList = map;
}

bool MyMoneySeqAccessMgr::isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipCheck) const
{
  // We delete all references in reports when an object
  // is deleted, so we don't need to check here. See
  // MyMoneySeqAccessMgr::removeReferences(). In case
  // you miss the report checks in the following lines ;)

  bool rc = false;
  const QString& id = obj.id();
  QMap<QString, MyMoneyTransaction>::const_iterator it_t;
  QMap<QString, MyMoneyAccount>::const_iterator it_a;
  QMap<QString, MyMoneyInstitution>::const_iterator it_i;
  QMap<QString, MyMoneyPayee>::const_iterator it_p;
  QMap<QString, MyMoneyBudget>::const_iterator it_b;
  QMap<QString, MyMoneySchedule>::const_iterator it_sch;
  QMap<QString, MyMoneySecurity>::const_iterator it_sec;
  MyMoneyPriceList::const_iterator it_pr;

  // FIXME optimize the list of objects we have to checks
  //       with a bit of knowledge of the internal structure, we
  //       could optimize the number of objects we check for references

  // Scan all engine objects for a reference
  if (!skipCheck[RefCheckTransaction]) {
    for (it_t = m_transactionList.begin(); !rc && it_t != m_transactionList.end(); ++it_t) {
      rc = (*it_t).hasReferenceTo(id);
    }
  }

  if (!skipCheck[RefCheckAccount]) {
    for (it_a = m_accountList.begin(); !rc && it_a != m_accountList.end(); ++it_a) {
      rc = (*it_a).hasReferenceTo(id);
    }
  }
  if (!skipCheck[RefCheckInstitution]) {
    for (it_i = m_institutionList.begin(); !rc && it_i != m_institutionList.end(); ++it_i) {
      rc = (*it_i).hasReferenceTo(id);
    }
  }
  if (!skipCheck[RefCheckPayee]) {
    for (it_p = m_payeeList.begin(); !rc && it_p != m_payeeList.end(); ++it_p) {
      rc = (*it_p).hasReferenceTo(id);
    }
  }

  if (!skipCheck[RefCheckBudget]) {
    for (it_b = m_budgetList.begin(); !rc && it_b != m_budgetList.end(); ++it_b) {
      rc = (*it_b).hasReferenceTo(id);
    }
  }
  if (!skipCheck[RefCheckSchedule]) {
    for (it_sch = m_scheduleList.begin(); !rc && it_sch != m_scheduleList.end(); ++it_sch) {
      rc = (*it_sch).hasReferenceTo(id);
    }
  }
  if (!skipCheck[RefCheckSecurity]) {
    for (it_sec = m_securitiesList.begin(); !rc && it_sec != m_securitiesList.end(); ++it_sec) {
      rc = (*it_sec).hasReferenceTo(id);
    }
  }
  if (!skipCheck[RefCheckCurrency]) {
    for (it_sec = m_currencyList.begin(); !rc && it_sec != m_currencyList.end(); ++it_sec) {
      rc = (*it_sec).hasReferenceTo(id);
    }
  }
  // within the pricelist we don't have to scan each entry. Checking the QPair
  // members of the MyMoneySecurityPair is enough as they are identical to the
  // two security ids
  if (!skipCheck[RefCheckPrice]) {
    for (it_pr = m_priceList.begin(); !rc && it_pr != m_priceList.end(); ++it_pr) {
      rc = (it_pr.key().first == id) || (it_pr.key().second == id);
    }
  }

  return rc;
}

void MyMoneySeqAccessMgr::startTransaction(void)
{
  m_payeeList.startTransaction(&m_nextPayeeID);
  m_institutionList.startTransaction(&m_nextInstitutionID);
  m_accountList.startTransaction(&m_nextPayeeID);
  m_transactionList.startTransaction(&m_nextTransactionID);
  m_transactionKeys.startTransaction();
  m_scheduleList.startTransaction(&m_nextScheduleID);
  m_securitiesList.startTransaction(&m_nextSecurityID);
  m_currencyList.startTransaction();
  m_reportList.startTransaction(&m_nextReportID);
  m_budgetList.startTransaction(&m_nextBudgetID);
  m_priceList.startTransaction();
}

bool MyMoneySeqAccessMgr::commitTransaction(void)
{
  bool rc = false;
  rc |= m_payeeList.commitTransaction();
  rc |= m_institutionList.commitTransaction();
  rc |= m_accountList.commitTransaction();
  rc |= m_transactionList.commitTransaction();
  rc |= m_transactionKeys.commitTransaction();
  rc |= m_scheduleList.commitTransaction();
  rc |= m_securitiesList.commitTransaction();
  rc |= m_currencyList.commitTransaction();
  rc |= m_reportList.commitTransaction();
  rc |= m_budgetList.commitTransaction();
  rc |= m_priceList.commitTransaction();

  // if there was a change, touch the whole storage object
  if (rc)
    touch();

  return rc;
}

void MyMoneySeqAccessMgr::rollbackTransaction(void)
{
  m_payeeList.rollbackTransaction();
  m_institutionList.rollbackTransaction();
  m_accountList.rollbackTransaction();
  m_transactionList.rollbackTransaction();
  m_transactionKeys.rollbackTransaction();
  m_scheduleList.rollbackTransaction();
  m_securitiesList.rollbackTransaction();
  m_currencyList.rollbackTransaction();
  m_reportList.rollbackTransaction();
  m_budgetList.rollbackTransaction();
  m_priceList.rollbackTransaction();
}

void MyMoneySeqAccessMgr::removeReferences(const QString& id)
{
  QMap<QString, MyMoneyReport>::const_iterator it_r;
  QMap<QString, MyMoneyBudget>::const_iterator it_b;

  // remove from reports
  for (it_r = m_reportList.begin(); it_r != m_reportList.end(); ++it_r) {
    MyMoneyReport r = *it_r;
    r.removeReference(id);
    m_reportList.modify(r.id(), r);
  }

  // remove from budgets
  for (it_b = m_budgetList.begin(); it_b != m_budgetList.end(); ++it_b) {
    MyMoneyBudget b = *it_b;
    b.removeReference(id);
    m_budgetList.modify(b.id(), b);
  }
}

#undef TRY
#undef CATCH
#undef PASS

// vim:cin:si:ai:et:ts=2:sw=2:
