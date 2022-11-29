/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "specialledgeritemfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

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
public:
    SpecialLedgerItemFilterPrivate(SpecialLedgerItemFilter* qq)
        : LedgerSortProxyModelPrivate(qq)
    {
    }

};

SpecialLedgerItemFilter::SpecialLedgerItemFilter(QObject* parent)
    : LedgerSortProxyModel(new SpecialLedgerItemFilterPrivate(this), parent)
{
    setObjectName("SpecialLedgerItemFilter");
}

void SpecialLedgerItemFilter::setSourceModel(QAbstractItemModel* model)
{
    Q_UNUSED(model)
    qDebug() << "This must never be called";
}

void SpecialLedgerItemFilter::setSourceModel(LedgerSortProxyModel* model)
{
    LedgerSortProxyModel::setSourceModel(model);
}

void SpecialLedgerItemFilter::sort(int column, Qt::SortOrder order)
{
    Q_D(SpecialLedgerItemFilter);

    if (column >= 0) {
        // propagate the sorting to the source model and make
        // sure the right sort role is set
        LedgerSortProxyModel* model = qobject_cast<LedgerSortProxyModel*>(sourceModel());
        const auto role = d->columnToSortRole(column);
        if (role != model->sortRole()) {
            // setting a different sort role implies sorting
            model->setSortRole(role);
        }
        model->sort(column, order);
        dumpSourceModel();
    }
}

bool SpecialLedgerItemFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const SpecialLedgerItemFilter);

    auto isSortingByDate = [&]() {
        const auto qsfpm = qobject_cast<QSortFilterProxyModel*>(sourceModel());
        if (qsfpm) {
            const auto role = qsfpm->sortRole();
            return (role == eMyMoney::Model::TransactionPostDateRole) || (role == eMyMoney::Model::TransactionEntryDateRole);
        }
        return false;
    };

    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    const auto baseModel = MyMoneyModelBase::baseModel(idx);

    if (d->isSpecialDatesModel(baseModel)) {
        // Don't show them if display is not sorted by date
        if (!isSortingByDate())
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
        if (!isSortingByDate())
            return false;
        // make sure we only show reconciliation entries that are followed by
        // a regular transaction
        int row = source_row + 1;
        const auto testIdx = sourceModel()->index(row, 0, source_parent);
        return !d->isReconciliationModel(testIdx);

    } else if (d->isAccountsModel(baseModel)) {
        // Don't show online balance items if display is not sorted by date
        if (!isSortingByDate())
            return false;
    }
    return true;
}

void SpecialLedgerItemFilter::forceReload()
{
    invalidateFilter();
}
