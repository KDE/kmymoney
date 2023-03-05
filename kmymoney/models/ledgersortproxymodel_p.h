/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LEDGERSORTPROXYMODEL_P_H
#define LEDGERSORTPROXYMODEL_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyfile.h"

class LedgerSortProxyModelPrivate
{
public:
    LedgerSortProxyModelPrivate(LedgerSortProxyModel* qq)
        : q(qq)
        , hideReconciledTransactions(false)
        , balanceCalculationPending(false)
        , sortEnabled(false)
        , sortPending(false)
        , sortPostponed(false)
    {
    }

    inline bool isAccountsModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->accountsModel()));
    }

    inline bool isSpecialDatesModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->specialDatesModel()));
    }

    inline bool isSpecialDatesModel(const QModelIndex& idx) const
    {
        return idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>() == eMyMoney::Model::SpecialDatesEntryRole;
    }

    inline bool isReconciliationModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->reconciliationModel()));
    }

    inline bool isReconciliationModel(const QModelIndex& idx) const
    {
        return idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>() == eMyMoney::Model::ReconciliationEntryRole;
    }

    inline bool isOnlineBalanceModel(const QModelIndex& idx) const
    {
        return idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>() == eMyMoney::Model::OnlineBalanceEntryRole;
    }

    inline bool isJournalModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->journalModel()));
    }

    inline bool isJournalModel(const QModelIndex& idx) const
    {
        return idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>() == eMyMoney::Model::JournalEntryRole;
    }

    inline bool isSchedulesJournalModel(const QAbstractItemModel* model) const
    {
        return (model == static_cast<void*>(MyMoneyFile::instance()->schedulesJournalModel()));
    }

    inline bool isSchedulesJournalModel(const QModelIndex& idx) const
    {
        return idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>() == eMyMoney::Model::SchedulesJournalEntryRole;
    }

    QString modelType(const QAbstractItemModel* model) const
    {
        if (isAccountsModel(model))
            return QLatin1String("AccountsModel");
        if (isSpecialDatesModel(model))
            return QLatin1String("SpecialDatesModel");
        if (isReconciliationModel(model))
            return QLatin1String("ReconciliationModel");
        if (isJournalModel(model))
            return QLatin1String("JournalModel");
        if (isSchedulesJournalModel(model))
            return QLatin1String("SchedulesJournalModel");
        return QLatin1String("unknown model");
    }

    int columnToSortRole(int column) const
    {
        const QHash<int, int> columnToRole = {
            {JournalModel::Column::Number, eMyMoney::Model::JournalSplitNumberRole},
            {JournalModel::Column::EntryDate, eMyMoney::Model::TransactionEntryDateRole},
            {JournalModel::Column::Date, eMyMoney::Model::TransactionPostDateRole},
            {JournalModel::Column::Account, Qt::DisplayRole},
            {JournalModel::Column::Payee, eMyMoney::Model::SplitPayeeRole},
            {JournalModel::Column::Security, Qt::DisplayRole},
            {JournalModel::Column::CostCenter, Qt::DisplayRole},
            {JournalModel::Column::Detail, Qt::DisplayRole},
            {JournalModel::Column::Reconciliation, Qt::DisplayRole},
            {JournalModel::Column::Payment, eMyMoney::Model::SplitValueRole},
            {JournalModel::Column::Deposit, eMyMoney::Model::SplitValueRole},
            {JournalModel::Column::Quantity, eMyMoney::Model::JournalSplitQuantitySortRole},
            {JournalModel::Column::Price, eMyMoney::Model::JournalSplitPriceSortRole},
            {JournalModel::Column::Amount, Qt::DisplayRole},
            {JournalModel::Column::Value, eMyMoney::Model::JournalSplitValueSortRole},
            {JournalModel::Column::Balance, eMyMoney::Model::JournalBalanceRole},
        };

        return columnToRole.value(column, Qt::DisplayRole);
    }

    LedgerSortProxyModel* q;
    QDate firstVisiblePostDate;
    LedgerSortOrder ledgerSortOrder;
    bool hideReconciledTransactions;
    bool balanceCalculationPending;
    /**
     * This flag controls if sorting is enabled or not. It is
     * used to temporarily bypass sorting to avoid sorting a
     * model multiple times with the same state.
     */
    bool sortEnabled;

    /**
     * This flag is set when a call to sort() happened while
     * sorting was disabled so that it can be fetched later.
     *
     * @sa sortEnabled
     */
    bool sortPending;

    /**
     * Some sort operations are postponed until the next
     * run of the event loop. This flag controls if such
     * a sort is still pending.
     */
    bool sortPostponed;
};

#endif // LEDGERSORTPROXYMODEL_P_H
