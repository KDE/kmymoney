/***************************************************************************
                          mymoneyseqaccessmgr.h  -  description
                             -------------------
    begin                : Sun May 5 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#ifndef MYMONEYSEQACCESSMGR_H
#define MYMONEYSEQACCESSMGR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// Project Includes

#include "imymoneystorage.h"
#include "imymoneyserialize.h"
#include "mymoneymap.h"

/**
  * @author Thomas Baumgart
  */

/**
  * The MyMoneySeqAccessMgr class represents the storage engine for sequential
  * files. The actual file type and it's internal storage format (e.g. binary
  * or XML) is not important and handled through the IMyMoneySerialize() interface.
  *
  * The MyMoneySeqAccessMgr must be loaded by an application using the
  * IMyMoneySerialize() interface and can then be accessed through the
  * IMyMoneyStorage() interface. All data is loaded into memory, modified
  * and kept there. It is the subject of an outside object to store the
  * modified data in a persistant storage area using the IMyMoneySerialize()
  * interface. As indication, if data has been changed, the retrun value
  * of the method dirty() can be used.
  */

class MyMoneySeqAccessMgr : public IMyMoneyStorage, public IMyMoneySerialize,
    public MyMoneyKeyValueContainer
{
  KMM_MYMONEY_UNIT_TESTABLE

public:

  MyMoneySeqAccessMgr();
  ~MyMoneySeqAccessMgr();

  // general get functions
  const MyMoneyPayee& user() const override {
    return m_user;
  };
  const QDate creationDate() const override {
    return m_creationDate;
  };
  const QDate lastModificationDate() const override {
    return m_lastModificationDate;
  };
  unsigned int currentFixVersion() const override {
    return m_currentFixVersion;
  };
  unsigned int fileFixVersion() const override {
    return m_fileFixVersion;
  };


  // general set functions
  void setUser(const MyMoneyPayee& user) override {
    m_user = user;
    touch();
  };
  void setCreationDate(const QDate& val) override {
    m_creationDate = val; touch();
  };
  void setLastModificationDate(const QDate& val) override {
    m_lastModificationDate = val; m_dirty = false;
  };
  void setFileFixVersion(const unsigned int v) override {
    m_fileFixVersion = v;
  };
  /**
    * This method is used to get a SQL reader for subsequent database access
    */
  QExplicitlySharedDataPointer <MyMoneyStorageSql> connectToDatabase(const QUrl &url) override;
  /**
  * This method is used when a database file is open, and the data is to
  * be saved in a different file or format. It will ensure that all data
  * from the database is available in memory to enable it to be written.
  */
  virtual void fillStorage() override {  };

  /**
    * This method is used to duplicate the MyMoneySeqAccessMgr object and return
    * a pointer to the newly created copy. The caller of this method is the
    * new owner of the object and must destroy it.
    */
  MyMoneySeqAccessMgr const * duplicate() override;

  /**
    * Returns the account addressed by it's id.
    *
    * @param id id of the account to locate.
    * @return reference to MyMoneyAccount object. An exception is thrown
    *         if the id is unknown
    */
  const MyMoneyAccount account(const QString& id) const override;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
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
    * @param id QString reference to one of the standard accounts. Possible
    *           values are:
    *
    *           @li STD_ACC_LIABILITY
    *           @li STD_ACC_ASSET
    *           @li STD_ACC_EXPENSE
    *           @li STD_ACC_INCOME
    *           @li STD_ACC_EQUITY
    *
    * @param name QString reference to the name to be set
    *
    */
  void setAccountName(const QString& id, const QString& name) override;

  /**
    * This method is used to create a new account
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account MyMoneyAccount filled with data
    */
  void addAccount(MyMoneyAccount& account) override;

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void addPayee(MyMoneyPayee& payee) override;

  /**
   * Create now onlineJob
   */
  void addOnlineJob(onlineJob& job) override;

  /**
    * This method is used to retrieve information about a payee
    * An exception will be thrown upon error conditions.
    *
    * @param id QString reference to id of payee
    *
    * @return MyMoneyPayee object of payee
    */
  const MyMoneyPayee payee(const QString& id) const override;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a payee/receiver.
    * An exception will be thrown upon error conditions.
    *
    * @param payee QString reference to name of payee
    *
    * @return MyMoneyPayee reference to object of payee
    */
  const MyMoneyPayee payeeByName(const QString& payee) const override;

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
  const QList<MyMoneyPayee> payeeList() const override;

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
  const MyMoneyTag tag(const QString& id) const override;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a tag.
    * An exception will be thrown upon error conditions.
    *
    * @param tag QString reference to name of tag
    *
    * @return MyMoneyTag reference to object of tag
    */
  const MyMoneyTag tagByName(const QString& tag) const override;

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
  const QList<MyMoneyTag> tagList() const override;

  /**
    * This method is used to add one account as sub-ordinate to another
    * (parent) account. The objects passed as arguments will be modified
    * accordingly.
    *
    * @param parent parent account the account should be added to
    * @param account the account to be added
    */

  void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account) override;

  /**
    * Adds an institution to the storage. A
    * respective institution-ID will be generated within this record.
    * The ID is stored as QString in the object passed as argument.
    * An exception will be thrown upon error conditions.
    *
    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    */
  void addInstitution(MyMoneyInstitution& institution) override;

  /**
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated within this record. The ID is stored
    * as QString in the transaction object. The accounts of the referenced splits
    * will be updated to have a reference to the transaction just added.
    *
    * @param transaction reference to the transaction
    * @param skipAccountUpdate if set, the transaction lists of the accounts
    *        referenced in the splits are not updated. This is used for
    *        bulk loading a lot of transactions but not during normal operation
    */
  void addTransaction(MyMoneyTransaction& transaction, const bool skipAccountUpdate = false) override;

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    * @param skipCheck if @p true, skips the built in consistency check for
    *                  the object to be updated. Do not set this parameter
    *                  to true. This is only used for the MyMoneyFile::consistencyCheck()
    *                  procedure to be able to reload accounts. The default
    *                  setting of this parameter is @p false.
    */
  void modifyAccount(const MyMoneyAccount& account, const bool skipCheck = false) override;

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

  /** @todo implement */
  void modifyOnlineJob(const onlineJob& job) override;

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
    * @param transaction const reference to transaction to be deleted
    */
  void removeTransaction(const MyMoneyTransaction& transaction) override;

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
    * @param institution institution to be deleted.
    */
  void removeInstitution(const MyMoneyInstitution& institution) override;

  const onlineJob getOnlineJob(const QString &id) const override;
  /** @todo implement */
  long unsigned int onlineJobId() const override {
    return 1;
  }

  void removeOnlineJob(const onlineJob &) override;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an id. In case of an invalid id, an
    * exception will be thrown.
    *
    * @param id id of transaction as QString.
    * @return reference to the requested transaction
    */
  const MyMoneyTransaction transaction(const QString& id) const override;

  /**
    * This method is used to extract a transaction from the file global
    * transaction pool through an index into an account.
    *
    * @param account id of the account as QString
    * @param idx number of transaction in this account
    * @return reference to MyMoneyTransaction object
    */
  const MyMoneyTransaction transaction(const QString& account, const int idx) const override;

  /**
    * This method is used to determince, if the account with the
    * given ID is referenced by any split in m_transactionList.
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
  const MyMoneyMoney balance(const QString& id, const QDate& date = QDate()) const override;

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
  const MyMoneyMoney totalBalance(const QString& id, const QDate& date = QDate()) const override;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  const MyMoneyInstitution institution(const QString& id) const override;

  /**
    * This method returns an indicator if the storage object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  bool dirty() const override {
    return m_dirty;
  }

  /**
    * This method can be used by an external object to force the
    * storage object to be dirty. This is used e.g. when an upload
    * to an external destination failed but the previous storage
    * to a local disk was ok.
    */
  void setDirty() override {
    m_dirty = true;
  };

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyFile object
    *
    * @return QMap containing the institution information
    */
  const QList<MyMoneyInstitution> institutionList() const override;

  /**
    * This method returns a list of accounts inside the storage object.
    *
    * @param list reference to QList receiving the account objects
    *
    * @note The standard accounts will not be returned
    */
  void accountList(QList<MyMoneyAccount>& list) const override;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns all those transactions
    * that match the filter passed as argument. If the filter is empty,
    * the whole journal will be returned.
    * The list returned is sorted according to the transactions posting date.
    * If more than one transaction exists for the same date, the order among
    * them is undefined.
    *
    * The @p list will be cleared by this method.
    *
    * @param list reference to list
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QList<MyMoneyTransaction>
    */
  void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const override;

  /**
    * This method is used to pull a list of transactions from the file
    * global transaction pool. It returns all those transactions
    * that match the filter passed as argument. If the filter is empty,
    * the whole journal will be returned.
    * The list returned is sorted according to the transactions posting date.
    * If more than one transaction exists for the same date, the order among
    * them is undefined.
    *
    * The @p list will be cleared by this method.
    *
    * @param list reference to list
    * @param filter MyMoneyTransactionFilter object with the match criteria
    *
    * @return set of transactions in form of a QList<QPair<MyMoneyTransaction,MyMoneySplit> >
    */
  void transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const override;

  /**
    * Compatibility interface for the previous method.
    */
  const QList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const override;

  /**
   * @brief Return all onlineJobs
   */
  const QList<onlineJob> onlineJobList() const override;

  /**
   * @brief Return all cost center objects
   */
  const QList< MyMoneyCostCenter > costCenterList() const override;

  /**
   * @brief Return cost center object by id
   */
  const MyMoneyCostCenter costCenter(const QString& id) const override;

  /**
    * This method returns whether a given transaction is already in memory, to avoid
    * reloading it from the database
    */
  bool isDuplicateTransaction(const QString& id) const override {
    return m_transactionKeys.contains(id);
  }

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
  unsigned int transactionCount(const QString& account = QString()) const override;

  const QMap<QString, unsigned long> transactionCountMap() const override;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  unsigned int institutionCount() const override;

  /**
    * This method returns the number of accounts currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  unsigned int accountCount() const override;

  /**
    * This method is used to return the standard liability account
    * @return MyMoneyAccount liability account(group)
    */
  const MyMoneyAccount liability() const override {
    return account(STD_ACC_LIABILITY);
  };

  /**
    * This method is used to return the standard asset account
    * @return MyMoneyAccount asset account(group)
    */
  const MyMoneyAccount asset() const override {
    return account(STD_ACC_ASSET);
  };

  /**
    * This method is used to return the standard expense account
    * @return MyMoneyAccount expense account(group)
    */
  const MyMoneyAccount expense() const override {
    return account(STD_ACC_EXPENSE);
  };

  /**
    * This method is used to return the standard income account
    * @return MyMoneyAccount income account(group)
    */
  const MyMoneyAccount income() const override {
    return account(STD_ACC_INCOME);
  };

  /**
    * This method is used to return the standard equity account
    * @return MyMoneyAccount equity account(group)
    */
  const MyMoneyAccount equity() const override {
    return account(STD_ACC_EQUITY);
  };

  virtual void loadAccounts(const QMap<QString, MyMoneyAccount>& acc) override;
  virtual void loadTransactions(const QMap<QString, MyMoneyTransaction>& map) override;
  virtual void loadInstitutions(const QMap<QString, MyMoneyInstitution>& map) override;
  virtual void loadPayees(const QMap<QString, MyMoneyPayee>& map) override;
  virtual void loadTags(const QMap<QString, MyMoneyTag>& map) override;
  virtual void loadSchedules(const QMap<QString, MyMoneySchedule>& map) override;
  virtual void loadSecurities(const QMap<QString, MyMoneySecurity>& map) override;
  virtual void loadCurrencies(const QMap<QString, MyMoneySecurity>& map) override;
  virtual void loadPrices(const MyMoneyPriceList& list) override;
  virtual void loadOnlineJobs(const QMap<QString, onlineJob>& onlineJobs) override;
  virtual void loadCostCenters(const QMap<QString, MyMoneyCostCenter>& costCenters) override;

  virtual void loadAccountId(const unsigned long id) override;
  virtual void loadTransactionId(const unsigned long id) override;
  virtual void loadPayeeId(const unsigned long id) override;
  virtual void loadTagId(const unsigned long id) override;
  virtual void loadInstitutionId(const unsigned long id) override;
  virtual void loadScheduleId(const unsigned long id) override;
  virtual void loadSecurityId(const unsigned long id) override;
  virtual void loadReportId(const unsigned long id) override;
  virtual void loadBudgetId(const unsigned long id) override;
  virtual void loadOnlineJobId(const unsigned long id) override;
  virtual void loadCostCenterId(const unsigned long id) override;

  virtual unsigned long accountId() const override {
    return m_nextAccountID;
  };
  virtual unsigned long transactionId() const override {
    return m_nextTransactionID;
  };
  virtual unsigned long payeeId() const override {
    return m_nextPayeeID;
  };
  virtual unsigned long tagId() const override {
    return m_nextTagID;
  };
  virtual unsigned long institutionId() const override {
    return m_nextInstitutionID;
  };
  virtual unsigned long scheduleId() const override {
    return m_nextScheduleID;
  };
  virtual unsigned long securityId() const override {
    return m_nextSecurityID;
  };
  virtual unsigned long reportId() const override {
    return m_nextReportID;
  };
  virtual unsigned long budgetId() const override {
    return m_nextBudgetID;
  };
  virtual unsigned long costCenterId() const override {
    return m_nextCostCenterID;
  }

  /**
    * This method is used to extract a value from
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::value().
    *
    * @param key const reference to QString containing the key
    * @return QString containing the value
    */
  const QString value(const QString& key) const override;

  /**
    * This method is used to set a value in the
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::setValue().
    *
    * @param key const reference to QString containing the key
    * @param val const reference to QString containing the value
    */
  void setValue(const QString& key, const QString& val) override;

  /**
    * This method is used to delete a key-value-pair from the
    * KeyValueContainer identified by the parameter
    * @p key. For details see MyMoneyKeyValueContainer::deletePair().
    *
    * @param key const reference to QString containing the key
    */
  void deletePair(const QString& key) override;

  // documented in IMyMoneySerialize base class
  const QMap<QString, QString> pairs() const override;

  // documented in IMyMoneySerialize base class
  void setPairs(const QMap<QString, QString>& list) override;

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
  const MyMoneySchedule schedule(const QString& id) const override;

  /**
    * This method is used to create a new security object.  The ID will be created
    * automatically. The object passed with the parameter @p security is modified
    * to contain the assigned id.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param security MyMoneySecurity filled with data
    */
  virtual void addSecurity(MyMoneySecurity& security) override;

  /**
    * This method is used to modify an existing MyMoneySchedule
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
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  const MyMoneySecurity security(const QString& id) const override;


  /**
    * This method returns a list of security objects that the engine has
    * knowledge of.
    */
  const QList<MyMoneySecurity> securityList() const override;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void addCurrency(const MyMoneySecurity& currency) override;

  /**
    * This method is used to modify an existing MyMoneyCurrency
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void modifyCurrency(const MyMoneySecurity& currency) override;

  /**
    * This method is used to remove an existing MyMoneyCurrency object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneyCurrency object
    */
  void removeCurrency(const MyMoneySecurity& currency) override;

  /**
    * This method is used to retrieve a single MyMoneySchedule object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  const MyMoneySecurity currency(const QString& id) const override;

  /**
    * This method is used to retrieve the list of all currencies
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyCurrency objects.
    */
  const QList<MyMoneySecurity> currencyList() const override;

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
  const QList<MyMoneySchedule> scheduleList(const QString& accountId = QString(),
      const eMyMoney::Schedule::Type type = eMyMoney::Schedule::Type::Any,
      const eMyMoney::Schedule::Occurrence occurrence = eMyMoney::Schedule::Occurrence::Any,
      const eMyMoney::Schedule::PaymentType paymentType = eMyMoney::Schedule::PaymentType::Any,
      const QDate& startDate = QDate(),
      const QDate& endDate = QDate(),
      const bool overdue = false) const override;

  const QList<MyMoneySchedule> scheduleListEx(int scheduleTypes,
      int scheduleOcurrences,
      int schedulePaymentTypes,
      QDate startDate,
      const QStringList& accounts = QStringList()) const override;

  /**
    * This method is used to retrieve the list of all reports
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneyReport objects.
    */
  const QList<MyMoneyReport> reportList() const override;

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
    * This method is used to load a set of reports into the engine.  This is
    * used when loading from storage, and an ID is already known.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param reports reference to the map of MyMoneyReport objects
    */
  void loadReports(const QMap<QString, MyMoneyReport>& reports) override;

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
  unsigned countReports() const override;

  /**
    * This method is used to retrieve a single MyMoneyReport object.
    * The id of the object must be supplied in the parameter @p id.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneyReport object
    * @return MyMoneyReport object
    */
  const MyMoneyReport report(const QString& id) const override;

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
  const QList<MyMoneyBudget> budgetList() const override;

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
    * This method is used to load a set of budgets into the engine.  This is
    * used when loading from storage, and an ID is already known.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param budgets reference to the map of MyMoneyBudget object
    */
  void loadBudgets(const QMap<QString, MyMoneyBudget>& budgets) override;

  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget reference to object of budget
    */
  const MyMoneyBudget budgetByName(const QString& budget) const override;

  /**
    * This method is used to modify an existing MyMoneyBudget
    * object. Therefore, the id attribute of the object must be set.
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
  unsigned countBudgets() const override;

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
    * This method adds/replaces a price to/from the price list
    */
  void addPrice(const MyMoneyPrice& price) override;

  /**
    * This method removes a price from the price list
    */
  void removePrice(const MyMoneyPrice& price) override;

  /**
    * This method retrieves a price from the price list.
    * If @p date is inValid, QDate::currentDate() is assumed.
    */
  MyMoneyPrice price(const QString& fromId, const QString& toId, const QDate& _date, const bool exactDate) const override;

  /**
    * This method returns a list of all price entries.
    */
  const MyMoneyPriceList priceList() const override;

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
    * This method recalculates the balances of all accounts
    * based on the transactions stored in the engine.
    */
  void rebuildAccountBalances() override;

  virtual void startTransaction() override;
  virtual bool commitTransaction() override;
  virtual void rollbackTransaction() override;

protected:
  void removeReferences(const QString& id);

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
  MyMoneyMoney calculateBalance(const QString& id, const QDate& date = QDate()) const;

private:

  static const int INSTITUTION_ID_SIZE = 6;
  static const int ACCOUNT_ID_SIZE = 6;
  static const int TRANSACTION_ID_SIZE = 18;
  static const int PAYEE_ID_SIZE = 6;
  static const int TAG_ID_SIZE = 6;
  static const int SCHEDULE_ID_SIZE = 6;
  static const int SECURITY_ID_SIZE = 6;
  static const int REPORT_ID_SIZE = 6;
  static const int BUDGET_ID_SIZE = 6;
  static const int ONLINE_JOB_ID_SIZE = 6;
  static const int COSTCENTER_ID_SIZE = 6;

  /**
    * This method is used to set the dirty flag and update the
    * date of the last modification.
    */
  void touch();

  /**
    * Adjust the balance for account @a acc by the amount of shares in split @a split.
    * The amount is added if @a reverse is @c false, subtracted in case it is @c true.
    */
  void adjustBalance(MyMoneyAccount& acc, const MyMoneySplit& split, bool reverse = false);

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
  unsigned long m_nextInstitutionID;

  /**
    * The member variable m_nextAccountID keeps the number that will be
    * assigned to the next institution created. It is maintained by
    * nextAccountID().
    */
  unsigned long m_nextAccountID;

  /**
    * The member variable m_nextTransactionID keeps the number that will be
    * assigned to the next transaction created. It is maintained by
    * nextTransactionID().
    */
  unsigned long m_nextTransactionID;

  /**
    * The member variable m_nextPayeeID keeps the number that will be
    * assigned to the next payee created. It is maintained by
    * nextPayeeID()
    */
  unsigned long m_nextPayeeID;

  /**
    * The member variable m_nextTagID keeps the number that will be
    * assigned to the next tag created. It is maintained by
    * nextTagID()
    */
  unsigned long m_nextTagID;

  /**
    * The member variable m_nextScheduleID keeps the number that will be
    * assigned to the next schedule created. It is maintained by
    * nextScheduleID()
    */
  unsigned long m_nextScheduleID;

  /**
    * The member variable m_nextSecurityID keeps the number that will be
    * assigned to the next security object created.  It is maintained by
    * nextSecurityID()
    */
  unsigned long m_nextSecurityID;

  unsigned long m_nextReportID;

  /**
    * The member variable m_nextBudgetID keeps the number that will be
    * assigned to the next budget object created.  It is maintained by
    * nextBudgetID()
    */
  unsigned long m_nextBudgetID;

  /**
    * This member variable keeps the number that will be assigned to the
    * next onlineJob object created. It is maintained by nextOnlineJobID()
    */
  unsigned long m_nextOnlineJobID;

  /**
    * This member variable keeps the number that will be assigned to the
    * next cost center object created. It is maintained by nextCostCenterID()
    */
  unsigned long m_nextCostCenterID;

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
  unsigned int m_currentFixVersion;
  /**
   * This member variable contains the current fix level of the
   *  presently open data file. (see kmymoneyview.cpp)
   */
  unsigned int m_fileFixVersion;
  /**
    * This method is used to get the next valid ID for a institution
    * @return id for a institution
    */
  QString nextInstitutionID();

  /**
    * This method is used to get the next valid ID for an account
    * @return id for an account
    */
  QString nextAccountID();

  /**
    * This method is used to get the next valid ID for a transaction
    * @return id for a transaction
    */
  QString nextTransactionID();

  /**
    * This method is used to get the next valid ID for a payee
    * @return id for a payee
    */
  QString nextPayeeID();

  /**
    * This method is used to get the next valid ID for a tag
    * @return id for a tag
    */
  QString nextTagID();

  /**
    * This method is used to get the next valid ID for a scheduled transaction
    * @return id for a scheduled transaction
    */
  QString nextScheduleID();

  /**
    * This method is used to get the next valid ID for an security object.
    * @return id for an security object
    */
  QString nextSecurityID();

  QString nextReportID();

  /**
    * This method is used to get the next valid ID for a budget object.
    * @return id for an budget object
    */
  QString nextBudgetID();

  /**
    * This method is used to get the next valid ID for an onlineJob object.
    */
  QString nextOnlineJobID();

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
  void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent, const bool sendNotification);
  /**
   * This method will close a database and log the use roff
   */
  void close() override {}

  /**
    * This member variable is set when all transactions have been read from the database.
    * This is would be probably the case when doing, for e.g., a full report,
    * or after some types of transaction search which cannot be easily implemented in SQL
    */
  bool m_transactionListFull;
};
#endif
