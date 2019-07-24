/*
 * Copyright 2000-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001-2002  Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2004  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2005  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MYMONEYFILE_H
#define MYMONEYFILE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// Project Includes
#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"

/**
  * @author Thomas Baumgart, Michael Edwardes, Kevin Tambascio, Christian Dávid
  */

/**
  * This class represents the interface to the MyMoney engine.
  * For historical reasons it is still called MyMoneyFile.
  * It is implemented using the singleton pattern and thus only
  * exists once for each running instance of an application.
  *
  * The instance of the MyMoneyFile object is accessed as follows:
  *
  * @code
  * MyMoneyFile *file = MyMoneyFile::instance();
  * file->anyMemberFunction();
  * @endcode
  *
  * The first line of the above code creates a unique MyMoneyFile
  * object if it is called for the first time ever. All subsequent
  * calls to this functions return a pointer to the object created
  * during the first call.
  *
  * As the MyMoneyFile object represents the business logic, a storage
  * manager must be attached to it. This mechanism allows to use different
  * access methods to store the objects. The interface to access such an
  * storage manager is defined in the class MyMoneyStorageMgr. The methods
  * attachStorage() and detachStorage() are used to attach/detach a
  * storage manager object. The following code can be used to create a
  * functional MyMoneyFile instance:
  *
  * @code
  * MyMoneyStorageMgr *storage = ....
  * MyMoneyFile *file = MyMoneyFile::instance();
  * file->attachStorage(storage);
  * @endcode
  *
  * The methods addAccount(), modifyAccount() and removeAccount() implement the
  * general account maintenance functions. The method reparentAccount() is
  * available to move an account from one superordinate account to another.
  * account() and accountList() are used to retrieve a single instance or a
  * QList of MyMoneyAccount objects.
  *
  * The methods addInstitution(), modifyInstitution() and removeInstitution()
  * implement the general institution maintenance functions. institution() and
  * institutionList() are used to retrieve a single instance or a
  * QList of MyMoneyInstitution objects.
  *
  * The methods addPayee(), modifyPayee() and removePayee()
  * implement the general payee maintenance functions.
  * payee() and payeeList() are used to retrieve a single instance or a
  * QList of MyMoneyPayee objects.
  *
  * The methods addTag(), modifyTag() and removeTag()
  * implement the general tag maintenance functions.
  * tag() and tagList() are used to retrieve a single instance or a
  * QList of MyMoneyTag objects.
  *
  * The methods addTransaction(), modifyTransaction() and removeTransaction()
  * implement the general transaction maintenance functions.
  * transaction() and transactionList() are used to retrieve
  * a single instance or a QList of MyMoneyTransaction objects.
  *
  * The methods addSecurity(), modifySecurity() and removeSecurity()
  * implement the general access to equities held in the engine.
  *
  * The methods addCurrency(), modifyCurrency() and removeCurrency()
  * implement the general access to multiple currencies held in the engine.
  * The methods baseCurrency() and setBaseCurrency() allow to retrieve/set
  * the currency selected by the user as base currency. If a currency
  * reference is empty, it will usually be interpreted as baseCurrency().
  *
  * The methods liability(), asset(), expense(), income() and equity() are
  * used to retrieve the five standard accounts. isStandardAccount()
  * checks if a given accountId references one of the or not.
  * setAccountName() is used to specify a name for the standard accounts
  * from the GUI.
  *
  * The MyMoneyFile object emits the dataChanged() signal when data
  * has been changed.
  *
  * For arbitrary values that have to be stored with the storage object
  * but are of importance to the application only, the object is derived
  * for MyMoneyKeyValueContainer which provides a container to store
  * these values indexed by an alphanumeric key.
  *
  * @exception MyMoneyException is thrown whenever an error occurs
  * while the engine code is running. The MyMoneyException:: object
  * describes the problem.
  */
template <class Key, class T> class QMap;
class QString;
class QStringList;
class QBitArray;
class MyMoneyStorageMgr;
class MyMoneyCostCenter;
class MyMoneyAccount;
class MyMoneyInstitution;
class MyMoneySecurity;
class MyMoneyPrice;
typedef QPair<QString, QString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;
class MyMoneySchedule;
class MyMoneyTag;
class MyMoneyPayee;
class MyMoneyBudget;
class MyMoneyReport;
class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyObject;
class MyMoneyTransaction;
class MyMoneyTransactionFilter;
class onlineJob;

/// @todo add new models here
class PayeesModel;
class CostCenterModel;
class SchedulesModel;
class TagsModel;
class SecuritiesModel;
class BudgetsModel;
class AccountsModel;
class InstitutionsModel;
class JournalModel;

namespace eMyMoney { namespace Account { enum class Type; }
                     namespace File { enum class Object; }
                     namespace Schedule { enum class Type;
                                          enum class Occurrence;
                                          enum class PaymentType; }
                     namespace TransactionFilter { enum class State; }
                   }

class KMM_MYMONEY_EXPORT MyMoneyFile : public QObject
{
  Q_OBJECT
  KMM_MYMONEY_UNIT_TESTABLE

public:
  friend class MyMoneyNotifier;

  /**
    * This is the function to access the MyMoneyFile object.
    * It returns a pointer to the single instance of the object.
    */
  static MyMoneyFile* instance();

  /**
    * This is the destructor for any MyMoneyFile object
    */
  ~MyMoneyFile();

  // general get functions
  MyMoneyPayee user() const;

  // general set functions
  void setUser(const MyMoneyPayee& user);

  /**
    * This method is used to attach a storage object to the MyMoneyFile object
    * Without an attached storage object, the MyMoneyFile object is
    * of no use.
    *
    * After successful completion, the dataChanged() signal is emitted.
    *
    * In case of an error condition, an exception is thrown.
    * The following error conditions are checked:
    *
    * - @a storage is not equal to 0
    * - there is no other @a storage object attached (use detachStorage()
    *   to revert the attachStorage() operation.
    *
    * @param storage pointer to object that implements the MyMoneyStorageMgr
    *                interface.
    *
    * @sa detachStorage()
    */
  void attachStorage(MyMoneyStorageMgr* const storage);

  /**
    * This method is used to detach a previously attached storage
    * object from the MyMoneyFile object. If no storage object
    * is attached to the engine, this is a NOP.
    *
    * @param storage pointer to object that implements the MyMoneyStorageMgr
    *                interface.
    *
    * @sa attachStorage()
    */
  void detachStorage(MyMoneyStorageMgr* const storage = 0);

  /**
    * This method returns whether a storage is currently attached to
    * the engine or not.
    *
    * @return true if storage object is attached, false otherwise
    */
  bool storageAttached() const;

  /**
    * This method returns a pointer to the storage object
    *
    * @return const pointer to the current attached storage object.
    *         If no object is attached, returns 0.
    */
  MyMoneyStorageMgr* storage() const;

  /**
   * This method clears all data in all storage models
   */
  void unload();

  /**
    * This method must be called before any single change or a series of changes
    * in the underlying storage area is performed.
    * Once all changes are complete (i.e. the transaction is completed),
    * commitTransaction() must be called to finalize all changes. If an error occurs
    * during the processing of the changes call rollbackTransaction() to undo the
    * changes done so far.
    */
  void startTransaction();

  /**
    * This method returns whether a transaction has been started (@a true)
    * or not (@a false).
    */
  bool hasTransaction() const;

  /**
    * @sa startTransaction()
    */
  void commitTransaction();

  /**
    * @sa startTransaction()
    */
  void rollbackTransaction();

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

  /**
    * This method returns the account information for the opening
    * balances account for the given @p security. If the respective
    * account does not exist, it will be created. The name is constructed
    * using MyMoneyFile::openingBalancesPrefix() and appending " (xxx)" in
    * case the @p security is not the baseCurrency(). The account created
    * will be a sub-account of the standard equity account provided by equity().
    *
    * @param security Security for which the account is searched
    *
    * @return The opening balance account
    *
    * @note No notifications will be sent!
    */
  MyMoneyAccount openingBalanceAccount(const MyMoneySecurity& security);

  /**
    * This method is essentially the same as the above, except it works on
    * const objects.  If there is no opening balance account, this method
    * WILL NOT create one.  Instead it will thrown an exception.
    *
    * @param security Security for which the account is searched
    *
    * @return The opening balance account
    *
    * @note No notifications will be sent!
    */
  MyMoneyAccount openingBalanceAccount(const MyMoneySecurity& security) const;

  /**
    * Create an opening balance transaction for the account @p acc
    * with a value of @p balance. If the corresponding opening balance account
    * for the account's currency does not exist it will be created. If it exists
    * and it's opening date is later than the opening date of @p acc,
    * the opening date of the opening balances account will be adjusted to the
    * one of @p acc.
    *
    * @param acc reference to account for which the opening balance transaction
    *            should be created
    * @param balance reference to the value of the opening balance transaction
    *
    * @returns The created MyMoneyTransaction object. In case no transaction has been
    *          created, the id of the object is empty.
    */
  MyMoneyTransaction createOpeningBalanceTransaction(const MyMoneyAccount& acc, const MyMoneyMoney& balance);

  /**
    * Retrieve the opening balance transaction for the account @p acc.
    * If there is no opening balance transaction, QString() will be returned.
    *
    * @param acc reference to account for which the opening balance transaction
    *            should be retrieved
    * @return QString id for the transaction, or QString() if no transaction exists
    */
  QString openingBalanceTransaction(const MyMoneyAccount& acc) const;

  /**
    * This method returns an indicator if the MyMoneyFile object has been
    * changed after it has last been saved to permanent storage.
    *
    * @return true if changed, false if not
    */
  bool dirty() const;

  /**
    * This method is used to force the attached storage object to
    * be dirty. This is used by the application to re-set the dirty
    * flag after a failed upload to a server when the save operation
    * to a local temp file was OK.
    */
  void setDirty() const;

  /**
    * Adds an institution to the file-global institution pool. A
    * respective institution-ID will be generated for this object.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.

    * @param institution The complete institution information in a
    *        MyMoneyInstitution object
    */
  void addInstitution(MyMoneyInstitution& institution);

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
    * Deletes an existing institution from the file global institution pool
    * Also modifies the accounts that reference this institution as
    * their institution.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param institution institution to be deleted.
    */
  void removeInstitution(const MyMoneyInstitution& institution);

  void createAccount(MyMoneyAccount& newAccount, MyMoneyAccount& parentAccount, MyMoneyAccount& brokerageAccount, MyMoneyMoney openingBal);
  /**
    * Adds an account to the file-global account pool. A respective
    * account-ID will be generated within this record. The modified
    * members of @a account will be updated.
    *
    * A few parameters of the account to be added are checked against
    * the following conditions. If they do not match, an exception is
    * thrown.
    *
    * An account must match the following conditions:
    *
    *   a) the account must have a name with length > 0
    *   b) the account must not have an id assigned
    *   c) the transaction list must be empty
    *   d) the account must not have any sub-ordinate accounts
    *   e) the account must have no parent account
    *   f) the account must not have any reference to a MyMoneyFile object
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account The complete account information in a MyMoneyAccount object
    * @param parent  The complete account information of the parent account
    */
  void addAccount(MyMoneyAccount& account, MyMoneyAccount& parent);

  /**
    * Modifies an already existing account in the file global account pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param account reference to the new account information
    */
  void modifyAccount(const MyMoneyAccount& account);

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
    * moves splits from one account to another
    *
    * @param oldAccount id of the current account
    * @param newAccount if of the new account
    *
    * @return the number of modified splits
    */
  unsigned int moveSplits(const QString& oldAccount, const QString& newAccount);

  /**
    * This method is used to determine, if the account with the
    * given ID is referenced by any split in m_transactionList.
    *
    * @param id id of the account to be checked for
    * @return true if account is referenced, false otherwise
    */
  bool hasActiveSplits(const QString& id) const;

  /**
    * This method is used to check whether a given
    * account id references one of the standard accounts or not.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param id account id
    * @return true if account-id is one of the standards, false otherwise
    */
  bool isStandardAccount(const QString& id) const;

  /**
    * Returns @a true, if transaction @p t is a transfer transaction.
    * A transfer transaction has two splits, both referencing either
    * an asset, a liability or an equity account.
    */
  bool isTransfer(const MyMoneyTransaction& t) const;

  /**
    * This method is used to set the name for the specified standard account
    * within the storage area. An exception will be thrown, if an error
    * occurs
    *
    * @param id QString reference to one of the standard accounts.
    * @param name QString reference to the name to be set
    *
    */
  void setAccountName(const QString& id, const QString& name) const;

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
    * Deletes existing accounts and their subaccounts recursively
    * from the global account pool.
    * This method expects that all accounts and their subaccounts
    * are no longer assigned to any transactions or splits.
    * An exception is thrown in case of a problem deleting an account.
    *
    * The optional parameter level is used to keep track of the recursion level.
    * If the recursion level exceeds 100 (some arbitrary number which seems a good
    * maximum), an exception is thrown.
    *
    * @param account_list Reference to a list of account IDs to be deleted.
    * @param level Parameter to keep track of recursion level (do not pass a value here).
    */
  void removeAccountList(const QStringList& account_list, unsigned int level = 0);

  /**
    * This member function checks all accounts identified by account_list
    * and their subaccounts whether they are assigned to transactions/splits or not.
    * The function calls itself recursively with the list of sub-accounts of
    * the currently processed account.
    *
    * The optional parameter level is used to keep track of the recursion level.
    * If the recursion level exceeds 100 (some arbitrary number which seems a good
    * maximum), an exception is thrown.
    *
    * @param account_list  A QStringList with account IDs that need to be checked.
    * @param level         (optional) Optional parameter to indicate recursion level.
    * @return              Returns 'false' if at least one account has been found that
    *                      is still referenced by a transaction.
    */
  bool hasOnlyUnusedAccounts(const QStringList& account_list, unsigned int level = 0);

  /**
    * Adds a transaction to the file-global transaction pool. A respective
    * transaction-ID will be generated for this object. The ID is stored
    * as QString in the object passed as argument.
    * Splits must reference valid accounts and valid payees. The payee
    * id can be empty.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to the transaction
    */
  void addTransaction(MyMoneyTransaction& transaction);

  /**
    * This method is used to update a specific transaction in the
    * transaction pool of the MyMoneyFile object.
    * Splits must reference valid accounts and valid payees. The payee
    * id can be empty.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param transaction reference to transaction to be changed
    */
  void modifyTransaction(const MyMoneyTransaction& transaction);

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
  QList<MyMoneyTransaction> transactionList(MyMoneyTransactionFilter& filter) const;

  void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const;

  void transactionList(QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const;

  /**
    * This method is used to remove a transaction from the transaction
    * pool (journal).
    *
    * @param transaction const reference to transaction to be deleted
    */
  void removeTransaction(const MyMoneyTransaction& transaction);

  /**
    * This method is used to return the actual balance of an account
    * without it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date (default = QDate())
    * @return balance of the account as MyMoneyMoney object
    */
  MyMoneyMoney balance(const QString& id, const QDate& date) const;
  MyMoneyMoney balance(const QString& id) const;

  /**
    * This method is used to return the cleared balance of an account
    * without it's sub-ordinate accounts for a specific date. All
    * recorded  transactions are included in the balance.
    * This method is used by the reconciliation functionality
    *
    * @param id id of the account in question
    * @param date return cleared balance for specific date
    * @return balance of the account as MyMoneyMoney object
    */
  MyMoneyMoney clearedBalance(const QString& id, const QDate& date) const;


  /**
    * This method is used to return the actual balance of an account
    * including it's sub-ordinate accounts. If a @p date is presented,
    * the balance at the beginning of this date (not including any
    * transaction on this date) is returned. Otherwise all recorded
    * transactions are included in the balance.
    *
    * @param id id of the account in question
    * @param date return balance for specific date (default = QDate())
    * @return balance of the account as MyMoneyMoney object
    */
  MyMoneyMoney totalBalance(const QString& id, const QDate& date) const;
  MyMoneyMoney totalBalance(const QString& id) const;

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
  unsigned int transactionCount(const QString& account) const;
  unsigned int transactionCount() const;

  /**
    * This method returns a QMap filled with the number of transactions
    * per account. The account id serves as index into the map. If one
    * needs to have all transactionCounts() for many accounts, this method
    * is faster than calling transactionCount(const QString& account) many
    * times.
    *
    * @return QMap with numbers of transactions per account
    */
  QMap<QString, unsigned long> transactionCountMap() const;

  /**
    * This method returns the number of institutions currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of institutions known to file
    */
  unsigned int institutionCount() const;

  /**
    * This method returns the number of accounts currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of accounts currently known inside a MyMoneyFile object
    */
  unsigned int accountCount() const;

  /**
    * Returns the institution of a given ID
    *
    * @param id id of the institution to locate
    * @return MyMoneyInstitution object filled with data. If the institution
    *         could not be found, an exception will be thrown
    */
  MyMoneyInstitution institution(const QString& id) const;

  /**
    * This method returns a list of the institutions
    * inside a MyMoneyFile object. This is a convenience method
    * to the one above
    *
    * @return QList containing the institution objects
    */
  QList<MyMoneyInstitution> institutionList() const;

  /**
    * Returns the account addressed by its id.
    *
    * @param id id of the account to locate.
    * @return MyMoneyAccount object carrying the @p id. An exception is thrown
    *         if the id is unknown
    */
  MyMoneyAccount account(const QString& id) const;

  /**
   * Returns the account addressed by its name.
   *
   * @param name  name of the account to locate.
   * @return First MyMoneyAccount object found carrying the @p name.
   * An empty MyMoneyAccount object will be returned if the name is not found.
   */
  MyMoneyAccount accountByName(const QString& name) const;

  /**
   * Returns the sub-account addressed by its name.
   *
   * @param acc account to search in
   * @param name  name of the account to locate.
   * @return First MyMoneyAccount object found carrying the @p name.
   * An empty MyMoneyAccount object will be returned if the name is not found.
   */
  MyMoneyAccount subAccountByName(const MyMoneyAccount& account, const QString& name) const;

  /**
    * This method returns a list of accounts inside a MyMoneyFile object.
    * An optional parameter is a list of id's. If this list is empty (the default)
    * the returned list contains all accounts, otherwise only those referenced
    * in the id-list.
    *
    * @param list reference to QList receiving the account objects
    * @param idlist QStringList of account ids of those accounts that
    *        should be returned. If this list is empty, all accounts
    *        currently known will be returned.
    *
    * @param recursive if @p true, then recurse in all found accounts. The default is @p false
    */
  void accountList(QList<MyMoneyAccount>& list, const QStringList& idlist = QStringList(), const bool recursive = false) const;

  /**
    * This method is used to convert an account id to a string representation
    * of the names which can be used as a category description. If the account
    * is part of a hierarchy, the category name will be the concatenation of
    * the single account names separated by MyMoneyFile::AccountSeparator.
    *
    * @param accountId QString reference of the account's id
    * @param includeStandardAccounts if true, the standard top account will be part
    *                   of the name, otherwise it will not be included (default is @c false)
    *
    * @return QString of the constructed name.
    */
  QString accountToCategory(const QString& accountId, bool includeStandardAccounts = false) const;

  /**
    * This method is used to convert a string representing a category to
    * an account id. A category can be the concatenation of multiple accounts
    * representing a hierarchy of accounts. They have to be separated by
    * MyMoneyFile::AccountSeparator.
    *
    * @param category const reference to QString containing the category
    * @param type account type if a specific type is required (defaults to Unknown)
    *
    * @return QString of the corresponding account. If account was not found
    *         the return value will be an empty string.
    */
  QString categoryToAccount(const QString& category, eMyMoney::Account::Type type) const;
  QString categoryToAccount(const QString& category) const;

  /**
    * This method is used to convert a string representing an asset or
    * liability account to an account id. An account name can be the
    * concatenation of multiple accounts representing a hierarchy of
    * accounts. They have to be separated by MyMoneyFile::AccountSeparator.
    *
    * @param name const reference to QString containing the account name
    *
    * @return QString of the corresponding account. If account was not found
    *         the return value will be an empty string.
    */
  QString nameToAccount(const QString& name) const;

  /**
    * This method is used to extract the parent part of an account hierarchy
    * name who's parts are separated by MyMoneyFile::AccountSeparator.
    *
    * @param name full account name
    * @return parent name (full account name excluding the last part)
    */
  QString parentName(const QString& name) const;

  /**
    * This method is used to create a new payee
    *
    * An exception will be thrown upon error conditions
    *
    * @param payee MyMoneyPayee reference to payee information
    */
  void addPayee(MyMoneyPayee& payee);

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
    * @return MyMoneyPayee object of payee
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
    * This method is used to remove an existing payee.
    * An error condition occurs, if the payee is still referenced
    * by a split.
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
   * The payees model instance
   */
  PayeesModel* payeesModel() const;

  /**
   * The costcenter model instance
   */
  CostCenterModel* costCenterModel() const;

  /**
   * The schedules model instance
   */
  SchedulesModel* schedulesModel() const;

  /**
   * The tags model instance
   */
  TagsModel* tagsModel() const;

  /**
   * The securities model instance
   */
  SecuritiesModel* securitiesModel() const;

  /**
   * The currencies model instance
   */
  SecuritiesModel* currenciesModel() const;

  /**
   * The budgets model instance
   */
  BudgetsModel* budgetsModel() const;

  /**
   * The accounts model instance
   */
  AccountsModel* accountsModel() const;

  /**
   * The institutions model instance
   */
  InstitutionsModel* institutionsModel() const;

  /**
   * The journal model instance
   */
  JournalModel* journalModel() const;


  /// @todo add new models here
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
    * @return MyMoneyTag object of tag
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
    * This method is used to remove an existing tag.
    * An error condition occurs, if the tag is still referenced
    * by a split.
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
    * This method returns a list of the cost centers
    * inside a MyMoneyStorage object
    *
    * @return QList<MyMoneyCostCenter> containing the cost center information
    */
    void costCenterList(QList< MyMoneyCostCenter >& list) const;

  /**
    * This method is used to extract a value from the storage's
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::value().
    * @note Do not use this method to return the value of the key @p kmm-id. Use
    * storageId() instead.
    *
    * @param key const reference to QString containing the key
    * @return QString containing the value
    */
  QString value(const QString& key) const;

  /**
    * This method is used to set a value in the storage's
    * KeyValueContainer. For details see MyMoneyKeyValueContainer::setValue().
    *
    * @param key const reference to QString containing the key
    * @param val const reference to QString containing the value
    *
    * @note Keys starting with the leading @p kmm- are reserved for internal use
    *       by the MyMoneyFile object.
    */
  void setValue(const QString& key, const QString& val);

  /**
   * This method returns the unique id of the attached storage object.
   * In case the storage object does not have an id yet, a new one will be
   * assigned.
   *
   * @return QString containing the value
   *
   * An exception is thrown if no storage object is attached.
   */
  QString storageId();

  /**
    * This method is used to delete a key-value-pair from the
    * storage's KeyValueContainer identified by the parameter
    * @p key. For details see MyMoneyKeyValueContainer::deletePair().
    *
    * @param key const reference to QString containing the key
    */
  void deletePair(const QString& key);

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
    * @return const QList<MyMoneySchedule> list of schedule objects.
    */
  QList<MyMoneySchedule> scheduleList(const QString& accountId,
      const eMyMoney::Schedule::Type type,
      const eMyMoney::Schedule::Occurrence occurrence,
      const eMyMoney::Schedule::PaymentType paymentType,
      const QDate& startDate,
      const QDate& endDate,
      const bool overdue) const;
  QList<MyMoneySchedule> scheduleList(const QString& accountId) const;
  QList<MyMoneySchedule> scheduleList() const;

  QStringList consistencyCheck();

  /**
    * MyMoneyFile::openingBalancesPrefix() is a special string used
    * to generate the name for opening balances accounts. See openingBalanceAccount()
    * for details.
    */
  static QString openingBalancesPrefix();

  /**
    * MyMoneyFile::AccountSeparator is used as the separator
    * between account names to form a hierarchy.
    */
  static const QString AccountSeparator;

  /**
    * createCategory creates a category from a text name.
    *
    * The whole account hierarchy is created if it does not
    * already exist.  e.g if name = Bills:Credit Card and
    * base = expense(), Bills will first be checked to see if
    * it exists and created if not.  Credit Card will then
    * be created with Bills as it's parent.  The Credit Card account
    * will have it's id returned.
    *
    * @param base The base account (expense or income)
    * @param name The category to create
    *
    * @return The category account id or empty on error.
    *
    * @exception An exception will be thrown, if @p base is not equal
    *            expense() or income().
    **/
  QString createCategory(const MyMoneyAccount& base, const QString& name);

  /**
    * This method is used to get the account id of the split for
    * a transaction from the text found in the QIF $ or L record.
    * If an account with the name is not found, the user is asked
    * if it should be created.
    *
    * @param name name of account as found in the QIF file
    * @param value value found in the T record
    * @param value2 value found in the $ record for split transactions
    *
    * @return id of the account for the split. If no name is specified
    *            or the account was not found and not created the
    *            return value will be "".
    */
  QString checkCategory(const QString& name, const MyMoneyMoney& value, const MyMoneyMoney& value2);

  /**
    * This method is used to add a new security object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param security reference to the MyMoneySecurity object
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
    * If no security with the given id is found, then a corresponding
    * currency is searched. If @p id is empty, the baseCurrency() is returned.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySecurity object
    * @return MyMoneySecurity object
    */
  MyMoneySecurity security(const QString& id) const;

  /**
    * This method is used to retrieve a list of all MyMoneySecurity objects.
    */
  QList<MyMoneySecurity> securityList() const;

  /**
    * This method is used to add a new currency object to the engine.
    * The ID of the object is the trading symbol, so there is no need for an additional
    * ID since the symbol is guaranteed to be unique.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void addCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to modify an existing MyMoneySecurity
    * object.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void modifyCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to remove an existing MyMoneySecurity object
    * from the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency reference to the MyMoneySecurity object
    */
  void removeCurrency(const MyMoneySecurity& currency);

  /**
    * This method is used to retrieve a single MyMoneySecurity object.
    * The id of the object must be supplied in the parameter @p id.
    * If @p id is empty, this method returns baseCurrency(). In case
    * no currency is found, @p id is searched in the loaded set of securities.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param id QString containing the id of the MyMoneySchedule object
    * @return MyMoneySchedule object
    */
  MyMoneySecurity currency(const QString& id) const;

  /**
    * This method is used to retrieve the map of ancient currencies (together with their last prices)
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QMap of all MyMoneySecurity and MyMoneyPrice objects.
    */
  QMap<MyMoneySecurity, MyMoneyPrice> ancientCurrencies() const;

  /**
    * This method is used to retrieve the list of available currencies
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneySecurity objects.
    */
  QList<MyMoneySecurity> availableCurrencyList() const;

  /**
    * This method is used to retrieve the list of stored currencies
    * known to the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return QList of all MyMoneySecurity objects.
    */
  QList<MyMoneySecurity> currencyList() const;

  /**
    * This method retrieves a MyMoneySecurity object representing
    * the selected base currency. If the base currency is not
    * selected (e.g. due to a previous call to setBaseCurrency())
    * a standard MyMoneySecurity object will be returned. See
    * MyMoneySecurity() for details.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @return MyMoneySecurity describing base currency
    */
  MyMoneySecurity baseCurrency() const;

  /**
    * This method returns the foreign currency of the given two
    * currency ids. If second is the base currency id then @a first
    * is returned otherwise @a second is returned.
    */
  QString foreignCurrency(const QString& first, const QString& second) const;

  /**
    * This method allows to select the base currency. It does
    * not perform any changes to the data in the engine. It merely
    * stores a reference to the base currency. The currency
    * passed as argument must exist in the engine.
    *
    * An exception will be thrown upon erroneous situations.
    *
    * @param currency
    */
  void setBaseCurrency(const MyMoneySecurity& currency);

  /**
    * This method adds/replaces a price to/from the price list
    */
  void addPrice(const MyMoneyPrice& price);

  /**
    * This method removes a price from the price list
    */
  void removePrice(const MyMoneyPrice& price);

  /**
    * This method is used to retrieve a price for a specific security
    * on a specific date. If there is no price for this date, the last
    * known price for this currency is used. If no price information
    * is available, 1.0 will be returned as price.
    *
    * @param fromId the id of the currency in question
    * @param toId the id of the currency to convert to (if empty, baseCurrency)
    * @param date the date for which the price should be returned (default = today)
    * @param exactDate if true, entry for date must exist, if false any price information
    *                  with a date less or equal to @p date will be returned
    *
    * @return price found as MyMoneyPrice object
    * @note This throws an exception when the base currency is not set and toId is empty
    */
  MyMoneyPrice price(const QString& fromId, const QString& toId, const QDate& date, const bool exactDate = false) const;
  MyMoneyPrice price(const QString& fromId, const QString& toId) const;
  MyMoneyPrice price(const QString& fromId) const;

  /**
    * This method returns a list of all prices.
    *
    * @return MyMoneyPriceList of all MyMoneyPrice objects.
    */
  MyMoneyPriceList priceList() const;

  /**
    * This method allows to interrogate the engine, if a known account
    * with id @p id has a subaccount with the name @p name.
    *
    * @param id id of the account to look at
    * @param name account name that needs to be searched force
    * @retval true account with name @p name found as subaccounts
    * @retval false no subaccount present with that name
    */
  bool hasAccount(const QString& id, const QString& name) const;

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
    * Adds a report to the file-global institution pool. A
    * respective report-ID will be generated for this object.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param report The complete report information in a
    *        MyMoneyReport object
    */
  void addReport(MyMoneyReport& report);

  /**
    * Modifies an already existing report in the file global
    * report pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param report The complete new report information
    */
  void modifyReport(const MyMoneyReport& report);

  /**
    * This method returns the number of reports currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of reports known to file
    */
  unsigned countReports() const;

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
    * Adds a budget to the file-global institution pool. A
    * respective budget-ID will be generated for this object.
    * The ID is stored as QString in the object passed as argument.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param budget The complete budget information in a
    *        MyMoneyBudget object
    */
  void addBudget(MyMoneyBudget &budget);


  /**
    * This method is used to retrieve the id to a corresponding
    * name of a budget.
    * An exception will be thrown upon error conditions.
    *
    * @param budget QString reference to name of budget
    *
    * @return MyMoneyBudget reference to object of budget
    */
  MyMoneyBudget budgetByName(const QString& budget) const;


  /**
    * Modifies an already existing budget in the file global
    * budget pool.
    *
    * An exception will be thrown upon error conditions.
    *
    * @param budget The complete new budget information
    */
  void modifyBudget(const MyMoneyBudget& budget);

  /**
    * This method returns the number of budgets currently known to file
    * in the range 0..MAXUINT
    *
    * @return number of budgets known to file
    */
  unsigned countBudgets() const;

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
    * This method is used to add a VAT split to a transaction.
    *
    * @param transaction reference to the transaction
    * @param account reference to the account
    * @param category reference to the category
    * @param amount reference to the amount of the VAT split
    *
    * @return true if a VAT split has been added
    */
  bool addVATSplit(MyMoneyTransaction& transaction, const MyMoneyAccount& account, const MyMoneyAccount& category, const MyMoneyMoney& amount) const;

  /**
    * This method checks if the @a transaction has two or three splits and
    * one references an asset or liability, one references an income or expense and one
    * references a tax account. All three accounts must be denominated in the same
    * transaction.commodity(). Also, if there is a tax split it must reference the
    * same account as the one that is assigned to the income/expense category.
    *
    * If that all matches, the @a transaction is updated such that the amount
    * of the asset/liability is split according to the tax settings.
    *
    * @param transaction reference to the transaction
    */
  void updateVAT(MyMoneyTransaction& transaction) const;

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
  bool isReferenced(const MyMoneyObject& obj) const;

  /**
    * Returns true if any of the accounts referenced by the splits
    * of transaction @a t is closed.
    */
  bool referencesClosedAccount(const MyMoneyTransaction& t) const;

  /**
    * Returns true if the accounts referenced by the split @a s is closed.
    */
  bool referencesClosedAccount(const MyMoneySplit& s) const;

  /**
    * This method checks if the given check no &p no is used in
    * a transaction referencing account &p accId. If  @p accId is empty,
    * @p false is returned.
    *
    * @param accId id of account to checked
    * @param no check number to be verified if used or not
    * @retval false @p no is not in use
    * @retval true @p no is already assigned
    */
  bool checkNoUsed(const QString& accId, const QString& no) const;

  /**
    * This method returns the highest assigned check no for
    * account @p accId.
    *
    * @param accId id of account to be scanned
    * @return highest check no. used
    */
  QString highestCheckNo(const QString& accId) const;


  /**
    * This method checks if there is a transaction
    * after the date @p date for account @p accId.
    *
    * @param accId id of account to be scanned
    * @param date date to compare with
    * @retval false if there is no transaction after @p date
    * @retval true if there is a transaction after @p date
    */
  bool hasNewerTransaction(const QString& accId, const QDate& date) const;


  /**
    * Clear all internal caches (used internally for performance measurements)
    */
  void clearCache();

  void forceDataChanged();

  /**
    * This returns @p true if file and online balance of a specific
    * @p account are matching. Returns false if there is no online balance.
    *
    * @param account @p account to be checked
    * @retval false if @p account has balance mismatch or if there is no online balance.
    * @retval true if @p account has matching balances
    */
  bool hasMatchingOnlineBalance(const MyMoneyAccount& account) const;

  /**
    * This returns the number of transactions of a specific reconciliation state @p state of account with id @p accId.
    *
    * @param accId @p accId is the account id of account to be checked
    * @param state @p state reconciliation state
    * @return number of transactions with state @p state
    */
  int countTransactionsWithSpecificReconciliationState(const QString& accId, eMyMoney::TransactionFilter::State state) const;
  QMap< QString, QVector<int> > countTransactionsWithSpecificReconciliationState() const;

  /**
   * @brief Saves a new onlineJob
   * @param job you stay owner of the object (a copy will be created)
   */
  void addOnlineJob(onlineJob& job);

  /**
   * @brief Saves a onlineJob
   * @param job you stay owner of the object (a copy will be created)
   */
  void modifyOnlineJob(const onlineJob job);

  /**
   * @brief Returns onlineJob identified by jobId
   * @param jobId
   * @return
   */
  onlineJob getOnlineJob(const QString &jobId) const;

  /**
   * @brief Returns all onlineJobs
   * @return all online jobs, caller gains ownership
   */
  QList<onlineJob> onlineJobList() const;

  /**
   * @brief Returns the number of onlineJobs
   */
  int countOnlineJobs() const;

  /**
   * @brief Remove onlineJob
   *
   * @note Removing an onlineJob fails if it is locked
   */
  void removeOnlineJob(const onlineJob& job);

  /**
   * @brief Removes multiple onlineJobs by id
   *
   * @note Removing an onlineJob fails if it is locked
   */
  void removeOnlineJob(const QStringList onlineJobIds);

protected:
  /**
    * This is the constructor for a new empty file description
    */
  MyMoneyFile();

Q_SIGNALS:
  /**
   * This signal is emitted when a transaction has been committed and
   * the notifications are about to be sent out.
   */
  void beginChangeNotification();

  /**
   * This signal is emitted when a transaction has been committed and
   * all notifications have been sent out.
   */
  void endChangeNotification();

  /**
    * This signal is emitted whenever any data has been changed in the engine
    * via any of the methods of this object
    */
  void dataChanged();

  /**
    * This signal is emitted by the engine whenever a new object
    * had been added. The data for the new object is contained in
    * @a obj.
    */
  void objectAdded(eMyMoney::File::Object objType, const QString& id);

  /**
    * This signal is emitted by the engine whenever an object
    * had been removed.
    *
    * @note: The data contained in @a obj is only for reference
    * purposes and should not be used to call any MyMoneyFile
    * method anymore as the object is already deleted in the storage
    * when the signal is emitted.
    */
  void objectRemoved(eMyMoney::File::Object objType, const QString& id);

  /**
    * This signal is emitted by the engine whenever an object
    * had been changed. The new state of the object is contained
    * in @a obj.
    */
  void objectModified(eMyMoney::File::Object objType, const QString& id);

  /**
    * This signal is emitted by the engine whenever the balance
    * of an account had been changed by adding, modifying or
    * removing transactions from the MyMoneyFile object.
    */
  void balanceChanged(const MyMoneyAccount& acc);

  /**
    * This signal is emitted by the engine whenever the value
    * of an account had been changed by adding or removing
    * prices from the MyMoneyFile object.
    */
  void valueChanged(const MyMoneyAccount& acc);

  /**
   * This signal is emitted once all data of a new backend is loaded.
   */
  void modelsLoaded();

  /**
   * This signal is emitted once all data of a new backend is ready to be used.
   * The difference to modelsLoaded() is that some internal fixes can be applied
   * in the meantime. Right before this signal is sent out, the dirty flag of all
   * models will be reset.
   */
  void modelsReadyToUse();

private:
  static MyMoneyFile file;

  MyMoneyFile& operator=(MyMoneyFile&); // not allowed for singleton
  MyMoneyFile(const MyMoneyFile&);      // not allowed for singleton

  QString locateSubAccount(const MyMoneyAccount& base, const QString& category) const;

  void ensureDefaultCurrency(MyMoneyAccount& acc) const;

  void warningMissingRate(const QString& fromId, const QString& toId) const;

  /**
    * This method creates an opening balances account. The name is constructed
    * using MyMoneyFile::openingBalancesPrefix() and appending " (xxx)" in
    * case the @p security is not the baseCurrency(). The account created
    * will be a sub-account of the standard equity account provided by equity().
    *
    * @param security Security for which the account is searched
    */
  MyMoneyAccount createOpeningBalanceAccount(const MyMoneySecurity& security);

  MyMoneyAccount openingBalanceAccount_internal(const MyMoneySecurity& security) const;

  /**
   * Make sure that the splits value has the precision of the corresponding account
   */
  void fixSplitPrecision(MyMoneyTransaction& t) const;

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

class MyMoneyFileTransactionPrivate;
class KMM_MYMONEY_EXPORT MyMoneyFileTransaction
{
  Q_DISABLE_COPY(MyMoneyFileTransaction)

public:
  MyMoneyFileTransaction();
  ~MyMoneyFileTransaction();

  /**
   * Commit the current transaction.
   *
   * @warning Make sure not to use any variable that might have been altered by
   *          the transaction. Please keep in mind, that changing transactions
   *          can also affect account objects. If you still need those variables
   *          just reload them from the engine.
   */
  void commit();
  void rollback();
  void restart();

private:
  MyMoneyFileTransactionPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(MyMoneyFileTransaction)
};

#endif

