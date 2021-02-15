/*
    SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERITEM_H
#define LEDGERITEM_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qnamespace.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;
class QDate;

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;

namespace eMyMoney { namespace Split { enum class State; } }

class LedgerItem
{
public:
  explicit LedgerItem();
  virtual ~LedgerItem();

  /**
   * This method returns the complete raw transaction from the engine
   */
  virtual MyMoneyTransaction transaction() const = 0;

  /**
   * This method returns the complete raw split from the engine
   */
  virtual const MyMoneySplit& split() const = 0;

  /**
   * Returns the postDate of the object. This can be used for sorting purposes.
   */
  virtual QDate postDate() const = 0;

  /**
   * Returns the account id that this entry references. Default is
   * to return an empty string.
   */
  virtual QString accountId() const;

  /**
   * Returns the id of the counter account. If no counter split is present,
   * it returns an empty string, in case more than one counter
   * split is present it returns the fixed string '????'.
   * @todo figure out how to handle the three+ split case.
   */
  virtual QString counterAccountId() const = 0;

  /**
   * Returns the cost center id. This depends on how many slits the transaction has:
   *
   * two splits - returns the costcenter entry which is set in one of the splits
   * otherwise  - returns the costcenter entry of the split
   */
  virtual QString costCenterId() const = 0;

  /**
   * Returns the full name and hierarchy of the account.
   */
  virtual QString account() const = 0;

  /**
   * Returns the full name and hierarchy of the counter account. If no counter
   * split is present, it returns an empty string, in case more than one counter
   * split is present it returns the fixed string 'Split transaction'.
   */
  virtual QString counterAccount() const = 0;

  /**
   * Returns the name of the payee that is assigned to the split
   * or one that is found with other splits.
   */
  virtual QString payeeName() const = 0;

  /**
   * Returns the id of the payee that is assigned to the split
   * or one that is found with other splits.
   */
  virtual QString payeeId() const = 0;

  /**
   * Returns the number of the transaction assigned by the user/institution
   */
  virtual QString transactionNumber() const = 0;

  /**
   * Return information if this item is selectable, editable, etc.
   * @sa QAbstractItemModel::flags()
   */
  virtual Qt::ItemFlags flags() const = 0;

  /**
   * Returns an id for the selected transaction.
   */
  virtual QString transactionId() const = 0;

  /**
   * Returns an id for the selected transaction and split.
   */
  virtual QString transactionSplitId() const = 0;

  /**
   * Returns the number of splits in this transaction
   */
  virtual int splitCount() const = 0;

  /**
   * Returns the internal reconciliation status for the selected transaction and split.
   */
  virtual eMyMoney::Split::State reconciliationState() const = 0;

  /**
   * Returns the short reconciliation status text for the selected transaction and split.
   */
  virtual QString reconciliationStateShort() const = 0;

  /**
   * Returns the full reconciliation status text for the selected transaction and split.
   */
  virtual QString reconciliationStateLong() const = 0;

  /**
   * Returns the display string for the payment column.
   */
  virtual QString payment() const = 0;

  /**
   * Returns the display string for the deposit column.
   */
  virtual QString deposit() const = 0;

  /**
   * Allows to set the display string for the balance column.
   */
  virtual void setBalance(QString txt) = 0;

  /**
   * Returns the display string for the balance column.
   */
  virtual QString balance() const = 0;

  /**
   * Returns the amount of the shares of the split
   */
  virtual MyMoneyMoney shares() const = 0;

  /**
   * Returns the amount of the shares of the split as QString (always positive)
   */
  virtual QString sharesAmount() const = 0;

  /**
   * Returns the amount of the shares of the split as QString (with sign)
   */
  virtual QString signedSharesAmount() const = 0;

  /**
   * Returns the suffix of the shares (Dr. or Cr.)
   */
  virtual QString sharesSuffix() const = 0;

  /**
   * Returns the value in the transaction commodity of the split
   */
  virtual MyMoneyMoney value() const = 0;

  /**
   * Returns the lines of a memo
   */
  virtual QString memo() const = 0;

  /**
   * Returns true if an item is erroneous
   */
  virtual bool isErroneous() const = 0;

  /**
   * Returns true if an item is imported
   */
  virtual bool isImported() const = 0;

  /**
   * Returns true if this is the empty entry at the end of the ledger
   */
  virtual bool isNewTransactionEntry() const = 0;

  /**
   * Returns the symbol of the commodity this transaction is kept in
   */
  virtual QString transactionCommodity() const = 0;

};

#endif // LEDGERITEM_H

