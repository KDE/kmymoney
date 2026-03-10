/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RECONCILIATIONMODEL_H
#define RECONCILIATIONMODEL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KColorScheme>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneymodel.h"
#include "mymoneymoney.h"

class /* no export here on purpose */ ReconciliationEntry
{
public:
    explicit ReconciliationEntry()
    {
    }
    explicit ReconciliationEntry(const QString& id, const ReconciliationEntry& other)
        : m_id(id)
        , m_accountId(other.m_accountId)
        , m_amount(other.m_amount)
        , m_date(other.m_date)
        , m_reconciliationInProgress(other.m_reconciliationInProgress)
        , m_filterHint(other.m_filterHint)
        , m_backgroundRole(other.m_backgroundRole)
    {
    }

    ReconciliationEntry(const QString& id,
                        const QString& accountId,
                        const QDate& date,
                        const MyMoneyMoney& amount,
                        eMyMoney::Model::ReconciliationFilterHint filterHint,
                        bool inProgress = false)
        : m_id(id)
        , m_accountId(accountId)
        , m_amount(amount)
        , m_date(date)
        , m_reconciliationInProgress(inProgress)
        , m_lastReconciliation(false)
        , m_filterHint(filterHint)
        , m_backgroundRole(KColorScheme::PositiveBackground)
    {
    }

    inline const QString& id() const
    {
        return m_id;
    }
    inline const QString& accountId() const
    {
        return m_accountId;
    }
    inline const MyMoneyMoney& amount() const
    {
        return m_amount;
    }
    inline const QDate& date() const
    {
        return m_date;
    }
    inline bool hasReferenceTo(const QString&) const
    {
        return false;
    }
    inline eMyMoney::Model::ReconciliationFilterHint filterHint() const
    {
        return m_filterHint;
    }
    inline bool isReconciliationInProgress() const
    {
        return m_reconciliationInProgress;
    }
    inline KColorScheme::BackgroundRole backgroundColorRole() const
    {
        return m_backgroundRole;
    }
    inline void setBackgroundColorRole(KColorScheme::BackgroundRole colorRole)
    {
        m_backgroundRole = colorRole;
    }
    inline void setLastReconciliation(bool isLast)
    {
        m_lastReconciliation = isLast;
    }
    inline bool isLastReconciliation() const
    {
        return m_lastReconciliation;
    }

    /**
     * @copydoc MyMoneyObject::referencedObjects
     */
    inline KMMStringSet referencedObjects() const
    {
        return {};
    }

private:
    QString m_id; ///< the record's ID'
    QString m_accountId; ///< the account's ID
    MyMoneyMoney m_amount; ///< the reconciliation amount
    QDate m_date; ///< the reconciliation date
    bool m_reconciliationInProgress; ///< set to true for the current reconciliation record
    bool m_lastReconciliation; ///< set to true for the last record in history
    eMyMoney::Model::ReconciliationFilterHint m_filterHint; ///< hint for filtering records
    KColorScheme::BackgroundRole m_backgroundRole; ///< background color control
};

class QUndoStack;
/**
 */
class KMM_MYMONEY_EXPORT ReconciliationModel : public MyMoneyModel<ReconciliationEntry>
{
    Q_OBJECT

public:
    explicit ReconciliationModel(QObject* parent = nullptr, QUndoStack* undoStack = nullptr);
    ~ReconciliationModel();

    static const int ID_SIZE = 4;

    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const final override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    void setOptions(bool showDateHeaders);

    /**
     * Return the index to the record for the current reconciliation
     * in account @a accountId.
     */
    QModelIndex currentReconciliationIndex(const QString& accountId) const;

public Q_SLOTS:
    void updateData();

private Q_SLOTS:
    /**
     * override the MyMoneyModel::load() method here so that it cannot
     * be called, as it is useless in the context of this class
     */
    void load(const QMap<QString, ReconciliationEntry>& list) override;
    void doLoad();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // RECONCILIATIONMODEL_H
