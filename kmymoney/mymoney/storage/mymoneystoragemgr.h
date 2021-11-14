/*
    SPDX-FileCopyrightText: 2002-2003 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSTORAGEMGR_H
#define MYMONEYSTORAGEMGR_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"

class QUrl;
class QString;
class QStringList;
class QDate;
class QBitArray;

class MyMoneyObject;
class MyMoneyMoney;
class MyMoneyInstitution;
class MyMoneyAccount;
class MyMoneySecurity;
class MyMoneyPayee;
class MyMoneyTag;
class MyMoneyPrice;
class MyMoneyReport;
class MyMoneySchedule;
class MyMoneyBudget;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneyTransactionFilter;
class MyMoneyCostCenter;
class onlineJob;
class MyMoneyStorageSql;

template <class Key, class T> class QMap;
template <typename T> class QList;
template <class T1, class T2> struct QPair;

typedef QPair<QString, QString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;

namespace eMyMoney {
enum class StockSplitDirection;
namespace Schedule {
enum class Type;
}
}
namespace eMyMoney {
namespace Schedule {
enum class Occurrence;
}
}
namespace eMyMoney {
namespace Schedule {
enum class PaymentType;
}
}

/**
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

/**
  * The MyMoneyStorageMgr class represents the storage engine for sequential
  * files. The actual file type and it's internal storage format (e.g. binary
  * or XML) is not important.
  *
  * The MyMoneyStorageMgr must be loaded by an application using loadAccounts()
  * method and etc. and can then be accessed through the
  * account() method and etc.. All data is loaded into memory, modified
  * and kept there. It is the subject of an outside object to store the
  * modified data in a persistent storage area using the accountList() method and etc.
  * As indication, if data has been changed, the return value
  * of the method dirty() can be used.
  */

class MyMoneyStorageMgrPrivate;
class KMM_MYMONEY_EXPORT MyMoneyStorageMgr : public MyMoneyKeyValueContainer
{
    Q_DISABLE_COPY(MyMoneyStorageMgr)
    KMM_MYMONEY_UNIT_TESTABLE
    friend class MyMoneyStorageDump;

public:
    MyMoneyStorageMgr();
    ~MyMoneyStorageMgr();

    // general get functions
    MyMoneyPayee user() const;
    QDate creationDate() const;
    QDate lastModificationDate() const;
    uint currentFixVersion() const;
    uint fileFixVersion() const;

    // general set functions
    void setUser(const MyMoneyPayee& user);
    void setCreationDate(const QDate& val);
    void setLastModificationDate(const QDate& val);
    void setFileFixVersion(uint v);

    /**
      * Returns the account addressed by it's id.
      *
      * @param id id of the account to locate.
      * @return reference to MyMoneyAccount object. An exception is thrown
      *         if the id is unknown
      */
    MyMoneyAccount account(const QString& id) const;

    /**
      * This method is used to retrieve the id to a corresponding
      * name of an account.
      * An exception will be thrown upon error conditions.
      *
      * @param name QString reference to name of account
      *
      * @return MyMoneyAccount reference to object of account
      */
    MyMoneyAccount accountByName(const QString& name) const;

    /**
      * This method is used to check whether a given
      * account id references one of the standard accounts or not.
      *
      * @param id account id
      * @return true if account-id is one of the standards, false otherwise
      */
    bool isStandardAccount(const QString& id) const;

    /**
      * This method is used to set the name for the specified standard account
      * within the storage area. An exception will be thrown, if an error
      * occurs
      *
      * @param id QString reference to one of the standard accounts. Possible
      *           values are:
      *
      *           @li MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Liability)
      *           @li MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Asset)
      *           @li MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Expense)
      *           @li MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Income)
      *           @li MyMoneyAccount::stdAccName(eMyMoney::Account::Standard::Equity)
      *
      * @param name QString reference to the name to be set
      *
      */
    void setAccountName(const QString& id, const QString& name);

    /**
      * This method is used to create a new account
      *
      * An exception will be thrown upon error conditions.
      *
      * @param account MyMoneyAccount filled with data
      */
    void addAccount(MyMoneyAccount& account);

    /**
      * This method is used to create a new payee
      *
      * An exception will be thrown upon error conditions
      *
      * @param payee MyMoneyPayee reference to payee information
      */
    void addPayee(MyMoneyPayee& payee);

    /**
     * Create now onlineJob
     */
    void addOnlineJob(onlineJob& job);

    /**
      * This method is used to retrieve information about a payee
      * An exception will be thrown upon error conditions.
      *
      * @param id QString reference to id of payee
      *
      * @return MyMoneyPayee object of payee
      */
    MyMoneyPayee payee(const QString& id) const;

    /**
      * This method is used to retrieve the id to a corresponding
      * name of a payee/receiver.
      * An exception will be thrown upon error conditions.
      *
      * @param payee QString reference to name of payee
      *
      * @return MyMoneyPayee reference to object of payee
      */
    MyMoneyPayee payeeByName(const QString& payee) const;

    /**
      * This method is used to modify an existing payee
      *
      * An exception will be thrown upon error conditions
      *
      * @param payee MyMoneyPayee reference to payee information
      */
    void modifyPayee(const MyMoneyPayee& payee);

    /**
      * This method is used to remove an existing payee
      *
      * An exception will be thrown upon error conditions
      *
      * @param payee MyMoneyPayee reference to payee information
      */
    void removePayee(const MyMoneyPayee& payee);

    /**
      * This method returns a list of the payees
      * inside a MyMoneyStorage object
      *
      * @return QList<MyMoneyPayee> containing the payee information
      */
    QList<MyMoneyPayee> payeeList() const;

    /**
      * This method is used to create a new tag
      *
      * An exception will be thrown upon error conditions
      *
      * @param tag MyMoneyTag reference to tag information
      */
    void addTag(MyMoneyTag& tag);

    /**
      * This method is used to retrieve information about a tag
      * An exception will be thrown upon error conditions.
      *
      * @param id QString reference to id of tag
      *
      * @return MyMoneyTag object of tag
      */
    MyMoneyTag tag(const QString& id) const;

    /**
      * This method is used to retrieve the id to a corresponding
      * name of a tag.
      * An exception will be thrown upon error conditions.
      *
      * @param tag QString reference to name of tag
      *
      * @return MyMoneyTag reference to object of tag
      */
    MyMoneyTag tagByName(const QString& tag) const;

    /**
      * This method is used to modify an existing tag
      *
      * An exception will be thrown upon error conditions
      *
      * @param tag MyMoneyTag reference to tag information
      */
    void modifyTag(const MyMoneyTag& tag);

    /**
      * This method is used to remove an existing tag
      *
      * An exception will be thrown upon error conditions
      *
      * @param tag MyMoneyTag reference to tag information
      */
    void removeTag(const MyMoneyTag& tag);

    /**
      * This method returns a list of the tags
      * inside a MyMoneyStorage object
      *
      * @return QList<MyMoneyTag> containing the tag information
      */
    QList<MyMoneyTag> tagList() const;

    /**
      * This method is used to add one account as sub-ordinate to another
      * (parent) account. The objects passed as arguments will be modified
      * accordingly.
      *
      * @param parent parent account the account should be added to
      * @param account the account to be added
      */

    void addAccount(MyMoneyAccount& parent, MyMoneyAccount& account);

    /**
      * Adds an institution to the storage. A
      * respective institution-ID will be generated within this record.
      * The ID is stored as QString in the object passed as argument.
      * An exception will be thrown upon error conditions.
      *
      * @param institution The complete institution information in a
      *        MyMoneyInstitution object
      */
    void addInstitution(MyMoneyInstitution& institution);

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
    void addTransaction(MyMoneyTransaction& transaction, bool skipAccountUpdate = false);

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
    void modifyAccount(const MyMoneyAccount& account, bool skipCheck = false);

    /**
      * Modifies an already existing institution in the file global
      * institution pool.
      *
      * An exception will be thrown upon error conditions.
      *
      * @param institution The complete new institution information
      */
    void modifyInstitution(const MyMoneyInstitution& institution);

    /**
      * This method is used to update a specific transaction in the
      * transaction pool of the MyMoneyFile object
      *
      * An exception will be thrown upon error conditions.
      *
      * @param transaction reference to transaction to be changed
      */
    void modifyTransaction(const MyMoneyTransaction& transaction);

    /** @todo implement */
    void modifyOnlineJob(const onlineJob& job);

    /**
      * This method re-parents an existing account
      *
      * An exception will be thrown upon error conditions.
      *
      * @param account MyMoneyAccount reference to account to be re-parented
      * @param parent  MyMoneyAccount reference to new parent account
      */
    void reparentAccount(MyMoneyAccount &account, MyMoneyAccount& parent);

    /**
      * This method is used to remove a transaction from the transaction
      * pool (journal).
      *
      * @param transaction const reference to transaction to be deleted
      */
    void removeTransaction(const MyMoneyTransaction& transaction);

    /**
      * Deletes an existing account from the file global account pool
      * This method only allows to remove accounts that are not
      * referenced by any split. Use moveSplits() to move splits
      * to another account. An exception is thrown in case of a
      * problem.
      *
      * @param account reference to the account to be deleted.
      */
    void removeAccount(const MyMoneyAccount& account);

    /**
      * Deletes an existing institution from the file global institution pool
      * Also modifies the accounts that reference this institution as
      * their institution.
      *
      * @param institution institution to be deleted.
      */
    void removeInstitution(const MyMoneyInstitution& institution);

    onlineJob getOnlineJob(const QString& id) const;
    /** @todo implement */
    ulong onlineJobId() const;

    void removeOnlineJob(const onlineJob &);

    /**
      * This method is used to extract a transaction from the file global
      * transaction pool through an id. In case of an invalid id, an
      * exception will be thrown.
      *
      * @param id id of transaction as QString.
      * @return reference to the requested transaction
      */
    MyMoneyTransaction transaction(const QString& id) const;

    /**
      * This method is used to extract a transaction from the file global
      * transaction pool through an index into an account.
      *
      * @param account id of the account as QString
      * @param idx number of transaction in this account
      * @return reference to MyMoneyTransaction object
      */
    MyMoneyTransaction transaction(const QString& account, const int idx) const;

    /**
      * This method is used to determine, if the account with the
      * given ID is referenced by any split in m_transactionList.
      *
      * @param id id of the account to be checked for
      * @return true if account is referenced, false otherwise
      */
    bool hasActiveSplits(const QString& id) const;

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
    MyMoneyMoney balance(const QString& id, const QDate& date) const;

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
    MyMoneyMoney totalBalance(const QString& id, const QDate& date) const;

    /**
      * Returns the institution of a given ID
      *
      * @param id id of the institution to locate
      * @return MyMoneyInstitution object filled with data. If the institution
      *         could not be found, an exception will be thrown
      */
    MyMoneyInstitution institution(const QString& id) const;

    /**
      * This method returns an indicator if the storage object has been
      * changed after it has last been saved to permanent storage.
      *
      * @return true if changed, false if not
      */
    bool dirty() const;

    /**
      * This method can be used by an external object to force the
      * storage object to be dirty. This is used e.g. when an upload
      * to an external destination failed but the previous storage
      * to a local disk was ok.
      */
    void setDirty();

    /**
      * This method returns a list of the institutions
      * inside a MyMoneyFile object
      *
      * @return QMap containing the institution information
      */
    QList<MyMoneyInstitution> institutionList() const;

    /**
      * This method returns a list of accounts inside the storage object.
      *
      * @param list reference to QList receiving the account objects
      *
      * @note The standard accounts will not be returned
      */
    void accountList(QList<MyMoneyAccount>& list) const;

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
    void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const;

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
    void transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const;

    /**
      * Compatibility interface for the previous method.
      */
    QList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const;

    /**
     * @brief Return all onlineJobs
     */
    QList<onlineJob> onlineJobList() const;

    /**
     * @brief Return all cost center objects
     */
    QList< MyMoneyCostCenter > costCenterList() const;

    /**
     * @brief Return cost center object by id
     */
    MyMoneyCostCenter costCenter(const QString& id) const;

    /**
      * This method returns whether a given transaction is already in memory, to avoid
      * reloading it from the database
      */
    bool isDuplicateTransaction(const QString& id) const;

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
    uint transactionCount(const QString& account) const;

    QMap<QString, ulong> transactionCountMap() const;

    /**
      * This method returns the number of institutions currently known to file
      * in the range 0..MAXUINT
      *
      * @return number of institutions known to file
      */
    uint institutionCount() const;

    /**
      * This method returns the number of accounts currently known to file
      * in the range 0..MAXUINT
      *
      * @return number of accounts currently known inside a MyMoneyFile object
      */
    uint accountCount() const;

    /**
      * This method is used to return the standard liability account
      * @return MyMoneyAccount liability account(group)
      */
    MyMoneyAccount liability() const;

    /**
      * This method is used to return the standard asset account
      * @return MyMoneyAccount asset account(group)
      */
    MyMoneyAccount asset() const;

    /**
      * This method is used to return the standard expense account
      * @return MyMoneyAccount expense account(group)
      */
    MyMoneyAccount expense() const;

    /**
      * This method is used to return the standard income account
      * @return MyMoneyAccount income account(group)
      */
    MyMoneyAccount income() const;

    /**
      * This method is used to return the standard equity account
      * @return MyMoneyAccount equity account(group)
      */
    MyMoneyAccount equity() const;

    void loadAccounts(const QMap<QString, MyMoneyAccount>& acc);
    void loadTransactions(const QMap<QString, MyMoneyTransaction>& map);
    void loadInstitutions(const QMap<QString, MyMoneyInstitution>& map);
    void loadPayees(const QMap<QString, MyMoneyPayee>& map);
    void loadTags(const QMap<QString, MyMoneyTag>& map);
    void loadSchedules(const QMap<QString, MyMoneySchedule>& map);
    void loadSecurities(const QMap<QString, MyMoneySecurity>& map);
    void loadCurrencies(const QMap<QString, MyMoneySecurity>& map);
    void loadPrices(const MyMoneyPriceList& list);
    void loadOnlineJobs(const QMap<QString, onlineJob>& onlineJobs);
    void loadCostCenters(const QMap<QString, MyMoneyCostCenter>& costCenters);

    /**
      * This method is used to set a value in the
      * KeyValueContainer. For details see MyMoneyKeyValueContainer::setValue().
      *
      * @param key const reference to QString containing the key
      * @param val const reference to QString containing the value
      */
    void setValue(const QString& key, const QString& val);

    /**
      * This method is used to delete a key-value-pair from the
      * KeyValueContainer identified by the parameter
      * @p key. For details see MyMoneyKeyValueContainer::deletePair().
      *
      * @param key const reference to QString containing the key
      */
    void deletePair(const QString& key);

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
    void setPairs(const QMap<QString, QString>& list);

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
    void addSchedule(MyMoneySchedule& sched);

    /**
      * This method is used to modify an existing MyMoneySchedule
      * object. Therefor, the id attribute of the object must be set.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param sched const reference to the MyMoneySchedule object to be updated
      */
    void modifySchedule(const MyMoneySchedule& sched);

    /**
      * This method is used to remove an existing MyMoneySchedule object
      * from the engine. The id attribute of the object must be set.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param sched const reference to the MyMoneySchedule object to be updated
      */
    void removeSchedule(const MyMoneySchedule& sched);

    /**
      * This method is used to retrieve a single MyMoneySchedule object.
      * The id of the object must be supplied in the parameter @p id.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param id QString containing the id of the MyMoneySchedule object
      * @return MyMoneySchedule object
      */
    MyMoneySchedule schedule(const QString& id) const;

    /**
      * This method is used to create a new security object.  The ID will be created
      * automatically. The object passed with the parameter @p security is modified
      * to contain the assigned id.
      *
      * An exception will be thrown upon error conditions.
      *
      * @param security MyMoneySecurity filled with data
      */
    void addSecurity(MyMoneySecurity& security);

    /**
      * This method is used to modify an existing MyMoneySchedule
      * object.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param security reference to the MyMoneySecurity object to be updated
      */
    void modifySecurity(const MyMoneySecurity& security);

    /**
      * This method is used to remove an existing MyMoneySecurity object
      * from the engine.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param security reference to the MyMoneySecurity object to be removed
      */
    void removeSecurity(const MyMoneySecurity& security);

    /**
      * This method is used to retrieve a single MyMoneySecurity object.
      * The id of the object must be supplied in the parameter @p id.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param id QString containing the id of the MyMoneySchedule object
      * @return MyMoneySchedule object
      */
    MyMoneySecurity security(const QString& id) const;


    /**
      * This method returns a list of security objects that the engine has
      * knowledge of.
      */
    QList<MyMoneySecurity> securityList() const;

    /**
      * This method is used to add a new currency object to the engine.
      * The ID of the object is the trading symbol, so there is no need for an additional
      * ID since the symbol is guaranteed to be unique.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param currency reference to the MyMoneyCurrency object
      */
    void addCurrency(const MyMoneySecurity& currency);

    /**
      * This method is used to modify an existing MyMoneyCurrency
      * object.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param currency reference to the MyMoneyCurrency object
      */
    void modifyCurrency(const MyMoneySecurity& currency);

    /**
      * This method is used to remove an existing MyMoneyCurrency object
      * from the engine.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param currency reference to the MyMoneyCurrency object
      */
    void removeCurrency(const MyMoneySecurity& currency);

    /**
      * This method is used to retrieve a single MyMoneySecurity object.
      * The id of the object must be supplied in the parameter @p id.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param id QString containing the id of the MyMoneySchedule object
      * @return MyMoneySchedule object
      */
    MyMoneySecurity currency(const QString& id) const;

    /**
      * This method is used to retrieve the list of all currencies
      * known to the engine.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @return QList of all MyMoneyCurrency objects.
      */
    QList<MyMoneySecurity> currencyList() const;

    /**
      * This method is used to extract a list of scheduled transactions
      * according to the filter criteria passed as arguments.
      *
      * @param accountId only search for scheduled transactions that reference
      *                  account @p accountId. If accountId is the empty string,
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
                                        bool overdue) const;

    QList<MyMoneySchedule> scheduleListEx(int scheduleTypes,
                                          int scheduleOcurrences,
                                          int schedulePaymentTypes,
                                          QDate startDate,
                                          const QStringList& accounts) const;

    /**
      * This method is used to retrieve the list of all reports
      * known to the engine.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @return QList of all MyMoneyReport objects.
      */
    QList<MyMoneyReport> reportList() const;

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
    void addReport(MyMoneyReport& report);

    /**
      * This method is used to load a set of reports into the engine.  This is
      * used when loading from storage, and an ID is already known.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param reports reference to the map of MyMoneyReport objects
      */
    void loadReports(const QMap<QString, MyMoneyReport>& reports);

    /**
      * This method is used to modify an existing MyMoneyReport
      * object. Therefor, the id attribute of the object must be set.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param report const reference to the MyMoneyReport object to be updated
      */
    void modifyReport(const MyMoneyReport& report);

    /**
      * This method returns the number of reports currently known to file
      * in the range 0..MAXUINT
      *
      * @return number of reports known to file
      */
    uint countReports() const;

    /**
      * This method is used to retrieve a single MyMoneyReport object.
      * The id of the object must be supplied in the parameter @p id.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param id QString containing the id of the MyMoneyReport object
      * @return MyMoneyReport object
      */
    MyMoneyReport report(const QString& id) const;

    /**
      * This method is used to remove an existing MyMoneyReport object
      * from the engine. The id attribute of the object must be set.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param report const reference to the MyMoneyReport object to be updated
      */
    void removeReport(const MyMoneyReport& report);

    /**
      * This method is used to retrieve the list of all budgets
      * known to the engine.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @return QList of all MyMoneyBudget objects.
      */
    QList<MyMoneyBudget> budgetList() const;

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
    void addBudget(MyMoneyBudget& budget);

    /**
      * This method is used to load a set of budgets into the engine.  This is
      * used when loading from storage, and an ID is already known.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param budgets reference to the map of MyMoneyBudget object
      */
    void loadBudgets(const QMap<QString, MyMoneyBudget>& budgets);

    /**
      * This method is used to retrieve the id to a corresponding
      * name of a budget
      * An exception will be thrown upon error conditions.
      *
      * @param budget QString reference to name of budget
      *
      * @return MyMoneyBudget reference to object of budget
      */
    MyMoneyBudget budgetByName(const QString& budget) const;

    /**
      * This method is used to modify an existing MyMoneyBudget
      * object. Therefore, the id attribute of the object must be set.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param budget const reference to the MyMoneyBudget object to be updated
      */
    void modifyBudget(const MyMoneyBudget& budget);

    /**
      * This method returns the number of budgets currently known to file
      * in the range 0..MAXUINT
      *
      * @return number of budgets known to file
      */
    uint countBudgets() const;

    /**
      * This method is used to retrieve a single MyMoneyBudget object.
      * The id of the object must be supplied in the parameter @p id.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param id QString containing the id of the MyMoneyBudget object
      * @return MyMoneyBudget object
      */
    MyMoneyBudget budget(const QString& id) const;

    /**
      * This method is used to remove an existing MyMoneyBudget object
      * from the engine. The id attribute of the object must be set.
      *
      * An exception will be thrown upon erroneous situations.
      *
      * @param budget const reference to the MyMoneyBudget object to be updated
      */
    void removeBudget(const MyMoneyBudget& budget);

    /**
      * This method adds/replaces a price to/from the price list
      */
    void addPrice(const MyMoneyPrice& price);

    /**
      * This method removes a price from the price list
      */
    void removePrice(const MyMoneyPrice& price);

    /**
      * This method retrieves a price from the price list.
      * If @p date is inValid, QDate::currentDate() is assumed.
      */
    MyMoneyPrice price(const QString& fromId, const QString& toId, const QDate& _date, bool exactDate) const;

    /**
      * This method returns a list of all price entries.
      */
    MyMoneyPriceList priceList() const;

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
    bool isReferenced(const MyMoneyObject& obj, const QBitArray& skipCheck) const;

    /**
      * This method recalculates the balances of all accounts
      * based on the transactions stored in the engine.
      */
    void rebuildAccountBalances();

    MyMoneyMoney stockSplit(const QString& accountId, MyMoneyMoney balance, MyMoneyMoney factor, eMyMoney::StockSplitDirection direction) const;

    void startTransaction();
    bool commitTransaction();
    void rollbackTransaction();

    /**
     * This method will close a database and log the use roff
     */
    void close();

private:
    MyMoneyStorageMgrPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(MyMoneyStorageMgr)
};
#endif
