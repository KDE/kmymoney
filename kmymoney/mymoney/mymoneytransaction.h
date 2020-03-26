/*
 * Copyright 2000-2002  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2001       Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2003       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2005  Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
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

#ifndef MYMONEYTRANSACTION_H
#define MYMONEYTRANSACTION_H

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

class MyMoneyMoney;
class MyMoneySplit;
class QStringList;

template <typename T> class QList;

/**
  * This class represents a transaction within the MyMoneyEngine. A transaction
  * contains none, one or more splits of type MyMoneySplit. They are stored in
  * a QList<MyMoneySplit> within this object. A transaction containing only
  * a single split with an amount not equal to 0 is an unbalanced transaction. It
  * is tolerated by the engine, but in general not a good idea as it is financially
  * wrong.
  */
class MyMoneyTransactionPrivate;
class KMM_MYMONEY_EXPORT MyMoneyTransaction : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  Q_DECLARE_PRIVATE_D(MyMoneyObject::d_ptr, MyMoneyTransaction)

  KMM_MYMONEY_UNIT_TESTABLE

public:

  MyMoneyTransaction();
  explicit MyMoneyTransaction(const QString &id);

  MyMoneyTransaction(const QString& id,
                     const MyMoneyTransaction& other);

  MyMoneyTransaction(const MyMoneyTransaction & other);
  MyMoneyTransaction(MyMoneyTransaction && other);
  MyMoneyTransaction & operator=(MyMoneyTransaction other);
  friend void swap(MyMoneyTransaction& first, MyMoneyTransaction& second);

  ~MyMoneyTransaction();

  friend QDataStream &operator<<(QDataStream &, MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

  QDate entryDate() const;
  void setEntryDate(const QDate& date);

  QDate postDate() const;
  void setPostDate(const QDate& date);

  QString memo() const;
  void setMemo(const QString& memo);

  QList<MyMoneySplit> splits() const;
  QList<MyMoneySplit>& splits();
  MyMoneySplit firstSplit() const;
  uint splitCount() const;

  QString commodity() const;
  void setCommodity(const QString& commodityId);

  QString bankID() const;
  void setBankID(const QString& bankID);

  bool operator ==  (const MyMoneyTransaction& right) const;
  bool operator !=  (const MyMoneyTransaction& r) const;
  bool operator <   (const MyMoneyTransaction& r) const;
  bool operator <=  (const MyMoneyTransaction& r) const;
  bool operator >   (const MyMoneyTransaction& r) const;

  /**
    * This method is used to extract a split for a given accountId
    * from a transaction. A parameter controls, whether the accountId
    * should match or not. In case of 'not match', the first not-matching
    * split is returned.
    *
    * @param accountId the account to look for
    * @param match if true, the account Id must match
    *              if false, the account Id must not match
    *
    * @return reference to split within the transaction is returned
    */
  MyMoneySplit splitByAccount(const QString& accountId, const bool match = true) const;

  /**
    * This method is essentially the same as the previous method, except that
    * takes a list of accounts instead of just one.
    *
    * @param accountIds the list of accounts to look for
    * @param match if true, the account Id must match
    *              if false, the account Id must not match
    *
    * @return reference to split within the transaction is returned
    */
  MyMoneySplit splitByAccount(const QStringList& accountIds, const bool match = true) const;

  /**
    * This method is used to extract a split from a transaction.
    *
    * @param splitId the split to look for
    *
    * @return reference to split within the transaction is returned
    */
  MyMoneySplit splitById(const QString& splitId) const;

  /**
    * This method is used to extract a split for a given payeeId
    * from a transaction.
    *
    * @param payeeId the payee to look for
    *
    * @return reference to split within the transaction is returned
    */
  MyMoneySplit splitByPayee(const QString& payeeId) const;

  /**
    * This method is used to check if the given account is used
    * in any of the splits of this transaction
    *
    * @param id account id that should be checked for usage
    */
  bool accountReferenced(const QString& id) const;

  /**
    * This method is used to add a split to the transaction. The split
    * will be assigned an id. The id member must be empty and the
    * accountId member must be filled.
    *
    * @param split reference to the split that should be added
    *
    */
  void addSplit(MyMoneySplit &split);

  /**
    * This method is used to modify a split in a transaction
    */
  void modifySplit(const MyMoneySplit& split);

  /**
    * This method is used to remove a split from a transaction
    */
  void removeSplit(const MyMoneySplit& split);

  /**
    * This method is used to remove all splits from a transaction
    */
  void removeSplits();

  /**
    * This method is used to return the sum of all splits of this transaction
    *
    * @return MyMoneyMoney value of sum of all splits
    */
  MyMoneyMoney splitSum() const;

  /**
    * This method is used to reverse a transaction by reversing the values of each split
    */
  void reverse();

  /**
    * This method returns information if the transaction
    * contains information of a loan payment or not.
    * Loan payment transactions have at least one
    * split that is identified with a MyMoneySplit::action() of type
    * MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization).
    *
    * @retval false transaction is no loan payment transaction
    * @retval true  transaction is a loan payment transaction
    *
    * @note Upon internal failures, the return value @p false will be used.
    */
  bool isLoanPayment() const;

  /**
    * This method returns a const reference to the amortization split.
    * In case none is found, a reference to an empty split will be returned.
    */
  MyMoneySplit amortizationSplit() const;

  /**
   * This method returns a const reference to the interest split.
   * In case none is found, a reference to an empty split will be returned.
   */
  MyMoneySplit interestSplit() const;

  /**
    * returns @a true if this is a stock split transaction
    */
  bool isStockSplit() const;

  /**
    * returns @a true if this is an imported transaction
    */
  bool isImported() const;

  /**
   * Sets the imported state of this transaction to be the value of @a state .
   * @p state defaults to @p true.
   */
  void setImported(bool state = true);

  /**
    * This static method returns the id which will be assigned to the
    * first split added to a transaction. This ID can be used to figure
    * out the split that references the account through which a transaction
    * was entered.
    *
    * @return QString with ID of the first split of transactions
    */
  static QString firstSplitID();

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
    * Checks whether any split contains an autocalc split.
    *
    * @retval true at least one split has an autocalc value
    * @retval false all splits have fixed values
    */
  bool hasAutoCalcSplit() const;

  /**
    * Returns a signature consisting of the account ids and the
    * number of times they occur in the transaction if @a includeSplitCount
    * is @a true. The signature is independent from the order of splits.
    *
    * Example: Having splits referencing the account B, A and B, the returned
    * value will be "A-B" if @p includeSplitCount is @p false or A*1-B*2 if it
    * is @p true.
    *
    * The same result will be returned if the list of splits is A, B, B.
    *
    * @param includeSplitCount if @p true, the string @p *n with @p n being
    *        the number of splits referencing this account. The default for
    *        this parameter is @p false.
    */
  QString accountSignature(bool includeSplitCount = false) const;

  QString uniqueSortKey() const;

  /**
   * This module implements an algorithm used by P.J. Weinberger
   * for fast hashing. Source: COMPILERS by Alfred V. Aho,
   * pages 435-437.
   *
   * It converts the string passed in @p txt into a non-unique
   * unsigned long integer value.
   *
   * @param txt the text to be hashed
   * @param h initial hash value (default 0)
   * @return non-unique hash value of the text @p txt
   */
  static unsigned long hash(const QString& txt, unsigned long h = 0);

  /**
   * This method replaces all occurrences of id @a oldId with
   * @a newId.  All other ids are not changed.
   *
   * @return true if any change has been performed
   * @return false if nothing has been modified
   */
  bool replaceId(const QString& newId, const QString& oldId);
};

inline void swap(MyMoneyTransaction& first, MyMoneyTransaction& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.MyMoneyObject::d_ptr, second.MyMoneyObject::d_ptr);
  swap(first.MyMoneyKeyValueContainer::d_ptr, second.MyMoneyKeyValueContainer::d_ptr);
}

inline MyMoneyTransaction::MyMoneyTransaction(MyMoneyTransaction && other) : MyMoneyTransaction() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyTransaction & MyMoneyTransaction::operator=(MyMoneyTransaction other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

/**
  * Make it possible to hold @ref MyMoneyTransaction objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyTransaction)

#endif
