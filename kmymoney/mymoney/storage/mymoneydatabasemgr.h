/***************************************************************************
                          mymoneydatabasemgr.h  -  description
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

#ifndef MYMONEYDATABASEMGR_H
#define MYMONEYDATABASEMGR_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneyserialize.h"
#include "imymoneystorage.h"
#include "mymoneykeyvaluecontainer.h"

class MyMoneyStorageSql;

/**
  * The MyMoneyDatabaseMgr class represents the storage engine for databases.
  * The actual connection and internal storage is handled through the
  * MyMoneyStorageSql interface.
  *
  * The MyMoneyDatabaseMgr must have a MyMoneyStorageSql connected to a
  * database to be useful. Once connected, data will be loaded from/sent to the
  * database synchronously. The method dirty() will always return false. Making
  * this many trips to the database is not very fast, so when possible, the
  * data cache in MyMoneyFile is used.
  *
  */
class MyMoneyDatabaseMgrPrivate;
class MyMoneyDatabaseMgr : public IMyMoneyStorage, public IMyMoneySerialize,
    public MyMoneyKeyValueContainer
{
  Q_DISABLE_COPY(MyMoneyDatabaseMgr)
  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneyDatabaseMgr();
  ~MyMoneyDatabaseMgr();

  // general get functions
  MyMoneyPayee user() const override;
  QDate creationDate() const override;
  QDate lastModificationDate() const override;
  uint currentFixVersion() const override;
  uint fileFixVersion() const override;

  // general set functions
  void setUser(const MyMoneyPayee& user) override;
  void setFileFixVersion(uint v) override;

  // methods provided by MyMoneyKeyValueContainer
  void setValue(const QString& key, const QString& value) override;
  QString value(const QString& key) const override;
  void deletePair(const QString& key) override;

  /**
    * This method is used to duplicate an IMyMoneyStorage object and return
    * a pointer to the newly created copy. The caller of this method is the
    * new owner of the object and must destroy it.
    */
//  MyMoneyDatabaseMgr const * duplicate() override;

  /**
    * This method is used to create a new account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount filled with data
    */
  void addAccount(MyMoneyAccount& account) override;

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
  void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account) override;

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void addPayee(MyMoneyPayee& payee) override;

  /**
    * This method is used to retrieve information about a payee
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of payee
    *
    * @return MyMoneyPayee object of payee
    */
  MyMoneyPayee payee(const QString& id) const override;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  MyMoneyPayee payeeByName(const QString& payee) const override;

  /**
    * This method is used to modify an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void modifyPayee(const MyMoneyPayee& payee) override;

  /**
    * This method is used to remove an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void removePayee(const MyMoneyPayee& payee) override;

  /**
    * This method returns a list of the payees
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyPayee> containing the payee information
    */
  QList<MyMoneyPayee> payeeList() const override;

  /**
    * This method is used to create a new tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  void addTag(MyMoneyTag& tag) override;

  /**
    * This method is used to retrieve information about a tag
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of tag
    *
    * @return MyMoneyTag object of tag
    */
  MyMoneyTag tag(const QString& id) const override;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a tag.
    * An exception will be thrown upon error conditions.
    *
    * @param tag QString reference to name of tag
    *
    * @return MyMoneyTag object of tag
    */
  MyMoneyTag tagByName(const QString& tag) const override;

  /**
    * This method is used to modify an existing tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  void modifyTag(const MyMoneyTag& tag) override;

  /**
    * This method is used to remove an existing tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  void removeTag(const MyMoneyTag& tag) override;

  /**
    * This method returns a list of the tags
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyTag> containing the tag information
    */
  QList<MyMoneyTag> tagList() const override;

  /** @todo implement all onlineJob related functions @{ */
  void modifyOnlineJob(const onlineJob& job) override;
  void addOnlineJob(onlineJob& job) override;
  onlineJob getOnlineJob(const QString &jobId) const override;
  QList<onlineJob> onlineJobList() const override;
  void removeOnlineJob(const onlineJob&) override;
  /** @} */

  /**
    * Returns the account addressed by it's id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  MyMoneyAccount account(const QString& id) const override;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  bool isStandardAccount(const QString& id) const override;

  /**
    * This method is used to set the name for the specified standard account
    * within the storage area. An exception will be thrown, if an error
    * occurs
    *
    * @param id QString reference to one of the standard accounts.
    * @param name QString reference to the name to be set
    *
    */
  void setAccountName(const QString& id, const QString& name) override;

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
  void addInstitution(MyMoneyInstitution& institution) override;

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
  void addTransaction(MyMoneyTransaction& transaction, bool skipAccountUpdate = false) override;

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  bool hasActiveSplits(const QString& id) const override;

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
  MyMoneyMoney balance(const QString& id, const QDate& date) const override;

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
  MyMoneyMoney totalBalance(const QString& id, const QDate& date) const override;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  MyMoneyInstitution institution(const QString& id) const override;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not (for a database, always false).
    */
  bool dirty() const override;

  /**
    * This method can be used by an external object to force the
    * storage object to be dirty. This is used e.g. when an upload
    * to an external destination failed but the previous storage
    * to a local disk was ok.
    *
    * Since the database is synchronized with the application, this method
    * is a no-op.
    */
  void setDirty() override;

  /**
    * This method returns the number of accounts currently known to this storage
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  uint accountCount() const override;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyInstitution> containing the
    *         institution information
    */
  QList<MyMoneyInstitution> institutionList() const override;

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    * @param skipCheck allows to skip the builtin consistency checks
    */
  void modifyAccount(const MyMoneyAccount& account, bool skipCheck = false) override;

  /**
    * Modifies an already existing institution in the file global
    * institution pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete new institution information
    */
  void modifyInstitution(const MyMoneyInstitution& institution) override;

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  void modifyTransaction(const MyMoneyTransaction& transaction) override;

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    */
  void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent) override;

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction const reference to transaction to be deleted
    */
  void removeTransaction(const MyMoneyTransaction& transaction) override;

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
  uint transactionCount(const QString& account) const override;

  /**
    * This method returns a QMap filled with the number of transactions
    * per account. The account id serves as index into the map. If one
    * needs to have all transactionCounts() for many accounts, this method
    * is faster than calling transactionCount(const QString& account) many
    * times.
    *
    * @return QMap with numbers of transactions per account
    */
  QMap<QString, ulong> transactionCountMap() const override;

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
  QList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const override;

  /**
    * This method is the same as above, but instead of a return value, a
    * parameter is used.
    *
    * @param list The set of transactions returned. The list passed in will
    *             be cleared before filling with results.
    * @param filter MyMoneyTransactionFilter object with the match criteria
    */
  void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const override;

  /**
    * This method is the same as above, but the list contains pairs of
    * transactions and splits.
    *
    * @param list The set of transactions returned. The list passed in will
    *             be cleared before filling with results.
    * @param filter MyMoneyTransactionFilter object with the match criteria
    */
  void transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const override;

  /**
    * Deletes an existing account from the file global account pool
    * This method only allows to remove accounts that are not
    * referenced by any split. Use moveSplits() to move splits
    * to another account. An exception is thrown in case of a
    * problem.
    *
    * @param account reference to the account to be deleted.
    */
  void removeAccount(const MyMoneyAccount& account) override;

  /**
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution institution to be deleted.
    */
  void removeInstitution(const MyMoneyInstitution& institution) override;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return the requested transaction
    */
  MyMoneyTransaction transaction(const QString& id) const override;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return MyMoneyTransaction object
    */
  MyMoneyTransaction transaction(const QString& account, const int idx) const override;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  uint institutionCount() const override;

  /**
    * This method returns a list of accounts inside the storage object.
    *
    * @param list reference to QList receiving the account objects
    *
    * @note The standard accounts will not be returned
    */
  void accountList(QList<MyMoneyAccount>& list) const override;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  MyMoneyAccount liability() const override;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  MyMoneyAccount asset() const override;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  MyMoneyAccount expense() const override;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  MyMoneyAccount income() const override;

  /**
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  MyMoneyAccount equity() const override;

  /**
    * This method is used to create a new security object.  The ID will be
    * created automatically. The object passed with the parameter @p security
    * is modified to contain the assigned id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param security MyMoneySecurity filled with data
    */
  void addSecurity(MyMoneySecurity& security) override;

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object to be updated
    */
  void modifySecurity(const MyMoneySecurity& security) override;

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object to be removed
    */
  void removeSecurity(const MyMoneySecurity& security) override;

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneySecurity object
    */
  MyMoneySecurity security(const QString& id) const override;

  /**
    * This method returns a list of the security objects
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneySecurity> containing objects
    */
  QList<MyMoneySecurity> securityList() const override;

  void addPrice(const MyMoneyPrice& price) override;
  void removePrice(const MyMoneyPrice& price) override;
  MyMoneyPrice price(const QString& fromId, const QString& toId, const QDate& _date, bool exactDate) const override;

  /**
    * This method returns a list of all prices.
    *
    * @return MyMoneyPriceList of all MyMoneyPrice objects.
    */
  MyMoneyPriceList priceList() const override;

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
  void addSchedule(MyMoneySchedule& sched) override;

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  void modifySchedule(const MyMoneySchedule& sched) override;

  /**
    * This method is used to remove an existing MyMoneySchedule object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  void removeSchedule(const MyMoneySchedule& sched) override;

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  MyMoneySchedule schedule(const QString& id) const override;

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
    *                  See eMyMoney::Schedule::Occurrence for details.
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
    * @return QList<MyMoneySchedule> list of schedule objects.
    */
  QList<MyMoneySchedule> scheduleList(const QString& accountId,
      eMyMoney::Schedule::Type type,
      eMyMoney::Schedule::Occurrence occurrence,
      eMyMoney::Schedule::PaymentType paymentType,
      const QDate& startDate,
      const QDate& endDate,
      bool overdue) const override;

  QList<MyMoneySchedule> scheduleListEx(int scheduleTypes,
      int scheduleOcurrences,
      int schedulePaymentTypes,
      QDate startDate,
      const QStringList& accounts) const override;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void addCurrency(const MyMoneySecurity& currency) override;

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void modifyCurrency(const MyMoneySecurity& currency) override;

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void removeCurrency(const MyMoneySecurity& currency) override;

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneyCurrency object
    */
  MyMoneySecurity currency(const QString& id) const override;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneySecurity objects representing a currency.
    */
  QList<MyMoneySecurity> currencyList() const override;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyReport objects.
    */
  QList<MyMoneyReport> reportList() const override;

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
  void addReport(MyMoneyReport& report) override;

  /**
    * This method is used to modify an existing MyMoneyReport
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  void modifyReport(const MyMoneyReport& report) override;

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  uint countReports() const override;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  MyMoneyReport report(const QString& id) const override;

  /**
    * This method is used to remove an existing MyMoneyReport object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  void removeReport(const MyMoneyReport& report) override;

  /**
    * This method is used to retrieve the list of all budgets
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyBudget objects.
    */
  QList<MyMoneyBudget> budgetList() const override;

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
  void addBudget(MyMoneyBudget& budget) override;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget object of budget
    */
  MyMoneyBudget budgetByName(const QString& budget) const override;

  /**
    * This method is used to modify an existing MyMoneyBudget
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  void modifyBudget(const MyMoneyBudget& budget) override;

  /**
    * This method returns the number of budgets currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of budgets known to file
    */
  uint countBudgets() const override;

  /**
    * This method is used to retrieve a single MyMoneyBudget object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyBudget object
    * @return MyMoneyBudget object
    */
  MyMoneyBudget budget(const QString& id) const override;

  /**
    * This method is used to remove an existing MyMoneyBudget object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  void removeBudget(const MyMoneyBudget& budget) override;

  /**
   * This method returns a list of all cost center objects
   */
  QList<MyMoneyCostCenter> costCenterList() const override;

  /**
   * @brief Return cost center object by id
   */
  MyMoneyCostCenter costCenter(const QString& id) const override;

  /**
    * Clear all internal caches (used internally for performance measurements)
    */
  void clearCache();

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
  bool isReferenced(const MyMoneyObject& obj, const QBitArray& skipCheck) const override;

  /**
    * This method is provided to allow closing of the database before logoff
    */
  void close() override;

  /**
    * These methods have to be provided to allow transaction safe data handling.
    */
  void startTransaction() override;
  bool commitTransaction() override;
  void rollbackTransaction() override;

  // general set functions
  void setCreationDate(const QDate& val) override;

  /**
   * This method is used to get a SQL reader for subsequent database access
   */
  QExplicitlySharedDataPointer <MyMoneyStorageSql> connectToDatabase
  (const QUrl &url) override;
  /**
    * This method is used when a database file is open, and the data is to
    * be saved in a different file or format. It will ensure that all data
    * from the database is available in memory to enable it to be written.
    */
  void fillStorage() override;

  /**
    * This method is used to set the last modification date of
    * the storage object. It also clears the dirty flag and should
    * therefor be called as last operation when loading from a
    * file.
    *
    * @param val QDate of last modification
    */
  void setLastModificationDate(const QDate& val) override;

  /**
   * This method returns whether a given transaction is already in memory, to avoid
   * reloading it from the database
   */
  bool isDuplicateTransaction(const QString&) const override;

  void loadAccounts(const QMap<QString, MyMoneyAccount>& map) override;
  void loadTransactions(const QMap<QString, MyMoneyTransaction>& map) override;
  void loadInstitutions(const QMap<QString, MyMoneyInstitution>& map) override;
  void loadPayees(const QMap<QString, MyMoneyPayee>& map) override;
  void loadTags(const QMap<QString, MyMoneyTag>& map) override;
  void loadSchedules(const QMap<QString, MyMoneySchedule>& map) override;
  void loadSecurities(const QMap<QString, MyMoneySecurity>& map) override;
  void loadCurrencies(const QMap<QString, MyMoneySecurity>& map) override;
  void loadReports(const QMap<QString, MyMoneyReport>& reports) override;
  void loadBudgets(const QMap<QString, MyMoneyBudget>& budgets) override;
  void loadPrices(const MyMoneyPriceList& list) override;
  void loadOnlineJobs(const QMap<QString, onlineJob>& onlineJobs) override;
  void loadCostCenters(const QMap<QString, MyMoneyCostCenter>& costCenters) override;

  //void loadPayeeIdentifier(const QMap<QString, payeeIdentifier>& idents);

  ulong accountId() const override;
  ulong transactionId() const override;
  ulong payeeId() const override;
  ulong tagId() const override;
  ulong institutionId() const override;
  ulong scheduleId() const override;
  ulong securityId() const override;
  ulong reportId() const override;
  ulong budgetId() const override;
  ulong onlineJobId() const override;
  ulong payeeIdentifierId() const;
  ulong costCenterId() const override;

  void loadAccountId(ulong id) override;
  void loadTransactionId(ulong id) override;
  void loadPayeeId(ulong id) override;
  void loadTagId(ulong id) override;
  void loadInstitutionId(ulong id) override;
  void loadScheduleId(ulong id) override;
  void loadSecurityId(ulong id) override;
  void loadReportId(ulong id) override;
  void loadBudgetId(ulong id) override;
  void loadOnlineJobId(ulong id) override;
  void loadPayeeIdentifierId(ulong id);
  void loadCostCenterId(ulong id) override;

  /**
    * This method is used to retrieve the whole set of key/value pairs
    * from the container. It is meant to be used for permanent storage
    * functionality. See MyMoneyKeyValueContainer::pairs() for details.
    *
    * @return QMap<QString, QString> containing all key/value pairs of
    *         this container.
    */
  QMap<QString, QString> pairs() const override;

  /**
    * This method is used to initially store a set of key/value pairs
    * in the container. It is meant to be used for loading functionality
    * from permanent storage. See MyMoneyKeyValueContainer::setPairs()
    * for details
    *
    * @param list const QMap<QString, QString> containing the set of
    *             key/value pairs to be loaded into the container.
    *
    * @note All existing key/value pairs in the container will be deleted.
    */
  void setPairs(const QMap<QString, QString>& list) override;

  /**
    * This method recalculates the balances of all accounts
    * based on the transactions stored in the engine.
    */
  void rebuildAccountBalances() override;

private:
  MyMoneyDatabaseMgrPrivate* const d_ptr;
  Q_DECLARE_PRIVATE_D(MyMoneyDatabaseMgr::d_ptr, MyMoneyDatabaseMgr)

  /**
    * This method is used to get the next valid ID for a institution
    * @return id for a institution
    */
  QString nextInstitutionID() override;

  /**
    * This method is used to get the next valid ID for an account
    * @return id for an account
    */
  QString nextAccountID() override;

  /**
    * This method is used to get the next valid ID for a transaction
    * @return id for a transaction
    */
  QString nextTransactionID() override;

  /**
    * This method is used to get the next valid ID for a payee
    * @return id for a payee
    */
  QString nextPayeeID() override;

  /**
    * This method is used to get the next valid ID for a tag
    * @return id for a tag
    */
  QString nextTagID() override;

  /**
    * This method is used to get the next valid ID for a scheduled transaction
    * @return id for a scheduled transaction
    */
  QString nextScheduleID() override;

  /**
    * This method is used to get the next valid ID for an security object.
    * @return id for an security object
    */
  QString nextSecurityID() override;

  QString nextReportID() override;

  /** @brief get next valid id for an onlineJob */
  QString nextOnlineJobID() override;

  /** @brief get next valid id for payeeIdentifier */
  QString nextPayeeIdentifierID();

  /** @brief get next valid id for a cost center */
  QString nextCostCenterID() override;

  /**
    * This method is used to get the next valid ID for a budget object.
    * @return id for an budget object
    */
  QString nextBudgetID() override;

};
#endif
