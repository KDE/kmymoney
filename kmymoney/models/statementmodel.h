/*
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATEMENTMODEL_H
#define STATEMENTMODEL_H

#include "kmm_models_export.h"
// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneymodel.h"
#include "mymoneystatement.h"

class QUndoStack;

/**
 * The class representing a single statement result entry
 */
class /* no export here on purpose */ StatementEntry
{
public:
    friend class JournalModel;

    explicit StatementEntry()
    {
    }
    friend void swap(StatementEntry& first, StatementEntry& second);

    explicit StatementEntry(const QString& id, const StatementEntry& other)
        : StatementEntry(other)
    {
        m_id = id;
    }

    inline QString id() const
    {
        return m_id;
    }

    inline void setName(const QString& name)
    {
        m_name = name;
    }
    inline QString name() const
    {
        return m_name;
    }

    // Match! type: 'QDate' name: 'm_statementDate'
    inline QDate statementDate() const
    {
        return m_statementDate;
    }
    inline void setStatementDate(const QDate& statementDate)
    {
        m_statementDate = statementDate;
    }

    inline MyMoneyMoney balance() const
    {
        return m_balance;
    }
    inline void setBalance(const MyMoneyMoney& balance)
    {
        m_balance = balance;
    }

    inline int transactionsCount() const
    {
        return m_transactionsCount;
    }
    inline void setTransactionsCount(const int& transactionsCount)
    {
        m_transactionsCount = transactionsCount;
    }

    inline int transactionsAdded() const
    {
        return m_transactionsAdded;
    }
    inline void setTransactionsAdded(const int& transactionsAdded)
    {
        m_transactionsAdded = transactionsAdded;
    }

    inline int transactionsMatched() const
    {
        return m_transactionsMatched;
    }
    inline void setTransactionsMatched(const int& transactionsMatched)
    {
        m_transactionsMatched = transactionsMatched;
    }

    inline int transactionDuplicates() const
    {
        return m_transactionDuplicates;
    }
    inline void setTransactionDuplicates(const int& transactionDuplicates)
    {
        m_transactionDuplicates = transactionDuplicates;
    }

    inline int payeesCreated() const
    {
        return m_payeesCreated;
    }
    inline void setPayeesCreated(const int& payeesCreated)
    {
        m_payeesCreated = payeesCreated;
    }

    KMMStringSet referencedObjects() const
    {
        return {};
    }

    inline bool hasReferenceTo(const QString& id) const
    {
        Q_UNUSED(id)
        return false;
    }

private:
    QString m_id;
    QString m_name;
    QDate m_statementDate;
    MyMoneyMoney m_balance;
    int m_transactionsCount;
    int m_transactionsAdded;
    int m_transactionsMatched;
    int m_transactionDuplicates;
    int m_payeesCreated;
};

inline void swap(StatementEntry& first, StatementEntry& second)
{
    using std::swap;
    swap(first.m_id, second.m_id);
    swap(first.m_name, second.m_name);
    swap(first.m_transactionsCount, second.m_transactionsCount);
    swap(first.m_transactionsAdded, second.m_transactionsAdded);
    swap(first.m_transactionsMatched, second.m_transactionsMatched);
    swap(first.m_transactionDuplicates, second.m_transactionDuplicates);
    swap(first.m_payeesCreated, second.m_payeesCreated);
}

/**
 */
class KMM_MODELS_EXPORT StatementModel : public MyMoneyModel<StatementEntry>
{
    Q_OBJECT
    Q_DISABLE_COPY(StatementModel)

public:
    enum Column {
        AccountName = 0,
        StatementDate,
        StatementBalance,
        Read,
        Added,
        Matched,
        Duplicate,
        PayeesCreated,

        // insert new columns above this line
        MaxColumns,
    };

    enum ColorScheme {
        Positive,
        Negative,
    };

    explicit StatementModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    virtual ~StatementModel();

    static const int ID_SIZE = 6;

    int columnCount(const QModelIndex& parent = QModelIndex()) const final override;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const final override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex& idx, const QVariant& v, int role = Qt::EditRole) override;
    QModelIndex addItem(const QString& id, const QString& name, const QModelIndex& parent);

    void clearModelItems() override;

    QString noInstitutionId() const;

    uint statementCount() const;

protected:
public Q_SLOTS:

Q_SIGNALS:

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // STATEMENTMODEL_H
