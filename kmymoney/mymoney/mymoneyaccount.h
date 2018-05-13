
/***************************************************************************
                          mymoneyaccount.h
                          -------------------
    copyright            : (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef MYMONEYACCOUNT_H
#define MYMONEYACCOUNT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneykeyvaluecontainer.h"
#include "mymoneyobject.h"
#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"

class QString;
class QDate;
class QPixmap;
class QDomElement;
class MyMoneySecurity;
class MyMoneyMoney;
class MyMoneySplit;
class payeeIdentifier;
namespace payeeIdentifiers { class ibanBic; }
namespace eMyMoney { namespace Account { enum class Type; } }
template <class T> class payeeIdentifierTyped;

/**
  * A representation of an account.
  * This object represents any type of account, those held at an
  * institution as well as the accounts used for double entry
  * accounting.
  *
  * Currently, the following account types are known:
  *
  * @li  Unknown
  * @li  Checkings
  * @li  Savings
  * @li  Cash
  * @li  CreditCard
  * @li  Loan (collected)
  * @li  CertificateDep
  * @li  Investment
  * @li  MoneyMarket
  * @li  Currency
  * @li  Asset
  * @li  Liability
  * @li  Income
  * @li  Expense
  * @li  Loan (given)
  * @li  Stock
  * @li  Equity
  *
  * @see MyMoneyInstitution
  * @see MyMoneyFile
  *
  * @author Michael Edwardes 2000-2001
  * @author Thomas Baumgart 2002
  * @author Łukasz Wojniłowicz 2017
  *
**/

class MyMoneyAccountPrivate;
class KMM_MYMONEY_EXPORT MyMoneyAccount : public MyMoneyObject, public MyMoneyKeyValueContainer /*, public MyMoneyPayeeIdentifierContainer */
{
  Q_DECLARE_PRIVATE_D(MyMoneyObject::d_ptr, MyMoneyAccount)

  KMM_MYMONEY_UNIT_TESTABLE

public:
  /**
    * This is the constructor for a new empty account
    */
  MyMoneyAccount();

  /**
    * This is the constructor for a new account known to the current file
    * This is the only constructor that will set the attribute m_openingDate
    * to a correct value.
    *
    * @param id id assigned to the account
    * @param right account definition
    */
  MyMoneyAccount(const QString& id,
                 const MyMoneyAccount& other);

  /**
    * This is the constructor for an account that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  explicit MyMoneyAccount(const QDomElement& el);

  MyMoneyAccount(const MyMoneyAccount & other);
  MyMoneyAccount(MyMoneyAccount && other);
  MyMoneyAccount & operator=(MyMoneyAccount other);
  friend void swap(MyMoneyAccount& first, MyMoneyAccount& second);

  /**
    * This is the destructor for any MyMoneyAccount object
    */
  ~MyMoneyAccount();

  /**
    * This operator tests for equality of two MyMoneyAccount objects
    */
  bool operator == (const MyMoneyAccount &) const;

  /**
    * This converts the account type into one of the four
    * major account types liability, asset, expense or income.
    *
    * The current assignment is as follows:
    *
    * - Asset
    *   - Asset
    *   - Checkings
    *   - Savings
    *   - Cash
    *   - Currency
    *   - Investment
    *   - MoneyMarket
    *   - CertificateDep
    *   - AssetLoan
    *   - Stock
    *
    * - Liability
    *   - Liability
    *   - CreditCard
    *   - Loan
    *
    * - Income
    *   - Income
    *
    * - Expense
    *   - Expense
    *
    * @return accountTypeE of major account type
    */
  eMyMoney::Account::Type accountGroup() const;

  /**
    * This method returns the id of the MyMoneyInstitution object this account
    * belongs to.
    * @return id of MyMoneyInstitution object. QString() if it is
    *         an internal account
    * @see setInstitution
    */
  QString institutionId() const;

  /**
    * This method is used to set the id of the institution this account
    * belongs to.
    *
    * @param id id of the institution this account belongs to
    * @see institution
    */
  void setInstitutionId(const QString& id);

  /**
    * This method returns the name of the account
    * @return name of account
    * @see setName()
    */
  QString name() const;

  /**
    * This method is used to set the name of the account
    * @param name name of the account
    * @see name
    */
  void setName(const QString& name);

  /**
    * This method returns the number of the account at the institution
    * @return number of account at the institution
    * @see setNumber
    */
  QString number() const;

  /**
    * This method is used to set the number of the account at the institution
    * @param number number of the account
    * @see number
    */
  void setNumber(const QString& number);

  /**
    * This method returns the descriptive text of the account.
    * @return description of account
    * @see setDescription
    */
  QString description() const;

  /**
    * This method is used to set the descriptive text of the account
    *
    * @param desc text that serves as description
    * @see setDescription
    */
  void setDescription(const QString& desc);

  /**
    * This method returns the opening date of this account
    * @return date of opening of this account as const QDate value
    * @see setOpeningDate()
    */
  QDate openingDate() const;

  /**
    * This method is used to set the opening date information of an
    * account.
    *
    * @param date QDate of opening date
    * @see openingDate
    */
  void setOpeningDate(const QDate& date);

  /**
    * This method returns the date of the last reconciliation of this account
    * @return date of last reconciliation as const QDate value
    * @see setLastReconciliationDate
    */
  QDate lastReconciliationDate() const;

  /**
    * This method is used to set the date of the last reconciliation
    * of an account.
    * @param date QDate of last reconciliation
    * @see lastReconciliationDate
    */
  void setLastReconciliationDate(const QDate& date);

  /**
    * This method returns the date the account was last modified
    * @return date of last modification as const QDate value
    * @see setLastModified
    */
  QDate lastModified() const;

  /**
    * This method is used to modify the date of the last
    * modification access.
    * @param date date of last modification
    * @see lastModified
    */
  void setLastModified(const QDate& date);

  /**
    * This method is used to return the ID of the parent account
    * @return QString with the ID of the parent of this account
    */
  QString parentAccountId() const;

  /**
    * This method is used to set a new parent account id
    * @param parent QString reference to new parent account
    */
  void setParentAccountId(const QString& parent);

  /**
    * This method returns the list of the account id's of
    * subordinate accounts
    * @return QStringList account ids
    */
  QStringList accountList() const;

  /**
    * This method returns the number of entries in the m_accountList
    * @return number of entries in the accountList
    */
  int accountCount() const;

  /**
    * This method is used to add an account id as sub-ordinate account
    * @param account const QString reference to account ID
    */
  void addAccountId(const QString& account);

  /**
    * This method is used to remove an account from the list of
    * sub-ordinate accounts.
    * @param account const QString reference to account ID to be removed
    */
  void removeAccountId(const QString& account);

  /**
    * This method is used to remove all accounts from the list of
    * sub-ordinate accounts.
    */
  void removeAccountIds();

  /**
   * Return the stored account identifiers
   *
   * @internal This method is temporary until MyMoneyAccount is a MyMoneyPayeeIdentifierContainer. But before this
   * can happen the account numbers and iban kvp data must be moved there.
   */
  QList< payeeIdentifier > payeeIdentifiers() const;

  /**
   * @see MyMoneyPayeeIdentifierContainer::payeeIdentifiersByType()
   */
  template< class type >
  KMM_MYMONEY_EXPORT QList< ::payeeIdentifierTyped<type> > payeeIdentifiersByType() const;

  /**
    * This method is used to update m_lastModified to the current date
    */
  void touch();

  /**
    * This method returns the type of the account.
    */
  eMyMoney::Account::Type accountType() const;

  /**
    * This method is used to change the account type
    *
    * @param type account type
    */
  void setAccountType(const eMyMoney::Account::Type type);

  /**
    * This method retrieves the id of the currency used with this account.
    * If the return value is empty, the base currency should be used.
    *
    * @return id of currency
    */
  QString currencyId() const;

  /**
    * This method sets the id of the currency used with this account.
    *
    * @param id ID of currency to be associated with this account.
    */
  void setCurrencyId(const QString& id);

  void writeXML(QDomDocument& document, QDomElement& parent) const override;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  bool hasReferenceTo(const QString& id) const override;

  /**
    * This member returns the balance of this account based on
    * all transactions stored in the journal.
    */
  MyMoneyMoney balance() const;

  /**
    * This method adjusts the balance of this account
    * according to the difference contained in the split @p s.
    * If the s.action() is MyMoneySplit::actionName(eMyMoney::Split::Action::SplitShares) then
    * the balance will be adjusted accordingly.
    *
    * @param s const reference to MyMoneySplit object containing the
    *             value to be added/subtracted to/from the balance
    * @param reverse add (false) or subtract (true) the shares contained in the split.
    *          It also affects the balance for share splits in the opposite direction.
    */
  void adjustBalance(const MyMoneySplit& s, bool reverse = false);

  /**
    * This method sets the balance of this account
    * according to the value provided by @p val.
    *
    * @param val const reference to MyMoneyMoney object containing the
    *             value to be assigned to the balance
    */
  void setBalance(const MyMoneyMoney& val);

  /**
    * This method sets the kvp's for online banking with this account
    *
    * @param values The container of kvp's needed when connecting to this account
    */
  void setOnlineBankingSettings(const MyMoneyKeyValueContainer& values);

  /**
    * This method retrieves the kvp's for online banking with this account
    *
    * @return The container of kvp's needed when connecting to this account
    */
  MyMoneyKeyValueContainer onlineBankingSettings() const;

  /**
    * This method sets the closed flag for the account. This is just
    * an informational flag for the application. It has no other influence
    * on the behaviour of the account object. The default for
    * new objects @p open.
    *
    * @param isClosed mark the account closed (@p true) or open (@p false).
    */
  void setClosed(bool isClosed);

  /**
    * Return the closed flag for the account.
    *
    * @retval false account is marked open (the default for new accounts)
    * @retval true account is marked closed
    */
  bool isClosed() const;

  /**
    * returns the applicable smallest fraction for this account
    * for the given security based on the account type. At the same
    * time, m_fraction is updated to the value returned.
    *
    * @param sec const reference to currency (security)
    *
    * @retval sec.smallestCashFraction() for account type Cash
    * @retval sec.smallestAccountFraction() for all other account types
    */
  int fraction(const MyMoneySecurity& sec);

  /**
   * Same as the above method, but does not modify m_fraction.
   */
  int fraction(const MyMoneySecurity& sec) const;

  /**
    * This method returns the stored value for the fraction of this
    * account or -1 if not initialized. It can be initialized by
    * calling fraction(const MyMoneySecurity& sec) once.
    *
    * @note Don't use this method outside of KMyMoney application context (eg. testcases).
    * Use the above method instead.
    */
  int fraction() const;

  /**
    * This method returns @a true if the account type is
    * either Income or Expense
    *
    * @retval true account is of type income or expense
    * @retval false for all other account types
    *
    * @deprecated use isIncomeExpense() instead
    */
  KMM_MYMONEY_DEPRECATED bool isCategory() const;

  /**
    * This method returns @a true if the account type is
    * either Income or Expense
    *
    * @retval true account is of type income or expense
    * @retval false for all other account types
    */
  bool isIncomeExpense() const;

  /**
    * This method returns @a true if the account type is
    * either Asset or Liability
    *
    * @retval true account is of type asset or liability
    * @retval false for all other account types
    */
  bool isAssetLiability() const;

  /**
    * This method returns @a true if the account type is
    * either AssetLoan or Loan
    *
    * @retval true account is of type Loan or AssetLoan
    * @retval false for all other account types
    */
  bool isLoan() const;

  /**
    * This method returns @a true if the account type is
    * Stock
    *
    * @retval true account is of type Stock
    * @retval false for all other account types
    */
  bool isInvest() const;

  /**
    * This method returns @a true if the account type is
    * Checkings, Savings or Cash
    *
    * @retval true account is of type Checkings, Savings or Cash
    * @retval false for all other account types
    */
  bool isLiquidAsset() const;

  /**
   * This method returns true if a costcenter assignment is required for this account
   */
  bool isCostCenterRequired() const;

  /**
   * This method allows to control if a cost center assignment is required
   * for this account. It is if @a required is @c true (the default).
   */
  void setCostCenterRequired(bool required = true);

  /**
   * This method returns a name that has a brokerage suffix of
   * the current name. It only works on investment accounts and
   * returns the name for all other cases.
   */
  QString brokerageName() const;

  /**
   * @param reconcileFlag if set to @a true a reconcile overlay will be
   *                      added to the pixmap returned
   * @param size is a hint for the size of the icon
   * @return a pixmap using DesktopIcon for the account type
   */
  QPixmap accountPixmap(const bool reconcileFlag = false, const int size = 64) const;

  /**
   * This method is used to convert the internal representation of
   * an account type into a human readable format
   *
   * @param accountType numerical representation of the account type.
   *                    For possible values, see eMyMoney::Account::Type
   * @return QString representing the human readable form
   */
  static QString accountTypeToString(const eMyMoney::Account::Type accountType);

  /**
    * keeps a history record of a reconciliation for this account on @a date
    * with @a amount.
    *
    * @return @p true in case entry was added, @p false otherwise
    *
    * @sa reconciliationHistory()
    */
  bool addReconciliation(const QDate& date, const MyMoneyMoney& amount);

  /**
    * @return QMap with the reconciliation history for the account
    *
    * @sa addReconciliation()
    */
  QMap<QDate, MyMoneyMoney> reconciliationHistory();

  /**
    * @return @c true if account has an online mapping, @c false otherwise
    */
  bool hasOnlineMapping() const;

  QDataStream &operator<<(const MyMoneyAccount &);
  QDataStream &operator>>(MyMoneyAccount &);
};

inline void swap(MyMoneyAccount& first, MyMoneyAccount& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.MyMoneyObject::d_ptr, second.MyMoneyObject::d_ptr);
  swap(first.MyMoneyKeyValueContainer::d_ptr, second.MyMoneyKeyValueContainer::d_ptr);
}

inline MyMoneyAccount::MyMoneyAccount(MyMoneyAccount && other) : MyMoneyAccount() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyAccount & MyMoneyAccount::operator=(MyMoneyAccount other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

template< class type >
QList< payeeIdentifierTyped<type> > MyMoneyAccount::payeeIdentifiersByType() const
{
  QList< payeeIdentifierTyped<type> > typedList;
  return typedList;
}

template<>
KMM_MYMONEY_EXPORT QList< payeeIdentifierTyped< ::payeeIdentifiers::ibanBic> > MyMoneyAccount::payeeIdentifiersByType() const;

/**
 * Make it possible to hold @ref MyMoneyAccount objects,
 * @ref accountTypeE and @ref amountTypeE inside @ref QVariant objects.
 */
Q_DECLARE_METATYPE(MyMoneyAccount)
Q_DECLARE_METATYPE(eMyMoney::Account::Type)

#endif
