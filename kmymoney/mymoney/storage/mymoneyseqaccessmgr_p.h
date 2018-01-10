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

#ifndef MYMONEYSEQACCESSMGR_P_H
#define MYMONEYSEQACCESSMGR_P_H

#include "mymoneyseqaccessmgr.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QBitArray>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneystoragesql.h"
#include "storageenums.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneytag.h"
#include "mymoneypayee.h"
#include "mymoneybudget.h"
#include "mymoneyschedule.h"
#include "mymoneyreport.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneycostcenter.h"
#include "mymoneymap.h"
#include "mymoneyenums.h"

using namespace eStorage;

class MyMoneySeqAccessMgrPrivate
{
  Q_DISABLE_COPY(MyMoneySeqAccessMgrPrivate)
  Q_DECLARE_PUBLIC(MyMoneySeqAccessMgr)

public:
  explicit MyMoneySeqAccessMgrPrivate(MyMoneySeqAccessMgr* qq) :
    q_ptr(qq),
    m_nextInstitutionID(0),
    m_nextAccountID(0),
    m_nextTransactionID(0),
    m_nextPayeeID(0),
    m_nextTagID(0),
    m_nextScheduleID(0),
    m_nextSecurityID(0),
    m_nextReportID(0),
    m_nextBudgetID(0),
    m_nextOnlineJobID(0),
    m_nextCostCenterID(0),
    m_dirty(false),
    m_creationDate(QDate::currentDate()),
    // initialize for file fixes (see kmymoneyview.cpp)
    m_currentFixVersion(4),
    m_fileFixVersion(0), // default value if no fix-version in file
    m_transactionListFull(false)
  {
  }

  ~MyMoneySeqAccessMgrPrivate()
  {
  }

  /**
    * This method is used to set the dirty flag and update the
    * date of the last modification.
    */
  void touch()
  {
    m_dirty = true;
    m_lastModificationDate = QDate::currentDate();
  }

  /**
    * Adjust the balance for account @a acc by the amount of shares in split @a split.
    * The amount is added if @a reverse is @c false, subtracted in case it is @c true.
    */
  void adjustBalance(MyMoneyAccount& acc, const MyMoneySplit& split, bool reverse)
  {
    // in case of an investment we can't just add or subtract the
    // amount of the split since we don't know about stock splits.
    // so in the case of those stocks, we simply recalculate the balance from scratch
    acc.isInvest() ? acc.setBalance(calculateBalance(acc.id(), QDate())) :
                     acc.adjustBalance(split, reverse);
  }

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    * @param sendNotification if true, notifications with the ids
    *                of all modified objects are send
    */
  void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent, bool /* sendNotification */)
  {
    Q_Q(MyMoneySeqAccessMgr);
    QMap<QString, MyMoneyAccount>::ConstIterator oldParent;
    QMap<QString, MyMoneyAccount>::ConstIterator newParent;
    QMap<QString, MyMoneyAccount>::ConstIterator childAccount;

    // verify that accounts exist. If one does not,
    // an exception is thrown
    q->account(account.id());
    q->account(parent.id());
    if (!account.parentAccountId().isEmpty()) {
      q->account(account.parentAccountId());
      oldParent = m_accountList.find(account.parentAccountId());
    }

    if (account.accountType() == eMyMoney::Account::Type::Stock && parent.accountType() != eMyMoney::Account::Type::Investment)
      throw MYMONEYEXCEPTION("Cannot move a stock acocunt into a non-investment account");

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
    if (account.accountType() != eMyMoney::Account::Type::Stock && account.accountType() != eMyMoney::Account::Type::Investment)
      (*childAccount).setAccountType((*newParent).accountType());
  #endif
  }

  /**
    * This method is used to calculate the actual balance of an account
    * without it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date
    * @return balance of the account as MyMoneyMoney object
    */
  MyMoneyMoney calculateBalance(const QString& id, const QDate& date) const
  {
    Q_Q(const MyMoneySeqAccessMgr);
    MyMoneyMoney balance;
    QList<MyMoneyTransaction> list;

    MyMoneyTransactionFilter filter;
    filter.setDateFilter(QDate(), date);
    filter.setReportAllSplits(false);
    q->transactionList(list, filter);

    for (const auto& transaction : list) {
      const auto splits = transaction.splits();
      for (const auto& split : splits) {
        if (split.accountId().compare(id) != 0)
          continue;
        else if (split.action().compare(MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares)) == 0)
          balance *= split.shares();
        else
          balance += split.shares();
      }
    }

    return balance;
  }

  void removeReferences(const QString& id)
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

  MyMoneySeqAccessMgr *q_ptr;
  /**
    * This member variable keeps the User information.
    * @see setUser()
    */
  MyMoneyPayee m_user;

  /**
    * The member variable m_nextInstitutionID keeps the number that will be
    * assigned to the next institution created. It is maintained by
    * nextInstitutionID().
    */
  ulong m_nextInstitutionID;

  /**
    * The member variable m_nextAccountID keeps the number that will be
    * assigned to the next institution created. It is maintained by
    * nextAccountID().
    */
  ulong m_nextAccountID;

  /**
    * The member variable m_nextTransactionID keeps the number that will be
    * assigned to the next transaction created. It is maintained by
    * nextTransactionID().
    */
  ulong m_nextTransactionID;

  /**
    * The member variable m_nextPayeeID keeps the number that will be
    * assigned to the next payee created. It is maintained by
    * nextPayeeID()
    */
  ulong m_nextPayeeID;

  /**
    * The member variable m_nextTagID keeps the number that will be
    * assigned to the next tag created. It is maintained by
    * nextTagID()
    */
  ulong m_nextTagID;

  /**
    * The member variable m_nextScheduleID keeps the number that will be
    * assigned to the next schedule created. It is maintained by
    * nextScheduleID()
    */
  ulong m_nextScheduleID;

  /**
    * The member variable m_nextSecurityID keeps the number that will be
    * assigned to the next security object created.  It is maintained by
    * nextSecurityID()
    */
  ulong m_nextSecurityID;

  ulong m_nextReportID;

  /**
    * The member variable m_nextBudgetID keeps the number that will be
    * assigned to the next budget object created.  It is maintained by
    * nextBudgetID()
    */
  ulong m_nextBudgetID;

  /**
    * This member variable keeps the number that will be assigned to the
    * next onlineJob object created. It is maintained by nextOnlineJobID()
    */
  ulong m_nextOnlineJobID;

  /**
    * This member variable keeps the number that will be assigned to the
    * next cost center object created. It is maintained by nextCostCenterID()
    */
  ulong m_nextCostCenterID;

  /**
    * The member variable m_institutionList is the container for the
    * institutions known within this file.
    */
  MyMoneyMap<QString, MyMoneyInstitution> m_institutionList;

  /**
    * The member variable m_accountList is the container for the accounts
    * known within this file.
    */
  MyMoneyMap<QString, MyMoneyAccount> m_accountList;

  /**
    * The member variable m_transactionList is the container for all
    * transactions within this file.
    * @see m_transactionKeys
    */
  MyMoneyMap<QString, MyMoneyTransaction> m_transactionList;

  /**
    * The member variable m_transactionKeys is used to convert
    * transaction id's into the corresponding key used in m_transactionList.
    * @see m_transactionList;
    */
  MyMoneyMap<QString, QString> m_transactionKeys;

  /**
    * A list containing all the payees that have been used
    */
  MyMoneyMap<QString, MyMoneyPayee> m_payeeList;

  /**
    * A list containing all the tags that have been used
    */
  MyMoneyMap<QString, MyMoneyTag> m_tagList;

  /**
    * A list containing all the scheduled transactions
    */
  MyMoneyMap<QString, MyMoneySchedule> m_scheduleList;

  /**
    * A list containing all the security information objects.  Each object
    * can represent a stock, bond, or mutual fund.  It contains a price
    * history that a user can add entries to.  The price history will be used
    * to determine the cost basis for sales, as well as the source of
    * information for reports in a security account.
    */
  MyMoneyMap<QString, MyMoneySecurity> m_securitiesList;

  /**
    * A list containing all the currency information objects.
    */
  MyMoneyMap<QString, MyMoneySecurity> m_currencyList;

  MyMoneyMap<QString, MyMoneyReport> m_reportList;

  /**
    * A list containing all the budget information objects.
    */
  MyMoneyMap<QString, MyMoneyBudget> m_budgetList;

  MyMoneyMap<MyMoneySecurityPair, MyMoneyPriceEntries> m_priceList;

  /**
    * A list containing all the onlineJob information objects.
    */
  MyMoneyMap<QString, onlineJob> m_onlineJobList;

  /**
   * A list containing all the cost center information objects
   */
  MyMoneyMap<QString, MyMoneyCostCenter> m_costCenterList;

  /**
    * This member signals if the file has been modified or not
    */
  bool  m_dirty;

  /**
    * This member variable keeps the creation date of this MyMoneySeqAccessMgr
    * object. It is set during the constructor and can only be modified using
    * the stream read operator.
    */
  QDate m_creationDate;

  /**
    * This member variable keeps the date of the last modification of
    * the MyMoneySeqAccessMgr object.
    */
  QDate m_lastModificationDate;

  /**
    * This member variable contains the current fix level of application
    * data files. (see kmymoneyview.cpp)
    */
  uint m_currentFixVersion;
  /**
   * This member variable contains the current fix level of the
   *  presently open data file. (see kmymoneyview.cpp)
   */
  uint m_fileFixVersion;

  /**
    * This member variable is set when all transactions have been read from the database.
    * This is would be probably the case when doing, for e.g., a full report,
    * or after some types of transaction search which cannot be easily implemented in SQL
    */
  bool m_transactionListFull;
};
#endif
