/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "specialledgeritemfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "ledgersortproxymodel_p.h"
#include "mymoneyfile.h"
#include "reconciliationmodel.h"
#include "specialdatesmodel.h"

using namespace eMyMoney;

class SpecialLedgerItemFilterPrivate : public LedgerSortProxyModelPrivate
{
    struct BalanceParameter {
        Qt::SortOrder sortOrder;
        int startRow;
        int lastRow;
        int firstRow;
        bool valid = false;

        void setFirstRow(int row)
        {
            firstRow = row;
            valid = true;
        }
        bool isValid() const
        {
            return valid;
        }
    };

public:
    SpecialLedgerItemFilterPrivate(SpecialLedgerItemFilter* qq)
        : LedgerSortProxyModelPrivate(qq)
    {
        updateDelayTimer.setSingleShot(true);
        updateDelayTimer.setInterval(20);
    }

    bool isSortingByDate() const
    {
        if (sourceModel) {
            const auto role = sourceModel->sortRole();
            return (role == eMyMoney::Model::TransactionPostDateRole) || (role == eMyMoney::Model::TransactionEntryDateRole);
        }
        return false;
    };

    bool showBalance() const
    {
        const auto filterActive = q->sourceModel()->data(QModelIndex(), eMyMoney::Model::ActiveFilterRole).toBool();
        return isSortingByDate() && !filterActive;
    }

    /**
     * initializeBalanceCalculation sets up the BalanceParameter structure
     * with the row values to be visited for balance calculation. If sort order
     * is ascending, rows are positive. If sort order is descending, row values
     * are negative, so that startRow is always smaller than lastRow. This
     * simplifies the calculation of the balance.
     *
     * @Note One must pay attention to use @c qAbs(x) when creating the QModelIndex
     * to access the data.
     */
    BalanceParameter initializeBalanceCalculation() const
    {
        BalanceParameter parameter;
        // we only display balances when sorted by a date field
        if ((sourceModel->sortRole() == eMyMoney::Model::TransactionEntryDateRole) || (sourceModel->sortRole() == eMyMoney::Model::TransactionPostDateRole)) {
            const auto rows = q->rowCount();
            if (rows > 0) {
                parameter.sortOrder = sourceModel->sortOrder();
                parameter.startRow = (parameter.sortOrder == Qt::AscendingOrder) ? 0 : -(rows - 1);
                parameter.lastRow = (parameter.sortOrder == Qt::AscendingOrder) ? rows - 1 : 0;

                for (int row = parameter.startRow; (row >= parameter.startRow) && (row <= parameter.lastRow); ++row) {
                    const auto idx = q->index(qAbs(row), 0);
                    if (isJournalModel(idx) || isSchedulesJournalModel(idx)) {
                        parameter.setFirstRow(row);
                        break;
                    }
                }
            }
        }
        return parameter;
    }

    void recalculateBalances()
    {
        QMap<QString, MyMoneyMoney> balances;

        const auto parameter = initializeBalanceCalculation();

        if (parameter.isValid()) {
            auto idx = q->index(qAbs(parameter.firstRow), 0);
            const bool showValuesInverted(idx.data(eMyMoney::Model::ShowValueInvertedRole).toBool());
            auto accountId = idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString();
            const auto account = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
            const bool isInvestmentAccount = (account.accountType() == eMyMoney::Account::Type::Investment) || account.isInvest();

            QDate startDate = idx.data(eMyMoney::Model::TransactionPostDateRole).toDate();
            if (sourceModel->sortRole() == eMyMoney::Model::TransactionEntryDateRole) {
                // if sorted by entry date, we need to find the balance of the oldest
                // transaction shown. This may not be the date of the first transaction entered
                for (int row = parameter.firstRow; (row >= parameter.startRow) && (row <= parameter.lastRow); ++row) {
                    idx = q->index(qAbs(row), 0);
                    if (!idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                        if (idx.data(eMyMoney::Model::TransactionPostDateRole).toDate() < startDate) {
                            startDate = idx.data(eMyMoney::Model::TransactionPostDateRole).toDate();
                        }
                    }
                }
            }

            // we need to get the balance of the day prior to the first day found
            startDate = startDate.addDays(-1);

            // take care of
            for (int row = parameter.firstRow; (row >= parameter.startRow) && (row <= parameter.lastRow); ++row) {
                // check if we have the balance for this account
                idx = q->index(qAbs(row), 0);
                if (!(isJournalModel(idx) || isSchedulesJournalModel(idx))) {
                    continue;
                }

                accountId = idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString();
                if (balances.constFind(accountId) == balances.constEnd()) {
                    balances[accountId] = MyMoneyFile::instance()->balance(accountId, startDate);
                }

                const auto shares = idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                if (isInvestmentAccount) {
                    if (idx.data(eMyMoney::Model::TransactionIsStockSplitRole).toBool()) {
                        balances[accountId] = MyMoneyFile::instance()->journalModel()->stockSplitBalance(accountId, balances[accountId], shares);
                    } else {
                        balances[accountId] += shares;
                    }

                } else {
                    if (showValuesInverted) {
                        balances[accountId] -= shares;
                    } else {
                        balances[accountId] += shares;
                    }
                }
                q->setData(idx, QVariant::fromValue(balances[accountId]), eMyMoney::Model::JournalBalanceRole);
            }
        }
    }

    LedgerSortProxyModel* sourceModel;
    QTimer updateDelayTimer;
};

SpecialLedgerItemFilter::SpecialLedgerItemFilter(QObject* parent)
    : LedgerSortProxyModel(new SpecialLedgerItemFilterPrivate(this), parent)
{
    Q_D(SpecialLedgerItemFilter);
    setObjectName("SpecialLedgerItemFilter");
    connect(&d->updateDelayTimer, &QTimer::timeout, this, [&]() {
        Q_D(SpecialLedgerItemFilter);
        invalidateFilter();
        d->recalculateBalances();
    });
}

void SpecialLedgerItemFilter::setSourceModel(QAbstractItemModel* model)
{
    Q_UNUSED(model)
    qDebug() << "This must never be called";
}

void SpecialLedgerItemFilter::setSourceModel(LedgerSortProxyModel* model)
{
    Q_D(SpecialLedgerItemFilter);
    if (sourceModel()) {
        disconnect(sourceModel(), &QAbstractItemModel::rowsRemoved, this, &SpecialLedgerItemFilter::forceReload);
        disconnect(sourceModel(), &QAbstractItemModel::rowsInserted, this, &SpecialLedgerItemFilter::forceReload);
        disconnect(sourceModel(), &QAbstractItemModel::modelReset, this, &SpecialLedgerItemFilter::forceReload);
    }
    LedgerSortProxyModel::setSourceModel(model);
    if (model) {
        connect(model, &QAbstractItemModel::rowsRemoved, this, &SpecialLedgerItemFilter::forceReload);
        connect(model, &QAbstractItemModel::rowsInserted, this, &SpecialLedgerItemFilter::forceReload);
        connect(model, &QAbstractItemModel::modelReset, this, &SpecialLedgerItemFilter::forceReload);
    }
    d->sourceModel = model;
}

void SpecialLedgerItemFilter::sort(int column, Qt::SortOrder order)
{
    Q_D(SpecialLedgerItemFilter);

    if (column >= 0) {
        // propagate the sorting to the source model and make
        // sure the right sort role is set
        const auto role = d->columnToSortRole(column);
        if (role != d->sourceModel->sortRole()) {
            // setting a different sort role implies sorting
            d->sourceModel->setSortRole(role);
        }
        d->sourceModel->sort(column, order);
        d->recalculateBalances();
    }
}

bool SpecialLedgerItemFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const SpecialLedgerItemFilter);

    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    const auto baseModel = MyMoneyModelBase::baseModel(idx);

    if (d->isSpecialDatesModel(baseModel)) {
        // Don't show them if display is not sorted by date
        if (!d->isSortingByDate())
            return false;
        // make sure we don't show trailing special date entries
        const auto rows = sourceModel()->rowCount(source_parent);
        int row = source_row + 1;
        bool visible = false;
        QModelIndex testIdx;
        for (; !visible && row < rows; ++row) {
            testIdx = sourceModel()->index(row, 0, source_parent);
            if (testIdx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                // the empty id is the entry for the new transaction entry
                // we're done scanning
                break;
            }
            if (!d->isSpecialDatesModel(testIdx)) {
                // we did not hit a special date entry
                // now we need to check for a real transaction or the online balance one
                if (!testIdx.data(eMyMoney::Model::JournalTransactionIdRole).toString().isEmpty()) {
                    visible = true;
                }
                break;
            }
        }

        // in case this is not a trailing date entry, we need to check
        // if it is the last of a row of date entries.
        if (visible && ((source_row + 1) < rows)) {
            // check if the next is also a date entry
            testIdx = sourceModel()->index(source_row + 1, 0, source_parent);
            if (d->isSpecialDatesModel(testIdx)) {
                visible = false;
            }
        }
        return visible;

    } else if (d->isReconciliationModel(baseModel)) {
        // Don't show them if display is not sorted by date
        if (!d->isSortingByDate())
            return false;
        // make sure we only show reconciliation entries that are followed by
        // a regular transaction
        int row = source_row + 1;
        const auto testIdx = sourceModel()->index(row, 0, source_parent);
        return !(d->isReconciliationModel(testIdx) || d->isSpecialDatesModel(testIdx));

    } else if (d->isAccountsModel(baseModel)) {
        // Don't show online balance items if display is not sorted by date
        if (!d->isSortingByDate())
            return false;
    }
    return true;
}

void SpecialLedgerItemFilter::forceReload()
{
    Q_D(SpecialLedgerItemFilter);
    d->updateDelayTimer.start();
}

QVariant SpecialLedgerItemFilter::data(const QModelIndex& index, int role) const
{
    Q_D(const SpecialLedgerItemFilter);
    if (index.column() == JournalModel::Balance) {
        switch (role) {
        case Qt::DisplayRole:
            if (!d->showBalance() && !index.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                return QLatin1String("---");
            }
            break;
        case Qt::TextAlignmentRole:
            if (!d->showBalance()) {
                return QVariant(Qt::AlignHCenter | Qt::AlignTop);
            }
            break;
        default:
            break;
        }
    }
    return LedgerSortProxyModel::data(index, role);
}
