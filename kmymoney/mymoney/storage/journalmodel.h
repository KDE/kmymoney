/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef JOURNALMODEL_H
#define JOURNALMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QSharedDataPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"

#include "accountsmodel.h"
#include "mymoneyenums.h"
#include "mymoneymodel.h"
#include "mymoneymoney.h"
#include "mymoneyobject.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"

class MyMoneyTransactionFilter;
class QUndoStack;

/**
 * The class representing a single journal entry (one split of a transaction)
 */
class /* no export here on purpose */ JournalEntry
{
public:
    friend class JournalModel;

    explicit JournalEntry()
    {
    }
    friend void swap(JournalEntry& first, JournalEntry& second);

    explicit JournalEntry(const QString& id, const JournalEntry& other)
        : m_id(id)
        , m_transaction(other.m_transaction)
        , m_split(other.m_split)
        , m_balance(other.m_balance)
        , m_linesInLedger(other.m_linesInLedger)
    {
    }

    JournalEntry(QString id, QSharedPointer<MyMoneyTransaction> t, const MyMoneySplit& sp)
        : m_id(id)
        , m_transaction(t)
        , m_split(sp)
        , m_linesInLedger(0)
    {}

    inline QSharedPointer<MyMoneyTransaction> sharedtransactionPtr() const
    {
        return m_transaction;
    }
    inline const MyMoneyTransaction* transactionPtr() const
    {
        return m_transaction.data();
    }
    inline const MyMoneyTransaction& transaction() const
    {
        return *m_transaction;
    }
    inline const MyMoneySplit& split() const
    {
        return m_split;
    }
    inline const MyMoneyMoney& balance() const
    {
        return m_balance;
    }
    inline const QString& id() const
    {
        return m_id;
    }
    inline uint8_t linesInLedger() const
    {
        return m_linesInLedger;
    }
    inline bool hasReferenceTo(const QString& id) const {
        return m_transaction->hasReferenceTo(id);
    }

    /**
     * @copydoc MyMoneyObject::referencedObjects
     */
    inline QSet<QString> referencedObjects() const
    {
        return m_transaction->referencedObjects();
    }

    inline void setBalance(const MyMoneyMoney& balance)
    {
        m_balance = balance;
    }
    inline void setLinesInLedger(uint8_t lines)
    {
        m_linesInLedger = lines;
    }

private:
    QString m_id;
    QSharedPointer<MyMoneyTransaction> m_transaction;
    MyMoneySplit m_split;
    MyMoneyMoney m_balance;
    uint8_t m_linesInLedger;
};

inline void swap(JournalEntry& first, JournalEntry& second)
{
    using std::swap;
    swap(first.m_id, second.m_id);
    swap(first.m_transaction, second.m_transaction);
    swap(first.m_split, second.m_split);
    swap(first.m_balance, second.m_balance);
    swap(first.m_linesInLedger, second.m_linesInLedger);
}

class JournalModelNewTransaction;

/**
  */
class KMM_MYMONEY_EXPORT JournalModel : public MyMoneyModel<JournalEntry>
{
    Q_OBJECT

public:
    enum Column {
        Number = 0,
        EntryDate,
        Date,
        Account,
        Payee,
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
        MaxColumns,
    };
    Q_ENUMS(Column);

    struct DateRange {
        QDate firstTransaction;
        QDate lastTransaction;
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
    MyMoneyTransaction transactionByIndex(const QModelIndex& idx) const;

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

    void load(const QMap<QString, QSharedPointer<MyMoneyTransaction>>& list);
    void unload();

    JournalModelNewTransaction* newTransaction();

    MyMoneyMoney balance(const QString& accountId, const QDate& date) const;

    /**
     * This method returns the balance after a stock split in account referenced
     * by @a accountId. The starting balance is provided by @a balance and
     * the stock split factor by @a factor.
     */
    MyMoneyMoney stockSplitBalance(const QString& accountId, MyMoneyMoney balance, MyMoneyMoney factor) const;

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

    /**
     * In case a journal id changes (e.g. because the date changes during merging)
     * this method returns the update journalEntryId based on the old one.
     */
    QString updateJournalId(const QString& journalId) const;

    MyMoneyMoney clearedBalance(const QString& accountId, const QDate& date) const;

    bool matchTransaction(const QModelIndex& idx, MyMoneyTransactionFilter& filter) const;

    DateRange dateRange() const;

protected:
    explicit JournalModel(const QString& idLeadin, QObject* parent = nullptr, QUndoStack* undoStack = nullptr);

    Operation undoOperation(const JournalEntry& before, const JournalEntry& after) const override;
    void doAddItem(const JournalEntry& item, const QModelIndex& parentIdx) override;
    void doRemoveItem(const JournalEntry& before) override;
    void doModifyItem(const JournalEntry& before, const JournalEntry& after) override;

public Q_SLOTS:
    void updateBalances();

    /**
     * Reset the information cached for each journalEntry about
     * the height of the row. While painting the next time, this
     * information will be updated through the JournalDelegate.
     * Once done, it emits a dataChanged signal for all rows
     * and colums.
     */
    void resetRowHeightInformation();

private:
    void addTransaction(const QString& id, MyMoneyTransaction& t);
    void removeTransaction(const QModelIndex& idx);

    QModelIndex firstIndexById(const QString& id) const;
    QModelIndex firstIndexByKey(const QString& key) const;


Q_SIGNALS:
    void balancesChanged(const QHash<QString, AccountBalances>& balances);
    void balanceChanged(const QString& accountId);
    void idChanged(const QString& current, const QString& previous);

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
    void addTransaction(MyMoneyTransaction& t) {
        Q_UNUSED(t);
    }
    void removeTransaction(const MyMoneyTransaction& t) {
        Q_UNUSED(t);
    }
    void modifyTransaction(const MyMoneyTransaction& newTransaction) {
        Q_UNUSED(newTransaction);
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) final override
    {
        Q_UNUSED(index);
        Q_UNUSED(value);
        Q_UNUSED(role);
        return false;
    }

    void load(const QMap<QString, MyMoneyTransaction>& list) {
        Q_UNUSED(list);
    };
};
#endif // JOURNALMODEL_H

