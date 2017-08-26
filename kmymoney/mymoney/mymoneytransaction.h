/***************************************************************************
                          mymoneytransaction.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2002 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYTRANSACTION_H
#define MYMONEYTRANSACTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDateTime>
#include <QList>
#include <QStringList>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneysplit.h"
#include <kmm_mymoney_export.h>
#include <mymoneyunittestable.h>

/**
  * This class represents a transaction within the MyMoneyEngine. A transaction
  * contains none, one or more splits of type MyMoneySplit. They are stored in
  * a QList<MyMoneySplit> within this object. A transaction containing only
  * a single split with an amount not equal to 0 is an unbalanced transaction. It
  * is tolerated by the engine, but in general not a good idea as it is financially
  * wrong.
  */
class KMM_MYMONEY_EXPORT MyMoneyTransaction : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneyTransaction();
  MyMoneyTransaction(const QString& id,
                     const MyMoneyTransaction& transaction);
  /**
    * @param node reference to QDomNode
    * @param forceId see MyMoneyObject(const QDomElement&, const bool)
    */
  explicit MyMoneyTransaction(const QDomElement& node, const bool forceId = true);
  ~MyMoneyTransaction();

public:
  friend QDataStream &operator<<(QDataStream &, MyMoneyTransaction &);
  friend QDataStream &operator>>(QDataStream &, MyMoneyTransaction &);

  // Simple get operations
  const QDate& entryDate() const {
    return m_entryDate;
  };
  const QDate& postDate() const {
    return m_postDate;
  };
  const QString& memo() const {
    return m_memo;
  };
  const QList<MyMoneySplit>& splits() const {
    return m_splits;
  };
  QList<MyMoneySplit>& splits() {
    return m_splits;
  };
  unsigned int splitCount() const {
    return m_splits.count();
  };
  const QString& commodity() const {
    return m_commodity;
  };
  const QString& bankID() const {
    return m_bankID;
  };

  // Simple set operations
  void setPostDate(const QDate& date);
  void setEntryDate(const QDate& date);
  void setMemo(const QString& memo);
  void setCommodity(const QString& commodityId) {
    m_commodity = commodityId;
  };
  void setBankID(const QString& bankID) {
    m_bankID = bankID;
  };

  bool operator == (const MyMoneyTransaction&) const;
  inline bool operator != (const MyMoneyTransaction& r) const {
    return !(*this == r);
  };
  bool operator< (const MyMoneyTransaction& r) const {
    return postDate() < r.postDate();
  };
  bool operator<= (const MyMoneyTransaction& r) const {
    return postDate() <= r.postDate();
  };
  bool operator> (const MyMoneyTransaction& r) const {
    return postDate() > r.postDate();
  };

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
  const MyMoneySplit& splitByAccount(const QString& accountId, const bool match = true) const;

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
  const MyMoneySplit& splitByAccount(const QStringList& accountIds, const bool match = true) const;

  /**
    * This method is used to extract a split from a transaction.
    *
    * @param splitId the split to look for
    *
    * @return reference to split within the transaction is returned
    */
  const MyMoneySplit& splitById(const QString& splitId) const;

  /**
    * This method is used to extract a split for a given payeeId
    * from a transaction.
    *
    * @param payeeId the payee to look for
    *
    * @return reference to split within the transaction is returned
    */
  const MyMoneySplit& splitByPayee(const QString& payeeId) const;

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
  void addSplit(MyMoneySplit& split);

  /**
    * This method is used to modify a split in a transaction
    */
  void modifySplit(MyMoneySplit& split);

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
  const MyMoneyMoney splitSum() const;

  /**
    * This method returns information if the transaction
    * contains information of a loan payment or not.
    * Loan payment transactions have at least one
    * split that is identified with a MyMoneySplit::action() of type
    * MyMoneySplit::ActionAmortization.
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
  const MyMoneySplit& amortizationSplit() const;

  /**
   * This method returns a const reference to the interest split.
   * In case none is found, a reference to an empty split will be returned.
   */
  const MyMoneySplit& interestSplit() const;

  /**
    * This method is used to check if two transactions are identical.
    * Identical transactions have:
    *
    * - the same number of splits
    * - reference the same accounts
    * - have the same values in the splits
    * - have a postDate wihtin 3 days
    *
    * @param transaction reference to the transaction to be checked
    *                    against this transaction
    * @retval true transactions are identical
    * @retval false transactions are not identical
    */
  bool isDuplicate(const MyMoneyTransaction& transaction) const;

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
  static const QString firstSplitID();

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const;

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


private:
  /**
    * This method returns the next id to be used for a split
    */
  const QString nextSplitID();

private:
  static const int SPLIT_ID_SIZE = 4;

  /**
    * This member contains the date when the transaction was entered
    * into the engine
    */
  QDate m_entryDate;

  /**
    * This member contains the date the transaction was posted
    */
  QDate m_postDate;

  /**
    * This member keeps the memo text associated with this transaction
    */
  QString m_memo;

  /**
    * This member contains the splits for this transaction
    */
  QList<MyMoneySplit> m_splits;

  /**
    * This member keeps the unique numbers of splits within this
    * transaction. Upon creation of a MyMoneyTransaction object this
    * value will be set to 1.
    */
  unsigned int m_nextSplitID;

  /**
    * This member keeps the base commodity (e.g. currency) for this transaction
    */
  QString  m_commodity;

  /**
    * This member keeps the bank's unique ID for the transaction, so we can
    * avoid duplicates.  This is only used for electronic statement downloads.
    *
    * Note this is now deprecated!  Bank ID's should be set on splits, not transactions.
    */
  QString m_bankID;

  /** constants for unique sort key */
  static const int YEAR_SIZE = 4;
  static const int MONTH_SIZE = 2;
  static const int DAY_SIZE = 2;
};

/**
  * Make it possible to hold @ref MyMoneyTransaction objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyTransaction)
QDebug operator<<(QDebug dbg, const MyMoneyTransaction &a);
#endif
