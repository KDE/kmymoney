/***************************************************************************
                          ledgermodel.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LEDGERMODEL_H
#define LEDGERMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneytransaction.h"
#include "mymoneyaccount.h"
#include "mymoneysplit.h"
#include "mymoneyschedule.h"
#include "mymoneyenums.h"

/**
  * Forward declarations for the returned models.
  */

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
  virtual QString accountId() const { return QString(); }

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
   * Returns the full name and hierarchiy of the account.
   */
  virtual QString account() const = 0;

  /**
   * Returns the full name and hierarchiy of the counter account. If no counter
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
  virtual MyMoneySplit::reconcileFlagE reconciliationState() const = 0;

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

class LedgerTransaction : public LedgerItem
{
public:
  explicit LedgerTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s);
  virtual ~LedgerTransaction();

  static LedgerTransaction newTransactionEntry();

  /// @copydoc LedgerItem::postDate()
  virtual QDate postDate() const;

  /// @copydoc LedgerItem::transaction()
  MyMoneyTransaction transaction() const { return m_transaction; }

  /// @copydoc LedgerItem::split()
  const MyMoneySplit& split() const { return m_split; }

  /// @copydoc LedgerItem::accountId()
  virtual QString accountId() const { return m_split.accountId(); }

  /// @copydoc LedgerItem::account()
  virtual QString account() const { return m_account; }

  /// @copydoc LedgerItem::counterAccountId()
  virtual QString counterAccountId() const { return m_counterAccountId; }

  /// @copydoc LedgerItem::counterAccount()
  virtual QString counterAccount() const { return m_counterAccount; }

  /// @copydoc LedgerItem::costCenterId()
  virtual QString costCenterId() const { return m_costCenterId; }

  /// @copydoc LedgerItem::payeeName()
  virtual QString payeeName() const { return m_payeeName; }

  /// @copydoc LedgerItem::payeeId()
  virtual QString payeeId() const { return m_payeeId; }

  /// @copydoc LedgerItem::transactionNumber()
  virtual QString transactionNumber() const { return m_split.number(); }

  /// @copydoc LedgerItem::flags()
  virtual Qt::ItemFlags flags() const {return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable; }

  /// @copydoc LedgerItem::transactionSplitId()
  virtual QString transactionSplitId() const;

  /// @copydoc LedgerItem::splitCount()
  virtual int splitCount() const { return m_transaction.splitCount(); }

  /// @copydoc LedgerItem::transactionId()
  virtual QString transactionId() const { return m_transaction.id(); }

  /// @copydoc LedgerItem::reconciliationState()
  virtual MyMoneySplit::reconcileFlagE reconciliationState() const;

  /// @copydoc LedgerItem::reconciliationStateShort()
  virtual QString reconciliationStateShort() const;

  /// @copydoc LedgerItem::reconciliationStateShort()
  virtual QString reconciliationStateLong() const;

  /// @copydoc LedgerItem::payment()
  virtual QString payment() const { return m_payment; }

  /// @copydoc LedgerItem::deposit()
  virtual QString deposit() const { return m_deposit; }

  /// @copydoc LedgerItem::setBalance()
  virtual void setBalance(QString txt);

  /// @copydoc LedgerItem::balance()
  virtual QString balance() const { return m_balance; }

  /// @copydoc LedgerItem::shares()
  virtual MyMoneyMoney shares() const { return m_split.shares(); }

  /// @copydoc LedgerItem::sharesAmount()
  virtual QString sharesAmount() const { return m_shares; }

  /// @copydoc LedgerItem::signedSharesAmount()
  virtual QString signedSharesAmount() const { return m_signedShares; }

  /// @copydoc LedgerItem::sharesSuffix()
  virtual QString sharesSuffix() const { return m_sharesSuffix; }

  /// @copydoc LedgerItem::value()
  virtual MyMoneyMoney value() const { return m_split.value(); }

  /// @copydoc LedgerItem::memo()
  virtual QString memo() const;

  /// @copydoc LedgerItem::isErroneous()
  virtual bool isErroneous() const { return m_erroneous; }

  /// @copydoc LedgerItem::isImported()
  virtual bool isImported() const { return m_transaction.isImported(); }

  /// @copydoc LedgerItem::isNewTransactionEntry()
  virtual bool isNewTransactionEntry() const;

  /// @copydoc LedgerItem::transactionCommodity()
  virtual QString transactionCommodity() const;

protected:
  void setupValueDisplay();

protected:
  MyMoneyTransaction  m_transaction;
  MyMoneySplit        m_split;
  QString             m_counterAccountId;
  QString             m_counterAccount;
  QString             m_account;
  QString             m_costCenterId;
  QString             m_payeeName;
  QString             m_payeeId;
  QString             m_shares;
  QString             m_signedShares;
  QString             m_payment;
  QString             m_deposit;
  QString             m_balance;
  QString             m_sharesSuffix; // shows Cr or Dr
  bool                m_erroneous;
};

class LedgerSchedule : public LedgerTransaction
{
public:
  explicit LedgerSchedule(const MyMoneySchedule& s, const MyMoneyTransaction& t, const MyMoneySplit& sp);
  virtual ~LedgerSchedule();

  /// @copydoc LedgerItem::transactionSplitId()
  virtual QString transactionSplitId() const;

  QString scheduleId() const;

  /// @copydoc LedgerItem::isImported()
  virtual bool isImported() const { return false; }

private:
  MyMoneySchedule   m_schedule;
};

class LedgerSplit : public LedgerTransaction
{
public:
  explicit LedgerSplit(const MyMoneyTransaction& t, const MyMoneySplit& s);
  virtual ~LedgerSplit();

  /// @copydoc LedgerItem::memo()
  virtual QString memo() const;
};

namespace LedgerRole {
  enum Roles {
    // Roles returning values
    PostDateRole = Qt::UserRole,
    PayeeNameRole,
    AccountRole,
    CounterAccountRole,
    SplitCountRole,
    ReconciliationRole,
    ReconciliationRoleShort,
    ReconciliationRoleLong,
    SplitSharesRole,
    ShareAmountRole,
    ShareAmountSuffixRole,
    SplitValueRole,
    MemoRole,
    SingleLineMemoRole,
    NumberRole,
    ErroneousRole,
    ImportRole,
    SplitRole,
    TransactionRole,

    // Roles returning ids
    TransactionIdRole,
    SplitIdRole,
    TransactionSplitIdRole,
    PayeeIdRole,
    AccountIdRole,
    CounterAccountIdRole,
    CostCenterIdRole,
    ScheduleIdRole,
    TransactionCommodityRole,

    // A pseudo role to emit the dataChanged() signal when
    // used with setData()
    EmitDataChangedRole
  };

} // namespace LedgerRole

/**
  */
class LedgerModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  explicit LedgerModel(QObject* parent = 0);
  virtual ~LedgerModel();

  enum Columns {
    NumberColumn = 0,
    DateColumn,
    SecurityColumn,
    CostCenterColumn,
    DetailColumn,
    ReconciliationColumn,
    PaymentColumn,
    DepositColumn,
    QuantityColumn,
    PriceColumn,
    AmountColumn,
    ValueColumn,
    BalanceColumn,

    // insert new columns above this line
    NumberOfLedgerColumns
  };

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  /**
   * clears all objects currently in the model
   */
  void unload();

  /**
   * Adds the transaction items in the @a list to the model
   */
  void addTransactions(const QList<QPair<MyMoneyTransaction, MyMoneySplit> >& list);

  /**
   * Adds a single transaction @a t to the model
   */
  void addTransaction(const LedgerTransaction& t);

  /**
   * Adds a single split based on its transactionSplitId
   */
  void addTransaction(const QString& transactionSplitId);

  /**
   * Adds the schedule items in the  @a list to the model
   */
  void addSchedules(const QList< MyMoneySchedule >& list, int previewPeriod);

  /**
   * Loads the model with data from the engine
   */
  void load();

  /**
   * This method extracts the transaction id from a combined
   * transactionSplitId and returns it. In case the @a transactionSplitId does
   * not resembles a transactionSplitId an empty string is returned.
   */
  QString transactionIdFromTransactionSplitId(const QString& transactionSplitId) const;

public Q_SLOTS:

protected Q_SLOTS:
  void removeTransaction(eMyMoney::File::Object objType, const QString& id);
  void addTransaction(eMyMoney::File::Object objType, const MyMoneyObject * const obj);
  void modifyTransaction(eMyMoney::File::Object objType, const MyMoneyObject * const obj);
  void removeSchedule(eMyMoney::File::Object objType, const QString& id);
  void addSchedule(eMyMoney::File::Object objType, const MyMoneyObject * const obj);
  void modifySchedule(eMyMoney::File::Object objType, const MyMoneyObject * const obj);

protected:
  struct Private;
  QScopedPointer<Private> d;
};

class LedgerSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  LedgerSortFilterProxyModel(QObject* parent = 0);
  virtual ~LedgerSortFilterProxyModel();

  void setShowNewTransaction(bool show);
  void setAccountType(eMyMoney::Account type);

  /**
   * This method maps the @a index to the base model and calls setData on it
   */
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

  /**
   * This method returns the headerData adjusted to the current
   * accountType
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

protected:
  virtual bool lessThan(const QModelIndex& left, const QModelIndex& right) const;
  virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const;

private:
  bool                            m_showNewTransaction;
  eMyMoney::Account    m_accountType;
};


class SplitModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  enum Columns {
    NumberColumn = 0,
    DateColumn,
    SecurityColumn,
    CostCenterColumn,
    DetailColumn,
    ReconciliationColumn,
    PaymentColumn,
    DepositColumn,
    QuantityColumn,
    PriceColumn,
    AmountColumn,
    ValueColumn,
    BalanceColumn,

    // insert new columns above this line
    NumberOfLedgerColumns
  };

  explicit SplitModel(QObject* parent = 0);
  virtual ~SplitModel();
  void deepCopy(const SplitModel& right, bool revertSplitSign = false);

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;

  /**
   * Adds a single split @a t to the model
   */
  void addSplit(const QString& transactionSplitId);

  /**
   * Adds a single dummy split to the model which is used for
   * createion of new splits.
   */
  void addEmptySplitEntry();

  /**
   * Remove the single dummy split to the model which is used for
   * createion of new splits from the model.
   */
  void removeEmptySplitEntry();

  virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());

  /**
   * This method returns the string to be used for new split ids
   */
  static QString newSplitId();

  /**
   * This method compares the @a id against the one provided
   * by newSplitId() and returns true if it is identical.
   */
  static bool isNewSplitId(const QString& id);

  // void removeSplit(const LedgerTransaction& t);

private:
  struct Private;
  QScopedPointer<Private> d;
};
#endif // LEDGERMODEL_H

