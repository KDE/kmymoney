/*
    SPDX-FileCopyrightText: 2000-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneyfile.h"

#include <utility>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>
#include <QUuid>
#include <QLocale>
#include <QBitArray>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneystoragemgr.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneyaccountloan.h"
#include "mymoneysecurity.h"
#include "mymoneyreport.h"
#include "mymoneybalancecache.h"
#include "mymoneybudget.h"
#include "mymoneyprice.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneycostcenter.h"
#include "mymoneyexception.h"
#include "onlinejob.h"
#include "storageenums.h"
#include "mymoneyenums.h"

// include the following line to get a 'cout' for debug purposes
// #include <iostream>

using namespace eMyMoney;

const QString MyMoneyFile::AccountSeparator = QChar(':');

MyMoneyFile MyMoneyFile::file;

typedef QList<std::pair<QString, QDate> > BalanceNotifyList;
typedef QMap<QString, bool> CacheNotifyList;

/// @todo make this template based
class MyMoneyNotification
{
public:
  MyMoneyNotification(File::Mode mode, const MyMoneyTransaction& t) :
      m_objType(File::Object::Transaction),
      m_notificationMode(mode),
      m_id(t.id()) {
  }

  MyMoneyNotification(File::Mode mode, const MyMoneyAccount& acc) :
      m_objType(File::Object::Account),
      m_notificationMode(mode),
      m_id(acc.id()) {
  }

  MyMoneyNotification(File::Mode mode, const MyMoneyInstitution& institution) :
      m_objType(File::Object::Institution),
      m_notificationMode(mode),
      m_id(institution.id()) {
  }

  MyMoneyNotification(File::Mode mode, const MyMoneyPayee& payee) :
      m_objType(File::Object::Payee),
      m_notificationMode(mode),
      m_id(payee.id()) {
  }

  MyMoneyNotification(File::Mode mode, const MyMoneyTag& tag) :
      m_objType(File::Object::Tag),
      m_notificationMode(mode),
      m_id(tag.id()) {
  }

  MyMoneyNotification(File::Mode mode, const MyMoneySchedule& schedule) :
      m_objType(File::Object::Schedule),
      m_notificationMode(mode),
      m_id(schedule.id()) {
  }

  MyMoneyNotification(File::Mode mode, const MyMoneySecurity& security) :
      m_objType(File::Object::Security),
      m_notificationMode(mode),
      m_id(security.id()) {
  }

  MyMoneyNotification(File::Mode mode, const onlineJob& job) :
      m_objType(File::Object::OnlineJob),
      m_notificationMode(mode),
      m_id(job.id()) {
  }

  File::Object objectType() const {
    return m_objType;
  }
  File::Mode notificationMode() const {
    return m_notificationMode;
  }
  const QString& id() const {
    return m_id;
  }

protected:
  MyMoneyNotification(File::Object obj,
                      File::Mode mode,
                      const QString& id) :
      m_objType(obj),
      m_notificationMode(mode),
      m_id(id) {}

private:
  File::Object   m_objType;
  File::Mode     m_notificationMode;
  QString        m_id;
};





class MyMoneyFile::Private
{
public:
  Private() :
      m_storage(0),
      m_inTransaction(false) {}

  ~Private() {
    delete m_storage;
  }
  /**
    * This method is used to add an id to the list of objects
    * to be removed from the cache. If id is empty, then nothing is added to the list.
    *
    * @param id id of object to be notified
    * @param reload reload the object (@c true) or not (@c false). The default is @c true
    * @see attach, detach
    */
  void addCacheNotification(const QString& id, const QDate& date) {
    if (!id.isEmpty())
      m_balanceNotifyList.append(std::make_pair(id, date));
  }

  /**
    * This method is used to clear the notification list
    */
  void clearCacheNotification() {
    // reset list to be empty
    m_balanceNotifyList.clear();
  }

  /**
    * This method is used to clear all
    * objects mentioned in m_notificationList from the cache.
    */
  void notify() {
    foreach (const BalanceNotifyList::value_type & i, m_balanceNotifyList) {
      m_balanceChangedSet += i.first;
      if (i.second.isValid()) {
        m_balanceCache.clear(i.first, i.second);
      } else {
        m_balanceCache.clear(i.first);
      }
    }

    clearCacheNotification();
  }

  /**
    * This method checks if a storage object is attached and
    * throws and exception if not.
    */
  inline void checkStorage() const {
    if (m_storage == 0)
      throw MYMONEYEXCEPTION_CSTRING("No storage object attached to MyMoneyFile");
  }

  /**
    * This method checks that a transaction has been started with
    * startTransaction() and throws an exception otherwise. Calls
    * checkStorage() to make sure a storage object is present and attached.
    */
  void checkTransaction(const char* txt) const {
    checkStorage();
    if (!m_inTransaction)
      throw MYMONEYEXCEPTION(QString::fromLatin1("No transaction started for %1").arg(QString::fromLatin1(txt)));
  }

  void priceChanged(const MyMoneyFile& file, const MyMoneyPrice price) {
    // get all affected accounts and add them to the m_valueChangedSet
    QList<MyMoneyAccount> accList;
    file.accountList(accList);
    QList<MyMoneyAccount>::const_iterator account_it;
    for (account_it = accList.constBegin(); account_it != accList.constEnd(); ++account_it) {
      QString currencyId = account_it->currencyId();
      if (currencyId != file.baseCurrency().id() && (currencyId == price.from() || currencyId == price.to())) {
        // this account is not in the base currency and the price affects it's value
        m_valueChangedSet.insert(account_it->id());
      }
    }
  }

  /**
    * This member points to the storage strategy
    */
  MyMoneyStorageMgr *m_storage;


  bool                   m_inTransaction;
  MyMoneySecurity        m_baseCurrency;

  /**
   * @brief Cache for MyMoneyObjects
   *
   * It is also used to emit the objectAdded() and objectModified() signals.
   * => If one of these signals is used, you must use this cache.
   */
  MyMoneyPriceList       m_priceCache;
  MyMoneyBalanceCache    m_balanceCache;

  /**
    * This member keeps a list of account ids to notify
    * after a single operation is completed. The balance cache
    * is cleared for that account and all dates on or after
    * the one supplied. If the date is invalid, the entire
    * balance cache is cleared for that account.
    */
  BalanceNotifyList m_balanceNotifyList;

  /**
    * This member keeps a list of account ids for which
    * a balanceChanged() signal needs to be emitted when
    * a set of operations has been committed.
    *
    * @sa MyMoneyFile::commitTransaction()
    */
  QSet<QString>     m_balanceChangedSet;

  /**
    * This member keeps a list of account ids for which
    * a valueChanged() signal needs to be emitted when
    * a set of operations has been committed.
    *
    * @sa MyMoneyFile::commitTransaction()
    */
  QSet<QString>     m_valueChangedSet;

  /**
    * This member keeps the list of changes in the engine
    * in historical order. The type can be 'added', 'modified'
    * or removed.
    */
  QList<MyMoneyNotification> m_changeSet;
};


class MyMoneyNotifier
{
public:
  MyMoneyNotifier(MyMoneyFile::Private* file) {
    m_file = file; m_file->clearCacheNotification();
  }
  ~MyMoneyNotifier() {
    m_file->notify();
  }
private:
  MyMoneyFile::Private* m_file;
};



MyMoneyFile::MyMoneyFile() :
    d(new Private)
{
}

MyMoneyFile::~MyMoneyFile()
{
  delete d;
}

MyMoneyFile::MyMoneyFile(MyMoneyStorageMgr *storage) :
    d(new Private)
{
  attachStorage(storage);
}

MyMoneyFile* MyMoneyFile::instance()
{
  return &file;
}

void MyMoneyFile::attachStorage(MyMoneyStorageMgr* const storage)
{
  if (d->m_storage != 0)
    throw MYMONEYEXCEPTION_CSTRING("Storage already attached");

  if (storage == 0)
    throw MYMONEYEXCEPTION_CSTRING("Storage must not be 0");

  d->m_storage = storage;

  // force reload of base currency
  d->m_baseCurrency = MyMoneySecurity();

  // and the whole cache
  d->m_balanceCache.clear();
  d->m_priceCache.clear();

  // notify application about new data availability
  emit beginChangeNotification();
  emit dataChanged();
  emit endChangeNotification();
}

void MyMoneyFile::detachStorage(MyMoneyStorageMgr* const /* storage */)
{
  d->m_balanceCache.clear();
  d->m_priceCache.clear();
  d->m_storage = nullptr;
}

MyMoneyStorageMgr* MyMoneyFile::storage() const
{
  return d->m_storage;
}

bool MyMoneyFile::storageAttached() const
{
  return d->m_storage != 0;
}

void MyMoneyFile::startTransaction()
{
  d->checkStorage();
  if (d->m_inTransaction) {
    throw MYMONEYEXCEPTION_CSTRING("Already started a transaction!");
  }

  d->m_storage->startTransaction();
  d->m_inTransaction = true;
  d->m_changeSet.clear();
}

bool MyMoneyFile::hasTransaction() const
{
  return d->m_inTransaction;
}

void MyMoneyFile::commitTransaction()
{
  d->checkTransaction(Q_FUNC_INFO);

  // commit the transaction in the storage
  const auto changed = d->m_storage->commitTransaction();
  d->m_inTransaction = false;

  // collect notifications about removed objects
  QStringList removedObjects;
  const auto& set = d->m_changeSet;
  for (const auto& change : set) {
    switch (change.notificationMode()) {
      case File::Mode::Remove:
        removedObjects += change.id();
        break;
      default:
        break;
    }
  }

  // inform the outside world about the beginning of notifications
  emit beginChangeNotification();

  // Now it's time to send out some signals to the outside world
  // First we go through the d->m_changeSet and emit respective
  // signals about addition, modification and removal of engine objects
  const auto& changes = d->m_changeSet;
  for (const auto& change : changes) {
    switch (change.notificationMode()) {
      case File::Mode::Remove:
        emit objectRemoved(change.objectType(), change.id());
        // if there is a balance change recorded for this account remove it since the account itself will be removed
        // this can happen when deleting categories that have transactions and the reassign category feature was used
        d->m_balanceChangedSet.remove(change.id());
        break;
      case File::Mode::Add:
        if (!removedObjects.contains(change.id())) {
          emit objectAdded(change.objectType(), change.id());
        }
        break;
      case File::Mode::Modify:
        if (!removedObjects.contains(change.id())) {
          emit objectModified(change.objectType(), change.id());
        }
        break;
    }
  }

  // we're done with the change set, so we clear it
  d->m_changeSet.clear();

  // now send out the balanceChanged signal for all those
  // accounts for which we have an indication about a possible
  // change.
  const auto& balanceChanges = d->m_balanceChangedSet;
  for (const auto& id : balanceChanges) {
    if (!removedObjects.contains(id)) {
      // if we notify about balance change we don't need to notify about value change
      // for the same account since a balance change implies a value change
      d->m_valueChangedSet.remove(id);
      emit balanceChanged(account(id));
    }
  }
  d->m_balanceChangedSet.clear();

  // now notify about the remaining value changes
  const auto& m_valueChanges = d->m_valueChangedSet;
  for (const auto& id : m_valueChanges) {
    if (!removedObjects.contains(id)) {
      emit valueChanged(account(id));
    }
  }

  d->m_valueChangedSet.clear();

  // as a last action, send out the global dataChanged signal
  if (changed)
    emit dataChanged();

  // inform the outside world about the end of notifications
  emit endChangeNotification();
}

void MyMoneyFile::rollbackTransaction()
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->rollbackTransaction();
  d->m_inTransaction = false;
  d->m_balanceChangedSet.clear();
  d->m_valueChangedSet.clear();
  d->m_changeSet.clear();
}

void MyMoneyFile::addInstitution(MyMoneyInstitution& institution)
{
  // perform some checks to see that the institution stuff is OK. For
  // now we assume that the institution must have a name, the ID is not set
  // and it does not have a parent (MyMoneyFile).

  if (institution.name().length() == 0
      || institution.id().length() != 0)
    throw MYMONEYEXCEPTION_CSTRING("Not a new institution");

  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addInstitution(institution);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, institution);
}

void MyMoneyFile::modifyInstitution(const MyMoneyInstitution& institution)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->modifyInstitution(institution);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, institution);
}

void MyMoneyFile::modifyTransaction(const MyMoneyTransaction& transaction)
{
  d->checkTransaction(Q_FUNC_INFO);

  MyMoneyTransaction tCopy(transaction);

  // now check the splits
  bool loanAccountAffected = false;
  const auto splits1 = transaction.splits();
  for (const auto& split : splits1) {
    // the following line will throw an exception if the
    // account does not exist
    auto acc = MyMoneyFile::account(split.accountId());
    if (acc.id().isEmpty())
      throw MYMONEYEXCEPTION_CSTRING("Cannot store split with no account assigned");
    if (isStandardAccount(split.accountId()))
      throw MYMONEYEXCEPTION_CSTRING("Cannot store split referencing standard account");
    if (acc.isLoan() && (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer)))
      loanAccountAffected = true;
  }

  // change transfer splits between asset/liability and loan accounts
  // into amortization splits
  if (loanAccountAffected) {
    const auto splits = transaction.splits();
    for (const auto& split : splits) {
      if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer)) {
        auto acc = MyMoneyFile::account(split.accountId());

        if (acc.isAssetLiability()) {
          MyMoneySplit s = split;
          s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
          tCopy.modifySplit(s);
        }
      }
    }
  }

  // clear all changed objects from cache
  MyMoneyNotifier notifier(d);

  // get the current setting of this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());

  // scan the splits again to update notification list
  // and mark all accounts that are referenced
  const auto splits2 = tr.splits();
  foreach (const auto& split, splits2)
    d->addCacheNotification(split.accountId(), tr.postDate());

  // make sure the value is rounded to the accounts precision
  fixSplitPrecision(tCopy);

  // perform modification
  d->m_storage->modifyTransaction(tCopy);

  // and mark all accounts that are referenced
  const auto splits3 = tCopy.splits();
  for (const auto& split : splits3)
    d->addCacheNotification(split.accountId(), tCopy.postDate());

  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, transaction);
}

void MyMoneyFile::modifyAccount(const MyMoneyAccount& _account)
{
  d->checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount account(_account);

  auto acc = MyMoneyFile::account(account.id());

  // check that for standard accounts only specific parameters are changed
  if (isStandardAccount(account.id())) {
    // make sure to use the stuff we found on file
    account = acc;

    // and only use the changes that are allowed
    account.setName(_account.name());
    account.setCurrencyId(_account.currencyId());

    // now check that it is the same
    if (!(account == _account))
      throw MYMONEYEXCEPTION_CSTRING("Unable to modify the standard account groups");
  }

  if (account.accountType() != acc.accountType() &&
      !account.isLiquidAsset() && !acc.isLiquidAsset())
    throw MYMONEYEXCEPTION_CSTRING("Unable to change account type");

  // if the account was moved to another institution, we notify
  // the old one as well as the new one and the structure change
  if (acc.institutionId() != account.institutionId()) {
    MyMoneyInstitution inst;
    if (!acc.institutionId().isEmpty()) {
      inst = institution(acc.institutionId());
      inst.removeAccountId(acc.id());
      modifyInstitution(inst);
      // modifyInstitution updates d->m_changeSet already
    }
    if (!account.institutionId().isEmpty()) {
      inst = institution(account.institutionId());
      inst.addAccountId(acc.id());
      modifyInstitution(inst);
      // modifyInstitution updates d->m_changeSet already
    }
  }

  d->m_storage->modifyAccount(account);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, account);
}

void MyMoneyFile::reparentAccount(MyMoneyAccount &acc, MyMoneyAccount& parent)
{
  d->checkTransaction(Q_FUNC_INFO);

  // check that it's not one of the standard account groups
  if (isStandardAccount(acc.id()))
    throw MYMONEYEXCEPTION_CSTRING("Unable to reparent the standard account groups");

  if (acc.accountGroup() == parent.accountGroup()
      || (acc.accountType() == Account::Type::Income && parent.accountType() == Account::Type::Expense)
      || (acc.accountType() == Account::Type::Expense && parent.accountType() == Account::Type::Income)) {

    if (acc.isInvest() && parent.accountType() != Account::Type::Investment)
      throw MYMONEYEXCEPTION_CSTRING("Unable to reparent Stock to non-investment account");

    if (parent.accountType() == Account::Type::Investment && !acc.isInvest())
      throw MYMONEYEXCEPTION_CSTRING("Unable to reparent non-stock to investment account");

    // keep a notification of the current parent
    MyMoneyAccount curParent = account(acc.parentAccountId());

    d->m_storage->reparentAccount(acc, parent);

    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, curParent);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, parent);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, acc);

  } else
    throw MYMONEYEXCEPTION_CSTRING("Unable to reparent to different account type");
}

MyMoneyInstitution MyMoneyFile::institution(const QString& id) const
{
  return d->m_storage->institution(id);
}

MyMoneyAccount MyMoneyFile::account(const QString& id) const
{
  if (Q_UNLIKELY(id.isEmpty())) // FIXME: Stop requesting accounts with empty id
    return MyMoneyAccount();

  return d->m_storage->account(id);
}

MyMoneyAccount MyMoneyFile::subAccountByName(const MyMoneyAccount& account, const QString& name) const
{
  static MyMoneyAccount nullAccount;

  const auto accounts = account.accountList();
  for (const auto& acc : accounts) {
    const auto sacc = MyMoneyFile::account(acc);
    if (sacc.name().compare(name) == 0)
      return sacc;
  }
  return nullAccount;
}

MyMoneyAccount MyMoneyFile::accountByName(const QString& name) const
{
  try {
    return d->m_storage->accountByName(name);
  } catch (const MyMoneyException &) {
  }
  return MyMoneyAccount();
}

void MyMoneyFile::removeTransaction(const MyMoneyTransaction& transaction)
{
  d->checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(d);

  // get the engine's idea about this transaction
  MyMoneyTransaction tr = MyMoneyFile::transaction(transaction.id());

  // scan the splits again to update notification list
  const auto splits = tr.splits();
  for (const auto& split : splits) {
    auto acc = account(split.accountId());
    if (acc.isClosed())
      throw MYMONEYEXCEPTION(QString::fromLatin1("Cannot remove transaction that references a closed account."));
    d->addCacheNotification(split.accountId(), tr.postDate());
    //FIXME-ALEX Do I need to add d->addCacheNotification(split.tagList()); ??
  }

  d->m_storage->removeTransaction(transaction);

  // remove a possible notification of that same object from the changeSet
  QList<MyMoneyNotification>::iterator it;
  for(it = d->m_changeSet.begin(); it != d->m_changeSet.end();) {
    if((*it).id() == transaction.id()) {
      it = d->m_changeSet.erase(it);
    } else {
      ++it;
    }
  }

  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, transaction);
}


bool MyMoneyFile::hasActiveSplits(const QString& id) const
{
  d->checkStorage();

  return d->m_storage->hasActiveSplits(id);
}

bool MyMoneyFile::isStandardAccount(const QString& id) const
{
  d->checkStorage();

  return d->m_storage->isStandardAccount(id);
}

void MyMoneyFile::setAccountName(const QString& id, const QString& name) const
{
  d->checkTransaction(Q_FUNC_INFO);

  auto acc = account(id);
  d->m_storage->setAccountName(id, name);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, acc);
}

void MyMoneyFile::removeAccount(const MyMoneyAccount& account)
{
  d->checkTransaction(Q_FUNC_INFO);

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
    throw MYMONEYEXCEPTION_CSTRING("Unable to remove the standard account groups");

  if (hasActiveSplits(account.id())) {
    throw MYMONEYEXCEPTION_CSTRING("Unable to remove account with active splits");
  }

  // collect all sub-ordinate accounts for notification
  const auto accounts = acc.accountList();
  for (const auto& id : accounts)
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, MyMoneyFile::account(id));

  // don't forget the parent and a possible institution

  if (!institution.id().isEmpty()) {
    institution.removeAccountId(account.id());
    d->m_storage->modifyInstitution(institution);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, institution);
  }
  acc.setInstitutionId(QString());

  d->m_storage->removeAccount(acc);

  d->m_balanceCache.clear(acc.id());

  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, parent);
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, acc);
}

void MyMoneyFile::removeAccountList(const QStringList& account_list, unsigned int level)
{
  if (level > 100)
    throw MYMONEYEXCEPTION_CSTRING("Too deep recursion in [MyMoneyFile::removeAccountList]!");

  d->checkTransaction(Q_FUNC_INFO);

  // upon entry, we check that we could proceed with the operation
  if (!level) {
    if (!hasOnlyUnusedAccounts(account_list, 0)) {
      throw MYMONEYEXCEPTION_CSTRING("One or more accounts cannot be removed");
    }
  }

  // process all accounts in the list and test if they have transactions assigned
  foreach (const auto sAccount, account_list) {
    auto a = d->m_storage->account(sAccount);
    //qDebug() << "Deleting account '"<< a.name() << "'";

    // first remove all sub-accounts
    if (!a.accountList().isEmpty()) {
      removeAccountList(a.accountList(), level + 1);

      // then remove account itself, but we first have to get
      // rid of the account list that is still stored in
      // the MyMoneyAccount object. Easiest way is to get a fresh copy.
      a = d->m_storage->account(sAccount);
    }

    // make sure to remove the item from the cache
    removeAccount(a);
  }
}

bool MyMoneyFile::hasOnlyUnusedAccounts(const QStringList& account_list, unsigned int level)
{
  if (level > 100)
    throw MYMONEYEXCEPTION_CSTRING("Too deep recursion in [MyMoneyFile::hasOnlyUnusedAccounts]!");
  // process all accounts in the list and test if they have transactions assigned
  for (const auto& sAccount : account_list) {
    if (transactionCount(sAccount) != 0)
      return false; // the current account has a transaction assigned
    if (!hasOnlyUnusedAccounts(account(sAccount).accountList(), level + 1))
      return false; // some sub-account has a transaction assigned
  }
  return true; // all subaccounts unused
}


void MyMoneyFile::removeInstitution(const MyMoneyInstitution& institution)
{
  d->checkTransaction(Q_FUNC_INFO);

  MyMoneyInstitution inst = MyMoneyFile::institution(institution.id());

  bool blocked = signalsBlocked();
  blockSignals(true);
  const auto accounts = inst.accountList();
  for (const auto& acc : accounts) {
    auto a = account(acc);
    a.setInstitutionId(QString());
    modifyAccount(a);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, a);
  }
  blockSignals(blocked);

  d->m_storage->removeInstitution(institution);

  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, institution);
}

void MyMoneyFile::createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal)
{
  // make sure we have a currency. If none is assigned, we assume base currency
  if (newAccount.currencyId().isEmpty())
    newAccount.setCurrencyId(baseCurrency().id());

  MyMoneyFileTransaction ft;
  try {
    int pos;
    // check for ':' in the name and use it as separator for a hierarchy
    while ((pos = newAccount.name().indexOf(MyMoneyFile::AccountSeparator)) != -1) {
      QString part = newAccount.name().left(pos);
      QString remainder = newAccount.name().mid(pos + 1);
      const MyMoneyAccount& existingAccount = subAccountByName(parentAccount, part);
      if (existingAccount.id().isEmpty()) {
        newAccount.setName(part);

        addAccount(newAccount, parentAccount);
        parentAccount = newAccount;
      } else {
        parentAccount = existingAccount;
      }
      newAccount.setParentAccountId(QString());  // make sure, there's no parent
      newAccount.clearId();                       // and no id set for adding
      newAccount.removeAccountIds();              // and no sub-account ids
      newAccount.setName(remainder);
    }

    addAccount(newAccount, parentAccount);

    // in case of a loan account, we add the initial payment
    if ((newAccount.accountType() == Account::Type::Loan
         || newAccount.accountType() == Account::Type::AssetLoan)
        && !newAccount.value("kmm-loan-payment-acc").isEmpty()
        && !newAccount.value("kmm-loan-payment-date").isEmpty()) {
      MyMoneyAccountLoan acc(newAccount);
      MyMoneyTransaction t;
      MyMoneySplit a, b;
      a.setAccountId(acc.id());
      b.setAccountId(acc.value("kmm-loan-payment-acc"));
      a.setValue(acc.loanAmount());
      if (acc.accountType() == Account::Type::Loan)
        a.setValue(-a.value());

      a.setShares(a.value());
      b.setValue(-a.value());
      b.setShares(b.value());
      a.setMemo(i18n("Loan payout"));
      b.setMemo(i18n("Loan payout"));
      t.setPostDate(QDate::fromString(acc.value("kmm-loan-payment-date"), Qt::ISODate));
      newAccount.deletePair("kmm-loan-payment-acc");
      newAccount.deletePair("kmm-loan-payment-date");
      MyMoneyFile::instance()->modifyAccount(newAccount);

      t.addSplit(a);
      t.addSplit(b);
      addTransaction(t);
      createOpeningBalanceTransaction(newAccount, openingBal);

      // in case of an investment account we check if we should create
      // a brokerage account
    } else if (newAccount.accountType() == Account::Type::Investment
               && !brokerageAccount.name().isEmpty()) {
      addAccount(brokerageAccount, parentAccount);

      // set a link from the investment account to the brokerage account
      modifyAccount(newAccount);
      createOpeningBalanceTransaction(brokerageAccount, openingBal);

    } else
      createOpeningBalanceTransaction(newAccount, openingBal);

    ft.commit();
  } catch (const MyMoneyException &e) {
    qWarning("Unable to create account: %s", e.what());
    throw;
  }
}

void MyMoneyFile::addAccount(MyMoneyAccount& account, MyMoneyAccount& parent)
{
  d->checkTransaction(Q_FUNC_INFO);

  MyMoneyInstitution institution;

  // perform some checks to see that the account stuff is OK. For
  // now we assume that the account must have a name, has no
  // transaction and sub-accounts and parent account
  // it's own ID is not set and it does not have a pointer to (MyMoneyFile)

  if (account.name().length() == 0)
    throw MYMONEYEXCEPTION_CSTRING("Account has no name");

  if (account.id().length() != 0)
    throw MYMONEYEXCEPTION_CSTRING("New account must have no id");

  if (account.accountList().count() != 0)
    throw MYMONEYEXCEPTION_CSTRING("New account must have no sub-accounts");

  if (!account.parentAccountId().isEmpty())
    throw MYMONEYEXCEPTION_CSTRING("New account must have no parent-id");

  if (account.accountType() == Account::Type::Unknown)
    throw MYMONEYEXCEPTION_CSTRING("Account has invalid type");

  // make sure, that the parent account exists
  // if not, an exception is thrown. If it exists,
  // get a copy of the current data
  auto acc = MyMoneyFile::account(parent.id());

#if 0
  // TODO: remove the following code as we now can have multiple accounts
  // with the same name even in the same hierarchy position of the account tree
  //
  // check if the selected name is currently not among the child accounts
  // if we find one, then return it as the new account
  QStringList::const_iterator it_a;
  foreach (const auto accountID, acc.accountList()) {
    MyMoneyAccount a = MyMoneyFile::account(accountID);
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
    throw MYMONEYEXCEPTION_CSTRING("Stock account cannot be parent account");

  if (account.isInvest() && parent.accountType() != Account::Type::Investment)
    throw MYMONEYEXCEPTION_CSTRING("Stock account must have investment account as parent ");

  if (!account.isInvest() && parent.accountType() == Account::Type::Investment)
    throw MYMONEYEXCEPTION_CSTRING("Investment account can only have stock accounts as children");

  // if an institution is set, verify that it exists
  if (account.institutionId().length() != 0) {
    // check the presence of the institution. if it
    // does not exist, an exception is thrown
    institution = MyMoneyFile::institution(account.institutionId());
  }

  // if we don't have a valid opening date use today
  if (!account.openingDate().isValid()) {
    account.setOpeningDate(QDate::currentDate());
  }

  // make sure to set the opening date for categories to a
  // fixed date (1900-1-1). See #313793 on b.k.o for details
  if (account.isIncomeExpense()) {
    account.setOpeningDate(QDate(1900, 1, 1));
  }

  // if we don't have a currency assigned use the base currency
  if (account.currencyId().isEmpty()) {
    account.setCurrencyId(baseCurrency().id());
  }

  // make sure the parent id is setup
  account.setParentAccountId(parent.id());

  d->m_storage->addAccount(account);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, account);

  d->m_storage->addAccount(parent, account);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, parent);

  if (account.institutionId().length() != 0) {
    institution.addAccountId(account.id());
    d->m_storage->modifyInstitution(institution);
    d->m_changeSet += MyMoneyNotification(File::Mode::Modify, institution);
  }
}

MyMoneyTransaction MyMoneyFile::createOpeningBalanceTransaction(const MyMoneyAccount& acc, const MyMoneyMoney& balance)
{
  MyMoneyTransaction t;
  // if the opening balance is not zero, we need
  // to create the respective transaction
  if (!balance.isZero()) {
    d->checkTransaction(Q_FUNC_INFO);

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
  } catch (const MyMoneyException &) {
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
    } catch (const MyMoneyException &) {
      // If not, keep searching
      ++it_t;
    }
  }

  return result;
}

MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security)
{
  if (!security.isCurrency())
    throw MYMONEYEXCEPTION_CSTRING("Opening balance for non currencies not supported");

  try {
    return openingBalanceAccount_internal(security);
  } catch (const MyMoneyException &) {
    MyMoneyFileTransaction ft;
    MyMoneyAccount acc;

    try {
      acc = createOpeningBalanceAccount(security);
      ft.commit();

    } catch (const MyMoneyException &) {
      qDebug("Unable to create opening balance account for security %s", qPrintable(security.id()));
    }
    return acc;
  }
}

MyMoneyAccount MyMoneyFile::openingBalanceAccount(const MyMoneySecurity& security) const
{
  return openingBalanceAccount_internal(security);
}

MyMoneyAccount MyMoneyFile::openingBalanceAccount_internal(const MyMoneySecurity& security) const
{
  if (!security.isCurrency())
    throw MYMONEYEXCEPTION_CSTRING("Opening balance for non currencies not supported");

  MyMoneyAccount acc;
  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount>::ConstIterator it;

  accountList(accounts, equity().accountList(), true);

  for (it = accounts.constBegin(); it != accounts.constEnd(); ++it) {
    if (it->value("OpeningBalanceAccount") == QLatin1String("Yes")
        && it->currencyId() == security.id()) {
      acc = *it;
      break;
    }
  }

  if (acc.id().isEmpty()) {
    for (it = accounts.constBegin(); it != accounts.constEnd(); ++it) {
      if (it->name().startsWith(MyMoneyFile::openingBalancesPrefix())
          && it->currencyId() == security.id()) {
        acc = *it;
        break;
      }
    }
  }

  if (acc.id().isEmpty())
    throw MYMONEYEXCEPTION(QString::fromLatin1("No opening balance account for %1").arg(security.tradingSymbol()));

  return acc;
}

MyMoneyAccount MyMoneyFile::createOpeningBalanceAccount(const MyMoneySecurity& security)
{
  d->checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount acc;
  QList<MyMoneyAccount> accounts;
  QList<MyMoneyAccount>::ConstIterator it;

  accountList(accounts, equity().accountList(), true);

  // find present opening balance accounts without containing '('
  QString name;
  QString parentAccountId;
  QRegExp exp(QString("\\([A-Z]{3}\\)"));

  for (it = accounts.constBegin(); it != accounts.constEnd(); ++it) {
    if (it->value("OpeningBalanceAccount") == QLatin1String("Yes")
        && exp.indexIn(it->name()) == -1) {
      name = it->name();
      parentAccountId = it->parentAccountId();
      break;
    }
  }

  if (name.isEmpty())
    name = MyMoneyFile::openingBalancesPrefix();
  if (security.id() != baseCurrency().id()) {
    name += QString(" (%1)").arg(security.id());
  }
  acc.setName(name);
  acc.setAccountType(Account::Type::Equity);
  acc.setCurrencyId(security.id());
  acc.setValue("OpeningBalanceAccount", "Yes");

  MyMoneyAccount parent = !parentAccountId.isEmpty() ? account(parentAccountId) : equity();
  this->addAccount(acc, parent);
  return acc;
}

void MyMoneyFile::addTransaction(MyMoneyTransaction& transaction)
{
  d->checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  MyMoneyNotifier notifier(d);

  // perform some checks to see that the transaction stuff is OK. For
  // now we assume that
  // * no ids are assigned
  // * the date valid (must not be empty)
  // * the referenced accounts in the splits exist

  // first perform all the checks
  if (!transaction.id().isEmpty())
    throw MYMONEYEXCEPTION_CSTRING("Unable to add transaction with id set");
  if (!transaction.postDate().isValid())
    throw MYMONEYEXCEPTION_CSTRING("Unable to add transaction with invalid postdate");

  // now check the splits
  auto loanAccountAffected = false;
  const auto splits1 = transaction.splits();
  for (const auto& split : splits1) {
    // the following line will throw an exception if the
    // account does not exist or is one of the standard accounts
    auto acc = MyMoneyFile::account(split.accountId());
    if (acc.id().isEmpty())
      throw MYMONEYEXCEPTION_CSTRING("Cannot add split with no account assigned");
    if (acc.isLoan())
      loanAccountAffected = true;
    if (isStandardAccount(split.accountId()))
      throw MYMONEYEXCEPTION_CSTRING("Cannot add split referencing standard account");
  }

  // change transfer splits between asset/liability and loan accounts
  // into amortization splits
  if (loanAccountAffected) {
    foreach (const auto split, transaction.splits()) {
      if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer)) {
        auto acc = MyMoneyFile::account(split.accountId());

        if (acc.isAssetLiability()) {
          MyMoneySplit s = split;
          s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
          transaction.modifySplit(s);
        }
      }
    }
  }

  // check that we have a commodity
  if (transaction.commodity().isEmpty()) {
    transaction.setCommodity(baseCurrency().id());
  }

  // make sure the value is rounded to the accounts precision
  fixSplitPrecision(transaction);

  // then add the transaction to the file global pool
  d->m_storage->addTransaction(transaction);

  // scan the splits again to update notification list
  const auto splits2 = transaction.splits();
  for (const auto& split : splits2)
    d->addCacheNotification(split.accountId(), transaction.postDate());

  d->m_changeSet += MyMoneyNotification(File::Mode::Add, transaction);
}

MyMoneyTransaction MyMoneyFile::transaction(const QString& id) const
{
  d->checkStorage();

  return d->m_storage->transaction(id);
}

MyMoneyTransaction MyMoneyFile::transaction(const QString& account, const int idx) const
{
  d->checkStorage();

  return d->m_storage->transaction(account, idx);
}

void MyMoneyFile::addPayee(MyMoneyPayee& payee)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addPayee(payee);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, payee);
}

MyMoneyPayee MyMoneyFile::payee(const QString& id) const
{
  if (Q_UNLIKELY(id.isEmpty()))
    return MyMoneyPayee();

  return d->m_storage->payee(id);
}

MyMoneyPayee MyMoneyFile::payeeByName(const QString& name) const
{
  d->checkStorage();

  return d->m_storage->payeeByName(name);
}

void MyMoneyFile::modifyPayee(const MyMoneyPayee& payee)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->modifyPayee(payee);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, payee);
}

void MyMoneyFile::removePayee(const MyMoneyPayee& payee)
{
  d->checkTransaction(Q_FUNC_INFO);

  // FIXME we need to make sure, that the payee is not referenced anymore
  d->m_storage->removePayee(payee);
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, payee);
}

void MyMoneyFile::addTag(MyMoneyTag& tag)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addTag(tag);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, tag);
}

MyMoneyTag MyMoneyFile::tag(const QString& id) const
{
  return d->m_storage->tag(id);
}

MyMoneyTag MyMoneyFile::tagByName(const QString& name) const
{
  d->checkStorage();

  return d->m_storage->tagByName(name);
}

void MyMoneyFile::modifyTag(const MyMoneyTag& tag)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->modifyTag(tag);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, tag);
}

void MyMoneyFile::removeTag(const MyMoneyTag& tag)
{
  d->checkTransaction(Q_FUNC_INFO);

  // FIXME we need to make sure, that the tag is not referenced anymore
  d->m_storage->removeTag(tag);
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, tag);
}

void MyMoneyFile::accountList(QList<MyMoneyAccount>& list, const QStringList& idlist, const bool recursive) const
{
  d->checkStorage();

  if (idlist.isEmpty()) {
    d->m_storage->accountList(list);

#if 0
    // TODO: I have no idea what this was good for, but it caused the networth report
    //       to show double the numbers so I commented it out (ipwizard, 2008-05-24)
    if (d->m_storage && (list.isEmpty() || list.size() != d->m_storage->accountCount())) {
      d->m_storage->accountList(list);
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
    d->m_storage->accountList(list_a);

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

QList<MyMoneyInstitution> MyMoneyFile::institutionList() const
{
  return d->m_storage->institutionList();
}

// general get functions
MyMoneyPayee MyMoneyFile::user() const
{
  d->checkStorage();
  return d->m_storage->user();
}

// general set functions
void MyMoneyFile::setUser(const MyMoneyPayee& user)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->setUser(user);
}

bool MyMoneyFile::dirty() const
{
  if (!d->m_storage)
    return false;

  return d->m_storage->dirty();
}

void MyMoneyFile::setDirty() const
{
  d->checkStorage();

  d->m_storage->setDirty();
}

unsigned int MyMoneyFile::accountCount() const
{
  d->checkStorage();

  return d->m_storage->accountCount();
}

void MyMoneyFile::ensureDefaultCurrency(MyMoneyAccount& acc) const
{
  if (acc.currencyId().isEmpty()) {
    if (!baseCurrency().id().isEmpty())
      acc.setCurrencyId(baseCurrency().id());
  }
}

MyMoneyAccount MyMoneyFile::liability() const
{
  d->checkStorage();

  return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability));
}

MyMoneyAccount MyMoneyFile::asset() const
{
  d->checkStorage();

  return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset));
}

MyMoneyAccount MyMoneyFile::expense() const
{
  d->checkStorage();

  return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense));
}

MyMoneyAccount MyMoneyFile::income() const
{
  d->checkStorage();

  return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income));
}

MyMoneyAccount MyMoneyFile::equity() const
{
  d->checkStorage();

  return account(MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Equity));
}

unsigned int MyMoneyFile::transactionCount(const QString& account) const
{
  d->checkStorage();

  return d->m_storage->transactionCount(account);
}

unsigned int MyMoneyFile::transactionCount() const
{
  return transactionCount(QString());
}

QMap<QString, unsigned long> MyMoneyFile::transactionCountMap() const
{
  d->checkStorage();

  return d->m_storage->transactionCountMap();
}

unsigned int MyMoneyFile::institutionCount() const
{
  d->checkStorage();

  return d->m_storage->institutionCount();
}

MyMoneyMoney MyMoneyFile::balance(const QString& id, const QDate& date) const
{
  if (date.isValid()) {
    MyMoneyBalanceCacheItem bal = d->m_balanceCache.balance(id, date);
    if (bal.isValid())
      return bal.balance();
  }

  d->checkStorage();

  MyMoneyMoney returnValue = d->m_storage->balance(id, date);

  if (date.isValid()) {
    d->m_balanceCache.insert(id, date, returnValue);
  }

  return returnValue;
}

MyMoneyMoney MyMoneyFile::balance(const QString& id) const
{
  return balance(id, QDate());
}

MyMoneyMoney MyMoneyFile::clearedBalance(const QString &id, const QDate& date) const
{
  MyMoneyMoney cleared;
  QList<MyMoneyTransaction> list;

  cleared = balance(id, date);

  MyMoneyAccount account = this->account(id);
  MyMoneyMoney factor(1, 1);
  if (account.accountGroup() == Account::Type::Liability || account.accountGroup() == Account::Type::Equity)
    factor = -factor;

  MyMoneyTransactionFilter filter;
  filter.addAccount(id);
  filter.setDateFilter(QDate(), date);
  filter.setReportAllSplits(false);
  filter.addState((int)TransactionFilter::State::NotReconciled);
  transactionList(list, filter);

  for (QList<MyMoneyTransaction>::const_iterator it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
    const QList<MyMoneySplit>& splits = (*it_t).splits();
    for (QList<MyMoneySplit>::const_iterator it_s = splits.constBegin(); it_s != splits.constEnd(); ++it_s) {
      const MyMoneySplit &split = (*it_s);
      if (split.accountId() != id)
        continue;
      cleared -= split.shares();
    }
  }
  return cleared * factor;
}

MyMoneyMoney MyMoneyFile::totalBalance(const QString& id, const QDate& date) const
{
  d->checkStorage();

  return d->m_storage->totalBalance(id, date);
}

MyMoneyMoney MyMoneyFile::totalBalance(const QString& id) const
{
  return totalBalance(id, QDate());
}

void MyMoneyFile::warningMissingRate(const QString& fromId, const QString& toId) const
{
  MyMoneySecurity from, to;
  try {
    from = security(fromId);
    to = security(toId);
    qWarning("Missing price info for conversion from %s to %s", qPrintable(from.name()), qPrintable(to.name()));

  } catch (const MyMoneyException &e) {
    qWarning("Missing security caught in MyMoneyFile::warningMissingRate(). %s", e.what());
  }
}

void MyMoneyFile::transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const
{
  d->checkStorage();
  d->m_storage->transactionList(list, filter);
}

void MyMoneyFile::transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const
{
  d->checkStorage();
  d->m_storage->transactionList(list, filter);
}

QList<MyMoneyTransaction> MyMoneyFile::transactionList(MyMoneyTransactionFilter& filter) const
{
  d->checkStorage();
  return d->m_storage->transactionList(filter);
}

QList<MyMoneyPayee> MyMoneyFile::payeeList() const
{
  return d->m_storage->payeeList();
}

QList<MyMoneyTag> MyMoneyFile::tagList() const
{
  return d->m_storage->tagList();
}

QString MyMoneyFile::accountToCategory(const QString& accountId, bool includeStandardAccounts) const
{
  MyMoneyAccount acc;
  QString rc;

  if (!accountId.isEmpty()) {
    acc = account(accountId);
    do {
      if (!rc.isEmpty())
        rc = AccountSeparator + rc;
      rc = acc.name() + rc;
      acc = account(acc.parentAccountId());
    } while (!acc.id().isEmpty() && (includeStandardAccounts || !isStandardAccount(acc.id())));
  }
  return rc;
}

QString MyMoneyFile::categoryToAccount(const QString& category, Account::Type type) const
{
  QString id;

  // search the category in the expense accounts and if it is not found, try
  // to locate it in the income accounts
  if (type == Account::Type::Unknown
      || type == Account::Type::Expense) {
    id = locateSubAccount(MyMoneyFile::instance()->expense(), category);
  }

  if ((id.isEmpty() && type == Account::Type::Unknown)
      || type == Account::Type::Income) {
    id = locateSubAccount(MyMoneyFile::instance()->income(), category);
  }

  return id;
}

QString MyMoneyFile::categoryToAccount(const QString& category) const
{
  return categoryToAccount(category, Account::Type::Unknown);
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
  return name.section(AccountSeparator, 0, -2);
}

QString MyMoneyFile::locateSubAccount(const MyMoneyAccount& base, const QString& category) const
{
  MyMoneyAccount nextBase;
  QString level, remainder;
  level = category.section(AccountSeparator, 0, 0);
  remainder = category.section(AccountSeparator, 1);

  foreach (const auto sAccount, base.accountList()) {
    nextBase = account(sAccount);
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
  d->checkStorage();

  return d->m_storage->value(key);
}

void MyMoneyFile::setValue(const QString& key, const QString& val)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->setValue(key, val);
}

void MyMoneyFile::deletePair(const QString& key)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->deletePair(key);
}

void MyMoneyFile::addSchedule(MyMoneySchedule& sched)
{
  d->checkTransaction(Q_FUNC_INFO);

  const auto splits = sched.transaction().splits();
  for (const auto& split : splits) {
    // the following line will throw an exception if the
    // account does not exist or is one of the standard accounts
    const auto acc = account(split.accountId());
    if (acc.id().isEmpty())
      throw MYMONEYEXCEPTION_CSTRING("Cannot add split with no account assigned");
    if (isStandardAccount(split.accountId()))
      throw MYMONEYEXCEPTION_CSTRING("Cannot add split referencing standard account");
  }

  d->m_storage->addSchedule(sched);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, sched);
}

void MyMoneyFile::modifySchedule(const MyMoneySchedule& sched)
{
  d->checkTransaction(Q_FUNC_INFO);

  foreach (const auto split, sched.transaction().splits()) {
    // the following line will throw an exception if the
    // account does not exist or is one of the standard accounts
    auto acc = MyMoneyFile::account(split.accountId());
    if (acc.id().isEmpty())
      throw MYMONEYEXCEPTION_CSTRING("Cannot store split with no account assigned");
    if (isStandardAccount(split.accountId()))
      throw MYMONEYEXCEPTION_CSTRING("Cannot store split referencing standard account");
  }

  d->m_storage->modifySchedule(sched);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, sched);
}

void MyMoneyFile::removeSchedule(const MyMoneySchedule& sched)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->removeSchedule(sched);
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, sched);
}

MyMoneySchedule MyMoneyFile::schedule(const QString& id) const
{
  return d->m_storage->schedule(id);
}

QList<MyMoneySchedule> MyMoneyFile::scheduleList(
  const QString& accountId,
  const Schedule::Type type,
  const Schedule::Occurrence occurrence,
  const Schedule::PaymentType paymentType,
  const QDate& startDate,
  const QDate& endDate,
  const bool overdue) const
{
  d->checkStorage();

  return d->m_storage->scheduleList(accountId, type, occurrence, paymentType, startDate, endDate, overdue);
}

QList<MyMoneySchedule> MyMoneyFile::scheduleList(
    const QString& accountId) const
{
  return scheduleList(accountId, Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                      QDate(), QDate(), false);
}

QList<MyMoneySchedule> MyMoneyFile::scheduleList() const
{
  return scheduleList(QString(), Schedule::Type::Any, Schedule::Occurrence::Any, Schedule::PaymentType::Any,
                      QDate(), QDate(), false);
}

QStringList MyMoneyFile::consistencyCheck()
{
  QList<MyMoneyAccount> list;
  QList<MyMoneyAccount>::Iterator it_a;
  QList<MyMoneySchedule>::Iterator it_sch;
  QList<MyMoneyPayee>::Iterator it_p;
  QList<MyMoneyTransaction>::Iterator it_t;
  QList<MyMoneyReport>::Iterator it_r;
  QStringList accountRebuild;

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
  d->checkTransaction(Q_FUNC_INFO);

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
      case Account::Type::Asset:
        toplevel = asset();
        break;
      case Account::Type::Liability:
        toplevel = liability();
        break;
      case Account::Type::Expense:
        toplevel = expense();
        break;
      case Account::Type::Income:
        toplevel = income();
        break;
      case Account::Type::Equity:
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

    } catch (const MyMoneyException &) {
      // if we don't know about a parent, we catch it later
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
    } catch (const MyMoneyException &) {
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

      // make sure to rebuild the sub-accounts of the top account
      if (accountRebuild.contains(toplevel.id()) == 0)
        accountRebuild << toplevel.id();
    }

    // now check that all the children exist and have the correct type
    foreach (const auto accountID, (*it_a).accountList()) {
      // check that the child exists
      try {
        child = account(accountID);
        if (child.parentAccountId() != (*it_a).id()) {
          throw MYMONEYEXCEPTION_CSTRING("Child account has a different parent");
        }
      } catch (const MyMoneyException &) {
        problemCount++;
        if (problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << i18n("* Problem with account '%1'", problemAccount);
        }
        rc << i18n("  * Child account with id %1 does not exist anymore.", accountID);
        rc << i18n("    The child account list will be reconstructed.");
        if (accountRebuild.contains((*it_a).id()) == 0)
          accountRebuild << (*it_a).id();
      }
    }

    // see if it is a loan account. if so, remember the assigned interest account
    if ((*it_a).isLoan()) {
      MyMoneyAccountLoan loan(*it_a);
      if (!loan.interestAccountId().isEmpty()) {
        interestAccounts[loan.interestAccountId()] = true;
      }
      try {
        payee(loan.payee());
      } catch (const MyMoneyException &) {
        problemCount++;
        if (problemAccount != (*it_a).name()) {
          problemAccount = (*it_a).name();
          rc << i18n("* Problem with account '%1'", problemAccount);
        }
        rc << i18n("  * The payee with id %1 referenced by the loan does not exist anymore.", loan.payee());
        rc << i18n("    The payee will be removed.");
        // remove the payee - the account will be modified in the engine later
        (*it_a).deletePair("payee");
      }
    }

    // check if it is a category and set the date to 1900-01-01 if different
    if ((*it_a).isIncomeExpense()) {
      if (((*it_a).openingDate().isValid() == false) || ((*it_a).openingDate() != QDate(1900, 1, 1))) {
        (*it_a).setOpeningDate(QDate(1900, 1, 1));
      }
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
    if (!(d->m_storage->account((*it_a).id()) == (*it_a))) {
      try {
        d->m_storage->modifyAccount(*it_a, true);
      } catch (const MyMoneyException &) {
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
      (*it_a).removeAccountIds();
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
        d->m_storage->modifyAccount(*it_a, true);
      } catch (const MyMoneyException &) {
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
      d->m_storage->addPayee(payee);
      payeeConversionMap[(*it_p).id()] = payee.id();
      rc << i18n("  * Payee %1 recreated with fixed id", payee.name());
      ++problemCount;
    }
  }

  // Fix the transactions
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  const auto tList = d->m_storage->transactionList(filter);
  // Generate the list of interest accounts
  for (const auto& transaction : tList) {
    const auto splits = transaction.splits();
    for (const auto& split : splits) {
      if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest))
        interestAccounts[split.accountId()] = true;
    }
  }
  QSet<Account::Type> supportedAccountTypes;
  supportedAccountTypes << Account::Type::Checkings
  << Account::Type::Savings
  << Account::Type::Cash
  << Account::Type::CreditCard
  << Account::Type::Asset
  << Account::Type::Liability;
  QSet<QString> reportedUnsupportedAccounts;

  for (const auto& transaction : tList) {
    MyMoneyTransaction t = transaction;
    bool tChanged = false;
    QDate accountOpeningDate;
    QStringList accountList;
    const auto splits = t.splits();
    foreach (const auto split, splits) {
      bool sChanged = false;
      MyMoneySplit s = split;
      if (payeeConversionMap.find(split.payeeId()) != payeeConversionMap.end()) {
        s.setPayeeId(payeeConversionMap[s.payeeId()]);
        sChanged = true;
        rc << i18n("  * Payee id updated in split of transaction '%1'.", t.id());
        ++problemCount;
      }

      try {
        const auto acc = this->account(s.accountId());
        // compute the newest opening date of all accounts involved in the transaction
        // in case the newest opening date is newer than the transaction post date, do one
        // of the following:
        //
        // a) for category and stock accounts: update the opening date of the account
        // b) for account types where the user cannot modify the opening date through
        //    the UI issue a warning (for each account only once)
        // c) others will be caught later
        if (!acc.isIncomeExpense() && !acc.isInvest()) {
          if (acc.openingDate() > t.postDate()) {
            if (!accountOpeningDate.isValid() || acc.openingDate() > accountOpeningDate) {
              accountOpeningDate = acc.openingDate();
            }
            accountList << this->accountToCategory(acc.id());
            if (!supportedAccountTypes.contains(acc.accountType())
                && !reportedUnsupportedAccounts.contains(acc.id())) {
              rc << i18n("  * Opening date of Account '%1' cannot be changed to support transaction '%2' post date.",
                         this->accountToCategory(acc.id()), t.id());
              reportedUnsupportedAccounts << acc.id();
              ++unfixedCount;
            }
          }
        } else {
          if (acc.openingDate() > t.postDate()) {
            rc << i18n("  * Transaction '%1' post date '%2' is older than opening date '%4' of account '%3'.",
                       t.id(), t.postDate().toString(Qt::ISODate), this->accountToCategory(acc.id()), acc.openingDate().toString(Qt::ISODate));

            rc << i18n("    Account opening date updated.");
            MyMoneyAccount newAcc = acc;
            newAcc.setOpeningDate(t.postDate());
            this->modifyAccount(newAcc);
            ++problemCount;
          }
        }

        // make sure, that shares and value have the same number if they
        // represent the same currency.
        if (t.commodity() == acc.currencyId() && s.shares().reduce() != s.value().reduce()) {
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
      } catch (const MyMoneyException &) {
        rc << i18n("  * Split %2 in transaction '%1' contains a reference to invalid account %3. Please fix manually.", t.id(), split.id(), split.accountId());
        ++unfixedCount;
      }

      // make sure the interest splits are marked correct as such
      if (interestAccounts.find(s.accountId()) != interestAccounts.end()
          && s.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
        s.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
        sChanged = true;
        rc << i18n("  * action marked as interest in split of transaction '%1'.", t.id());
        ++problemCount;
      }

      if (sChanged) {
        tChanged = true;
        t.modifySplit(s);
      }
    }

    // make sure that the transaction's post date is valid
    if (!t.postDate().isValid()) {
      tChanged = true;
      t.setPostDate(t.entryDate().isValid() ? t.entryDate() : QDate::currentDate());
      rc << i18n("  * Transaction '%1' has an invalid post date.", t.id());
      rc << i18n("    The post date was updated to '%1'.", QLocale().toString(t.postDate(), QLocale::ShortFormat));
      ++problemCount;
    }
    // check if the transaction's post date is after the opening date
    // of all accounts involved in the transaction. In case it is not,
    // issue a warning with the details about the transaction incl.
    // the account names and dates involved
    if (accountOpeningDate.isValid() && t.postDate() < accountOpeningDate) {
      QDate originalPostDate = t.postDate();
#if 0
      // for now we do not activate the logic to move the post date to a later
      // point in time. This could cause some severe trouble if you have lots
      // of ancient data collected with older versions of KMyMoney that did not
      // enforce certain conditions like we do now.
      t.setPostDate(accountOpeningDate);
      tChanged = true;
      // copy the price information for investments to the new date
      QList<MyMoneySplit>::const_iterator it_t;
      foreach (const auto split, t.splits()) {
        if ((split.action() != "Buy") &&
            (split.action() != "Reinvest")) {
          continue;
        }
        QString id = split.accountId();
        auto acc = this->account(id);
        MyMoneySecurity sec = this->security(acc.currencyId());
        MyMoneyPrice price(acc.currencyId(),
                           sec.tradingCurrency(),
                           t.postDate(),
                           split.price(), "Transaction");
        this->addPrice(price);
        break;
      }
#endif
      rc << i18n("  * Transaction '%1' has a post date '%2' before one of the referenced account's opening date.", t.id(), QLocale().toString(originalPostDate, QLocale::ShortFormat));
      rc << i18n("    Referenced accounts: %1", accountList.join(","));
      rc << i18n("    The post date was not updated to '%1'.", QLocale().toString(accountOpeningDate, QLocale::ShortFormat));
      ++unfixedCount;
    }

    if (tChanged) {
      d->m_storage->modifyTransaction(t);
    }
  }

  // Fix the schedules
  QList<MyMoneySchedule> schList = scheduleList();
  for (it_sch = schList.begin(); it_sch != schList.end(); ++it_sch) {
    MyMoneySchedule sch = (*it_sch);
    MyMoneyTransaction t = sch.transaction();
    auto tChanged = false;
    foreach (const auto split, t.splits()) {
      MyMoneySplit s = split;
      bool sChanged = false;
      if (payeeConversionMap.find(split.payeeId()) != payeeConversionMap.end()) {
        s.setPayeeId(payeeConversionMap[s.payeeId()]);
        sChanged = true;
        rc << i18n("  * Payee id updated in split of schedule '%1'.", (*it_sch).name());
        ++problemCount;
      }
      if (!split.value().isZero() && split.shares().isZero()) {
        s.setShares(s.value());
        sChanged = true;
        rc << i18n("  * Split in scheduled transaction '%1' contained value != 0 and shares == 0.", (*it_sch).name());
        rc << i18n("    Shares set to value.");
        ++problemCount;
      }

      // make sure, we don't have a bankid stored with a split in a schedule
      if (!split.bankID().isEmpty()) {
        s.setBankID(QString());
        sChanged = true;
        rc << i18n("  * Removed bankid from split in scheduled transaction '%1'.", (*it_sch).name());
        ++problemCount;
      }

      // make sure, that shares and value have the same number if they
      // represent the same currency.
      try {
        const auto acc = this->account(s.accountId());
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
      } catch (const MyMoneyException &) {
        rc << i18n("  * Split %2 in schedule '%1' contains a reference to invalid account %3. Please fix manually.", (*it_sch).name(), split.id(), split.accountId());
        ++unfixedCount;
      }
      if (sChanged) {
        t.modifySplit(s);
        tChanged = true;
      }
    }
    if (tChanged) {
      sch.setTransaction(t);
      d->m_storage->modifySchedule(sch);
    }
  }

  // Fix the reports
  QList<MyMoneyReport> rList = reportList();
  for (it_r = rList.begin(); it_r != rList.end(); ++it_r) {
    MyMoneyReport r = *it_r;
    QStringList payeeList;
    (*it_r).payees(payeeList);
    bool rChanged = false;
    for (auto it_payee = payeeList.begin(); it_payee != payeeList.end(); ++it_payee) {
      if (payeeConversionMap.find(*it_payee) != payeeConversionMap.end()) {
        rc << i18n("  * Payee id updated in report '%1'.", (*it_r).name());
        ++problemCount;
        r.removeReference(*it_payee);
        r.addPayee(payeeConversionMap[*it_payee]);
        rChanged = true;
      }
    }
    if (rChanged) {
      d->m_storage->modifyReport(r);
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
  //all accounts using base currency are excluded, since that's the base used for foreign currency calculation
  //thus it is considered as always present
  //accounts that represent Income/Expense categories are also excluded as price is irrelevant for their
  //fake opening date since a forex rate is required for all multi-currency transactions

  //get all currencies in use
  QStringList currencyList;
  QList<MyMoneyAccount> accountForeignCurrency;
  QList<MyMoneyAccount> accList;
  accountList(accList);
  QList<MyMoneyAccount>::const_iterator account_it;
  for (account_it = accList.constBegin(); account_it != accList.constEnd(); ++account_it) {
    MyMoneyAccount account = *account_it;
    if (!account.isIncomeExpense()
        && !currencyList.contains(account.currencyId())
        && account.currencyId() != baseCurrency().id()
        && !account.currencyId().isEmpty()) {
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
    if ((*accForeignList_it).isInvest()) {
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
        rc << i18n("* Potential problem with securities/currencies");
      }
      QDate openingDate = (*accForeignList_it).openingDate();
      MyMoneySecurity secError = security((*accForeignList_it).currencyId());
      if (!(*accForeignList_it).isInvest()) {
        rc << i18n("  * The account '%1' in currency '%2' has no price set for the opening date '%3'.", (*accForeignList_it).name(), secError.name(), openingDate.toString(Qt::ISODate));
        rc << i18n("    Please enter a price for the currency on or before the opening date.");
      } else {
        rc << i18n("  * The security '%1' has no price set for the opening date '%2'.", (*accForeignList_it).name(), openingDate.toString(Qt::ISODate));
        rc << i18n("    Please enter a price for the security on or before the opening date.");
      }
      ++unfixedCount;
    }
  }

  // Fix the budgets that somehow still reference invalid accounts
  QString problemBudget;
  QList<MyMoneyBudget> bList = budgetList();
  for (QList<MyMoneyBudget>::const_iterator it_b = bList.constBegin(); it_b != bList.constEnd(); ++it_b) {
    MyMoneyBudget b = *it_b;
    QList<MyMoneyBudget::AccountGroup> baccounts = b.getaccounts();
    bool bChanged = false;
    for (QList<MyMoneyBudget::AccountGroup>::const_iterator it_bacc = baccounts.constBegin(); it_bacc != baccounts.constEnd(); ++it_bacc) {
      try {
        account((*it_bacc).id());
      } catch (const MyMoneyException &) {
        problemCount++;
        if (problemBudget != b.name()) {
          problemBudget = b.name();
          rc << i18n("* Problem with budget '%1'", problemBudget);
        }
        rc << i18n("  * The account with id %1 referenced by the budget does not exist anymore.", (*it_bacc).id());
        rc << i18n("    The account reference will be removed.");
        // remove the reference to the account
        b.removeReference((*it_bacc).id());
        bChanged = true;
      }
    }
    if (bChanged) {
      d->m_storage->modifyBudget(b);
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
  d->checkTransaction(Q_FUNC_INFO);

  MyMoneyAccount parent = base;
  QString categoryText;

  if (base.id() != expense().id() && base.id() != income().id())
    throw MYMONEYEXCEPTION_CSTRING("Invalid base category");

  QStringList subAccounts = name.split(AccountSeparator);
  QStringList::Iterator it;
  for (it = subAccounts.begin(); it != subAccounts.end(); ++it) {
    MyMoneyAccount categoryAccount;

    categoryAccount.setName(*it);
    categoryAccount.setAccountType(base.accountType());

    if (it == subAccounts.begin())
      categoryText += *it;
    else
      categoryText += (AccountSeparator + *it);

    // Only create the account if it doesn't exist
    try {
      QString categoryId = categoryToAccount(categoryText);
      if (categoryId.isEmpty())
        addAccount(categoryAccount, parent);
      else {
        categoryAccount = account(categoryId);
      }
    } catch (const MyMoneyException &e) {
      qDebug("Unable to add account %s, %s, %s: %s",
             qPrintable(categoryAccount.name()),
             qPrintable(parent.name()),
             qPrintable(categoryText),
             e.what());
    }

    parent = categoryAccount;
  }

  return categoryToAccount(name);
}

QString MyMoneyFile::checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2)
{
  QString accountId;
  MyMoneyAccount newAccount;
  bool found = true;

  if (!name.isEmpty()) {
    // The category might be constructed with an arbitrary depth (number of
    // colon delimited fields). We try to find a parent account within this
    // hierarchy by searching the following sequence:
    //
    //    aaaa:bbbb:cccc:ddddd
    //
    // 1. search aaaa:bbbb:cccc:dddd, create nothing
    // 2. search aaaa:bbbb:cccc     , create dddd
    // 3. search aaaa:bbbb          , create cccc:dddd
    // 4. search aaaa               , create bbbb:cccc:dddd
    // 5. don't search              , create aaaa:bbbb:cccc:dddd

    newAccount.setName(name);
    QString accName;      // part to be created (right side in above list)
    QString parent(name); // a possible parent part (left side in above list)
    do {
      accountId = categoryToAccount(parent);
      if (accountId.isEmpty()) {
        found = false;
        // prepare next step
        if (!accName.isEmpty())
          accName.prepend(':');
        accName.prepend(parent.section(':', -1));
        newAccount.setName(accName);
        parent = parent.section(':', 0, -2);
      } else if (!accName.isEmpty()) {
        newAccount.setParentAccountId(accountId);
      }
    } while (!parent.isEmpty() && accountId.isEmpty());

    // if we did not find the category,
    // but have a name for it, we create it
    if (!found && !accName.isEmpty()) {
        MyMoneyAccount parentAccount;
      if (newAccount.parentAccountId().isEmpty()) {
        if (!value.isNegative() && value2.isNegative())
          parentAccount = income();
        else
          parentAccount = expense();
      } else {
        parentAccount = account(newAccount.parentAccountId());
      }
      newAccount.setAccountType((!value.isNegative() && value2.isNegative()) ? Account::Type::Income : Account::Type::Expense);
      MyMoneyAccount brokerage;
      // clear out the parent id, because createAccount() does not like that
      newAccount.setParentAccountId(QString());
      createAccount(newAccount, parentAccount, brokerage, MyMoneyMoney());
      accountId = newAccount.id();
    }
  }

  return accountId;
}

void MyMoneyFile::addSecurity(MyMoneySecurity& security)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addSecurity(security);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, security);
}

void MyMoneyFile::modifySecurity(const MyMoneySecurity& security)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->modifySecurity(security);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, security);
}

void MyMoneyFile::removeSecurity(const MyMoneySecurity& security)
{
  d->checkTransaction(Q_FUNC_INFO);

  // FIXME check that security is not referenced by other object

  d->m_storage->removeSecurity(security);
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, security);
}

MyMoneySecurity MyMoneyFile::security(const QString& id) const
{
  if (Q_UNLIKELY(id.isEmpty()))
    return baseCurrency();

  return d->m_storage->security(id);
}

QList<MyMoneySecurity> MyMoneyFile::securityList() const
{
  d->checkStorage();

  return d->m_storage->securityList();
}

void MyMoneyFile::addCurrency(const MyMoneySecurity& currency)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addCurrency(currency);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, currency);
}

void MyMoneyFile::modifyCurrency(const MyMoneySecurity& currency)
{
  d->checkTransaction(Q_FUNC_INFO);

  // force reload of base currency object
  if (currency.id() == d->m_baseCurrency.id())
    d->m_baseCurrency.clearId();

  d->m_storage->modifyCurrency(currency);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, currency);
}

void MyMoneyFile::removeCurrency(const MyMoneySecurity& currency)
{
  d->checkTransaction(Q_FUNC_INFO);

  if (currency.id() == d->m_baseCurrency.id())
    throw MYMONEYEXCEPTION_CSTRING("Cannot delete base currency.");

  // FIXME check that security is not referenced by other object

  d->m_storage->removeCurrency(currency);
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, currency);
}

MyMoneySecurity MyMoneyFile::currency(const QString& id) const
{
  if (id.isEmpty())
    return baseCurrency();

  try {
    const auto currency = d->m_storage->currency(id);
    if (currency.id().isEmpty())
      throw MYMONEYEXCEPTION(QString::fromLatin1("Currency '%1' not found.").arg(id));

    return currency;

  } catch (const MyMoneyException &) {
    const auto security = d->m_storage->security(id);
    if (security.id().isEmpty()) {
      throw MYMONEYEXCEPTION(QString::fromLatin1("Security '%1' not found.").arg(id));
    }
    return security;
  }
}

QMap<MyMoneySecurity, MyMoneyPrice> MyMoneyFile::ancientCurrencies() const
{
  QMap<MyMoneySecurity, MyMoneyPrice> ancientCurrencies;

  ancientCurrencies.insert(MyMoneySecurity("ATS", i18n("Austrian Schilling"), QString::fromUtf8("ÖS")),     MyMoneyPrice("ATS", "EUR", QDate(1998, 12, 31), MyMoneyMoney(10000, 137603), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("DEM", i18n("German Mark"), "DM"),            MyMoneyPrice("ATS", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 195583), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("FRF", i18n("French Franc"), "FF"),           MyMoneyPrice("FRF", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 655957), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("ITL", i18n("Italian Lira"), QChar(0x20A4)),  MyMoneyPrice("ITL", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100, 193627), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("ESP", i18n("Spanish Peseta"), QString()),    MyMoneyPrice("ESP", "EUR", QDate(1998, 12, 31), MyMoneyMoney(1000, 166386), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("NLG", i18n("Dutch Guilder"), QString()),     MyMoneyPrice("NLG", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 220371), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("BEF", i18n("Belgian Franc"), "Fr"),          MyMoneyPrice("BEF", "EUR", QDate(1998, 12, 31), MyMoneyMoney(10000, 403399), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("LUF", i18n("Luxembourg Franc"), "Fr"),       MyMoneyPrice("LUF", "EUR", QDate(1998, 12, 31), MyMoneyMoney(10000, 403399), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("PTE", i18n("Portuguese Escudo"), QString()), MyMoneyPrice("PTE", "EUR", QDate(1998, 12, 31), MyMoneyMoney(1000, 200482), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("IEP", i18n("Irish Pound"), QChar(0x00A3)),   MyMoneyPrice("IEP", "EUR", QDate(1998, 12, 31), MyMoneyMoney(1000000, 787564), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("FIM", i18n("Finnish Markka"), QString()),    MyMoneyPrice("FIM", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100000, 594573), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("GRD", i18n("Greek Drachma"), QChar(0x20AF)), MyMoneyPrice("GRD", "EUR", QDate(1998, 12, 31), MyMoneyMoney(100, 34075), QLatin1Literal("KMyMoney")));

    // https://en.wikipedia.org/wiki/Bulgarian_lev
  ancientCurrencies.insert(MyMoneySecurity("BGL", i18n("Bulgarian Lev"), "BGL"), MyMoneyPrice("BGL", "BGN", QDate(1999, 7, 5), MyMoneyMoney(1, 1000), QLatin1Literal("KMyMoney")));

  ancientCurrencies.insert(MyMoneySecurity("ROL", i18n("Romanian Leu"), "ROL"), MyMoneyPrice("ROL", "RON", QDate(2005, 6, 30), MyMoneyMoney(1, 10000), QLatin1Literal("KMyMoney")));

  ancientCurrencies.insert(MyMoneySecurity("RUR", i18n("Russian Ruble (old)"), "RUR"), MyMoneyPrice("RUR", "RUB", QDate(1998, 1, 1), MyMoneyMoney(1, 1000), QLatin1Literal("KMyMoney")));

  ancientCurrencies.insert(MyMoneySecurity("SIT", i18n("Slovenian Tolar"), "SIT"), MyMoneyPrice("SIT", "EUR", QDate(2006, 12, 31), MyMoneyMoney(1, 23964), QLatin1Literal("KMyMoney")));

    // Source: https://en.wikipedia.org/wiki/Turkish_lira
  ancientCurrencies.insert(MyMoneySecurity("TRL", i18n("Turkish Lira (old)"), "TL"), MyMoneyPrice("TRL", "TRY", QDate(2004, 12, 31), MyMoneyMoney(1, 1000000), QLatin1Literal("KMyMoney")));

    // Source: https://www.focus.de/finanzen/news/malta-und-zypern_aid_66058.html
  ancientCurrencies.insert(MyMoneySecurity("MTL", i18n("Maltese Lira"), "MTL"), MyMoneyPrice("MTL", "EUR", QDate(2008, 1, 1), MyMoneyMoney(429300, 1000000), QLatin1Literal("KMyMoney")));
  ancientCurrencies.insert(MyMoneySecurity("CYP", i18n("Cyprus Pound"), QString("C%1").arg(QChar(0x00A3))), MyMoneyPrice("CYP", "EUR", QDate(2008, 1, 1), MyMoneyMoney(585274, 1000000), QLatin1Literal("KMyMoney")));

    // Source: https://www.focus.de/finanzen/news/waehrungszone-slowakei-ist-neuer-euro-staat_aid_359025.html
  ancientCurrencies.insert(MyMoneySecurity("SKK", i18n("Slovak Koruna"), "SKK"), MyMoneyPrice("SKK", "EUR", QDate(2008, 12, 31), MyMoneyMoney(1000, 30126), QLatin1Literal("KMyMoney")));

    // Source: https://en.wikipedia.org/wiki/Mozambican_metical
  ancientCurrencies.insert(MyMoneySecurity("MZM", i18n("Mozambique Metical"), "MT"), MyMoneyPrice("MZM", "MZN", QDate(2006, 7, 1), MyMoneyMoney(1, 1000), QLatin1Literal("KMyMoney")));

    // Source https://en.wikipedia.org/wiki/Azerbaijani_manat
  ancientCurrencies.insert(MyMoneySecurity("AZM", i18n("Azerbaijani Manat"), "m."), MyMoneyPrice("AZM", "AZN", QDate(2006, 1, 1), MyMoneyMoney(1, 5000), QLatin1Literal("KMyMoney")));

    // Source: https://en.wikipedia.org/wiki/Litas
  ancientCurrencies.insert(MyMoneySecurity("LTL", i18n("Lithuanian Litas"), "Lt"), MyMoneyPrice("LTL", "EUR", QDate(2015, 1, 1), MyMoneyMoney(100000, 345280), QLatin1Literal("KMyMoney")));

    // Source: https://en.wikipedia.org/wiki/Belarusian_ruble
  ancientCurrencies.insert(MyMoneySecurity("BYR", i18n("Belarusian Ruble (old)"), "BYR"), MyMoneyPrice("BYR", "BYN", QDate(2016, 7, 1), MyMoneyMoney(1, 10000), QLatin1Literal("KMyMoney")));

  // Source: https://en.wikipedia.org/wiki/Zambian_kwacha, triggered by b.k.o ticket #425530
  ancientCurrencies.insert(MyMoneySecurity("ZMK", i18n("Zambian Kwacha (old)"), "K"), MyMoneyPrice("ZMK", "ZMW", QDate(2013, 1, 1), MyMoneyMoney(1, 1000), QLatin1Literal("KMyMoney")));

  return ancientCurrencies;
}

QList<MyMoneySecurity> MyMoneyFile::availableCurrencyList() const
{
  QList<MyMoneySecurity> currencyList;
  currencyList.append(MyMoneySecurity("AFA", i18n("Afghanistan Afghani")));
  currencyList.append(MyMoneySecurity("ALL", i18n("Albanian Lek")));
  currencyList.append(MyMoneySecurity("ANG", i18n("Netherland Antillian Guilder")));
  currencyList.append(MyMoneySecurity("DZD", i18n("Algerian Dinar")));
  currencyList.append(MyMoneySecurity("ADF", i18n("Andorran Franc")));
  currencyList.append(MyMoneySecurity("ADP", i18n("Andorran Peseta")));
  currencyList.append(MyMoneySecurity("AOA", i18n("Angolan Kwanza"),         "Kz"));
  currencyList.append(MyMoneySecurity("ARS", i18n("Argentine Peso"),         "$"));
  currencyList.append(MyMoneySecurity("AWG", i18n("Aruban Florin")));
  currencyList.append(MyMoneySecurity("AUD", i18n("Australian Dollar"),      "$"));
  currencyList.append(MyMoneySecurity("AZN", i18n("Azerbaijani Manat"),      "m."));
  currencyList.append(MyMoneySecurity("BSD", i18n("Bahamian Dollar"),        "$"));
  currencyList.append(MyMoneySecurity("BHD", i18n("Bahraini Dinar"),         "BHD", 1000));
  currencyList.append(MyMoneySecurity("BDT", i18n("Bangladeshi Taka")));
  currencyList.append(MyMoneySecurity("BBD", i18n("Barbados Dollar"),        "$"));
  currencyList.append(MyMoneySecurity("BTC", i18n("Bitcoin"),                "BTC", 100000000, 100000000));
  currencyList.append(MyMoneySecurity("BYN", i18n("Belarusian Ruble"),       "Br"));
  currencyList.append(MyMoneySecurity("BZD", i18n("Belize Dollar"),          "$"));
  currencyList.append(MyMoneySecurity("BMD", i18n("Bermudian Dollar"),       "$"));
  currencyList.append(MyMoneySecurity("BTN", i18n("Bhutan Ngultrum")));
  currencyList.append(MyMoneySecurity("BOB", i18n("Bolivian Boliviano")));
  currencyList.append(MyMoneySecurity("BAM", i18n("Bosnian Convertible Mark")));
  currencyList.append(MyMoneySecurity("BWP", i18n("Botswana Pula")));
  currencyList.append(MyMoneySecurity("BRL", i18n("Brazilian Real"),         "R$"));
  currencyList.append(MyMoneySecurity("GBP", i18n("British Pound"),          QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("BND", i18n("Brunei Dollar"),          "$"));
  currencyList.append(MyMoneySecurity("BGN", i18n("Bulgarian Lev (new)")));
  currencyList.append(MyMoneySecurity("BIF", i18n("Burundi Franc")));
  currencyList.append(MyMoneySecurity("XAF", i18n("CFA Franc BEAC")));
  currencyList.append(MyMoneySecurity("XOF", i18n("CFA Franc BCEAO")));
  currencyList.append(MyMoneySecurity("XPF", i18n("CFP Franc Pacifique"), "F", 1, 100));
  currencyList.append(MyMoneySecurity("KHR", i18n("Cambodia Riel")));
  currencyList.append(MyMoneySecurity("CAD", i18n("Canadian Dollar"),        "$"));
  currencyList.append(MyMoneySecurity("CVE", i18n("Cape Verde Escudo")));
  currencyList.append(MyMoneySecurity("KYD", i18n("Cayman Islands Dollar"),  "$"));
  currencyList.append(MyMoneySecurity("CLP", i18n("Chilean Peso")));
  currencyList.append(MyMoneySecurity("CNY", i18n("Chinese Yuan Renminbi")));
  currencyList.append(MyMoneySecurity("COP", i18n("Colombian Peso")));
  currencyList.append(MyMoneySecurity("KMF", i18n("Comoros Franc")));
  currencyList.append(MyMoneySecurity("CRC", i18n("Costa Rican Colon"),      QChar(0x20A1)));
  currencyList.append(MyMoneySecurity("HRK", i18n("Croatian Kuna")));
  currencyList.append(MyMoneySecurity("CUP", i18n("Cuban Peso")));
  currencyList.append(MyMoneySecurity("CUC", i18n("Cuban Convertible Peso")));
  currencyList.append(MyMoneySecurity("CZK", i18n("Czech Koruna")));
  currencyList.append(MyMoneySecurity("DKK", i18n("Danish Krone"),           "kr"));
  currencyList.append(MyMoneySecurity("DJF", i18n("Djibouti Franc")));
  currencyList.append(MyMoneySecurity("DOP", i18n("Dominican Peso")));
  currencyList.append(MyMoneySecurity("XCD", i18n("East Caribbean Dollar"),  "$"));
  currencyList.append(MyMoneySecurity("EGP", i18n("Egyptian Pound"),         QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("SVC", i18n("El Salvador Colon")));
  currencyList.append(MyMoneySecurity("ERN", i18n("Eritrean Nakfa")));
  currencyList.append(MyMoneySecurity("EEK", i18n("Estonian Kroon")));
  currencyList.append(MyMoneySecurity("ETB", i18n("Ethiopian Birr")));
  currencyList.append(MyMoneySecurity("EUR", i18n("Euro"),                   QChar(0x20ac)));
  currencyList.append(MyMoneySecurity("FKP", i18n("Falkland Islands Pound"), QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("FJD", i18n("Fiji Dollar"),            "$"));
  currencyList.append(MyMoneySecurity("GMD", i18n("Gambian Dalasi")));
  currencyList.append(MyMoneySecurity("GEL", i18n("Georgian Lari")));
  currencyList.append(MyMoneySecurity("GHC", i18n("Ghanaian Cedi")));
  currencyList.append(MyMoneySecurity("GIP", i18n("Gibraltar Pound"),        QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("GTQ", i18n("Guatemalan Quetzal")));
  currencyList.append(MyMoneySecurity("GWP", i18n("Guinea-Bissau Peso")));
  currencyList.append(MyMoneySecurity("GYD", i18n("Guyanan Dollar"),         "$"));
  currencyList.append(MyMoneySecurity("HTG", i18n("Haitian Gourde")));
  currencyList.append(MyMoneySecurity("HNL", i18n("Honduran Lempira")));
  currencyList.append(MyMoneySecurity("HKD", i18n("Hong Kong Dollar"),       "$"));
  currencyList.append(MyMoneySecurity("HUF", i18n("Hungarian Forint"),       "HUF", 1, 100));
  currencyList.append(MyMoneySecurity("ISK", i18n("Iceland Krona")));
  currencyList.append(MyMoneySecurity("INR", i18n("Indian Rupee"),           QChar(0x20B9)));
  currencyList.append(MyMoneySecurity("IDR", i18n("Indonesian Rupiah"),      "IDR", 1, 0, 10));
  currencyList.append(MyMoneySecurity("IRR", i18n("Iranian Rial"),           "IRR", 1));
  currencyList.append(MyMoneySecurity("IQD", i18n("Iraqi Dinar"),            "IQD", 1000));
  currencyList.append(MyMoneySecurity("ILS", i18n("Israeli New Shekel"),     QChar(0x20AA)));
  currencyList.append(MyMoneySecurity("JMD", i18n("Jamaican Dollar"),        "$"));
  currencyList.append(MyMoneySecurity("JPY", i18n("Japanese Yen"),           QChar(0x00A5), 1));
  currencyList.append(MyMoneySecurity("JOD", i18n("Jordanian Dinar"),        "JOD", 1000));
  currencyList.append(MyMoneySecurity("KZT", i18n("Kazakhstan Tenge")));
  currencyList.append(MyMoneySecurity("KES", i18n("Kenyan Shilling")));
  currencyList.append(MyMoneySecurity("KWD", i18n("Kuwaiti Dinar"),          "KWD", 1000));
  currencyList.append(MyMoneySecurity("KGS", i18n("Kyrgyzstan Som")));
  currencyList.append(MyMoneySecurity("LAK", i18n("Laos Kip"),               QChar(0x20AD)));
  currencyList.append(MyMoneySecurity("LVL", i18n("Latvian Lats")));
  currencyList.append(MyMoneySecurity("LBP", i18n("Lebanese Pound"),         QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("LSL", i18n("Lesotho Loti")));
  currencyList.append(MyMoneySecurity("LRD", i18n("Liberian Dollar"),        "$"));
  currencyList.append(MyMoneySecurity("LYD", i18n("Libyan Dinar"),           "LYD", 1000));
  currencyList.append(MyMoneySecurity("MOP", i18n("Macau Pataca")));
  currencyList.append(MyMoneySecurity("MKD", i18n("Macedonian Denar")));
  currencyList.append(MyMoneySecurity("MGF", i18n("Malagasy Franc"),         "MGF", 500));
  currencyList.append(MyMoneySecurity("MWK", i18n("Malawi Kwacha")));
  currencyList.append(MyMoneySecurity("MYR", i18n("Malaysian Ringgit")));
  currencyList.append(MyMoneySecurity("MVR", i18n("Maldive Rufiyaa")));
  currencyList.append(MyMoneySecurity("MLF", i18n("Mali Republic Franc")));
  currencyList.append(MyMoneySecurity("MRO", i18n("Mauritanian Ouguiya"),    "MRO", 5));
  currencyList.append(MyMoneySecurity("MUR", i18n("Mauritius Rupee")));
  currencyList.append(MyMoneySecurity("MXN", i18n("Mexican Peso"),           "$"));
  currencyList.append(MyMoneySecurity("MDL", i18n("Moldavian Leu")));
  currencyList.append(MyMoneySecurity("MNT", i18n("Mongolian Tugrik"),       QChar(0x20AE)));
  currencyList.append(MyMoneySecurity("MAD", i18n("Moroccan Dirham")));
  currencyList.append(MyMoneySecurity("MZN", i18n("Mozambique Metical"),     "MT"));
  currencyList.append(MyMoneySecurity("MMK", i18n("Myanmar Kyat")));
  currencyList.append(MyMoneySecurity("NAD", i18n("Namibian Dollar"),        "$"));
  currencyList.append(MyMoneySecurity("NPR", i18n("Nepalese Rupee")));
  currencyList.append(MyMoneySecurity("NZD", i18n("New Zealand Dollar"),     "$"));
  currencyList.append(MyMoneySecurity("NIC", i18n("Nicaraguan Cordoba Oro")));
  currencyList.append(MyMoneySecurity("NGN", i18n("Nigerian Naira"),         QChar(0x20A6)));
  currencyList.append(MyMoneySecurity("KPW", i18n("North Korean Won"),       QChar(0x20A9)));
  currencyList.append(MyMoneySecurity("NOK", i18n("Norwegian Kroner"),       "kr"));
  currencyList.append(MyMoneySecurity("OMR", i18n("Omani Rial"),             "OMR", 1000));
  currencyList.append(MyMoneySecurity("PKR", i18n("Pakistan Rupee")));
  currencyList.append(MyMoneySecurity("PAB", i18n("Panamanian Balboa")));
  currencyList.append(MyMoneySecurity("PGK", i18n("Papua New Guinea Kina")));
  currencyList.append(MyMoneySecurity("PYG", i18n("Paraguay Guarani")));
  currencyList.append(MyMoneySecurity("PEN", i18n("Peruvian Nuevo Sol")));
  currencyList.append(MyMoneySecurity("PHP", i18n("Philippine Peso"),        QChar(0x20B1)));
  currencyList.append(MyMoneySecurity("PLN", i18n("Polish Zloty")));
  currencyList.append(MyMoneySecurity("QAR", i18n("Qatari Rial")));
  currencyList.append(MyMoneySecurity("RON", i18n("Romanian Leu (new)")));
  currencyList.append(MyMoneySecurity("RUB", i18n("Russian Ruble")));
  currencyList.append(MyMoneySecurity("RWF", i18n("Rwanda Franc")));
  currencyList.append(MyMoneySecurity("WST", i18n("Samoan Tala")));
  currencyList.append(MyMoneySecurity("STD", i18n("Sao Tome and Principe Dobra")));
  currencyList.append(MyMoneySecurity("SAR", i18n("Saudi Riyal")));
  currencyList.append(MyMoneySecurity("RSD", i18n("Serbian Dinar")));
  currencyList.append(MyMoneySecurity("SCR", i18n("Seychelles Rupee")));
  currencyList.append(MyMoneySecurity("SLL", i18n("Sierra Leone Leone")));
  currencyList.append(MyMoneySecurity("SGD", i18n("Singapore Dollar"),       "$"));
  currencyList.append(MyMoneySecurity("SBD", i18n("Solomon Islands Dollar"), "$"));
  currencyList.append(MyMoneySecurity("SOS", i18n("Somali Shilling")));
  currencyList.append(MyMoneySecurity("ZAR", i18n("South African Rand")));
  currencyList.append(MyMoneySecurity("KRW", i18n("South Korean Won"),       QChar(0x20A9), 1));
  currencyList.append(MyMoneySecurity("LKR", i18n("Sri Lanka Rupee")));
  currencyList.append(MyMoneySecurity("SHP", i18n("St. Helena Pound"),       QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("SDD", i18n("Sudanese Dinar")));
  currencyList.append(MyMoneySecurity("SRG", i18n("Suriname Guilder")));
  currencyList.append(MyMoneySecurity("SZL", i18n("Swaziland Lilangeni")));
  currencyList.append(MyMoneySecurity("SEK", i18n("Swedish Krona")));
  currencyList.append(MyMoneySecurity("CHF", i18n("Swiss Franc"),            "SFr"));
  currencyList.append(MyMoneySecurity("SYP", i18n("Syrian Pound"),           QChar(0x00A3)));
  currencyList.append(MyMoneySecurity("TWD", i18n("Taiwan Dollar"),          "$"));
  currencyList.append(MyMoneySecurity("TJS", i18n("Tajikistan Somoni")));
  currencyList.append(MyMoneySecurity("TZS", i18n("Tanzanian Shilling")));
  currencyList.append(MyMoneySecurity("THB", i18n("Thai Baht"),              QChar(0x0E3F)));
  currencyList.append(MyMoneySecurity("TOP", i18n("Tongan Pa'anga")));
  currencyList.append(MyMoneySecurity("TTD", i18n("Trinidad and Tobago Dollar"), "$"));
  currencyList.append(MyMoneySecurity("TND", i18n("Tunisian Dinar"),         "TND", 1000));
  currencyList.append(MyMoneySecurity("TRY", i18n("Turkish Lira"), QChar(0x20BA)));
  currencyList.append(MyMoneySecurity("TMM", i18n("Turkmenistan Manat")));
  currencyList.append(MyMoneySecurity("USD", i18n("US Dollar"),              "$"));
  currencyList.append(MyMoneySecurity("UGX", i18n("Uganda Shilling")));
  currencyList.append(MyMoneySecurity("UAH", i18n("Ukraine Hryvnia")));
  currencyList.append(MyMoneySecurity("CLF", i18n("Unidad de Fometo")));
  currencyList.append(MyMoneySecurity("AED", i18n("United Arab Emirates Dirham")));
  currencyList.append(MyMoneySecurity("UYU", i18n("Uruguayan Peso")));
  currencyList.append(MyMoneySecurity("UZS", i18n("Uzbekistani Sum")));
  currencyList.append(MyMoneySecurity("VUV", i18n("Vanuatu Vatu")));
  currencyList.append(MyMoneySecurity("VEB", i18n("Venezuelan Bolivar")));
  currencyList.append(MyMoneySecurity("VND", i18n("Vietnamese Dong"),        QChar(0x20AB)));
  currencyList.append(MyMoneySecurity("ZMW", i18n("Zambian Kwacha"),         "K"));
  currencyList.append(MyMoneySecurity("ZWD", i18n("Zimbabwe Dollar"),        "$"));

  currencyList.append(ancientCurrencies().keys());

  // sort the currencies ...
  qSort(currencyList.begin(), currencyList.end(),
        [] (const MyMoneySecurity& c1, const MyMoneySecurity& c2)
        {
          return c1.name().compare(c2.name()) < 0;
        });

  // ... and add a few precious metals at the ned
  currencyList.append(MyMoneySecurity("XAU", i18n("Gold"),       "XAU", 1000000));
  currencyList.append(MyMoneySecurity("XPD", i18n("Palladium"),  "XPD", 1000000));
  currencyList.append(MyMoneySecurity("XPT", i18n("Platinum"),   "XPT", 1000000));
  currencyList.append(MyMoneySecurity("XAG", i18n("Silver"),     "XAG", 1000000));

  return currencyList;
}

QList<MyMoneySecurity> MyMoneyFile::currencyList() const
{
  d->checkStorage();

  return d->m_storage->currencyList();
}

QString MyMoneyFile::foreignCurrency(const QString& first, const QString& second) const
{
  if (baseCurrency().id() == second)
    return first;
  return second;
}

MyMoneySecurity MyMoneyFile::baseCurrency() const
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

  if (c.id() != d->m_baseCurrency.id()) {
    setValue("kmm-baseCurrency", curr.id());
    // force reload of base currency cache
    d->m_baseCurrency = MyMoneySecurity();
    emit dataChanged();
  }
}

void MyMoneyFile::addPrice(const MyMoneyPrice& price)
{
  if (price.rate(QString()).isZero())
    return;

  d->checkTransaction(Q_FUNC_INFO);

  auto adjustedPrice(price);
  const auto fromSecurity = security(price.from());
  const auto toSecurity = security(price.to());
  // if fromSecurity is a currency and toSecurity is not a currency
  // we swap both of them so that it is a security -> currency
  // conversion
  if (fromSecurity.isCurrency() && !toSecurity.isCurrency()) {
    adjustedPrice = MyMoneyPrice(price.to(),
                                 price.from(),
                                 price.date(),
                                 price.rate(price.from()),
                                 price.source());
  }
  // store the account's which are affected by this price regarding their value
  d->priceChanged(*this, adjustedPrice);
  d->m_storage->addPrice(adjustedPrice);
}

void MyMoneyFile::removePrice(const MyMoneyPrice& price)
{
  d->checkTransaction(Q_FUNC_INFO);

  // store the account's which are affected by this price regarding their value
  d->priceChanged(*this, price);
  d->m_storage->removePrice(price);
}

MyMoneyPrice MyMoneyFile::price(const QString& fromId, const QString& toId, const QDate& date, const bool exactDate) const
{
  d->checkStorage();

  QString to(toId);
  if (to.isEmpty())
    to = value("kmm-baseCurrency");
  // if some id is missing, we can return an empty price object
  if (fromId.isEmpty() || to.isEmpty())
    return MyMoneyPrice();

  // we don't search our tables if someone asks stupid stuff
  if (fromId == toId) {
    return MyMoneyPrice(fromId, toId, date, MyMoneyMoney::ONE, "KMyMoney");
  }

  // if not asking for exact date, try to find the exact date match first,
  // either the requested price or its reciprocal value. If unsuccessful, it will move
  // on and look for prices of previous dates
  MyMoneyPrice rc = d->m_storage->price(fromId, to, date, true);
  if (!rc.isValid()) {
    // not found, search 'to-from' rate and use reciprocal value
    rc = d->m_storage->price(to, fromId, date, true);

    // not found, search previous dates, if exact date is not needed
    if (!exactDate && !rc.isValid()) {
      // search 'from-to' and 'to-from', select the most recent one
      MyMoneyPrice fromPrice = d->m_storage->price(fromId, to, date, exactDate);
      MyMoneyPrice toPrice = d->m_storage->price(to, fromId, date, exactDate);

      // check first whether both prices are valid
      if (fromPrice.isValid() && toPrice.isValid()) {
        if (fromPrice.date() >= toPrice.date()) {
          // if 'from-to' is newer or the same date, prefer that one
          rc = fromPrice;
        } else {
          // otherwise, use the reciprocal price
          rc = toPrice;
        }
      } else if (fromPrice.isValid()) { // check if any of the prices is valid, return that one
        rc = fromPrice;
      } else if (toPrice.isValid()) {
        rc = toPrice;
      }
    }
  }
  return rc;
}

MyMoneyPrice MyMoneyFile::price(const QString& fromId, const QString& toId) const
{
  return price(fromId, toId, QDate::currentDate(), false);
}

MyMoneyPrice MyMoneyFile::price(const QString& fromId) const
{
  return price(fromId, QString(), QDate::currentDate(), false);
}

MyMoneyPriceList MyMoneyFile::priceList() const
{
  d->checkStorage();

  return d->m_storage->priceList();
}

bool MyMoneyFile::hasAccount(const QString& id, const QString& name) const
{
  const auto accounts = account(id).accountList();
  for (const auto& acc : accounts) {
    if (account(acc).name().compare(name) == 0)
      return true;
  }
  return false;
}

QList<MyMoneyReport> MyMoneyFile::reportList() const
{
  d->checkStorage();

  return d->m_storage->reportList();
}

void MyMoneyFile::addReport(MyMoneyReport& report)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addReport(report);
}

void MyMoneyFile::modifyReport(const MyMoneyReport& report)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->modifyReport(report);
}

unsigned MyMoneyFile::countReports() const
{
  d->checkStorage();

  return d->m_storage->countReports();
}

MyMoneyReport MyMoneyFile::report(const QString& id) const
{
  d->checkStorage();

  return d->m_storage->report(id);
}

void MyMoneyFile::removeReport(const MyMoneyReport& report)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->removeReport(report);
}


QList<MyMoneyBudget> MyMoneyFile::budgetList() const
{
  d->checkStorage();

  return d->m_storage->budgetList();
}

void MyMoneyFile::addBudget(MyMoneyBudget &budget)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addBudget(budget);
}

MyMoneyBudget MyMoneyFile::budgetByName(const QString& name) const
{
  d->checkStorage();

  return d->m_storage->budgetByName(name);
}

void MyMoneyFile::modifyBudget(const MyMoneyBudget& budget)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->modifyBudget(budget);
}

unsigned MyMoneyFile::countBudgets() const
{
  d->checkStorage();

  return d->m_storage->countBudgets();
}

MyMoneyBudget MyMoneyFile::budget(const QString& id) const
{
  d->checkStorage();

  return d->m_storage->budget(id);
}

void MyMoneyFile::removeBudget(const MyMoneyBudget& budget)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->removeBudget(budget);
}

void MyMoneyFile::addOnlineJob(onlineJob& job)
{
  d->checkTransaction(Q_FUNC_INFO);

  d->m_storage->addOnlineJob(job);
  d->m_changeSet += MyMoneyNotification(File::Mode::Add, job);
}

void MyMoneyFile::modifyOnlineJob(const onlineJob job)
{
  d->checkTransaction(Q_FUNC_INFO);
  d->m_storage->modifyOnlineJob(job);
  d->m_changeSet += MyMoneyNotification(File::Mode::Modify, job);
}

onlineJob MyMoneyFile::getOnlineJob(const QString &jobId) const
{
  d->checkStorage();
  return d->m_storage->getOnlineJob(jobId);
}

QList<onlineJob> MyMoneyFile::onlineJobList() const
{
  d->checkStorage();
  return d->m_storage->onlineJobList();
}

/** @todo improve speed by passing count job to m_storage */
int MyMoneyFile::countOnlineJobs() const
{
  return onlineJobList().count();
}

/**
 * @brief Remove onlineJob
 * @param job onlineJob to remove
 */
void MyMoneyFile::removeOnlineJob(const onlineJob& job)
{
  d->checkTransaction(Q_FUNC_INFO);

  // clear all changed objects from cache
  if (job.isLocked()) {
    return;
  }
  d->m_changeSet += MyMoneyNotification(File::Mode::Remove, job);
  d->m_storage->removeOnlineJob(job);
}

void MyMoneyFile::removeOnlineJob(const QStringList onlineJobIds)
{
  foreach (QString jobId, onlineJobIds) {
    removeOnlineJob(getOnlineJob(jobId));
  }
}

void MyMoneyFile::costCenterList(QList< MyMoneyCostCenter >& list) const
{
  d->checkStorage();
  list = d->m_storage->costCenterList();
}

void MyMoneyFile::updateVAT(MyMoneyTransaction& transaction) const
{
  // check if transaction qualifies
  const auto splitCount = transaction.splits().count();
  if (splitCount > 1 && splitCount <= 3) {
    MyMoneyMoney amount;
    MyMoneyAccount assetLiability;
    MyMoneyAccount category;
    MyMoneySplit taxSplit;
    const QString currencyId = transaction.commodity();
    foreach (const auto& split, transaction.splits()) {
      const auto acc = account(split.accountId());
      // all splits must reference accounts denoted in the same currency
      if (acc.currencyId() != currencyId) {
        return;
      }
      if (acc.isAssetLiability() && assetLiability.id().isEmpty()) {
        amount = split.shares();
        assetLiability = acc;
        continue;
      }
      if (acc.isAssetLiability()) {
        return;
      }
      if (category.id().isEmpty() && !acc.value("VatAccount").isEmpty()) {
        category = acc;
        continue;
      } else if(taxSplit.id().isEmpty() && !acc.value("Tax").toLower().compare(QLatin1String("yes"))) {
        taxSplit = split;
        continue;
      }
      return;
    }
    if (!category.id().isEmpty()) {
      // remove a possibly found tax split - we create a new one
      // but only if it is the same tax category
      if (!taxSplit.id().isEmpty()) {
        if (category.value("VatAccount").compare(taxSplit.accountId()))
          return;
        transaction.removeSplit(taxSplit);
      }
      addVATSplit(transaction, assetLiability, category, amount);
    }
  }
}

bool MyMoneyFile::addVATSplit(MyMoneyTransaction& transaction, const MyMoneyAccount& acc, const MyMoneyAccount& category, const MyMoneyMoney& amount) const
{
  bool rc = false;

  try {
    MyMoneySplit cat;  // category
    MyMoneySplit tax;  // tax

    if (category.value("VatAccount").isEmpty())
      return false;
    MyMoneyAccount vatAcc = account(category.value("VatAccount"));
    const MyMoneySecurity& asec = security(acc.currencyId());
    const MyMoneySecurity& csec = security(category.currencyId());
    const MyMoneySecurity& vsec = security(vatAcc.currencyId());
    if (asec.id() != csec.id() || asec.id() != vsec.id()) {
      qDebug("Auto VAT assignment only works if all three accounts use the same currency.");
      return false;
    }

    MyMoneyMoney vatRate(vatAcc.value("VatRate"));
    MyMoneyMoney gv, nv;    // gross value, net value
    int fract = acc.fraction();

    if (!vatRate.isZero()) {

      tax.setAccountId(vatAcc.id());

      // qDebug("vat amount is '%s'", category.value("VatAmount").toLatin1());
      if (category.value("VatAmount").toLower() != QString("net")) {
        // split value is the gross value
        gv = amount;
        nv = (gv / (MyMoneyMoney::ONE + vatRate)).convert(fract);
        MyMoneySplit catSplit = transaction.splitByAccount(acc.id(), false);
        catSplit.setShares(-nv);
        catSplit.setValue(catSplit.shares());
        transaction.modifySplit(catSplit);

      } else {
        // split value is the net value
        nv = amount;
        gv = (nv * (MyMoneyMoney::ONE + vatRate)).convert(fract);
        MyMoneySplit accSplit = transaction.splitByAccount(acc.id());
        accSplit.setValue(gv.convert(fract));
        accSplit.setShares(accSplit.value());
        transaction.modifySplit(accSplit);
      }

      tax.setValue(-(gv - nv).convert(fract));
      tax.setShares(tax.value());
      transaction.addSplit(tax);
      rc = true;
    }
  } catch (const MyMoneyException &) {
  }
  return rc;
}

bool MyMoneyFile::isReferenced(const MyMoneyObject& obj, const QBitArray& skipChecks) const
{
  d->checkStorage();
  return d->m_storage->isReferenced(obj, skipChecks);
}

bool MyMoneyFile::isReferenced(const MyMoneyObject& obj) const
{
  return isReferenced(obj, QBitArray((int)eStorage::Reference::Count));
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
    } catch (const MyMoneyException &) {
    }
    ++it_t;
  }
  return false;
}

QString MyMoneyFile::highestCheckNo(const QString& accId) const
{
  unsigned64 lno = 0;
  unsigned64 cno;
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
    } catch (const MyMoneyException &) {
    }
    ++it_t;
  }
  return no;
}

bool MyMoneyFile::hasNewerTransaction(const QString& accId, const QDate& date) const
{
  MyMoneyTransactionFilter filter;
  filter.addAccount(accId);
  filter.setDateFilter(date.addDays(+1), QDate());
  return !transactionList(filter).isEmpty();
}

void MyMoneyFile::clearCache()
{
  d->checkStorage();
  d->m_balanceCache.clear();
}

void MyMoneyFile::forceDataChanged()
{
  emit dataChanged();
}

bool MyMoneyFile::isTransfer(const MyMoneyTransaction& t) const
{
  auto rc = true;
  if (t.splitCount() == 2) {
    foreach (const auto split, t.splits()) {
      auto acc = account(split.accountId());
      if (acc.isIncomeExpense()) {
        rc = false;
        break;
      }
    }
  }
  return rc;
}

bool MyMoneyFile::referencesClosedAccount(const MyMoneyTransaction& t) const
{
  auto ret = false;
  foreach (const auto split, t.splits()) {
    if (referencesClosedAccount(split)) {
      ret = true;
      break;
    }
  }
  return ret;
}

bool MyMoneyFile::referencesClosedAccount(const MyMoneySplit& s) const
{
  if (s.accountId().isEmpty())
    return false;

  try {
    return account(s.accountId()).isClosed();
  } catch (const MyMoneyException &) {
  }
  return false;
}

QString MyMoneyFile::storageId()
{
  QString id = value("kmm-id");
  if (id.isEmpty()) {
    MyMoneyFileTransaction ft;
    try {
      QUuid uid = QUuid::createUuid();
      setValue("kmm-id", uid.toString());
      ft.commit();
      id = uid.toString();
    } catch (const MyMoneyException &) {
      qDebug("Unable to setup UID for new storage object");
    }
  }
  return id;
}

QString MyMoneyFile::openingBalancesPrefix()
{
    return i18n("Opening Balances");
}

bool MyMoneyFile::hasMatchingOnlineBalance(const MyMoneyAccount& _acc) const
{
  // get current values
  auto acc = account(_acc.id());

  // if there's no last transaction import data we are done
  if (acc.value("lastImportedTransactionDate").isEmpty()
      || acc.value("lastStatementBalance").isEmpty())
    return false;

  // otherwise, we compare the balances
  MyMoneyMoney balance(acc.value("lastStatementBalance"));
  MyMoneyMoney accBalance = this->balance(acc.id(), QDate::fromString(acc.value("lastImportedTransactionDate"), Qt::ISODate));

  return balance == accBalance;
}

int MyMoneyFile::countTransactionsWithSpecificReconciliationState(const QString& accId, TransactionFilter::State state) const
{
  MyMoneyTransactionFilter filter;
  filter.addAccount(accId);
  filter.addState((int)state);
  return transactionList(filter).count();
}

QMap<QString, QVector<int> > MyMoneyFile::countTransactionsWithSpecificReconciliationState() const
{
  QMap<QString, QVector<int> > result;
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);

  d->checkStorage();

  QList<MyMoneyAccount> list;
  accountList(list);
  for (const auto& account : list) {
    result[account.id()] = QVector<int>((int)eMyMoney::Split::State::MaxReconcileState, 0);
  }

  const auto transactions = d->m_storage->transactionList(filter);
  for (const auto& transaction : transactions) {
    const auto& splits = transaction.splits();
    for (const auto& split : splits) {
      if (!result.contains(split.accountId())) {
        result[split.accountId()] = QVector<int>((int)eMyMoney::Split::State::MaxReconcileState, 0);
      }
      const auto flag = split.reconcileFlag();
      switch(flag) {
        case eMyMoney::Split::State::NotReconciled:
        case eMyMoney::Split::State::Cleared:
        case eMyMoney::Split::State::Reconciled:
        case eMyMoney::Split::State::Frozen:
          result[split.accountId()][(int)flag]++;
          break;
        default:
          break;
      }

    }
  }
  return result;
}

  /**
   * Make sure that the splits value has the precision of the corresponding account
   */
void MyMoneyFile::fixSplitPrecision(MyMoneyTransaction& t) const
{
  auto transactionSecurity = security(t.commodity());
  auto transactionFraction = transactionSecurity.smallestAccountFraction();

  for (auto& split : t.splits()) {
    auto acc = account(split.accountId());
    auto fraction = acc.fraction();
    if(fraction == -1) {
      auto sec = security(acc.currencyId());
      fraction = acc.fraction(sec);
    }
    // Don't do any rounding on a split factor
    if (split.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) {
      split.setShares(static_cast<const MyMoneyMoney>(split.shares().convertDenominator(fraction).canonicalize()));
      split.setValue(static_cast<const MyMoneyMoney>(split.value().convertDenominator(transactionFraction).canonicalize()));
    }
  }
}

class MyMoneyFileTransactionPrivate
{
  Q_DISABLE_COPY(MyMoneyFileTransactionPrivate)

public:
  MyMoneyFileTransactionPrivate() :
    m_isNested(MyMoneyFile::instance()->hasTransaction()),
    m_needRollback(!m_isNested)
  {
  }

public:
  bool m_isNested;
  bool m_needRollback;

};


MyMoneyFileTransaction::MyMoneyFileTransaction() :
  d_ptr(new MyMoneyFileTransactionPrivate)
{
  Q_D(MyMoneyFileTransaction);
  if (!d->m_isNested)
    MyMoneyFile::instance()->startTransaction();
}

MyMoneyFileTransaction::~MyMoneyFileTransaction()
{
  try {
    rollback();
  } catch (const MyMoneyException &e) {
    qDebug() << e.what();
  }
  Q_D(MyMoneyFileTransaction);
  delete d;
}

void MyMoneyFileTransaction::restart()
{
  rollback();

  Q_D(MyMoneyFileTransaction);
  d->m_needRollback = !d->m_isNested;
  if (!d->m_isNested)
    MyMoneyFile::instance()->startTransaction();
}

void MyMoneyFileTransaction::commit()
{
  Q_D(MyMoneyFileTransaction);
  if (!d->m_isNested)
    MyMoneyFile::instance()->commitTransaction();
  d->m_needRollback = false;
}

void MyMoneyFileTransaction::rollback()
{
  Q_D(MyMoneyFileTransaction);
  if (d->m_needRollback)
    MyMoneyFile::instance()->rollbackTransaction();
  d->m_needRollback = false;
}
