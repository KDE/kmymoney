/***************************************************************************
                          imymoneystorage.h  -  description
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IMYMONEYSTORAGE_H
#define IMYMONEYSTORAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QList>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneytransaction.h"
#include "mymoneypayee.h"
#include "mymoneytag.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include <mymoneybudget.h>
#include <onlinejob.h>
#include <mymoneycostcenter.h>

class MyMoneySplit;
class MyMoneyTransactionFilter;

/**
  * @author Thomas Baumgart
  */

/**
  * The IMyMoneyStorage class describes the interface between the MyMoneyFile class
  * and the real storage manager.
  *
  * @see MyMoneySeqAccessMgr
  */
class IMyMoneyStorage
{
public:

  // definitions for the ID's of the standard accounts
#define STD_ACC_LIABILITY "AStd::Liability"
#define STD_ACC_ASSET     "AStd::Asset"
#define STD_ACC_EXPENSE   "AStd::Expense"
#define STD_ACC_INCOME    "AStd::Income"
#define STD_ACC_EQUITY    "AStd::Equity"

  IMyMoneyStorage();
  virtual ~IMyMoneyStorage();

  // general get functions
  virtual const MyMoneyPayee& user() const = 0;
  virtual const QDate creationDate() const = 0;
  virtual const QDate lastModificationDate() const = 0;
  virtual unsigned int currentFixVersion() const = 0;
  virtual unsigned int fileFixVersion() const = 0;

  // general set functions
  virtual void setUser(const MyMoneyPayee& user) = 0;
  virtual void setFileFixVersion(const unsigned int v) = 0;

  // methods provided by MyMoneyKeyValueContainer
  virtual void setValue(const QString& key, const QString& value) = 0;
  virtual const QString value(const QString& key) const = 0;
  virtual void deletePair(const QString& key) = 0;

  /**
    * This method is used to duplicate an IMyMoneyStorage object and return
    * a pointer to the newly created copy. The caller of this method is the
    * new owner of the object and must destroy it.
    */
  virtual IMyMoneyStorage const * duplicate() = 0;

  /**
    * This method is used to create a new account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount filled with data
    */
  virtual void addAccount(MyMoneyAccount& account) = 0;

  /**
    * This method is used to add one account as sub-ordinate to another
    * (parent) account. The objects that are passed will be modified
    * accordingly.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param parent parent account the account should be added to
    * @param account the account to be added
    */
  virtual void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account) = 0;

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void addPayee(MyMoneyPayee& payee) = 0;

  /**
    * This method is used to retrieve information about a payee
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee payee(const QString& id) const = 0;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee payeeByName(const QString& payee) const = 0;

  /**
    * This method is used to modify an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void modifyPayee(const MyMoneyPayee& payee) = 0;

  /**
    * This method is used to remove an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void removePayee(const MyMoneyPayee& payee) = 0;

  /**
    * This method returns a list of the payees
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyPayee> containing the payee information
    */
  virtual const QList<MyMoneyPayee> payeeList() const = 0;

  /**
    * This method is used to create a new tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  virtual void addTag(MyMoneyTag& tag) = 0;

  /**
    * This method is used to retrieve information about a tag
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of tag
    *
    * @return MyMoneyTag object of tag
    */
  virtual const MyMoneyTag tag(const QString& id) const = 0;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a tag.
    * An exception will be thrown upon error conditions.
    *
    * @param tag QString reference to name of tag
    *
    * @return MyMoneyTag object of tag
    */
  virtual const MyMoneyTag tagByName(const QString& tag) const = 0;

  /**
    * This method is used to modify an existing tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  virtual void modifyTag(const MyMoneyTag& tag) = 0;

  /**
    * This method is used to remove an existing tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  virtual void removeTag(const MyMoneyTag& tag) = 0;

  /**
    * This method returns a list of the tags
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyTag> containing the tag information
    */
  virtual const QList<MyMoneyTag> tagList() const = 0;

  /**
    * Returns the account addressed by it's id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  virtual const MyMoneyAccount account(const QString& id) const = 0;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  virtual bool isStandardAccount(const QString& id) const = 0;

  /**
    * This method is used to set the name for the specified standard account
    * within the storage area. An exception will be thrown, if an error
    * occurs
    *
    * @param id QString reference to one of the standard accounts.
    * @param name QString reference to the name to be set
    *
    */
  virtual void setAccountName(const QString& id, const QString& name) = 0;

  /**
    * Adds an institution to the storage. A
    * respective institution-ID will be generated within this record.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    */
  virtual void addInstitution(MyMoneyInstitution& institution) = 0;

  /**
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated within this record. The ID is stored
    * QString with the object.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to the transaction
    * @param skipAccountUpdate if set, the transaction lists of the accounts
    *        referenced in the splits are not updated. This is used for
    *        bulk loading a lot of transactions but not during normal operation
    */
  virtual void addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate = false) = 0;

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  virtual bool hasActiveSplits(const QString& id) const = 0;

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
  virtual const MyMoneyMoney balance(const QString& id, const QDate& date) const = 0;

  /**
    * This method is used to return the actual balance of an account
    * including it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date
    * @return balance of the account as MyMoneyMoney object
    */
  virtual const MyMoneyMoney totalBalance(const QString& id, const QDate& date) const = 0;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  virtual const MyMoneyInstitution institution(const QString& id) const = 0;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  virtual bool dirty() const = 0;

  /**
    * This method can be used by an external object to force the
    * storage object to be dirty. This is used e.g. when an upload
    * to an external destination failed but the previous storage
    * to a local disk was ok.
    */
  virtual void setDirty() = 0;

  /**
    * This method returns the number of accounts currently known to this storage
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  virtual unsigned int accountCount() const = 0;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyInstitution> containing the
    *         institution information
    */
  virtual const QList<MyMoneyInstitution> institutionList() const = 0;

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    * @param skipCheck allows to skip the builtin consistency checks
    */
  virtual void modifyAccount(const MyMoneyAccount& account, const bool skipCheck = false) = 0;

  /**
    * Modifies an already existing institution in the file global
    * institution pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete new institution information
    */
  virtual void modifyInstitution(const MyMoneyInstitution& institution) = 0;

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  virtual void modifyTransaction(const MyMoneyTransaction& transaction) = 0;

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    */
  virtual void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent) = 0;

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction const reference to transaction to be deleted
    */
  virtual void removeTransaction(const MyMoneyTransaction& transaction) = 0;

  /**
    * This method returns the number of transactions currently known to file
    * in the range 0..MAXUINT
    *
    * @param account QString reference to account id. If account is empty
    +                all transactions (the journal) will be counted. If account
    *                is not empty it returns the number of transactions
    *                that have splits in this account.
    *
    * @return number of transactions in journal/account
    */
  virtual unsigned int transactionCount(const QString& account = QString()) const = 0;

  /**
    * This method returns a QMap filled with the number of transactions
    * per account. The account id serves as index into the map. If one
    * needs to have all transactionCounts() for many accounts, this method
    * is faster than calling transactionCount(const QString& account) many
    * times.
    *
    * @return QMap with numbers of transactions per account
    */
  virtual const QMap<QString, unsigned long> transactionCountMap() const = 0;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns all those transactions
    * that match the filter passed as argument. If the filter is empty,
    * the whole journal will be returned.
    * The list returned is sorted according to the transactions posting date.
    * If more than one transaction exists for the same date, the order among
    * them is undefined.
    *
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QList<MyMoneyTransaction>
    */
  virtual const QList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const = 0;

  virtual void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const = 0;

  virtual void transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const = 0;

  /**
    * Deletes an existing account from the file global account pool
    * This method only allows to remove accounts that are not
    * referenced by any split. Use moveSplits() to move splits
    * to another account. An exception is thrown in case of a
    * problem.
    *
    * @param account reference to the account to be deleted.
    */
  virtual void removeAccount(const MyMoneyAccount& account) = 0;

  /**
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution institution to be deleted.
    */
  virtual void removeInstitution(const MyMoneyInstitution& institution) = 0;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return reference to the requested transaction
    */
  virtual const MyMoneyTransaction transaction(const QString& id) const = 0;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return reference to MyMoneyTransaction object
    */
  virtual const MyMoneyTransaction transaction(const QString& account, const int idx) const = 0;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  virtual unsigned int institutionCount() const = 0;

  /**
    * This method returns a list of accounts inside the storage object.
    *
    * @param list reference to QList receiving the account objects
    *
    * @note The standard accounts will not be returned
    */
  virtual void accountList(QList<MyMoneyAccount>& list) const = 0;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  virtual const MyMoneyAccount liability() const = 0;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  virtual const MyMoneyAccount asset() const = 0;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  virtual const MyMoneyAccount expense() const = 0;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  virtual const MyMoneyAccount income() const = 0;

  /**
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  virtual const MyMoneyAccount equity() const = 0;

  /**
    * This method is used to create a new security object.  The ID will be created
    * automatically. The object passed with the parameter @p security is modified
    * to contain the assigned id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param security MyMoneySecurity filled with data
    */
  virtual void addSecurity(MyMoneySecurity& security) = 0;

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object to be updated
    */
  virtual void modifySecurity(const MyMoneySecurity& security) = 0;

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object to be removed
    */
  virtual void removeSecurity(const MyMoneySecurity& security) = 0;

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneySecurity object
    */
  virtual const MyMoneySecurity security(const QString& id) const = 0;

  /**
    * This method returns a list of the security objects
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneySecurity> containing objects
    */
  virtual const QList<MyMoneySecurity> securityList() const = 0;

  virtual void addPrice(const MyMoneyPrice& price) = 0;
  virtual void removePrice(const MyMoneyPrice& price) = 0;
  virtual MyMoneyPrice price(const QString& fromId, const QString& toId, const QDate& date, const bool exactDate) const = 0;

  /**
    * This method returns a list of all prices.
    *
    * @return MyMoneyPriceList of all MyMoneyPrice objects.
    */
  virtual const MyMoneyPriceList priceList() const = 0;

  /**
    * This method is used to add a scheduled transaction to the engine.
    * It must be sure, that the id of the object is not filled. When the
    * method returns to the caller, the id will be filled with the
    * newly created object id value.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched reference to the MyMoneySchedule object
    */
  virtual void addSchedule(MyMoneySchedule& sched) = 0;

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  virtual void modifySchedule(const MyMoneySchedule& sched) = 0;

  /**
    * This method is used to remove an existing MyMoneySchedule object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  virtual void removeSchedule(const MyMoneySchedule& sched) = 0;

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  virtual const MyMoneySchedule schedule(const QString& id) const = 0;

  /**
    * This method is used to extract a list of scheduled transactions
    * according to the filter criteria passed as arguments.
    *
    * @param accountId only search for scheduled transactions that reference
    *                  accound @p accountId. If accountId is the empty string,
    *                  this filter is off. Default is @p QString().
    * @param type      only schedules of type @p type are searched for.
    *                  See eMyMoney::Schedule::Type for details.
    *                  Default is eMyMoney::Schedule::Type::Any
    * @param occurrence only schedules of occurrence type @p occurrence are searched for.
    *                  See eMyMoney::Schedule::Occurence for details.
    *                  Default is eMyMoney::Schedule::Occurrence::Any
    * @param paymentType only schedules of payment method @p paymentType
    *                  are searched for.
    *                  See eMyMoney::Schedule::PaymentType for details.
    *                  Default is eMyMoney::Schedule::PaymentType::Any
    * @param startDate only schedules with payment dates after @p startDate
    *                  are searched for. Default is all dates (QDate()).
    * @param endDate   only schedules with payment dates ending prior to @p endDate
    *                  are searched for. Default is all dates (QDate()).
    * @param overdue   if true, only those schedules that are overdue are
    *                  searched for. Default is false (all schedules will be returned).
    *
    * @return const QList<MyMoneySchedule> list of schedule objects.
    */
  virtual const QList<MyMoneySchedule> scheduleList(const QString& accountId = QString(),
      const eMyMoney::Schedule::Type type = eMyMoney::Schedule::Type::Any,
      const eMyMoney::Schedule::Occurrence occurrence = eMyMoney::Schedule::Occurrence::Any,
      const eMyMoney::Schedule::PaymentType paymentType = eMyMoney::Schedule::PaymentType::Any,
      const QDate& startDate = QDate(),
      const QDate& endDate = QDate(),
      const bool overdue = false) const = 0;

  virtual const QList<MyMoneySchedule> scheduleListEx(int scheduleTypes,
      int scheduleOcurrences,
      int schedulePaymentTypes,
      QDate startDate,
      const QStringList& accounts = QStringList()) const = 0;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  virtual void addCurrency(const MyMoneySecurity& currency) = 0;

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  virtual void modifyCurrency(const MyMoneySecurity& currency) = 0;

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  virtual void removeCurrency(const MyMoneySecurity& currency) = 0;

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneyCurrency object
    */
  virtual const MyMoneySecurity currency(const QString& id) const = 0;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneySecurity objects representing a currency.
    */
  virtual const QList<MyMoneySecurity> currencyList() const = 0;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyReport objects.
    */
  virtual const QList<MyMoneyReport> reportList() const = 0;

  /**
    * This method is used to add a new report to the engine.
    * It must be sure, that the id of the object is not filled. When the
    * method returns to the caller, the id will be filled with the
    * newly created object id value.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report reference to the MyMoneyReport object
    */
  virtual void addReport(MyMoneyReport& report) = 0;

  /**
    * This method is used to modify an existing MyMoneyReport
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  virtual void modifyReport(const MyMoneyReport& report) = 0;

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  virtual unsigned countReports() const = 0;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  virtual const MyMoneyReport report(const QString& id) const = 0;

  /**
    * This method is used to remove an existing MyMoneyReport object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  virtual void removeReport(const MyMoneyReport& report) = 0;

  /**
    * This method is used to retrieve the list of all budgets
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyBudget objects.
    */
  virtual const QList<MyMoneyBudget> budgetList() const = 0;

  /**
    * This method is used to add a new budget to the engine.
    * It must be sure, that the id of the object is not filled. When the
    * method returns to the caller, the id will be filled with the
    * newly created object id value.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget reference to the MyMoneyBudget object
    */
  virtual void addBudget(MyMoneyBudget& budget) = 0;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget object of budget
    */
  virtual const MyMoneyBudget budgetByName(const QString& budget) const = 0;

  /**
    * This method is used to modify an existing MyMoneyBudget
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  virtual void modifyBudget(const MyMoneyBudget& budget) = 0;

  /**
    * This method returns the number of budgets currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of budgets known to file
    */
  virtual unsigned countBudgets() const = 0;

  /**
    * This method is used to retrieve a single MyMoneyBudget object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyBudget object
    * @return MyMoneyBudget object
    */
  virtual MyMoneyBudget budget(const QString& id) const = 0;

  /**
    * This method is used to remove an existing MyMoneyBudget object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  virtual void removeBudget(const MyMoneyBudget& budget) = 0;


  /**
    * This method is used to add a new onlineJob. The id will be
    * overwritten.
    *
    * An exception will be thrown upon erronous situations.
    *
    * @param job The onlineJob, caller remains owner of it. Id might be updated.
    */
  virtual void addOnlineJob(onlineJob& job) = 0;

  /**
   * @brief Saves a onlineJob
   * @param job
   */
  virtual void modifyOnlineJob(const onlineJob& job) = 0;

  /**
   * @brief Get onlineJob by id
   *
   * @return onlineJob you are not the owner nor allowed to delete it.
   * @throw MyMoneyException if jobId was not found
   */
  virtual const onlineJob getOnlineJob(const QString& jobId) const = 0;

  /**
   * @brief Return all onlineJobs
   */
  virtual const QList<onlineJob> onlineJobList() const = 0;

  /**
   * @brief Remove an onlineJobs
   */
  virtual void removeOnlineJob(const onlineJob&) = 0;

  /**
   * @brief Returns a cost center by id
   */
  virtual const MyMoneyCostCenter costCenter(const QString& id) const = 0;

  /**
   * @brief Retruns a list of all costcenters
   */
  virtual const QList<MyMoneyCostCenter> costCenterList() const = 0;

  /**
    * This method checks, if the given @p object is referenced
    * by another engine object.
    *
    * @param obj const reference to object to be checked
    * @param skipCheck QBitArray with eStorage::Reference bits set for which
    *                  the check should be skipped
    *
    * @retval false @p object is not referenced
    * @retval true @p institution is referenced
    */
  virtual bool isReferenced(const MyMoneyObject& obj, const QBitArray& skipCheck) const = 0;

  /**
    * This method is provided to allow closing of the database before logoff
    */
  virtual void close() = 0;

  /**
    * These methods have to be provided to allow transaction safe data handling.
    */
  virtual void startTransaction() = 0;
  virtual bool commitTransaction() = 0;
  virtual void rollbackTransaction() = 0;
};

#endif
