/***************************************************************************
                          mymoneyfile.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2002, 2007-2008 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneyfile.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDateTime>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "storage/mymoneyseqaccessmgr.h"
#include "mymoneyreport.h"
#include "mymoneybudget.h"
#include "mymoneyprice.h"
#include "mymoneyobjectcontainer.h"

#ifndef HAVE_CONFIG_H
#define VERSION "UNKNOWN"
#else
#include <config-kmymoney.h>
#endif

const QString MyMoneyFile::OpeningBalancesPrefix = I18N_NOOP("Opening Balances");
const QString MyMoneyFile::AccountSeperator = QChar(':');

// include the following line to get a 'cout' for debug purposes
// #include <iostream>
MyMoneyFile* MyMoneyFile::_instance = 0;

class MyMoneyFile::Private
{
public:
  Private() :
      m_inTransaction(false) {}

  bool                   m_inTransaction;
  MyMoneySecurity        m_baseCurrency;
  MyMoneyObjectContainer m_cache;
  MyMoneyPriceList       m_priceCache;

  /**
    * This member keeps a list of ids to notify after an
    * operation is completed. The boolean is used as follows
    * during processing of the list:
    *
    * false - don't reload the object immediately
    * true  - reload the object immediately
    */
  QMap<QString, bool>   m_notificationList;

};

MyMoneyFile MyMoneyFile::file;

MyMoneyFile::MyMoneyFile() :
    d(new Private)
{
  m_storage = 0;
}

MyMoneyFile::~MyMoneyFile()
{
  _instance = 0;
  delete m_storage;
  delete d;
}

MyMoneyFile::MyMoneyFile(IMyMoneyStorage *storage) :
    d(new Private)
{
  m_storage = 0;
  attachStorage(storage);
}

void MyMoneyFile::attachStorage(IMyMoneyStorage* const storage)
{
  if (m_storage != 0)
    throw new MYMONEYEXCEPTION("Storage already attached");

  if (storage == 0)
    throw new MYMONEYEXCEPTION("Storage must not be 0");

  m_storage = storage;

  // force reload of base currency
  d->m_baseCurrency = MyMoneySecurity();

  // and the whole cache
  d->m_cache.clear(storage);
  d->m_priceCache.clear();
  preloadCache();

  // notify application about new data availability
  emit dataChanged();
}

void MyMoneyFile::detachStorage(IMyMoneyStorage* const /* storage */)
{
  d->m_cache.clear();
  d->m_priceCache.clear();
  m_storage = 0;
}

void MyMoneyFile::startTransaction(void)
{
  checkStorage();
  if (d->m_inTransaction) {
    throw new MYMONEYEXCEPTION("Already started a transaction!");
  }

  m_storage->startTransaction();
  d->m_inTransaction = true;
}

bool MyMoneyFile::hasTransaction(void) const
{
  return d->m_inTransaction;
}

void MyMoneyFile::checkTransaction(const char* txt) const
{
  checkStorage();
  if (!d->m_inTransaction) {
    throw new MYMONEYEXCEPTION(QString("No transaction started for %1").arg(txt));
  }
}

void MyMoneyFile::commitTransaction(void)
{
  checkTransaction(Q_FUNC_INFO);

  bool changed = m_storage->commitTransaction();
  d->m_inTransaction = false;
  preloadCache();
  if (changed) {
    emit dataChanged();
  }
}

void MyMoneyFile::rollbackTransaction(void)
{
  checkTransaction(Q_FUNC_INFO);

  m_storage->rollbackTransaction();
  d->m_inTransaction = false;
  preloadCache();
}

void MyMoneyFile::addInstitution(MyMoneyInstitution& institution)
{
  // perform some checks to see that the institution stuff is OK. For
  // now we assume that the institution must have a name, the ID is not set
  // and it does not have a parent (MyMoneyFile).

  if (institution.name().length() == 0
      || institution.id().length() != 0)
    throw new MYMONEYEXCEPTION("Not a new institution");

  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addInstitution(institution);

  d->m_cache.preloadInstitution(institution);
}

void MyMoneyFile::modifyInstitution(const MyMoneyInstitution& institution)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->modifyInstitution(institution);

  addNotification(institution.id());
}

void MyMoneyFile::modifyTransaction(const MyMoneyTransaction& transaction)
{
  checkTransaction(Q_FUNC_INFO);

  const MyMoneyTransaction* t = &transaction;
  MyMoneyTransaction tCopy;

  // now check the splits
  bool loanAccountAffected = false;
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist
    MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());
    if (acc.id().isEmpty())
      throw new MYMONEYEXCEPTION("Cannot store split with no account assigned");
    if (isStandardAccount((*it_s).accountId()))
      throw new MYMONEYEXCEPTION("Cannot store split referencing standard account");
    if (acc.isLoan() && ((*it_s).action() == MyMoneySplit::ActionTransfer))
      loanAccountAffected = true;
  }

  // change transfer splits between asset/liability and loan accounts
  // into amortization splits
  if (loanAccountAffected) {
    tCopy = transaction;
    QList<MyMoneySplit> list = transaction.splits();
    for (it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
      if ((*it_s).action() == MyMoneySplit::ActionTransfer) {
        MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());

        if (acc.isAssetLiability()) {
          MyMoneySplit s = (*it_s);
          s.setAction(MyMoneySplit::ActionAmortization);
          tCopy.modifySplit(s);
          t = &tCopy;
        }
      }
    }
  }

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // get the current setting of this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());

  // scan the splits again to update notification list
  // and mark all accounts that are referenced
  for (it_s = tr.splits().constBegin(); it_s != tr.splits().constEnd(); ++it_s) {
    addNotification((*it_s).accountId());
    addNotification((*it_s).payeeId());
  }

  // perform modification
  m_storage->modifyTransaction(*t);

  // and mark all accounts that are referenced
  for (it_s = t->splits().constBegin(); it_s != t->splits().constEnd(); ++it_s) {
    addNotification((*it_s).accountId());
    addNotification((*it_s).payeeId());
  }
}

void MyMoneyFile::modifyAccount(const MyMoneyAccount& _account)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount account(_account);

  MyMoneyAccount acc = MyMoneyFile::account(account.id());

  // check that for standard accounts only specific parameters are changed
  if (isStandardAccount(account.id())) {
    // make sure to use the stuff we found on file
    account = acc;

    // and only use the changes that are allowed
    account.setName(_account.name());
    account.setCurrencyId(_account.currencyId());

    // now check that it is the same
    if (!(account == _account))
      throw new MYMONEYEXCEPTION("Unable to modify the standard account groups");
  }

  if (account.accountType() != acc.accountType() &&
      !account.isLiquidAsset() && !acc.isLiquidAsset())
    throw new MYMONEYEXCEPTION("Unable to change account type");

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // if the account was moved to another insitution, we notify
  // the old one as well as the new one and the structure change
  if (acc.institutionId() != account.institutionId()) {
    MyMoneyInstitution inst;
    if (!acc.institutionId().isEmpty()) {
      inst = institution(acc.institutionId());
      inst.removeAccountId(acc.id());
      modifyInstitution(inst);
    }
    if (!account.institutionId().isEmpty()) {
      inst = institution(account.institutionId());
      inst.addAccountId(acc.id());
      modifyInstitution(inst);
    }
    addNotification(acc.institutionId());
    addNotification(account.institutionId());
  }

  m_storage->modifyAccount(account);

  addNotification(account.id());
}

void MyMoneyFile::reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent)
{
  checkTransaction(Q_FUNC_INFO);

  // check that it's not one of the standard account groups
  if (isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to reparent the standard account groups");

  if (account.accountGroup() == parent.accountGroup()
      || (account.accountType() == MyMoneyAccount::Income && parent.accountType() == MyMoneyAccount::Expense)
      || (account.accountType() == MyMoneyAccount::Expense && parent.accountType() == MyMoneyAccount::Income)) {

    if (account.isInvest() && parent.accountType() != MyMoneyAccount::Investment)
      throw new MYMONEYEXCEPTION("Unable to reparent Stock to non-investment account");

    if (parent.accountType() == MyMoneyAccount::Investment && !account.isInvest())
      throw new MYMONEYEXCEPTION("Unable to reparent non-stock to investment account");

    // clear all changed objects from cache
    MyMoneyNotifier notifier(this);

    // keep a notification of the current parent
    addNotification(account.parentAccountId());

    m_storage->reparentAccount(account, parent);

    // and also keep one for the account itself and the new parent
    addNotification(account.id());
    addNotification(parent.id());

  } else
    throw new MYMONEYEXCEPTION("Unable to reparent to different account type");
}

const MyMoneyInstitution& MyMoneyFile::institution(const QString& id) const
{
  return d->m_cache.institution(id);
}

const MyMoneyAccount& MyMoneyFile::account(const QString& id) const
{
  return d->m_cache.account(id);
}

const MyMoneyAccount& MyMoneyFile::subAccountByName(const MyMoneyAccount& acc, const QString& name) const
{
  static MyMoneyAccount nullAccount;

  QList<QString>::const_iterator it_a;
  for (it_a = acc.accountList().constBegin(); it_a != acc.accountList().constEnd(); ++it_a) {
    const MyMoneyAccount& sacc = account(*it_a);
    if (sacc.name() == name)
      return sacc;
  }
  return nullAccount;
}

const MyMoneyAccount& MyMoneyFile::accountByName(const QString& name) const
{
  return d->m_cache.accountByName(name);
}

void MyMoneyFile::removeTransaction(const MyMoneyTransaction& transaction)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // get the engine's idea about this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());
  QList<MyMoneySplit>::ConstIterator it_s;

  // scan the splits again to update notification list
  for (it_s = tr.splits().constBegin(); it_s != tr.splits().constEnd(); ++it_s) {
    MyMoneyAccount acc = account((*it_s).accountId());
    if (acc.isClosed())
      throw new MYMONEYEXCEPTION(i18n("Cannot remove transaction that references a closed account."));
    addNotification((*it_s).accountId());
    addNotification((*it_s).payeeId());
  }

  m_storage->removeTransaction(transaction);
}


bool MyMoneyFile::hasActiveSplits(const QString& id) const
{
  checkStorage();

  return m_storage->hasActiveSplits(id);
}

bool MyMoneyFile::isStandardAccount(const QString& id) const
{
  checkStorage();

  return m_storage->isStandardAccount(id);
}

void MyMoneyFile::setAccountName(const QString& id, const QString& name) const
{
  checkTransaction(Q_FUNC_INFO);

  m_storage->setAccountName(id, name);
}

void MyMoneyFile::removeAccount(const MyMoneyAccount& account)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount parent;
  MyMoneyAccount acc;
  MyMoneyInstitution institution;

  // check that the account and its parent exist
  // this will throw an exception if the id is unknown
  acc = MyMoneyFile::account(account.id());
  parent = MyMoneyFile::account(account.parentAccountId());
  if (!acc.institutionId().isEmpty())
    institution = MyMoneyFile::institution(acc.institutionId());

  // check that it's not one of the standard account groups
  if (isStandardAccount(account.id()))
    throw new MYMONEYEXCEPTION("Unable to remove the standard account groups");

  if (hasActiveSplits(account.id())) {
    throw new MYMONEYEXCEPTION("Unable to remove account with active splits");
  }

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // collect all sub-ordinate accounts for notification
  QStringList::ConstIterator it;
  for (it = acc.accountList().constBegin(); it != acc.accountList().constEnd(); ++it)
    addNotification(*it);
  // don't forget the parent and a possible institution
  addNotification(parent.id());
  addNotification(account.institutionId());

  if (!institution.id().isEmpty()) {
    institution.removeAccountId(account.id());
    m_storage->modifyInstitution(institution);
  }
  acc.setInstitutionId(QString());

  m_storage->removeAccount(acc);
  addNotification(acc.id(), false);
  d->m_cache.clear(acc.id());
}

void MyMoneyFile::removeAccountList(const QStringList& account_list, unsigned int level)
{
  if (level > 100)
    throw new MYMONEYEXCEPTION("Too deep recursion in [MyMoneyFile::removeAccountList]!");

  checkTransaction(Q_FUNC_INFO);

  // upon entry, we check that we could proceed with the operation
  if (!level) {
    if (!hasOnlyUnusedAccounts(account_list, 0))
      throw new MYMONEYEXCEPTION("One or more accounts cannot be removed");

    // NOTE: We don't use a MyMoneyNotifier here, but rather clear the whole cache
    d->m_cache.clear();
  }

  // process all accounts in the list and test if they have transactions assigned
  for (QStringList::ConstIterator it = account_list.constBegin(); it != account_list.constEnd(); ++it) {
    MyMoneyAccount a = m_storage->account(*it);
    //kDebug() << "Deleting account '"<< a.name() << "'";

    // first remove all sub-accounts
    if (!a.accountList().isEmpty())
      removeAccountList(a.accountList(), level + 1);

    // then remove account itself, but we first have to get
    // rid of the account list that is still stored in
    // the MyMoneyAccount object. Easiest way is to get a fresh copy.
    a = m_storage->account(*it);
    //kDebug() << "Deleting account '"<< a2.name() << "' itself";
    m_storage->removeAccount(a);
  }
}

bool MyMoneyFile::hasOnlyUnusedAccounts(const QStringList& account_list, unsigned int level)
{
  if (level > 100)
    throw new MYMONEYEXCEPTION("Too deep recursion in [MyMoneyFile::hasOnlyUnusedAccounts]!");
  // process all accounts in the list and test if they have transactions assigned
  for (QStringList::ConstIterator it = account_list.constBegin(); it != account_list.constEnd(); ++it) {
    if (transactionCount(*it) != 0)
      return false; // the current account has a transaction assigned
    if (!hasOnlyUnusedAccounts(account(*it).accountList(), level + 1))
      return false; // some sub-account has a transaction assigned
  }
  return true; // all subaccounts unused
}


void MyMoneyFile::removeInstitution(const MyMoneyInstitution& institution)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  QList<QString>::ConstIterator it_a;
  MyMoneyInstitution inst = MyMoneyFile::institution(institution.id());

  bool blocked = signalsBlocked();
  blockSignals(true);
  for (it_a = inst.accountList().constBegin(); it_a != inst.accountList().constEnd(); ++it_a) {
    MyMoneyAccount acc = account(*it_a);
    acc.setInstitutionId(QString());
    modifyAccount(acc);
  }
  blockSignals(blocked);

  m_storage->removeInstitution(institution);

  addNotification(institution.id(), false);
}

void MyMoneyFile::addAccount(MyMoneyAccount& account, MyMoneyAccount& parent)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyInstitution institution;

  // perform some checks to see that the account stuff is OK. For
  // now we assume that the account must have a name, has no
  // transaction and sub-accounts and parent account
  // it's own ID is not set and it does not have a pointer to (MyMoneyFile)

  if (account.name().length() == 0)
    throw new MYMONEYEXCEPTION("Account has no name");

  if (account.id().length() != 0)
    throw new MYMONEYEXCEPTION("New account must have no id");

  if (account.accountList().count() != 0)
    throw new MYMONEYEXCEPTION("New account must have no sub-accounts");

  if (!account.parentAccountId().isEmpty())
    throw new MYMONEYEXCEPTION("New account must have no parent-id");

  if (account.accountType() == MyMoneyAccount::UnknownAccountType)
    throw new MYMONEYEXCEPTION("Account has invalid type");

  // make sure, that the parent account exists
  // if not, an exception is thrown. If it exists,
  // get a copy of the current data
  MyMoneyAccount acc = MyMoneyFile::account(parent.id());

#if 0
  // TODO: remove the following code as we now can have multiple accounts
  // with the same name even in the same hierarchy position of the account tree
  //
  // check if the selected name is currently not among the child accounts
  // if we find one, then return it as the new account
  QStringList::const_iterator it_a;
  for (it_a = acc.accountList().begin(); it_a != acc.accountList().end(); ++it_a) {
    MyMoneyAccount a = MyMoneyFile::account(*it_a);
    if (account.name() == a.name()) {
      account = a;
      return;
    }
  }
#endif

  // FIXME: make sure, that the parent has the same type
  // I left it out here because I don't know, if there is
  // a tight coupling between e.g. checking accounts and the
  // class asset. It certainly does not make sense to create an
  // expense account under an income account. Maybe it does, I don't know.

  // We enforce, that a stock account can never be a parent and
  // that the parent for a stock account must be an investment. Also,
  // an investment cannot have another investment account as it's parent
  if (parent.isInvest())
    throw new MYMONEYEXCEPTION("Stock account cannot be parent account");

  if (account.isInvest() && parent.accountType() != MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Stock account must have investment account as parent ");

  if (!account.isInvest() && parent.accountType() == MyMoneyAccount::Investment)
    throw new MYMONEYEXCEPTION("Investment account can only have stock accounts as children");

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // if an institution is set, verify that it exists
  if (account.institutionId().length() != 0) {
    // check the presence of the institution. if it
    // does not exist, an exception is thrown
    institution = MyMoneyFile::institution(account.institutionId());
  }


  if (!account.openingDate().isValid()) {
    account.setOpeningDate(QDate::currentDate());
  }

  account.setParentAccountId(parent.id());

  m_storage->addAccount(account);
  m_storage->addAccount(parent, account);

  if (account.institutionId().length() != 0) {
    institution.addAccountId(account.id());
    m_storage->modifyInstitution(institution);
    addNotification(institution.id());
  }

  d->m_cache.preloadAccount(account);
  addNotification(parent.id());
}

MyMoneyTransaction MyMoneyFile::createOpeningBalanceTransaction(const MyMoneyAccount& acc, const MyMoneyMoney& balance)
{
  MyMoneyTransaction t;
  // if the opening balance is not zero, we need
  // to create the respective transaction
  if (!balance.isZero()) {
    checkTransaction(Q_FUNC_INFO);

    MyMoneySecurity currency = security(acc.currencyId());
    MyMoneyAccount openAcc = openingBalanceAccount(currency);

    if (openAcc.openingDate() > acc.openingDate()) {
      openAcc.setOpeningDate(acc.openingDate());
      modifyAccount(openAcc);
    }

    MyMoneySplit s;

    t.setPostDate(acc.openingDate());
    t.setCommodity(acc.currencyId());

    s.setAccountId(acc.id());
    s.setShares(balance);
    s.setValue(balance);
    t.addSplit(s);

    s.clearId();
    s.setAccountId(openAcc.id());
    s.setShares(-balance);
    s.setValue(-balance);
    t.addSplit(s);

    addTransaction(t);
  }
  return t;
}

QString MyMoneyFile::openingBalanceTransaction(const MyMoneyAccount& acc) const
{
  QString result;

  MyMoneySecurity currency = security(acc.currencyId());
  MyMoneyAccount openAcc;

  try {
    openAcc = openingBalanceAccount(currency);
  } catch (MyMoneyException *e) {
    delete e;
    return result;
  }

  // Iterate over all the opening balance transactions for this currency
  MyMoneyTransactionFilter filter;
  filter.addAccount(openAcc.id());
  QList<MyMoneyTransaction> transactions = transactionList(filter);
  QList<MyMoneyTransaction>::const_iterator it_t = transactions.constBegin();
  while (it_t != transactions.constEnd()) {
    try {
      // Test whether the transaction also includes a split into
      // this account
      (*it_t).splitByAccount(acc.id(), true /*match*/);

      // If so, we have a winner!
      result = (*it_t).id();
      break;
    } catch (MyMoneyException *e) {
      // If not, keep searching
      ++it_t;
      delete e;
    }
  }

  return result;
}

const MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security)
{
  if (!security.isCurrency())
    throw new MYMONEYEXCEPTION("Opening balance for non currencies not supported");

  try {
    return openingBalanceAccount_internal(security);
  } catch (MyMoneyException *e) {
    delete e;
    MyMoneyFileTransaction ft;
    MyMoneyAccount acc;

    try {
      acc = createOpeningBalanceAccount(security);
      ft.commit();

    } catch (MyMoneyException* e) {
      qDebug("Unable to create opening balance account for security %s", qPrintable(security.id()));
      delete e;
    }
    return acc;
  }
}

const MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security) const
{
  return openingBalanceAccount_internal(security);
}

const MyMoneyAccount MyMoneyFile::openingBalanceAccount_internal(const MyMoneySecurity& security) const
{
  if (!security.isCurrency())
    throw new MYMONEYEXCEPTION("Opening balance for non currencies not supported");

  MyMoneyAccount acc;
  QRegExp match(QString("^%1").arg((MyMoneyFile::OpeningBalancesPrefix)));

  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount>::ConstIterator it;

  accountList(accounts, equity().accountList(), true);

  for (it = accounts.constBegin(); it != accounts.constEnd(); ++it) {
    if (match.indexIn((*it).name()) != -1) {
      if ((*it).currencyId() == security.id()) {
        acc = *it;
        break;
      }
    }
  }

  if (acc.id().isEmpty()) {
    throw new MYMONEYEXCEPTION(QString("No opening balance account for %1").arg(security.tradingSymbol()));
  }

  return acc;
}

const MyMoneyAccount MyMoneyFile::createOpeningBalanceAccount(const MyMoneySecurity& security)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount acc;
  QString name(MyMoneyFile::OpeningBalancesPrefix);
  if (security.id() != baseCurrency().id()) {
    name += QString(" (%1)").arg(security.id());
  }
  acc.setName(name);
  acc.setAccountType(MyMoneyAccount::Equity);
  acc.setCurrencyId(security.id());

  MyMoneyAccount parent = equity();
  this->addAccount(acc, parent);
  return acc;
}

void MyMoneyFile::addTransaction(MyMoneyTransaction& transaction)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if (!transaction.id().isEmpty())
    throw new MYMONEYEXCEPTION("Unable to add transaction with id set");
  if (!transaction.postDate().isValid())
    throw new MYMONEYEXCEPTION("Unable to add transaction with invalid postdate");

  // now check the splits
  bool loanAccountAffected = false;
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist or is one of the standard accounts
    MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());
    if (acc.id().isEmpty())
      throw new MYMONEYEXCEPTION("Cannot add split with no account assigned");
    if (acc.isLoan())
      loanAccountAffected = true;
    if (isStandardAccount((*it_s).accountId()))
      throw new MYMONEYEXCEPTION("Cannot add split referencing standard account");
  }

  // change transfer splits between asset/liability and loan accounts
  // into amortization splits
  if (loanAccountAffected) {
    QList<MyMoneySplit> list = transaction.splits();
    for (it_s = list.constBegin(); it_s != list.constEnd(); ++it_s) {
      if ((*it_s).action() == MyMoneySplit::ActionTransfer) {
        MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());

        if (acc.isAssetLiability()) {
          MyMoneySplit s = (*it_s);
          s.setAction(MyMoneySplit::ActionAmortization);
          transaction.modifySplit(s);
        }
      }
    }
  }

  // check that we have a commodity
  if (transaction.commodity().isEmpty()) {
    transaction.setCommodity(baseCurrency().id());
  }

  // then add the transaction to the file global pool
  m_storage->addTransaction(transaction);

  // scan the splits again to update notification list
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    addNotification((*it_s).accountId());
    addNotification((*it_s).payeeId());
  }
}

const MyMoneyTransaction MyMoneyFile::transaction(const QString& id) const
{
  checkStorage();

  return m_storage->transaction(id);
}

const MyMoneyTransaction MyMoneyFile::transaction(const QString& account, const int idx) const
{
  checkStorage();

  return m_storage->transaction(account, idx);
}

void MyMoneyFile::addPayee(MyMoneyPayee& payee)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addPayee(payee);

  d->m_cache.preloadPayee(payee);
}

const MyMoneyPayee& MyMoneyFile::payee(const QString& id) const
{
  return d->m_cache.payee(id);
}

const MyMoneyPayee& MyMoneyFile::payeeByName(const QString& name) const
{
  checkStorage();

  return d->m_cache.payee(m_storage->payeeByName(name).id());
}

void MyMoneyFile::modifyPayee(const MyMoneyPayee& payee)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  addNotification(payee.id());

  m_storage->modifyPayee(payee);
}

void MyMoneyFile::removePayee(const MyMoneyPayee& payee)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->removePayee(payee);

  addNotification(payee.id(), false);
}

void MyMoneyFile::accountList(QList<MyMoneyAccount>& list, const QStringList& idlist, const bool recursive) const
{
  if (idlist.isEmpty()) {
    d->m_cache.account(list);

#if 0
    // TODO: I have no idea what this was good for, but it caused the networth report
    //       to show double the numbers so I commented it out (ipwizard, 2008-05-24)
    if (m_storage && (list.isEmpty() || list.size() != m_storage->accountCount())) {
      m_storage->accountList(list);
      d->m_cache.preloadAccount(list);
    }
#endif

    QList<MyMoneyAccount>::Iterator it;
    for (it = list.begin(); it != list.end();) {
      if (isStandardAccount((*it).id())) {
        it = list.erase(it);
      } else {
        ++it;
      }
    }
  } else {
    QList<MyMoneyAccount>::ConstIterator it;
    QList<MyMoneyAccount> list_a;
    d->m_cache.account(list_a);

    for (it = list_a.constBegin(); it != list_a.constEnd(); ++it) {
      if (!isStandardAccount((*it).id())) {
        if (idlist.indexOf((*it).id()) != -1) {
          list.append(*it);
          if (recursive == true && !(*it).accountList().isEmpty()) {
            accountList(list, (*it).accountList(), true);
          }
        }
      }
    }
  }
}

void MyMoneyFile::institutionList(QList<MyMoneyInstitution>& list) const
{
  d->m_cache.institution(list);
}

const QList<MyMoneyInstitution> MyMoneyFile::institutionList(void) const
{
  QList<MyMoneyInstitution> list;
  institutionList(list);
  return list;
}

// general get functions
const MyMoneyPayee MyMoneyFile::user(void) const
{
  checkStorage(); return m_storage->user();
}

// general set functions
void MyMoneyFile::setUser(const MyMoneyPayee& user)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->setUser(user);
}

bool MyMoneyFile::dirty(void) const
{
  if (!m_storage)
    return false;

  return m_storage->dirty();
}

void MyMoneyFile::setDirty(void) const
{
  checkStorage();

  m_storage->setDirty();
}

unsigned int MyMoneyFile::accountCount(void) const
{
  checkStorage();

  return m_storage->accountCount();
}

void MyMoneyFile::ensureDefaultCurrency(MyMoneyAccount& acc) const
{
  if (acc.currencyId().isEmpty()) {
    if (!baseCurrency().id().isEmpty())
      acc.setCurrencyId(baseCurrency().id());
  }
}

const MyMoneyAccount& MyMoneyFile::liability(void) const
{
  checkStorage();

  return d->m_cache.account(STD_ACC_LIABILITY);
}

const MyMoneyAccount& MyMoneyFile::asset(void) const
{
  checkStorage();

  return d->m_cache.account(STD_ACC_ASSET);
}

const MyMoneyAccount& MyMoneyFile::expense(void) const
{
  checkStorage();

  return d->m_cache.account(STD_ACC_EXPENSE);
}

const MyMoneyAccount& MyMoneyFile::income(void) const
{
  checkStorage();

  return d->m_cache.account(STD_ACC_INCOME);
}

const MyMoneyAccount& MyMoneyFile::equity(void) const
{
  checkStorage();

  return d->m_cache.account(STD_ACC_EQUITY);
}

unsigned int MyMoneyFile::transactionCount(const QString& account) const
{
  checkStorage();

  return m_storage->transactionCount(account);
}

const QMap<QString, unsigned long> MyMoneyFile::transactionCountMap(void) const
{
  checkStorage();

  return m_storage->transactionCountMap();
}

unsigned int MyMoneyFile::institutionCount(void) const
{
  checkStorage();

  return m_storage->institutionCount();
}

const MyMoneyMoney MyMoneyFile::balance(const QString& id, const QDate& date) const
{
  checkStorage();

  return m_storage->balance(id, date);
}

const MyMoneyMoney MyMoneyFile::totalBalance(const QString& id, const QDate& date) const
{
  checkStorage();

  return m_storage->totalBalance(id, date);
}

void MyMoneyFile::warningMissingRate(const QString& fromId, const QString& toId) const
{
  MyMoneySecurity from, to;
  try {
    from = security(fromId);
    to = security(toId);
    qWarning("Missing price info for conversion from %s to %s", qPrintable(from.name()), qPrintable(to.name()));

  } catch (MyMoneyException *e) {
    qFatal("Missing security caught in MyMoneyFile::warningMissingRate(): %s(%ld) %s", qPrintable(e->file()), e->line(), qPrintable(e->what()));
    delete e;
  }
}

void MyMoneyFile::notify(void)
{
  QMap<QString, bool>::ConstIterator it;
  for (it = d->m_notificationList.constBegin(); it != d->m_notificationList.constEnd(); ++it) {
    if (*it)
      d->m_cache.refresh(it.key());
    else
      d->m_cache.clear(it.key());
  }
  clearNotification();
}

void MyMoneyFile::addNotification(const QString& id, bool reload)
{
  if (!id.isEmpty())
    d->m_notificationList[id] = reload;
}

void MyMoneyFile::clearNotification()
{
  // reset list to be empty
  d->m_notificationList.clear();
}

void MyMoneyFile::transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  checkStorage();
  m_storage->transactionList(list, filter);
}

void MyMoneyFile::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  checkStorage();
  m_storage->transactionList(list, filter);
}

const QList<MyMoneyTransaction> MyMoneyFile::transactionList(MyMoneyTransactionFilter& filter) const
{
  QList<MyMoneyTransaction> list;
  transactionList(list, filter);
  return list;
}

const QList<MyMoneyPayee> MyMoneyFile::payeeList(void) const
{
  QList<MyMoneyPayee> list;
  d->m_cache.payee(list);
  return list;
}

QString MyMoneyFile::accountToCategory(const QString& accountId, bool includeStandardAccounts) const
{
  MyMoneyAccount acc;
  QString rc;

  if (!accountId.isEmpty()) {
    acc = account(accountId);
    do {
      if (!rc.isEmpty())
        rc = AccountSeperator + rc;
      rc = acc.name() + rc;
      acc = account(acc.parentAccountId());
    } while (!acc.id().isEmpty() && (includeStandardAccounts || !isStandardAccount(acc.id())));
  }
  return rc;
}

QString MyMoneyFile::categoryToAccount(const QString& category, MyMoneyAccount::accountTypeE type) const
{
  QString id;

  // search the category in the expense accounts and if it is not found, try
  // to locate it in the income accounts
  if (type == MyMoneyAccount::UnknownAccountType
      || type == MyMoneyAccount::Expense) {
    id = locateSubAccount(MyMoneyFile::instance()->expense(), category);
  }

  if ((id.isEmpty() && type == MyMoneyAccount::UnknownAccountType)
      || type == MyMoneyAccount::Income) {
    id = locateSubAccount(MyMoneyFile::instance()->income(), category);
  }

  return id;
}

QString MyMoneyFile::nameToAccount(const QString& name) const
{
  QString id;

  // search the category in the asset accounts and if it is not found, try
  // to locate it in the liability accounts
  id = locateSubAccount(MyMoneyFile::instance()->asset(), name);
  if (id.isEmpty())
    id = locateSubAccount(MyMoneyFile::instance()->liability(), name);

  return id;
}

QString MyMoneyFile::parentName(const QString& name) const
{
  return name.section(AccountSeperator, 0, -2);
}

QString MyMoneyFile::locateSubAccount(const MyMoneyAccount& base, const QString& category) const
{
  MyMoneyAccount nextBase;
  QString level, remainder;
  level = category.section(AccountSeperator, 0, 0);
  remainder = category.section(AccountSeperator, 1);

  QStringList list = base.accountList();
  QStringList::ConstIterator it_a;

  for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
    nextBase = account(*it_a);
    if (nextBase.name() == level) {
      if (remainder.isEmpty()) {
        return nextBase.id();
      }
      return locateSubAccount(nextBase, remainder);
    }
  }
  return QString();
}

QString MyMoneyFile::value(const QString& key) const
{
  checkStorage();

  return m_storage->value(key);
}

void MyMoneyFile::setValue(const QString& key, const QString& val)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->setValue(key, val);
}

void MyMoneyFile::deletePair(const QString& key)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->deletePair(key);
}

void MyMoneyFile::addSchedule(MyMoneySchedule& sched)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyTransaction transaction = sched.transaction();
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist or is one of the standard accounts
    MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());
    if (acc.id().isEmpty())
      throw new MYMONEYEXCEPTION("Cannot add split with no account assigned");
    if (isStandardAccount((*it_s).accountId()))
      throw new MYMONEYEXCEPTION("Cannot add split referencing standard account");
  }

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addSchedule(sched);
}

void MyMoneyFile::modifySchedule(const MyMoneySchedule& sched)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyTransaction transaction = sched.transaction();
  QList<MyMoneySplit>::ConstIterator it_s;
  for (it_s = transaction.splits().constBegin(); it_s != transaction.splits().constEnd(); ++it_s) {
    // the following line will throw an exception if the
    // account does not exist or is one of the standard accounts
    MyMoneyAccount acc = MyMoneyFile::account((*it_s).accountId());
    if (acc.id().isEmpty())
      throw new MYMONEYEXCEPTION("Cannot store split with no account assigned");
    if (isStandardAccount((*it_s).accountId()))
      throw new MYMONEYEXCEPTION("Cannot store split referencing standard account");
  }

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->modifySchedule(sched);

  addNotification(sched.id());
}

void MyMoneyFile::removeSchedule(const MyMoneySchedule& sched)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->removeSchedule(sched);

  addNotification(sched.id(), false);
}

const MyMoneySchedule MyMoneyFile::schedule(const QString& id) const
{
  return d->m_cache.schedule(id);
}

const QList<MyMoneySchedule> MyMoneyFile::scheduleList(
  const QString& accountId,
  const MyMoneySchedule::typeE type,
  const MyMoneySchedule::occurrenceE occurrence,
  const MyMoneySchedule::paymentTypeE paymentType,
  const QDate& startDate,
  const QDate& endDate,
  const bool overdue) const
{
  checkStorage();

  return m_storage->scheduleList(accountId, type, occurrence, paymentType, startDate, endDate, overdue);
}


const QStringList MyMoneyFile::consistencyCheck(void)
{
  QList<MyMoneyAccount> list;
  QList<MyMoneyAccount>::Iterator it_a;
  QList<MyMoneySchedule>::Iterator it_sch;
  QList<MyMoneyPayee>::Iterator it_p;
  QList<MyMoneyTransaction>::Iterator it_t;
  QList<MyMoneyReport>::Iterator it_r;
  QStringList accountRebuild;
  QStringList::ConstIterator it_c;

  QMap<QString, bool> interestAccounts;

  MyMoneyAccount parent;
  MyMoneyAccount child;
  MyMoneyAccount toplevel;

  QString parentId;
  QStringList rc;

  int problemCount = 0;
  int unfixedCount = 0;
  QString problemAccount;

  // check that we have a storage object
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // get the current list of accounts
  accountList(list);
  // add the standard accounts
  list << MyMoneyFile::instance()->asset();
  list << MyMoneyFile::instance()->liability();
  list << MyMoneyFile::instance()->income();
  list << MyMoneyFile::instance()->expense();

  for (it_a = list.begin(); it_a != list.end(); ++it_a) {
    // no more checks for standard accounts
    if (isStandardAccount((*it_a).id())) {
      continue;
    }

    switch ((*it_a).accountGroup()) {
      case MyMoneyAccount::Asset:
        toplevel = asset();
        break;
      case MyMoneyAccount::Liability:
        toplevel = liability();
        break;
      case MyMoneyAccount::Expense:
        toplevel = expense();
        break;
      case MyMoneyAccount::Income:
        toplevel = income();
        break;
      case MyMoneyAccount::Equity:
        toplevel = equity();
        break;
      default:
        qWarning("%s:%d This should never happen!", __FILE__ , __LINE__);
        break;
    }

    // check for loops in the hierarchy
    parentId = (*it_a).parentAccountId();
    try {
      bool dropOut = false;
      while (!isStandardAccount(parentId) && !dropOut) {
        parent = account(parentId);
        if (parent.id() == (*it_a).id()) {
          // parent loops, so we need to re-parent to toplevel account
          // find parent account in our list
          problemCount++;
          QList<MyMoneyAccount>::Iterator it_b;
          for (it_b = list.begin(); it_b != list.end(); ++it_b) {
            if ((*it_b).id() == parent.id()) {
              if (problemAccount != (*it_a).name()) {
                problemAccount = (*it_a).name();
                rc << i18n("* Problem with account '%1'", problemAccount);
                rc << i18n("  * Loop detected between this account and account '%1'.", (*it_b).name());
                rc << i18n("    Reparenting account '%2' to top level account '%1'.", toplevel.name(), (*it_a).name());
                (*it_a).setParentAccountId(toplevel.id());
                if (accountRebuild.contains(toplevel.id()) == 0)
                  accountRebuild << toplevel.id();
                if (accountRebuild.contains((*it_a).id()) == 0)
                  accountRebuild << (*it_a).id();
                dropOut = true;
                break;
              }
            }
          }
        }
        parentId = parent.parentAccountId();
      }

    } catch (MyMoneyException *e) {
      // if we don't know about a parent, we catch it later
      delete e;
    }

    // check that the parent exists
    parentId = (*it_a).parentAccountId();
    try {
      parent = account(parentId);
      if ((*it_a).accountGroup() != parent.accountGroup()) {
        problemCount++;
        if (problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << i18n("* Problem with account '%1'", problemAccount);
        }
        // the parent belongs to a different group, so we reconnect to the
        // master group account (asset, liability, etc) to which this account
        // should belong and update it in the engine.
        rc << i18n("  * Parent account '%1' belongs to a different group.", parent.name());
        rc << i18n("    New parent account is the top level account '%1'.", toplevel.name());
        (*it_a).setParentAccountId(toplevel.id());

        // make sure to rebuild the sub-accounts of the top account
        // and the one we removed this account from
        if (accountRebuild.contains(toplevel.id()) == 0)
          accountRebuild << toplevel.id();
        if (accountRebuild.contains(parent.id()) == 0)
          accountRebuild << parent.id();
      } else if (!parent.accountList().contains((*it_a).id())) {
        problemCount++;
        if (problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << i18n("* Problem with account '%1'", problemAccount);
        }
        // parent exists, but does not have a reference to the account
        rc << i18n("  * Parent account '%1' does not contain '%2' as sub-account.", parent.name(), problemAccount);
        if (accountRebuild.contains(parent.id()) == 0)
          accountRebuild << parent.id();
      }
    } catch (MyMoneyException *e) {
      delete e;
      // apparently, the parent does not exist anymore. we reconnect to the
      // master group account (asset, liability, etc) to which this account
      // should belong and update it in the engine.
      problemCount++;
      if (problemAccount != (*it_a).name()) {
        problemAccount = (*it_a).name();
        rc << i18n("* Problem with account '%1'", problemAccount);
      }
      rc << i18n("  * The parent with id %1 does not exist anymore.", parentId);
      rc << i18n("    New parent account is the top level account '%1'.", toplevel.name());
      (*it_a).setParentAccountId(toplevel.id());

      addNotification((*it_a).id());

      // make sure to rebuild the sub-accounts of the top account
      if (accountRebuild.contains(toplevel.id()) == 0)
        accountRebuild << toplevel.id();
    }

    // now check that all the children exist and have the correct type
    for (it_c = (*it_a).accountList().begin(); it_c != (*it_a).accountList().end(); ++it_c) {
      // check that the child exists
      try {
        child = account(*it_c);
      } catch (MyMoneyException *e) {
        problemCount++;
        if (problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << i18n("* Problem with account '%1'", problemAccount);
        }
        rc << i18n("  * Child account with id %1 does not exist anymore.", *it_c);
        rc << i18n("    The child account list will be reconstructed.");
        if (accountRebuild.contains((*it_a).id()) == 0)
          accountRebuild << (*it_a).id();
      }
    }

    // see if it is a loan account. if so, remember the assigned interest account
    if ((*it_a).isLoan()) {
      const MyMoneyAccountLoan* loan = dynamic_cast<MyMoneyAccountLoan*>(&(*it_a));
      if (!loan->interestAccountId().isEmpty())
        interestAccounts[loan->interestAccountId()] = true;
    }

    // check for clear text online password in the online settings
    if (!(*it_a).onlineBankingSettings().value("password").isEmpty()) {
      if (problemAccount != (*it_a).name()) {
        problemAccount = (*it_a).name();
        rc << i18n("* Problem with account '%1'", problemAccount);
      }
      rc << i18n("  * Older versions of KMyMoney stored an OFX password for this account in cleartext.");
      rc << i18n("    Please open it in the account editor (Account/Edit account) once and press OK.");
      rc << i18n("    This will store the password in the KDE wallet and remove the cleartext version.");
      ++unfixedCount;
    }

    // if the account was modified, we need to update it in the engine
    if (!(m_storage->account((*it_a).id()) == (*it_a))) {
      try {
        m_storage->modifyAccount(*it_a, true);
        addNotification((*it_a).id());
      } catch (MyMoneyException *e) {
        delete e;
        rc << i18n("  * Unable to update account data in engine.");
        return rc;
      }
    }
  }

  if (accountRebuild.count() != 0) {
    rc << i18n("* Reconstructing the child lists for");
  }

  // clear the affected lists
  for (it_a = list.begin(); it_a != list.end(); ++it_a) {
    if (accountRebuild.contains((*it_a).id())) {
      rc << QString("  %1").arg((*it_a).name());
      // clear the account list
      for (it_c = (*it_a).accountList().begin(); it_c != (*it_a).accountList().end();) {
        (*it_a).removeAccountId(*it_c);
        it_c = (*it_a).accountList().begin();
      }
    }
  }

  // reconstruct the lists
  for (it_a = list.begin(); it_a != list.end(); ++it_a) {
    QList<MyMoneyAccount>::Iterator it;
    parentId = (*it_a).parentAccountId();
    if (accountRebuild.contains(parentId)) {
      for (it = list.begin(); it != list.end(); ++it) {
        if ((*it).id() == parentId) {
          (*it).addAccountId((*it_a).id());
          break;
        }
      }
    }
  }

  // update the engine objects
  for (it_a = list.begin(); it_a != list.end(); ++it_a) {
    if (accountRebuild.contains((*it_a).id())) {
      try {
        m_storage->modifyAccount(*it_a, true);
        addNotification((*it_a).id());
      } catch (MyMoneyException *e) {
        delete e;
        rc << i18n("  * Unable to update account data for account %1 in engine", (*it_a).name());
      }
    }
  }

  // For some reason, files exist with invalid ids. This has been found in the payee id
  // so we fix them here
  QList<MyMoneyPayee> pList = payeeList();
  QMap<QString, QString>payeeConversionMap;

  for (it_p = pList.begin(); it_p != pList.end(); ++it_p) {
    if ((*it_p).id().length() > 7) {
      // found one of those with an invalid ids
      // create a new one and store it in the map.
      MyMoneyPayee payee = (*it_p);
      payee.clearId();
      m_storage->addPayee(payee);
      payeeConversionMap[(*it_p).id()] = payee.id();
      rc << i18n("  * Payee %1 recreated with fixed id", payee.name());
      ++problemCount;
    }
  }

  // Fix the transactions
  QList<MyMoneyTransaction> tList;
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  m_storage->transactionList(tList, filter);
  // Generate the list of interest accounts
  for (it_t = tList.begin(); it_t != tList.end(); ++it_t) {
    const MyMoneyTransaction& t = (*it_t);
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
      if ((*it_s).action() == MyMoneySplit::ActionInterest)
        interestAccounts[(*it_s).accountId()] = true;
    }
  }
  for (it_t = tList.begin(); it_t != tList.end(); ++it_t) {
    MyMoneyTransaction t = (*it_t);
    QList<MyMoneySplit> splits = t.splits();
    QList<MyMoneySplit>::const_iterator it_s;
    bool tChanged = false;
    for (it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
      bool sChanged = false;
      MyMoneySplit s = (*it_s);
      if (payeeConversionMap.find((*it_s).payeeId()) != payeeConversionMap.end()) {
        s.setPayeeId(payeeConversionMap[s.payeeId()]);
        sChanged = true;
        rc << i18n("  * Payee id updated in split of transaction '%1'.", t.id());
        ++problemCount;
      }

      // make sure, that shares and value have the same number if they
      // represent the same currency.
      try {
        const MyMoneyAccount& acc = this->account(s.accountId());
        if (t.commodity() == acc.currencyId()
            && s.shares().reduce() != s.value().reduce()) {
          // use the value as master if the transaction is balanced
          if (t.splitSum().isZero()) {
            s.setShares(s.value());
            rc << i18n("  * shares set to value in split of transaction '%1'.", t.id());
          } else {
            s.setValue(s.shares());
            rc << i18n("  * value set to shares in split of transaction '%1'.", t.id());
          }
          sChanged = true;
          ++problemCount;
        }
      } catch (MyMoneyException *e) {
        rc << i18n("  * Split %2 in transaction '%1' contains a reference to invalid account %3. Please fix manually.", t.id(), (*it_s).id(), (*it_s).accountId());
        ++problemCount;
        ++unfixedCount;
        delete e;
      }

      // make sure the interest splits are marked correct as such
      if (interestAccounts.find(s.accountId()) != interestAccounts.end()
          && s.action() != MyMoneySplit::ActionInterest) {
        s.setAction(MyMoneySplit::ActionInterest);
        sChanged = true;
        rc << i18n("  * action marked as interest in split of transaction '%1'.", t.id());
        ++problemCount;
      }

      if (sChanged) {
        tChanged = true;
        t.modifySplit(s);
      }
    }
    if (tChanged) {
      m_storage->modifyTransaction(t);
    }
  }

  // Fix the schedules
  QList<MyMoneySchedule> schList = scheduleList();
  for (it_sch = schList.begin(); it_sch != schList.end(); ++it_sch) {
    MyMoneySchedule sch = (*it_sch);
    MyMoneyTransaction t = sch.transaction();
    QList<MyMoneySplit> splits = t.splits();
    bool tChanged = false;
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
      MyMoneySplit s = (*it_s);
      bool sChanged = false;
      if (payeeConversionMap.find((*it_s).payeeId()) != payeeConversionMap.end()) {
        s.setPayeeId(payeeConversionMap[s.payeeId()]);
        sChanged = true;
        rc << i18n("  * Payee id updated in split of schedule '%1'.", (*it_sch).name());
        ++problemCount;
      }
      if (!(*it_s).value().isZero() && (*it_s).shares().isZero()) {
        s.setShares(s.value());
        sChanged = true;
        rc << i18n("  * Split in scheduled transaction '%1' contained value != 0 and shares == 0.", (*it_sch).name());
        rc << i18n("    Shares set to value.");
        ++problemCount;
      }

      // make sure, we don't have a bankid stored with a split in a schedule
      if (!(*it_s).bankID().isEmpty()) {
        s.setBankID(QString());
        sChanged = true;
        rc << i18n("  * Removed bankid from split in scheduled transaction '%1'.", (*it_sch).name());
        ++problemCount;
      }

      // make sure, that shares and value have the same number if they
      // represent the same currency.
      try {
        const MyMoneyAccount& acc = this->account(s.accountId());
        if (t.commodity() == acc.currencyId()
            && s.shares().reduce() != s.value().reduce()) {
          // use the value as master if the transaction is balanced
          if (t.splitSum().isZero()) {
            s.setShares(s.value());
            rc << i18n("  * shares set to value in split in schedule '%1'.", (*it_sch).name());
          } else {
            s.setValue(s.shares());
            rc << i18n("  * value set to shares in split in schedule '%1'.", (*it_sch).name());
          }
          sChanged = true;
          ++problemCount;
        }
      } catch (MyMoneyException *e) {
        rc << i18n("  * Split %2 in schedule '%1' contains a reference to invalid account %3. Please fix manually.", (*it_sch).name(), (*it_s).id(), (*it_s).accountId());
        ++problemCount;
        ++unfixedCount;
        delete e;
      }
      if (sChanged) {
        t.modifySplit(s);
        tChanged = true;
      }
    }
    if (tChanged) {
      sch.setTransaction(t);
      m_storage->modifySchedule(sch);
    }
  }

  // Fix the reports
  QList<MyMoneyReport> rList = reportList();
  for (it_r = rList.begin(); it_r != rList.end(); ++it_r) {
    MyMoneyReport r = *it_r;
    QStringList pList;
    QStringList::Iterator it_p;
    (*it_r).payees(pList);
    bool rChanged = false;
    for (it_p = pList.begin(); it_p != pList.end(); ++it_p) {
      if (payeeConversionMap.find(*it_p) != payeeConversionMap.end()) {
        rc << i18n("  * Payee id updated in report '%1'.", (*it_r).name());
        ++problemCount;
        r.removeReference(*it_p);
        r.addPayee(payeeConversionMap[*it_p]);
        rChanged = true;
      }
    }
    if (rChanged) {
      m_storage->modifyReport(r);
    }
  }

  // erase old payee ids
  QMap<QString, QString>::Iterator it_m;
  for (it_m = payeeConversionMap.begin(); it_m != payeeConversionMap.end(); ++it_m) {
    MyMoneyPayee payee = this->payee(it_m.key());
    removePayee(payee);
    rc << i18n("  * Payee '%1' removed.", payee.id());
    ++problemCount;
  }

  //look for accounts which have currencies other than the base currency but no price on the opening date
  //all accounts using base currency are excluded, since that's the base used for foreing currency calculation
  //thus it is considered as always present

  //get all currencies in use
  QStringList currencyList;
  QList<MyMoneyAccount> accountForeignCurrency;
  QList<MyMoneyAccount> accList;
  accountList(accList);
  QList<MyMoneyAccount>::const_iterator account_it;
  for (account_it = accList.constBegin(); account_it != accList.constEnd(); ++account_it) {
    MyMoneyAccount account = *account_it;
    if (!currencyList.contains(account.currencyId()) && account.currencyId() != baseCurrency().id()) {
      //add the currency and the account-currency pair
      currencyList.append(account.currencyId());
      accountForeignCurrency.append(account);
    }
  }

  MyMoneyPriceList pricesList = priceList();
  QMap<MyMoneySecurityPair, QDate> securityPriceDate;

  //get the first date of the price for each security
  MyMoneyPriceList::const_iterator prices_it;
  for (prices_it = pricesList.constBegin(); prices_it != pricesList.constEnd(); ++prices_it) {
    MyMoneyPrice firstPrice = (*((*prices_it).constBegin()));

    //only check the price if the currency is in use
    if (currencyList.contains(firstPrice.from()) || currencyList.contains(firstPrice.to())) {
      //check the security in the from field
      //if it is there, check if it is older
      QPair<QString, QString> pricePair = qMakePair(firstPrice.from(), firstPrice.to());
      securityPriceDate[pricePair] = firstPrice.date();
    }
  }

  //compare the dates with the opening dates of the accounts using each currency
  QList<MyMoneyAccount>::const_iterator accForeignList_it;
  bool firstInvProblem = true;
  for (accForeignList_it = accountForeignCurrency.constBegin(); accForeignList_it != accountForeignCurrency.constEnd(); ++accForeignList_it) {
    //setup the price pair correctly
    QPair<QString, QString> pricePair;
    //setup the reverse, which can also be used for rate conversion
    QPair<QString, QString> reversePricePair;
    if((*accForeignList_it).isInvest())
    {
      //if it is a stock, we have to search for a price from its stock to the currency of the account
      QString securityId = (*accForeignList_it).currencyId();
      QString tradingCurrencyId = security(securityId).tradingCurrency();
      pricePair = qMakePair(securityId, tradingCurrencyId);
      reversePricePair = qMakePair(tradingCurrencyId, securityId);
    } else {
      //if it is a regular account we search for a price from the currency of the account to the base currency
      QString currency = (*accForeignList_it).currencyId();
      QString baseCurrencyId = baseCurrency().id();
      pricePair = qMakePair(currency, baseCurrencyId);
      reversePricePair = qMakePair(baseCurrencyId, currency);
    }

    //compare the first price with the opening date of the account
    if ((!securityPriceDate.contains(pricePair) || securityPriceDate.value(pricePair) > (*accForeignList_it).openingDate())
      && (!securityPriceDate.contains(reversePricePair) || securityPriceDate.value(reversePricePair) > (*accForeignList_it).openingDate())) {
      if (firstInvProblem) {
        firstInvProblem = false;
        rc << i18n("* Potential problem with investments/currencies");
      }
      QDate openingDate = (*accForeignList_it).openingDate();
      MyMoneySecurity secError = security((*accForeignList_it).currencyId());
      if (!(*accForeignList_it).isInvest()) {
        rc << i18n("  * The account '%1' in currency '%2' has no price set for the opening date '%3'.", (*accForeignList_it).name(), secError.name(), openingDate.toString(Qt::ISODate));
        rc << i18n("    Please enter a price for the currency on or before the opening date.");
      } else {
        rc << i18n("  * The investment '%1' has no price set for the opening date '%2'.", (*accForeignList_it).name(), openingDate.toString(Qt::ISODate));
        rc << i18n("    Please enter a price for the investment on or before the opening date.");
      }
      ++unfixedCount;
    }
  }

  // add more checks here

  if (problemCount == 0 && unfixedCount == 0) {
    rc << i18n("Finished: data is consistent.");
  } else {
    const QString problemsCorrected = i18np("%1 problem corrected.", "%1 problems corrected.", problemCount);
    const QString problemsRemaining = i18np("%1 problem still present.", "%1 problems still present.", unfixedCount);

    rc << QString();
    rc << i18nc("%1 is a string, e.g. 7 problems corrected; %2 is a string, e.g. 3 problems still present", "Finished: %1 %2", problemsCorrected, problemsRemaining);
  }
  return rc;
}

QString MyMoneyFile::createCategory(const MyMoneyAccount& base, const QString& name)
{
  checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount parent = base;
  QString categoryText;

  if (base.id() != expense().id() && base.id() != income().id())
    throw new MYMONEYEXCEPTION("Invalid base category");

  QStringList subAccounts = name.split(AccountSeperator);
  QStringList::Iterator it;
  for (it = subAccounts.begin(); it != subAccounts.end(); ++it) {
    MyMoneyAccount categoryAccount;

    categoryAccount.setName(*it);
    categoryAccount.setAccountType(base.accountType());

    if (it == subAccounts.begin())
      categoryText += *it;
    else
      categoryText += (AccountSeperator + *it);

    // Only create the account if it doesn't exist
    try {
      QString categoryId = categoryToAccount(categoryText);
      if (categoryId.isEmpty())
        addAccount(categoryAccount, parent);
      else {
        categoryAccount = account(categoryId);
      }
    } catch (MyMoneyException *e) {
      qDebug("Unable to add account %s, %s, %s: %s",
             qPrintable(categoryAccount.name()),
             qPrintable(parent.name()),
             qPrintable(categoryText),
             qPrintable(e->what()));
      delete e;
    }

    parent = categoryAccount;
  }

  return categoryToAccount(name);
}

const QList<MyMoneySchedule> MyMoneyFile::scheduleListEx(int scheduleTypes,
    int scheduleOcurrences,
    int schedulePaymentTypes,
    QDate startDate,
    const QStringList& accounts) const
{
  checkStorage();

  return m_storage->scheduleListEx(scheduleTypes, scheduleOcurrences, schedulePaymentTypes, startDate, accounts);
}

void MyMoneyFile::addSecurity(MyMoneySecurity& security)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addSecurity(security);

  d->m_cache.preloadSecurity(security);
}

void MyMoneyFile::modifySecurity(const MyMoneySecurity& security)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->modifySecurity(security);

  addNotification(security.id());
}

void MyMoneyFile::removeSecurity(const MyMoneySecurity& security)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->removeSecurity(security);

  addNotification(security.id(), false);
}

const MyMoneySecurity& MyMoneyFile::security(const QString& id) const
{
  if (id.isEmpty())
    return baseCurrency();

  return d->m_cache.security(id);
}

const QList<MyMoneySecurity> MyMoneyFile::securityList(void) const
{
  checkStorage();

  return m_storage->securityList();
}

void MyMoneyFile::addCurrency(const MyMoneySecurity& currency)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addCurrency(currency);

  // we can't really use addNotification here, because there is
  // a difference in currency and security handling. So we just
  // preload the object right here.
  d->m_cache.preloadSecurity(currency);
}

void MyMoneyFile::modifyCurrency(const MyMoneySecurity& currency)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  // force reload of base currency object
  if (currency.id() == d->m_baseCurrency.id())
    d->m_baseCurrency.clearId();

  m_storage->modifyCurrency(currency);

  addNotification(currency.id());
}

void MyMoneyFile::removeCurrency(const MyMoneySecurity& currency)
{
  checkTransaction(Q_FUNC_INFO);

  if (currency.id() == d->m_baseCurrency.id()) {
    throw new MYMONEYEXCEPTION("Cannot delete base currency.");
  }

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->removeCurrency(currency);

  addNotification(currency.id(), false);
}

const MyMoneySecurity& MyMoneyFile::currency(const QString& id) const
{
  if (id.isEmpty())
    return baseCurrency();

  const MyMoneySecurity& curr = d->m_cache.security(id);
  if (curr.id().isEmpty()) {
    QString msg;
    msg = QString("Currency '%1' not found.").arg(id);
    throw new MYMONEYEXCEPTION(msg);
  }
  return curr;
}

const QList<MyMoneySecurity> MyMoneyFile::currencyList(void) const
{
  checkStorage();

  return m_storage->currencyList();
}

const QString& MyMoneyFile::foreignCurrency(const QString& first, const QString& second) const
{
  if (baseCurrency().id() == second)
    return first;
  return second;
}

const MyMoneySecurity& MyMoneyFile::baseCurrency(void) const
{
  if (d->m_baseCurrency.id().isEmpty()) {
    QString id = QString(value("kmm-baseCurrency"));
    if (!id.isEmpty())
      d->m_baseCurrency = currency(id);
  }

  return d->m_baseCurrency;
}

void MyMoneyFile::setBaseCurrency(const MyMoneySecurity& curr)
{
  // make sure the currency exists
  MyMoneySecurity c = currency(curr.id());

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  if (c.id() != d->m_baseCurrency.id()) {
    setValue("kmm-baseCurrency", curr.id());
    // force reload of base currency cache
    d->m_baseCurrency = MyMoneySecurity();
  }
}

void MyMoneyFile::addPrice(const MyMoneyPrice& price)
{
  if (price.rate(QString()).isZero())
    return;

  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addPrice(price);
}

void MyMoneyFile::removePrice(const MyMoneyPrice& price)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->removePrice(price);
}

const MyMoneyPrice MyMoneyFile::price(const QString& fromId, const QString& toId, const QDate& date, const bool exactDate) const
{
  checkStorage();

  QString to(toId);
  if (to.isEmpty())
    to = value("kmm-baseCurrency");
  // if some id is missing, we can return an empty price object
  if (fromId.isEmpty() || to.isEmpty())
    return MyMoneyPrice();

  // we don't search our tables if someone asks stupid stuff
  if (fromId == toId) {
    return MyMoneyPrice(fromId, toId, date, MyMoneyMoney(1, 1), "KMyMoney");
  }

  // search 'from-to' rate
  MyMoneyPrice rc = m_storage->price(fromId, to, date, exactDate);
  if (!rc.isValid()) {
    // not found, search 'to-fron' rate and use reciprocal value
    rc = m_storage->price(to, fromId, date, exactDate);
  }
  return rc;
}

const MyMoneyPriceList MyMoneyFile::priceList(void) const
{
  checkStorage();

  return m_storage->priceList();
}

bool MyMoneyFile::hasAccount(const QString& id, const QString& name) const
{
  MyMoneyAccount acc = d->m_cache.account(id);
  QStringList list = acc.accountList();
  QStringList::ConstIterator it;
  bool rc = false;
  for (it = list.constBegin(); rc == false && it != list.constEnd(); ++it) {
    MyMoneyAccount a = d->m_cache.account(*it);
    if (a.name() == name)
      rc = true;
  }
  return rc;
}

const QList<MyMoneyReport> MyMoneyFile::reportList(void) const
{
  checkStorage();

  return m_storage->reportList();
}

void MyMoneyFile::addReport(MyMoneyReport& report)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addReport(report);
}

void MyMoneyFile::modifyReport(const MyMoneyReport& report)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->modifyReport(report);

  addNotification(report.id());
}

unsigned MyMoneyFile::countReports(void) const
{
  checkStorage();

  return m_storage->countReports();
}

const MyMoneyReport MyMoneyFile::report(const QString& id) const
{
  checkStorage();

  return m_storage->report(id);
}

void MyMoneyFile::removeReport(const MyMoneyReport& report)
{
  checkTransaction(Q_FUNC_INFO);
  MyMoneyNotifier notifier(this);

  m_storage->removeReport(report);

  addNotification(report.id(), false);
}


const QList<MyMoneyBudget> MyMoneyFile::budgetList(void) const
{
  checkStorage();

  return m_storage->budgetList();
}

void MyMoneyFile::addBudget(MyMoneyBudget& budget)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->addBudget(budget);
}

const MyMoneyBudget MyMoneyFile::budgetByName(const QString& name) const
{
  checkStorage();

  return m_storage->budgetByName(name);
}

void MyMoneyFile::modifyBudget(const MyMoneyBudget& budget)
{
  checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(this);

  m_storage->modifyBudget(budget);

  addNotification(budget.id());
}

unsigned MyMoneyFile::countBudgets(void) const
{
  checkStorage();

  return m_storage->countBudgets();
}

const MyMoneyBudget MyMoneyFile::budget(const QString& id) const
{
  checkStorage();

  return m_storage->budget(id);
}

void MyMoneyFile::removeBudget(const MyMoneyBudget& budget)
{
  checkTransaction(Q_FUNC_INFO);
  MyMoneyNotifier notifier(this);

  m_storage->removeBudget(budget);

  addNotification(budget.id(), false);
}



bool MyMoneyFile::isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipChecks) const
{
  checkStorage();
  return m_storage->isReferenced(obj, skipChecks);
}

bool MyMoneyFile::checkNoUsed(const QString& accId, const QString& no) const
{
  // by definition, an empty string or a non-numeric string is not used
  QRegExp exp(QString("(.*\\D)?(\\d+)(\\D.*)?"));
  if (no.isEmpty() || exp.indexIn(no) == -1)
    return false;

  MyMoneyTransactionFilter filter;
  filter.addAccount(accId);
  QList<MyMoneyTransaction> transactions = transactionList(filter);
  QList<MyMoneyTransaction>::ConstIterator it_t = transactions.constBegin();
  while (it_t != transactions.constEnd()) {
    try {
      MyMoneySplit split;
      // Test whether the transaction also includes a split into
      // this account
      split = (*it_t).splitByAccount(accId, true /*match*/);
      if (!split.number().isEmpty() && split.number() == no)
        return true;
    } catch (MyMoneyException *e) {
      delete e;
    }
    ++it_t;
  }
  return false;
}

QString MyMoneyFile::highestCheckNo(const QString& accId) const
{
  unsigned64 lno = 0, cno;
  QString no;
  MyMoneyTransactionFilter filter;
  filter.addAccount(accId);
  QList<MyMoneyTransaction> transactions = transactionList(filter);
  QList<MyMoneyTransaction>::ConstIterator it_t = transactions.constBegin();
  while (it_t != transactions.constEnd()) {
    try {
      // Test whether the transaction also includes a split into
      // this account
      MyMoneySplit split = (*it_t).splitByAccount(accId, true /*match*/);
      if (!split.number().isEmpty()) {
        // non-numerical values stored in number will return 0 in the next line
        cno = split.number().toULongLong();
        if (cno > lno) {
          lno = cno;
          no = split.number();
        }
      }
    } catch (MyMoneyException *e) {
      delete e;
    }
    ++it_t;
  }
  return no;
}

void MyMoneyFile::clearCache(void)
{
  checkStorage();
  m_storage->clearCache();
  d->m_cache.clear();
}

void MyMoneyFile::preloadCache(void)
{
  checkStorage();

  d->m_cache.clear();
  QList<MyMoneyAccount> a_list;
  m_storage->accountList(a_list);
  d->m_cache.preloadAccount(a_list);
  d->m_cache.preloadPayee(m_storage->payeeList());
  d->m_cache.preloadInstitution(m_storage->institutionList());
  d->m_cache.preloadSecurity(m_storage->securityList() + m_storage->currencyList());
  d->m_cache.preloadSchedule(m_storage->scheduleList());
}

bool MyMoneyFile::isTransfer(const MyMoneyTransaction& t) const
{
  bool rc = false;
  if (t.splitCount() == 2) {
    QList<MyMoneySplit>::const_iterator it_s;
    for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
      MyMoneyAccount acc = account((*it_s).accountId());
      if (acc.isIncomeExpense())
        break;
    }
    if (it_s == t.splits().end())
      rc = true;
  }
  return rc;
}

bool MyMoneyFile::referencesClosedAccount(const MyMoneyTransaction& t) const
{
  QList<MyMoneySplit>::const_iterator it_s;
  const QList<MyMoneySplit>& list = t.splits();
  for (it_s = list.begin(); it_s != list.end(); ++it_s) {
    if (referencesClosedAccount(*it_s))
      break;
  }
  return it_s != list.end();
}

bool MyMoneyFile::referencesClosedAccount(const MyMoneySplit& s) const
{
  if (s.accountId().isEmpty())
    return false;

  try {
    return account(s.accountId()).isClosed();
  } catch (MyMoneyException *e) {
    delete e;
  }
  return false;
}

MyMoneyFileTransaction::MyMoneyFileTransaction() :
    m_isNested(MyMoneyFile::instance()->hasTransaction()),
    m_needRollback(!m_isNested)
{
  if (!m_isNested)
    MyMoneyFile::instance()->startTransaction();
}

MyMoneyFileTransaction::~MyMoneyFileTransaction()
{
  rollback();
}

void MyMoneyFileTransaction::restart(void)
{
  rollback();

  m_needRollback = !m_isNested;
  if (!m_isNested)
    MyMoneyFile::instance()->startTransaction();
}

void MyMoneyFileTransaction::commit(void)
{
  if (!m_isNested)
    MyMoneyFile::instance()->commitTransaction();
  m_needRollback = false;
}

void MyMoneyFileTransaction::rollback(void)
{
  if (m_needRollback)
    MyMoneyFile::instance()->rollbackTransaction();
  m_needRollback = false;
}


#include "mymoneyfile.moc"
