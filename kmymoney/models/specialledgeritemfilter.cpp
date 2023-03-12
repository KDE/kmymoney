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
#include "ledgerfilter.h"
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
        , showReconciliationEntries(LedgerViewSettings::DontShowReconciliationHeader)
        , filterBalanceMode(SpecialLedgerItemFilter::FilterBalanceNormal)
    {
        updateDelayTimer.setSingleShot(true);
        updateDelayTimer.setInterval(20);
    }

    bool isSortingByDateFirst() const
    {
        if (sourceModel) {
            const auto sourceLedgerSortOrder = sourceModel->ledgerSortOrder();
            if (!sourceLedgerSortOrder.isEmpty()) {
                const auto role = sourceLedgerSortOrder.at(0).sortRole;
                return dateRoles.contains(role);
            }
        }
        return false;
    }
    /**
     * Returns true if the main sort key is a date (post or entry)
     * or second and the main sort key is security
     */
    bool isSortingByDate() const
    {
        if (sourceModel) {
            const auto sourceLedgerSortOrder = sourceModel->ledgerSortOrder();
            const int maxIndex = qMin(sourceLedgerSortOrder.count(), 2);
            for (int i = 0; i < maxIndex; ++i) {
                const auto role = sourceLedgerSortOrder.at(i).sortRole;
                if (dateRoles.contains(role)) {
                    return true;
                }
                // if the first one is security then
                // check next sorting parameter too
                if ((i == 0) && (role == eMyMoney::Model::JournalSplitSecurityNameRole)) {
                    continue;
                }
                break;
            }
        }
        return false;
    };

    bool isSortingBySecurity() const
    {
        if (sourceModel) {
            const auto sourceLedgerSortOrder = sourceModel->ledgerSortOrder();
            if (!sourceLedgerSortOrder.isEmpty()) {
                return (sourceLedgerSortOrder.at(0).sortRole == eMyMoney::Model::JournalSplitSecurityNameRole);
            }
        }
        return false;
    }

    bool showBalance() const
    {
        bool filterActive(false);

        if (filterBalanceMode == SpecialLedgerItemFilter::FilterBalanceNormal) {
            filterActive = q->sourceModel()->data(QModelIndex(), eMyMoney::Model::ActiveFilterRole).toBool();

        } else if (filterBalanceMode == SpecialLedgerItemFilter::FilterBalanceReconciliation) {
            filterActive = q->sourceModel()->data(QModelIndex(), eMyMoney::Model::ActiveFilterTextRole).toBool();
            filterActive |= (q->sourceModel()->data(QModelIndex(), eMyMoney::Model::ActiveFilterStateRole).value<LedgerFilter::State>()
                             != LedgerFilter::State::NotReconciled);
        }
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
        const auto order = sourceModel->ledgerSortOrder();
        if (isSortingByDate()) {
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
            if (isSortingByDate()) {
                // if sorted by entry date or by security, we need to find the balance of the oldest
                // transaction in the set. This may not be the date of the first transaction displayed
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

    bool filterAcceptsRow(const QModelIndex& idx, const QModelIndex& source_parent, int rowCount) const
    {
        switch (idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>()) {
        case eMyMoney::Model::SpecialDatesEntryRole: {
            // Don't show them if display is not sorted by date
            if (!isSortingByDateFirst())
                return false;
            // make sure we don't show trailing special date entries
            int row = idx.row() + 1;
            bool visible = false;
            QModelIndex testIdx;
            for (; !visible && row < rowCount; ++row) {
                testIdx = q->sourceModel()->index(row, 0, source_parent);
                if (testIdx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                    // the empty id is the entry for the new transaction entry
                    // we're done scanning
                    break;
                }
                if (!isSpecialDatesModel(testIdx)) {
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
            if (visible && ((idx.row() + 1) < rowCount)) {
                // check if the next is also a date entry
                testIdx = q->sourceModel()->index(idx.row() + 1, 0, source_parent);
                if (isSpecialDatesModel(testIdx)) {
                    visible = false;
                }
            }
            return visible;
        }

        case eMyMoney::Model::ReconciliationEntryRole: {
            // Don't show them if view is not sorted by date
            if (!isSortingByDate()) {
                return false;
            }
            // Depending on the setting we only show a subset
            if (showReconciliationEntries != LedgerViewSettings::ShowAllReconciliationHeader) {
                const auto filterHint = idx.data(eMyMoney::Model::ReconciliationFilterHintRole).value<eMyMoney::Model::ReconciliationFilterHint>();
                switch (showReconciliationEntries) {
                case LedgerViewSettings::DontShowReconciliationHeader:
                    if (filterHint != eMyMoney::Model::DontFilter) {
                        return false;
                    }
                    break;

                case LedgerViewSettings::ShowLastReconciliationHeader:
                    if (filterHint == eMyMoney::Model::StdFilter) {
                        return false;
                    }
                    // intentional fall through

                case LedgerViewSettings::ShowAllReconciliationHeader:
                    break;
                }
            }

            // make sure we only show reconciliation entries that are not followed by
            // another reconciliation entry. Only inspect visible items
            int row = idx.row() + 1;
            while (row < rowCount) {
                const auto testIdx = q->sourceModel()->index(row, 0, source_parent);
                if (filterAcceptsRow(testIdx, source_parent, rowCount)) {
                    if (isReconciliationModel(testIdx)) {
                        return false;
                    }
                    return true;
                }
                ++row;
            }
            return true;
        }

        default:
            break;
        }
        return true;
    }

    QSet<int> dateRoles = {
        eMyMoney::Model::TransactionPostDateRole,
        eMyMoney::Model::TransactionEntryDateRole,
        eMyMoney::Model::SplitReconcileDateRole,
    };

    LedgerSortProxyModel* sourceModel;
    QTimer updateDelayTimer;
    LedgerViewSettings::ReconciliationHeader showReconciliationEntries;
    SpecialLedgerItemFilter::FilterBalanceMode filterBalanceMode;
};

SpecialLedgerItemFilter::SpecialLedgerItemFilter(QObject* parent)
    : LedgerSortProxyModel(new SpecialLedgerItemFilterPrivate(this), parent)
{
    Q_D(SpecialLedgerItemFilter);
    setObjectName("SpecialLedgerItemFilter");
    connect(&d->updateDelayTimer, &QTimer::timeout, this, [&]() {
        Q_D(SpecialLedgerItemFilter);
        d->sourceModel->invalidate();
        d->recalculateBalances();
    });

    connect(MyMoneyFile::instance()->journalModel(), &JournalModel::balanceChanged, this, [&](const QString& accountId) {
        Q_D(SpecialLedgerItemFilter);
        const auto model = sourceModel();
        // in case we have a model assigned and
        // then we check if accountId is referring this account
        if (model) {
            const auto rows = model->rowCount();
            // we scan the rows for the first one containing a split
            // referencing an account to check if it is ours
            for (int row = 0; row < rows; ++row) {
                const auto idx = model->index(row, 0);
                if (!idx.data(eMyMoney::Model::IdRole).toString().isEmpty() && d->isJournalModel(idx)) {
                    if (idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString() == accountId) {
                        forceReload();
                        // we found an account id in the ledger
                        // so we can stop scanning
                        break;
                    }
                }
            }
        }
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
        disconnect(sourceModel(), &QAbstractItemModel::rowsMoved, this, &SpecialLedgerItemFilter::forceReload);
        disconnect(sourceModel(), &QAbstractItemModel::modelReset, this, &SpecialLedgerItemFilter::forceReload);
    }
    LedgerSortProxyModel::setSourceModel(model);
    if (model) {
        connect(model, &QAbstractItemModel::rowsRemoved, this, &SpecialLedgerItemFilter::forceReload);
        connect(model, &QAbstractItemModel::rowsInserted, this, &SpecialLedgerItemFilter::forceReload);
        connect(model, &QAbstractItemModel::rowsMoved, this, &SpecialLedgerItemFilter::forceReload);
        connect(model, &QAbstractItemModel::modelReset, this, &SpecialLedgerItemFilter::forceReload);
    }
    d->sourceModel = model;
}

void SpecialLedgerItemFilter::setLedgerSortOrder(LedgerSortOrder sortOrder)
{
    Q_D(SpecialLedgerItemFilter);
    if (d->sourceModel) {
        d->ledgerSortOrder = sortOrder;
        d->sourceModel->setLedgerSortOrder(sortOrder);
    }
}

LedgerSortOrder SpecialLedgerItemFilter::ledgerSortOrder() const
{
    Q_D(const SpecialLedgerItemFilter);
    if (d->sourceModel) {
        return d->sourceModel->ledgerSortOrder();
    }
    return {};
}
void SpecialLedgerItemFilter::sort(int column, Qt::SortOrder order)
{
    Q_D(SpecialLedgerItemFilter);

    if (column >= 0) {
        // propagate the sorting to the source model and
        // update balances
        d->sourceModel->sort(column, order);
        d->recalculateBalances();
    }
}

void SpecialLedgerItemFilter::setSortingEnabled(bool enable)
{
    Q_D(SpecialLedgerItemFilter);

    if (d->sortEnabled != enable) {
        d->sortEnabled = enable;
        // propagate setting to source model. This
        // will do the sorting if needed. We only
        // need to recalc the balance afterwards.
        d->sourceModel->setSortingEnabled(enable);
        if (enable) {
            invalidateFilter();
            d->recalculateBalances();
        }
    }
}

void SpecialLedgerItemFilter::setShowReconciliationEntries(LedgerViewSettings::ReconciliationHeader show)
{
    Q_D(SpecialLedgerItemFilter);

    if (d->showReconciliationEntries != show) {
        d->showReconciliationEntries = show;
        invalidateFilter();
    }
}

void SpecialLedgerItemFilter::doSortOnIdle()
{
    Q_D(SpecialLedgerItemFilter);
    d->sourceModel->doSortOnIdle();
}

void SpecialLedgerItemFilter::setFilterBalanceMode(SpecialLedgerItemFilter::FilterBalanceMode mode)
{
    Q_D(SpecialLedgerItemFilter);
    d->filterBalanceMode = mode;
}

bool SpecialLedgerItemFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const SpecialLedgerItemFilter);

    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

    switch (idx.data(eMyMoney::Model::BaseModelRole).value<eMyMoney::Model::Roles>()) {
    case eMyMoney::Model::SpecialDatesEntryRole:
    case eMyMoney::Model::ReconciliationEntryRole:
        return d->filterAcceptsRow(idx, source_parent, sourceModel()->rowCount(source_parent));

    case eMyMoney::Model::OnlineBalanceEntryRole:
        // Don't show online balance items if display is not sorted by date
        if (!d->isSortingByDate()) {
            return false;
        }
        break;

    case eMyMoney::Model::SecurityAccountNameEntryRole:
        // Don't show online balance items if display is not sorted by date
        if (!d->isSortingBySecurity()) {
            return false;
        }
        break;

    default:
        break;
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
