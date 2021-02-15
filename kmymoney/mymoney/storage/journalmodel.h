/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOURNALMODEL_H
#define JOURNALMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSharedDataPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymodel.h"
#include "mymoneyenums.h"
#include "kmm_mymoney_export.h"

#include "mymoneyobject.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"
#include "mymoneymoney.h"

class MyMoneyTransactionFilter;
class QUndoStack;

/**
 * The class representing a single journal entry (one split of a transaction)
 */
class /* no export here on purpose */ JournalEntry
{
public:
  friend class JournalModel;

  explicit JournalEntry() {}
  explicit JournalEntry(const QString& id, const JournalEntry& other)
  : m_id(id)
  , m_transaction(other.m_transaction)
  , m_split(other.m_split)
  , m_balance(other.m_balance)
  {}
  JournalEntry(QString id, QSharedPointer<MyMoneyTransaction> t, const MyMoneySplit& sp)
  : m_id(id)
  , m_transaction(t)
  , m_split(sp)
  {}

  inline QSharedPointer<MyMoneyTransaction> sharedtransactionPtr() const { return m_transaction; }
  inline const MyMoneyTransaction* transactionPtr() const { return m_transaction.data(); }
  inline const MyMoneyTransaction& transaction() const { return *m_transaction; }
  inline const MyMoneySplit& split() const { return m_split; }
  inline const MyMoneyMoney& balance() const { return m_balance; }
  inline const QString& id() const { return m_id; }
  inline bool hasReferenceTo(const QString& id) const { return m_transaction->hasReferenceTo(id); }

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  inline QSet<QString> referencedObjects() const { return m_transaction->referencedObjects(); }

  inline void setBalance(const MyMoneyMoney& balance) { m_balance = balance; }
private:
  QString                             m_id;
  QSharedPointer<MyMoneyTransaction>  m_transaction;
  MyMoneySplit                        m_split;
  MyMoneyMoney                        m_balance;
};

class JournalModelNewTransaction;

/**
  */
class KMM_MYMONEY_EXPORT JournalModel : public MyMoneyModel<JournalEntry>
{
  Q_OBJECT

public:
  enum Column {
    Number = 0,
    Date,
    Account,
    Security,
    CostCenter,
    Detail,
    Reconciliation,
    Payment,
    Deposit,
    Quantity,
    Price,
    Amount,
    Value,
    Balance,
    // insert new columns above this line
    MaxColumns
  };

  explicit JournalModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
  virtual ~JournalModel();

  static const int ID_SIZE = 18;

  int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
  Qt::ItemFlags flags(const QModelIndex & index) const override;

  /**
   * Special implementation using a binary search algorithm instead
   * of the linear one provided by the template function
   */
  MyMoneyTransaction transactionById(const QString& id) const;

  /**
   * Returns all indexes for a given transaction @a id or empty
   * if it does not exist. The indexes contained are only for
   * columns 0.
   */
  QModelIndexList indexesByTransactionId(const QString& id) const;

  void addTransaction(MyMoneyTransaction& t);
  void removeTransaction(const MyMoneyTransaction& t);
  void modifyTransaction(const MyMoneyTransaction& newTransaction);

  void transactionList(QList<MyMoneyTransaction>& list, MyMoneyTransactionFilter& filter) const;
  void transactionList(QList< QPair<MyMoneyTransaction, MyMoneySplit> >& list, MyMoneyTransactionFilter& filter) const;
  unsigned int transactionCount(const QString& accountid) const;

  bool setData(const QModelIndex& idx, const QVariant& value, int role = Qt::EditRole) override;

  void load(const QMap<QString, MyMoneyTransaction>& list);
  void unload();

  JournalModelNewTransaction* newTransaction();

  MyMoneyMoney balance(const QString& accountId, const QDate& date) const;

  /**
   * This method returns a key for a specific date that can
   * be used in connection with lowerBound() and upperBound()
   * to get indexes into the journal based on the date.
   */
  QString keyForDate(const QDate& date) const;

  /**
   * Returns the string that shall be used for new transactions
   */
  QString fakeId() const;

  /**
   * Returns a QModelIndex to the first split of the transaction referenced
   * by @a index or returns an invalid QModelIndex in case
   * @a index points to a different model or is invalid itself.
   */
  QModelIndex adjustToFirstSplitIdx(const QModelIndex& index) const;

protected:
  explicit JournalModel(const QString& idLeadin, QObject* parent = nullptr, QUndoStack* undoStack = nullptr);

  Operation undoOperation(const JournalEntry& before, const JournalEntry& after) const override;
  void doAddItem(const JournalEntry& item, const QModelIndex& parentIdx) override;
  void doRemoveItem(const JournalEntry& before) override;
  void doModifyItem(const JournalEntry& before, const JournalEntry& after) override;

public Q_SLOTS:
  void updateBalances();

private:
  void addTransaction(const QString& id, MyMoneyTransaction& t);
  void removeTransaction(const QModelIndex& idx);

  QModelIndex firstIndexById(const QString& id) const;
  QModelIndex firstIndexByKey(const QString& key) const;


Q_SIGNALS:
  void balancesChanged(const QHash<QString, MyMoneyMoney>& balances);
  void balanceChanged(const QString& accountId);

public Q_SLOTS:

private:
  struct Private;
  QScopedPointer<Private> d;
};

class KMM_MYMONEY_EXPORT JournalModelNewTransaction : public JournalModel
{
  Q_OBJECT

public:
  explicit JournalModelNewTransaction(QObject* parent = nullptr);
  virtual ~JournalModelNewTransaction();

  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const final override;

protected:
  void addTransaction(MyMoneyTransaction& t) { Q_UNUSED(t); }
  void removeTransaction(const MyMoneyTransaction& t) { Q_UNUSED(t); }
  void modifyTransaction(const MyMoneyTransaction& newTransaction) { Q_UNUSED(newTransaction); }

  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override
  {
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
  }

  void load(const QMap<QString, MyMoneyTransaction>& list) { Q_UNUSED(list); };
};
#endif // JOURNALMODEL_H

