/***************************************************************************
                          mymoneysplit.h  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#ifndef MYMONEYSPLIT_H
#define MYMONEYSPLIT_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"
#include "mymoneykeyvaluecontainer.h"

class QString;
class QDate;

class MyMoneyMoney;
class MyMoneyTransaction;

namespace eMyMoney { namespace Split { enum class State;
                                       enum class InvestmentTransactionType; } }

/**
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

/**
  * This class represents a split of a transaction.
  */
class MyMoneySplitPrivate;
class KMM_MYMONEY_EXPORT MyMoneySplit : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  Q_DECLARE_PRIVATE(MyMoneySplit)
  MyMoneySplitPrivate* d_ptr;

  KMM_MYMONEY_UNIT_TESTABLE

public:

  MyMoneySplit();
  explicit MyMoneySplit(const QDomElement& node);
  MyMoneySplit(const QString& id,
               const MyMoneySplit& other);

  MyMoneySplit(const MyMoneySplit & other);
  MyMoneySplit(MyMoneySplit && other);
  MyMoneySplit & operator=(MyMoneySplit other);
  friend void swap(MyMoneySplit& first, MyMoneySplit& second);

  ~MyMoneySplit();

  bool operator == (const MyMoneySplit&) const;

  /**
   * Returns a copy of the MyMoneySplit where the sign of
   * shares and value is inverted.
   */
  MyMoneySplit operator-() const;

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

  MyMoneyMoney shares() const;
  void setShares(const MyMoneyMoney& shares);

  /**
   * This method returns the price. If the member m_price is not zero
   * its value is returned. Otherwise, if m_shares is not zero the quotient
   * of m_value / m_shares is returned. If m_values equals to zero, 1
   * will be returned.
   */
  MyMoneyMoney price() const;
  /** This method just returns what is in m_price, so when we write to the
   *  database, we don't just generate prices
  */
  MyMoneyMoney actualPrice() const;
  void setPrice(const MyMoneyMoney& price);

  MyMoneyMoney value() const;
  MyMoneyMoney value(const QString& transactionCurrencyId, const QString& splitCurrencyId) const;

  /**
   * Required to have (direct) access to the MyMoneyKeyValueContainer::value() method.
   */
  QString value(const QString& key) const;

  /**
   * Required to have (direct) access to the MyMoneyKeyValueContainer::setValue() method.
   */
  void setValue(const QString& key, const QString& value);
  void setValue(const MyMoneyMoney& value);
  /**
    * This method is used to set either the shares or the value depending on
    * the currencies assigned to the split/account and the transaction.
    *
    * If @p transactionCurrencyId equals @p splitCurrencyId this method
    * calls setValue(MyMoneyMoney) otherwise setShares(MyMoneyMoney).
    *
    * @param value the value to be assiged
    * @param transactionCurrencyId the id of the currency assigned to the transaction
    * @param splitCurrencyId the id of the currency assigned to the split (i.e. the
    *                        the id of the currency assigned to the account that is
    *                        referenced by the split)
    */
  void setValue(const MyMoneyMoney& value, const QString& transactionCurrencyId, const QString& splitCurrencyId);


  QString accountId() const;
  void setAccountId(const QString& account);

  QString costCenterId() const;
  void setCostCenterId(const QString& costCenter);

  QString memo() const;
  void setMemo(const QString& memo);

  eMyMoney::Split::State reconcileFlag() const;
  void setReconcileFlag(const eMyMoney::Split::State flag);

  QDate reconcileDate() const;
  void setReconcileDate(const QDate& date);

  QString payeeId() const;
  void setPayeeId(const QString& payee);

  QList<QString> tagIdList() const;
  void setTagIdList(const QList<QString>& tagList);

  QString action() const;
  void setAction(const QString& action);
  void setAction(eMyMoney::Split::InvestmentTransactionType type);
  bool isAmortizationSplit() const;
  bool isInterestSplit() const;

  QString number() const;
  void setNumber(const QString& number);

  bool isAutoCalc() const;

  QString bankID() const;
  void setBankID(const QString& bankID);

  QString transactionId() const;
  void setTransactionId(const QString& id);

  /**
  * returns @a true if this its a transaction matched against an imported
  * transaction. The imported and matched transaction can be extracted
  * using matchedTransaction() and can be removed using removeMatch().
   */
  bool isMatched() const;

  /**
   * add an imported transaction @p t as matching transaction. Any previously
   * added transaction will be overridden. @p t.isImported() must return true,
   * otherwise the transaction is not stored.
   */
  void addMatch(const MyMoneyTransaction& t);

  /**
   * remove the data of the imported transaction added with addMatch().
   */
  void removeMatch();

  /**
   * Return the matching imported transaction. If no such transaction
   * is available (isMatched() returns false) an empty transaction is returned.
   */
  MyMoneyTransaction matchedTransaction() const;

  /**
   * This method replaces all occurrences of id @a oldId with
   * @a newId.  All other ids are not changed.
   *
   * @return true if any change has been performed
   * @return false if nothing has been modified
   */
  bool replaceId(const QString& newId, const QString& oldId);

  static const char ActionCheck[];
  static const char ActionDeposit[];
  static const char ActionTransfer[];
  static const char ActionWithdrawal[];
  static const char ActionATM[];

  static const char ActionAmortization[];
  static const char ActionInterest[];

  static const char ActionBuyShares[];  // negative amount is sellShares
  static const char ActionDividend[];
  static const char ActionReinvestDividend[];
  static const char ActionYield[];
  static const char ActionAddShares[];  // negative amount is removeShares
  static const char ActionSplitShares[];
  static const char ActionInterestIncome[];

private:
  enum class Element { Split = 0,
                       Tag,
                       Match,
                       Container,
                       KeyValuePairs
                     };

  enum class Attribute { ID = 0,
                         BankID,
                         Account,
                         Payee,
                         Tag,
                         Number,
                         Action,
                         Value,
                         Shares,
                         Price,
                         Memo,
                         CostCenter,
                         ReconcileDate,
                         ReconcileFlag,
                         KMMatchedTx,
                         // insert new entries above this line
                         LastAttribute
                       };


  static QString getElName(const Element el);
  static QString getAttrName(const Attribute attr);
  friend uint qHash(const Attribute, uint seed);
  friend uint qHash(const Element, uint seed);
};

inline uint qHash(const MyMoneySplit::Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); } // krazy:exclude=inline
inline uint qHash(const MyMoneySplit::Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); } // krazy:exclude=inline

inline void swap(MyMoneySplit& first, MyMoneySplit& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
  swap(first.m_id, second.m_id);
  swap(first.m_kvp, second.m_kvp);
}

inline MyMoneySplit::MyMoneySplit(MyMoneySplit && other) : MyMoneySplit() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneySplit & MyMoneySplit::operator=(MyMoneySplit other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

/**
  * Make it possible to hold @ref MyMoneySplit objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneySplit)

#endif
