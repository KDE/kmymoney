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

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDateTime>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyutils.h"
#include "mymoneymoney.h"
#include <kmm_mymoney_export.h>
#include <mymoneyobject.h>
#include <mymoneykeyvaluecontainer.h>
class MyMoneyTransaction;

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents a split of a transaction.
  */
class KMM_MYMONEY_EXPORT MyMoneySplit : public MyMoneyObject, public MyMoneyKeyValueContainer
{
public:
  /**
    * This enum defines the possible reconciliation states a split
    * can be in. Possible values are as follows:
    *
    * @li NotReconciled
    * @li Cleared
    * @li Reconciled
    * @li Frozen
    *
    * Whenever a new split is created, it has the status NotReconciled. It
    * can be set to cleared when the transaction has been performed. Once the
    * account is reconciled, cleared splits will be set to Reconciled. The
    * state Frozen will be used, when the concept of books is introduced into
    * the engine and a split must not be changed anymore.
    */
  enum reconcileFlagE {
    Unknown = -1,
    NotReconciled = 0,
    Cleared,
    Reconciled,
    Frozen,
    // insert new values above
    MaxReconcileState
  };

  typedef enum {
    UnknownTransactionType = -1,
    BuyShares = 0,
    SellShares,
    Dividend,
    ReinvestDividend,
    Yield,
    AddShares,
    RemoveShares,
    SplitShares,
    InterestIncome///
  } investTransactionTypeE;

  MyMoneySplit();
  MyMoneySplit(const QDomElement& node);
  MyMoneySplit(const QString& id, const MyMoneySplit& right);
  ~MyMoneySplit();

  bool operator == (const MyMoneySplit&) const;

  /**
   * Returns a copy of the MyMoneySplit where the sign of
   * shares and value is inverted.
   */
  MyMoneySplit operator-() const;

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

  const MyMoneyMoney& shares() const {
    return m_shares;
  }
  const MyMoneyMoney& value() const {
    return m_value;
  }

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
  MyMoneyMoney actualPrice() const {
    return m_price;
  }

  const MyMoneyMoney value(const QString& transactionCurrencyId, const QString& splitCurrencyId) const;

  /**
   * Required to have (direct) access to the MyMoneyKeyValueContainer::value() method.
   */
  const QString& value(const QString& key) const {
    return MyMoneyKeyValueContainer::value(key);
  }

  /**
   * Required to have (direct) access to the MyMoneyKeyValueContainer::setValue() method.
   */
  void setValue(const QString& key, const QString& value) {
    MyMoneyKeyValueContainer::setValue(key, value);
  }

  const QString& accountId() const {
    return m_account;
  }
  const QString& costCenterId() const {
    return m_costCenter;
  }
  const QString& memo() const {
    return m_memo;
  }
  reconcileFlagE reconcileFlag() const {
    return m_reconcileFlag;
  }
  const QDate& reconcileDate() const {
    return m_reconcileDate;
  }
  const QString& payeeId() const {
    return m_payee;
  }
  const QList<QString>& tagIdList() const {
    return m_tagList;
  }
  const QString& action() const {
    return m_action;
  }
  const QString& number() const {
    return m_number;
  }
  bool isAmortizationSplit() const {
    return m_action == ActionAmortization;
  }
  bool isInterestSplit() const {
    return m_action == ActionInterest;
  }
  bool isAutoCalc() const {
    return (m_shares == MyMoneyMoney::autoCalc) || (m_value == MyMoneyMoney::autoCalc);
  }
  const QString& bankID() const {
    return m_bankID;
  }
  const QString& transactionId() const {
    return m_transactionId;
  }

  void setShares(const MyMoneyMoney& shares);
  void setValue(const MyMoneyMoney& value);
  void setPrice(const MyMoneyMoney& price);

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

  void setAccountId(const QString& account);
  void setCostCenterId(const QString& costCenter);
  void setMemo(const QString& memo);
  void setReconcileFlag(const reconcileFlagE flag);
  void setReconcileDate(const QDate& date);
  void setPayeeId(const QString& payee);
  void setTagIdList(const QList<QString>& tagList);
  void setAction(const QString& action);
  void setAction(investTransactionTypeE type);
  void setNumber(const QString& number);
  void setBankID(const QString& bankID) {
    m_bankID = bankID;
  };
  void setTransactionId(const QString& id) {
    m_transactionId = id;
  }

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
  /**
    * This member contains the ID of the payee
    */
  QString        m_payee;

  /**
    * This member contains a list of the IDs of the tags
    */
  QList<QString> m_tagList;

  /**
    * This member contains the ID of the account
    */
  QString        m_account;

  /**
   * This member contains the ID of the cost center
   */
  QString        m_costCenter;

  /**
    */
  MyMoneyMoney   m_shares;

  /**
    */
  MyMoneyMoney   m_value;

  /**
    * If the quotient of m_shares divided by m_values is not the correct price
    * because of truncation, the price can be stored in this member. For display
    * purpose and transaction edit this value can be used by the application.
    */
  MyMoneyMoney   m_price;

  QString        m_memo;

  /**
    * This member contains information about the reconciliation
    * state of the split. Possible values are
    *
    * @li NotReconciled
    * @li Cleared
    * @li Reconciled
    * @li Frozen
    *
    */
  reconcileFlagE m_reconcileFlag;

  /**
    * In case the reconciliation flag is set to Reconciled or Frozen
    * this member contains the date of the reconciliation.
    */
  QDate          m_reconcileDate;

  /**
    * The m_action member is an arbitrary string, but is intended to
    * be conveniently limited to a menu of selections such as
    * "Buy", "Sell", "Interest", etc.
    */
  QString        m_action;

  /**
    * The m_number member is used to store a reference number to
    * the split supplied by the user (e.g. check number, etc.).
    */
  QString        m_number;

  /**
    * This member keeps the bank's unique ID for the split, so we can
    * avoid duplicates.  This is only used for electronic statement downloads.
    *
    * This should only be set on the split which refers to the account
    * that was downloaded.
    */
  QString        m_bankID;

  /**
    * This member keeps a backward id to the transaction that this
    * split can be found in. It is the purpose of the MyMoneyTransaction
    * object to maintain this member variable.
    */
  QString        m_transactionId;
};

/**
  * Make it possible to hold @ref MyMoneySplit objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneySplit)

#endif
