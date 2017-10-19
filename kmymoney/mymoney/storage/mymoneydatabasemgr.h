/***************************************************************************
                          mymoneydatabasemgr.h  -  description
                             -------------------
    begin                : June 5 2007
    copyright            : (C) 2007 by Fernando Vilas
    email                : Fernando Vilas <fvilas@iname.com>
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
#include "mymoneystoragesql.h"

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
class MyMoneyDatabaseMgr : public IMyMoneyStorage, public IMyMoneySerialize,
    public MyMoneyKeyValueContainer
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneyDatabaseMgr();
  ~MyMoneyDatabaseMgr();

  // general get functions
  virtual const MyMoneyPayee& user() const;
  virtual const QDate creationDate() const;
  virtual const QDate lastModificationDate() const;
  virtual unsigned int currentFixVersion() const;
  virtual unsigned int fileFixVersion() const;

  // general set functions
  virtual void setUser(const MyMoneyPayee& user);
  virtual void setFileFixVersion(const unsigned int v);

  // methods provided by MyMoneyKeyValueContainer
  virtual void setValue(const QString& key, const QString& value);
  virtual const QString value(const QString& key) const;
  virtual void deletePair(const QString& key);

  /**
    * This method is used to duplicate an IMyMoneyStorage object and return
    * a pointer to the newly created copy. The caller of this method is the
    * new owner of the object and must destroy it.
    */
  virtual MyMoneyDatabaseMgr const * duplicate();

  /**
    * This method is used to create a new account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount filled with data
    */
  virtual void addAccount(MyMoneyAccount& account);

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
  virtual void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account);

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void addPayee(MyMoneyPayee& payee);

  /**
    * This method is used to retrieve information about a payee
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee payee(const QString& id) const;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee object of payee
    */
  virtual const MyMoneyPayee payeeByName(const QString& payee) const;

  /**
    * This method is used to modify an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void modifyPayee(const MyMoneyPayee& payee);

  /**
    * This method is used to remove an existing payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  virtual void removePayee(const MyMoneyPayee& payee);

  /**
    * This method returns a list of the payees
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyPayee> containing the payee information
    */
  virtual const QList<MyMoneyPayee> payeeList() const;

  /**
    * This method is used to create a new tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  virtual void addTag(MyMoneyTag& tag);

  /**
    * This method is used to retrieve information about a tag
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of tag
    *
    * @return MyMoneyTag object of tag
    */
  virtual const MyMoneyTag tag(const QString& id) const;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a tag.
    * An exception will be thrown upon error conditions.
    *
    * @param tag QString reference to name of tag
    *
    * @return MyMoneyTag object of tag
    */
  virtual const MyMoneyTag tagByName(const QString& tag) const;

  /**
    * This method is used to modify an existing tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  virtual void modifyTag(const MyMoneyTag& tag);

  /**
    * This method is used to remove an existing tag
    *
    * An exception will be thrown upon error conditions
    *
    * @param tag MyMoneyTag reference to tag information
    */
  virtual void removeTag(const MyMoneyTag& tag);

  /**
    * This method returns a list of the tags
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyTag> containing the tag information
    */
  virtual const QList<MyMoneyTag> tagList() const;

  /** @todo implement all onlineJob related functions @{ */
  void modifyOnlineJob(const onlineJob& job);
  void addOnlineJob(onlineJob& job);
  const onlineJob getOnlineJob(const QString &jobId) const;
  const QList<onlineJob> onlineJobList() const;
  void removeOnlineJob(const onlineJob&);
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
  virtual const MyMoneyAccount account(const QString& id) const;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  virtual bool isStandardAccount(const QString& id) const;

  /**
    * This method is used to set the name for the specified standard account
    * within the storage area. An exception will be thrown, if an error
    * occurs
    *
    * @param id QString reference to one of the standard accounts.
    * @param name QString reference to the name to be set
    *
    */
  virtual void setAccountName(const QString& id, const QString& name);

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
  virtual void addInstitution(MyMoneyInstitution& institution);

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
  virtual void addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate = false);

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  virtual bool hasActiveSplits(const QString& id) const;

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
  virtual const MyMoneyMoney balance(const QString& id, const QDate& date) const;

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
  virtual const MyMoneyMoney totalBalance(const QString& id, const QDate& date) const;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  virtual const MyMoneyInstitution institution(const QString& id) const;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not (for a database, always false).
    */
  virtual bool dirty() const;

  /**
    * This method can be used by an external object to force the
    * storage object to be dirty. This is used e.g. when an upload
    * to an external destination failed but the previous storage
    * to a local disk was ok.
    *
    * Since the database is synchronized with the application, this method
    * is a no-op.
    */
  virtual void setDirty();

  /**
    * This method returns the number of accounts currently known to this storage
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  virtual unsigned int accountCount() const;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyInstitution> containing the
    *         institution information
    */
  virtual const QList<MyMoneyInstitution> institutionList() const;

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    * @param skipCheck allows to skip the builtin consistency checks
    */
  virtual void modifyAccount(const MyMoneyAccount& account, const bool skipCheck = false);

  /**
    * Modifies an already existing institution in the file global
    * institution pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete new institution information
    */
  virtual void modifyInstitution(const MyMoneyInstitution& institution);

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  virtual void modifyTransaction(const MyMoneyTransaction& transaction);

  /**
    * This method re-parents an existing account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount reference to account to be re-parented
    * @param parent  MyMoneyAccount reference to new parent account
    */
  virtual void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent);

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction const reference to transaction to be deleted
    */
  virtual void removeTransaction(const MyMoneyTransaction& transaction);

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
  virtual unsigned int transactionCount(const QString& account = QString()) const;

  /**
    * This method returns a QMap filled with the number of transactions
    * per account. The account id serves as index into the map. If one
    * needs to have all transactionCounts() for many accounts, this method
    * is faster than calling transactionCount(const QString& account) many
    * times.
    *
    * @return QMap with numbers of transactions per account
    */
  virtual const QMap<QString, unsigned long> transactionCountMap() const;

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
  virtual const QList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const;

  /**
    * This method is the same as above, but instead of a return value, a
    * parameter is used.
    *
    * @param list The set of transactions returned. The list passed in will
    *             be cleared before filling with results.
    * @param filter MyMoneyTransactionFilter object with the match criteria
    */
  virtual void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const;

  /**
    * This method is the same as above, but the list contains pairs of
    * transactions and splits.
    *
    * @param list The set of transactions returned. The list passed in will
    *             be cleared before filling with results.
    * @param filter MyMoneyTransactionFilter object with the match criteria
    */
  virtual void transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const;

  /**
    * Deletes an existing account from the file global account pool
    * This method only allows to remove accounts that are not
    * referenced by any split. Use moveSplits() to move splits
    * to another account. An exception is thrown in case of a
    * problem.
    *
    * @param account reference to the account to be deleted.
    */
  virtual void removeAccount(const MyMoneyAccount& account);

  /**
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution institution to be deleted.
    */
  virtual void removeInstitution(const MyMoneyInstitution& institution);

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return the requested transaction
    */
  virtual const MyMoneyTransaction transaction(const QString& id) const;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return MyMoneyTransaction object
    */
  virtual const MyMoneyTransaction transaction(const QString& account, const int idx) const;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  virtual unsigned int institutionCount() const;

  /**
    * This method returns a list of accounts inside the storage object.
    *
    * @param list reference to QList receiving the account objects
    *
    * @note The standard accounts will not be returned
    */
  virtual void accountList(QList<MyMoneyAccount>& list) const;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  virtual const MyMoneyAccount liability() const;

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  virtual const MyMoneyAccount asset() const;

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  virtual const MyMoneyAccount expense() const;

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  virtual const MyMoneyAccount income() const;

  /**
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  virtual const MyMoneyAccount equity() const;

  /**
    * This method is used to create a new security object.  The ID will be
    * created automatically. The object passed with the parameter @p security
    * is modified to contain the assigned id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param security MyMoneySecurity filled with data
    */
  virtual void addSecurity(MyMoneySecurity& security);

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object to be updated
    */
  virtual void modifySecurity(const MyMoneySecurity& security);

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object to be removed
    */
  virtual void removeSecurity(const MyMoneySecurity& security);

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneySecurity object
    */
  virtual const MyMoneySecurity security(const QString& id) const;

  /**
    * This method returns a list of the security objects
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneySecurity> containing objects
    */
  virtual const QList<MyMoneySecurity> securityList() const;

  virtual void addPrice(const MyMoneyPrice& price);
  virtual void removePrice(const MyMoneyPrice& price);
  virtual MyMoneyPrice price(const QString& fromId, const QString& toId, const QDate& _date, const bool exactDate) const;

  /**
    * This method returns a list of all prices.
    *
    * @return MyMoneyPriceList of all MyMoneyPrice objects.
    */
  virtual const MyMoneyPriceList priceList() const;

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
  virtual void addSchedule(MyMoneySchedule& sched);

  /**
    * This method is used to modify an existing MyMoneySchedule
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  virtual void modifySchedule(const MyMoneySchedule& sched);

  /**
    * This method is used to remove an existing MyMoneySchedule object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param sched const reference to the MyMoneySchedule object to be updated
    */
  virtual void removeSchedule(const MyMoneySchedule& sched);

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  virtual const MyMoneySchedule schedule(const QString& id) const;

  /**
    * This method is used to extract a list of scheduled transactions
    * according to the filter criteria passed as arguments.
    *
    * @param accountId only search for scheduled transactions that reference
    *                  accound @p accountId. If accountId is the empty string,
    *                  this filter is off. Default is @p QString().
    * @param type      only schedules of type @p type are searched for.
    *                  See MyMoneySchedule::typeE for details.
    *                  Default is MyMoneySchedule::TYPE_ANY
    * @param occurrence only schedules of occurrence type @p occurrence are searched for.
    *                  See MyMoneySchedule::occurrenceE for details.
    *                  Default is MyMoneySchedule::OCCUR_ANY
    * @param paymentType only schedules of payment method @p paymentType
    *                  are searched for.
    *                  See MyMoneySchedule::paymentTypeE for details.
    *                  Default is MyMoneySchedule::STYPE_ANY
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
      const MyMoneySchedule::typeE type = MyMoneySchedule::TYPE_ANY,
      const MyMoneySchedule::occurrenceE occurrence = MyMoneySchedule::OCCUR_ANY,
      const MyMoneySchedule::paymentTypeE paymentType = MyMoneySchedule::STYPE_ANY,
      const QDate& startDate = QDate(),
      const QDate& endDate = QDate(),
      const bool overdue = false) const;

  virtual const QList<MyMoneySchedule> scheduleListEx(int scheduleTypes,
      int scheduleOcurrences,
      int schedulePaymentTypes,
      QDate startDate,
      const QStringList& accounts = QStringList()) const;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  virtual void addCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  virtual void modifyCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  virtual void removeCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneyCurrency object
    */
  virtual const MyMoneySecurity currency(const QString& id) const;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneySecurity objects representing a currency.
    */
  virtual const QList<MyMoneySecurity> currencyList() const;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyReport objects.
    */
  virtual const QList<MyMoneyReport> reportList() const;

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
  virtual void addReport(MyMoneyReport& report);

  /**
    * This method is used to modify an existing MyMoneyReport
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  virtual void modifyReport(const MyMoneyReport& report);

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  virtual unsigned countReports() const;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  virtual const MyMoneyReport report(const QString& id) const;

  /**
    * This method is used to remove an existing MyMoneyReport object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param report const reference to the MyMoneyReport object to be updated
    */
  virtual void removeReport(const MyMoneyReport& report);

  /**
    * This method is used to retrieve the list of all budgets
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyBudget objects.
    */
  virtual const QList<MyMoneyBudget> budgetList() const;

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
  virtual void addBudget(MyMoneyBudget& budget);

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget object of budget
    */
  virtual const MyMoneyBudget budgetByName(const QString& budget) const;

  /**
    * This method is used to modify an existing MyMoneyBudget
    * object. Therefor, the id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  virtual void modifyBudget(const MyMoneyBudget& budget);

  /**
    * This method returns the number of budgets currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of budgets known to file
    */
  virtual unsigned countBudgets() const;

  /**
    * This method is used to retrieve a single MyMoneyBudget object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyBudget object
    * @return MyMoneyBudget object
    */
  virtual MyMoneyBudget budget(const QString& id) const;

  /**
    * This method is used to remove an existing MyMoneyBudget object
    * from the engine. The id attribute of the object must be set.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budget const reference to the MyMoneyBudget object to be updated
    */
  virtual void removeBudget(const MyMoneyBudget& budget);

  /**
   * This method returns a list of all cost center objects
   */
  virtual const QList<MyMoneyCostCenter> costCenterList() const;

  /**
   * @brief Return cost center object by id
   */
  const MyMoneyCostCenter costCenter(const QString& id) const;

  /**
    * Clear all internal caches (used internally for performance measurements)
    */
  virtual void clearCache();

  /**
    * This method checks, if the given @p object is referenced
    * by another engine object.
    *
    * @param obj const reference to object to be checked
    * @param skipCheck MyMoneyFileBitArray with ReferenceCheckBits set for which
    *                  the check should be skipped
    *
    * @retval false @p object is not referenced
    * @retval true @p institution is referenced
    */
  virtual bool isReferenced(const MyMoneyObject& obj, const MyMoneyFileBitArray& skipCheck = MyMoneyFileBitArray()) const;

  /**
    * This method is provided to allow closing of the database before logoff
    */
  virtual void close();

  /**
    * These methods have to be provided to allow transaction safe data handling.
    */
  virtual void startTransaction();
  virtual bool commitTransaction();
  virtual void rollbackTransaction();

  // general set functions
  virtual void setCreationDate(const QDate& val);

  /**
   * This method is used to get a SQL reader for subsequent database access
   */
  virtual QExplicitlySharedDataPointer <MyMoneyStorageSql> connectToDatabase
  (const QUrl &url);
  /**
    * This method is used when a database file is open, and the data is to
    * be saved in a different file or format. It will ensure that all data
    * from the database is available in memory to enable it to be written.
    */
  virtual void fillStorage();

  /**
    * This method is used to set the last modification date of
    * the storage object. It also clears the dirty flag and should
    * therefor be called as last operation when loading from a
    * file.
    *
    * @param val QDate of last modification
    */
  virtual void setLastModificationDate(const QDate& val);

  /**
   * This method returns whether a given transaction is already in memory, to avoid
   * reloading it from the database
   */
  virtual bool isDuplicateTransaction(const QString&) const;

  virtual void loadAccounts(const QMap<QString, MyMoneyAccount>& map);
  virtual void loadTransactions(const QMap<QString, MyMoneyTransaction>& map);
  virtual void loadInstitutions(const QMap<QString, MyMoneyInstitution>& map);
  virtual void loadPayees(const QMap<QString, MyMoneyPayee>& map);
  virtual void loadTags(const QMap<QString, MyMoneyTag>& map);
  virtual void loadSchedules(const QMap<QString, MyMoneySchedule>& map);
  virtual void loadSecurities(const QMap<QString, MyMoneySecurity>& map);
  virtual void loadCurrencies(const QMap<QString, MyMoneySecurity>& map);
  virtual void loadReports(const QMap<QString, MyMoneyReport>& reports);
  virtual void loadBudgets(const QMap<QString, MyMoneyBudget>& budgets);
  virtual void loadPrices(const MyMoneyPriceList& list);
  virtual void loadOnlineJobs(const QMap<QString, onlineJob>& onlineJobs);
  virtual void loadCostCenters(const QMap<QString, MyMoneyCostCenter>& costCenters);

  //virtual void loadPayeeIdentifier(const QMap<QString, payeeIdentifier>& idents);

  virtual unsigned long accountId() const;
  virtual unsigned long transactionId() const;
  virtual unsigned long payeeId() const;
  virtual unsigned long tagId() const;
  virtual unsigned long institutionId() const;
  virtual unsigned long scheduleId() const;
  virtual unsigned long securityId() const;
  virtual unsigned long reportId() const;
  virtual unsigned long budgetId() const;
  virtual unsigned long onlineJobId() const;
  virtual unsigned long payeeIdentifierId() const;
  virtual unsigned long costCenterId() const;

  virtual void loadAccountId(const unsigned long id);
  virtual void loadTransactionId(const unsigned long id);
  virtual void loadPayeeId(const unsigned long id);
  virtual void loadTagId(const unsigned long id);
  virtual void loadInstitutionId(const unsigned long id);
  virtual void loadScheduleId(const unsigned long id);
  virtual void loadSecurityId(const unsigned long id);
  virtual void loadReportId(const unsigned long id);
  virtual void loadBudgetId(const unsigned long id);
  virtual void loadOnlineJobId(const unsigned long id);
  virtual void loadPayeeIdentifierId(const unsigned long id);
  virtual void loadCostCenterId(const unsigned long id);

  /**
    * This method is used to retrieve the whole set of key/value pairs
    * from the container. It is meant to be used for permanent storage
    * functionality. See MyMoneyKeyValueContainer::pairs() for details.
    *
    * @return QMap<QString, QString> containing all key/value pairs of
    *         this container.
    */
  virtual const QMap<QString, QString> pairs() const;

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
  virtual void setPairs(const QMap<QString, QString>& list);

  /**
    * This method recalculates the balances of all accounts
    * based on the transactions stored in the engine.
    */
  virtual void rebuildAccountBalances();

private:
  /**
    * This member variable keeps the creation date of this MyMoneySeqAccessMgr
    * object. It is set during the constructor and can only be modified using
    * the stream read operator.
    */
  QDate m_creationDate;

  /**
    * This member variable contains the current fix level of application
    * data files. (see kmymoneyview.cpp)
    */
  unsigned int m_currentFixVersion;

  /**
   * This member variable contains the current fix level of the
   *  presently open data file. (see kmymoneyview.cpp)
   */
  unsigned int m_fileFixVersion;

  /**
    * This member variable keeps the date of the last modification of
    * the MyMoneySeqAccessMgr object.
    */
  QDate m_lastModificationDate;

  /**
    * This contains the interface with SQL reader for database access
    */
  QExplicitlySharedDataPointer <MyMoneyStorageSql> m_sql;

  /**
    * This member variable keeps the User information.
    * @see setUser()
    */
  MyMoneyPayee m_user;

  /**
    * This method is used to get the next valid ID for a institution
    * @return id for a institution
    */
  const QString nextInstitutionID();

  /**
    * This method is used to get the next valid ID for an account
    * @return id for an account
    */
  const QString nextAccountID();

  /**
    * This method is used to get the next valid ID for a transaction
    * @return id for a transaction
    */
  const QString nextTransactionID();

  /**
    * This method is used to get the next valid ID for a payee
    * @return id for a payee
    */
  const QString nextPayeeID();

  /**
    * This method is used to get the next valid ID for a tag
    * @return id for a tag
    */
  const QString nextTagID();

  /**
    * This method is used to get the next valid ID for a scheduled transaction
    * @return id for a scheduled transaction
    */
  const QString nextScheduleID();

  /**
    * This method is used to get the next valid ID for an security object.
    * @return id for an security object
    */
  const QString nextSecurityID();

  const QString nextReportID();

  /** @brief get next valid id for an onlineJob */
  const QString nextOnlineJobID();

  /** @brief get next valid id for payeeIdentifier */
  const QString nextPayeeIdentifierID();

  /** @brief get next valid id for a cost center */
  const QString nextCostCenterID();

  /**
    * This method is used to get the next valid ID for a budget object.
    * @return id for an budget object
    */
  const QString nextBudgetID();
  void removeReferences(const QString& id);

  static const int INSTITUTION_ID_SIZE = 6;
  static const int ACCOUNT_ID_SIZE = 6;
  static const int TRANSACTION_ID_SIZE = 18;
  static const int PAYEE_ID_SIZE = 6;
  static const int TAG_ID_SIZE = 6;
  static const int SCHEDULE_ID_SIZE = 6;
  static const int SECURITY_ID_SIZE = 6;
  static const int REPORT_ID_SIZE = 6;
  static const int BUDGET_ID_SIZE = 6;
  static const int ONLINEJOB_ID_SIZE = 8;
  static const int PAYEEIDENTIFIER_ID_SIZE = 6;
  static const int COSTCENTER_ID_SIZE = 6;

  // Increment this to force an update in KMMView.
  // This is different from the db schema version stored in
  // MMStorageSql::m_majorVersion
  static const int CURRENT_FIX_VERSION = 4;

};
#endif
