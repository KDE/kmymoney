/*
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ledgersortproxymodel.h"
#include "ledgersortproxymodel_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "journalmodel.h"
#include "ledgerviewsettings.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "specialdatesmodel.h"

using namespace eMyMoney;

LedgerSortProxyModel::LedgerSortProxyModel(LedgerSortProxyModelPrivate* dd, QObject* parent)
    : QSortFilterProxyModel(parent)
    , d_ptr(dd)
{
    setSortRole(-1); // no sorting
}

LedgerSortProxyModel::~LedgerSortProxyModel()
{
}

void LedgerSortProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [&](const QModelIndex& /* parent */, int first, int last) {
        Q_UNUSED(first)
        Q_UNUSED(last)

        Q_D(LedgerSortProxyModel);
        // mark this view for sorting but don't actually start sorting
        // until we come back to the main event loop. This allows to collect
        // multiple row insertions into the model into a single sort run.
        // This is important during import of multiple transactions.
        if (!d->sortPending) {
            d->sortPending = true;
            // in case a recalc operation is pending, we turn it off
            // since we need to sort first. Once sorting is done,
            // the recalc will be triggered again
            d->balanceCalculationPending = false;
            QMetaObject::invokeMethod(this, &LedgerSortProxyModel::doSort, Qt::QueuedConnection);
        }
    });

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

bool LedgerSortProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    Q_D(const LedgerSortProxyModel);

    // make sure that the dummy transaction is shown last in any case
    if (left.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        return false;

    } else if (right.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        return true;
    }

    const auto model = MyMoneyFile::baseModel();
    // make sure that the online balance is the last entry of a day
    // and the date headers are the first
    switch (sortRole()) {
    case eMyMoney::Model::TransactionPostDateRole:
    case eMyMoney::Model::TransactionEntryDateRole: {
        const auto leftDate = left.data(sortRole()).toDate();
        const auto rightDate = right.data(sortRole()).toDate();

        if (leftDate == rightDate) {
            const auto leftModel = model->baseModel(left);
            const auto rightModel = model->baseModel(right);
            if (leftModel != rightModel) {
                // schedules will always be presented last on the same day
                // before that the online balance is shown
                // before that the reconciliation records are displayed
                // special date records are shown on top
                if (d->isSchedulesJournalModel(leftModel)) {
                    return false;
                } else if (d->isSchedulesJournalModel(rightModel)) {
                    return true;
                } else if (d->isAccountsModel(leftModel)) {
                    return false;
                } else if (d->isAccountsModel(rightModel)) {
                    return true;
                } else if (d->isSpecialDatesModel(leftModel)) {
                    return true;
                } else if (d->isSpecialDatesModel(rightModel)) {
                    return false;
                } else if (d->isReconciliationModel(leftModel)) {
                    return false;
                } else if (d->isReconciliationModel(rightModel)) {
                    return true;
                }
                // if we get here, both are transaction entries
            }
            // So sort by cleared status
            const auto leftState = left.data(eMyMoney::Model::SplitReconcileFlagRole).toInt();
            const auto rightState = right.data(eMyMoney::Model::SplitReconcileFlagRole).toInt();

            if (leftState != rightState) {
                if (leftState > rightState)
                    return true;
                return false;
            }

            // Reconciliation state is the same, so sort by deposits first, then withdrawals
            const auto leftDeposit = left.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
            const auto rightDeposit = right.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();

            // It's a deposit if the deposit string isn't empty
            bool leftIsDeposit = leftDeposit.isPositive();
            bool rightIsDeposit = rightDeposit.isPositive();

            if (leftIsDeposit != rightIsDeposit) {
                if (leftIsDeposit)
                    return true;
                return false;
            }

            // same model, same dates, and same everything else means that the ids decide
            return left.data(eMyMoney::Model::IdRole).toString() < right.data(eMyMoney::Model::IdRole).toString();
        }
        return leftDate < rightDate;
    }

    case eMyMoney::Model::JournalSplitQuantitySortRole:
    case eMyMoney::Model::JournalSplitPriceSortRole:
    case eMyMoney::Model::JournalSplitValueSortRole:
    case eMyMoney::Model::SplitValueRole:
    case eMyMoney::Model::SplitSharesRole: {
        const auto role = sortRole();
        const auto lValue = left.data(role).value<MyMoneyMoney>();
        const auto rValue = right.data(role).value<MyMoneyMoney>();
        if (lValue != rValue) {
            return lValue < rValue;
        }
        // same value means that the ids decide
        return left.data(eMyMoney::Model::IdRole).toString() < right.data(eMyMoney::Model::IdRole).toString();

    } break;
    default:
        break;
    }
    // otherwise use normal sorting
    return QSortFilterProxyModel::lessThan(left, right);
}

bool LedgerSortProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const LedgerSortProxyModel);

    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    // only check the start date if it's not the new transaction placeholder
    if (!idx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        if (d->firstVisiblePostDate.isValid() && d->firstVisiblePostDate > idx.data(eMyMoney::Model::TransactionPostDateRole).toDate()) {
            return false;
        }
    }

    // in case it's a special date entry or reconciliation entry, we accept it
    const auto baseModel = MyMoneyFile::baseModel()->baseModel(idx);
    if (d->isSpecialDatesModel(baseModel) || d->isReconciliationModel(baseModel)) {
        return true;
    }

    // now do the filtering

    if (d->hideReconciledTransactions
        && idx.data(eMyMoney::Model::SplitReconcileFlagRole).value<eMyMoney::Split::State>() >= eMyMoney::Split::State::Reconciled) {
        return false;
    }

    return true;
}

void LedgerSortProxyModel::setHideTransactionsBefore(const QDate& date)
{
    Q_D(LedgerSortProxyModel);
    if (d->firstVisiblePostDate != date) {
        d->firstVisiblePostDate = date;
        invalidateFilter();
    }
}

void LedgerSortProxyModel::setHideReconciledTransactions(bool hide)
{
    Q_D(LedgerSortProxyModel);
    if (d->hideReconciledTransactions != hide) {
        d->hideReconciledTransactions = hide;
        invalidateFilter();
    }
}

void LedgerSortProxyModel::sort(int column, Qt::SortOrder order)
{
    // call the actual sorting only if we really need to sort
    if (sortRole() >= 0 && column >= 0) {
        QSortFilterProxyModel::sort(column, order);
    }
}

void LedgerSortProxyModel::doSort()
{
    Q_D(LedgerSortProxyModel);
    sort(sortColumn(), sortOrder());
    d->sortPending = false;
}

void LedgerSortProxyModel::dumpSourceModel() const
{
#if 0
    qDebug() << objectName() << "Dump on model" << sourceModel()->metaObject()->className() << sourceModel()->objectName();
    int row = 0;
    for (;; ++row) {
        const auto idx = sourceModel()->index(row, 0);
        if (idx.isValid()) {
            qDebug() << "Row" << row << "ID" << idx.data(eMyMoney::Model::IdRole).toString() << idx.data(eMyMoney::Model::JournalSplitAccountIdRole).toString();
        } else {
            break;
        }
    }

#if 0
    const auto qsfpm = qobject_cast<LedgerSortProxyModel*>(sourceModel());
    if (qsfpm) {
        qsfpm->dumpSourceModel();
    }
#endif
#endif
}
