/*
    SPDX-FileCopyrightText: 2019-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "specialdatesfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyfile.h"
#include "reconciliationmodel.h"
#include "specialdatesmodel.h"

using namespace eMyMoney;

class SpecialDatesFilterPrivate
{
public:
    SpecialDatesFilterPrivate()
        : specialDatesModel(nullptr)
    {
    }

    inline bool isSpecialDatesModel(const QAbstractItemModel* model) const
    {
        return (model == specialDatesModel);
    }

    inline bool isReconciliatonModel(const QAbstractItemModel* model) const
    {
        return (model == reconciliationModel);
    }

    QAbstractItemModel* specialDatesModel;
    QAbstractItemModel* reconciliationModel;
};

SpecialDatesFilter::SpecialDatesFilter(QObject* parent)
    : QSortFilterProxyModel(parent)
    , d_ptr(new SpecialDatesFilterPrivate)
{
    Q_D(SpecialDatesFilter);
    const auto file = MyMoneyFile::instance();
    d->specialDatesModel = file->specialDatesModel();
    d->reconciliationModel = file->reconciliationModel();
}

bool SpecialDatesFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const SpecialDatesFilter);

    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    const auto baseModel = MyMoneyModelBase::baseModel(idx);
    if (d->isSpecialDatesModel(baseModel)) {
        // make sure we don't show trailing special date entries
        const auto rows = sourceModel()->rowCount(source_parent);
        int row = source_row + 1;
        bool visible = false;
        QModelIndex testIdx;
        for (; !visible && row < rows; ++row) {
            testIdx = sourceModel()->index(row, 0, source_parent);
            if(testIdx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                // the empty id is the entry for the new transaction entry
                // we're done scanning
                break;
            }
            const auto testModel = MyMoneyModelBase::baseModel(idx);
            if (!d->isSpecialDatesModel(testModel)) {
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
            testIdx = sourceModel()->index(source_row+1, 0, source_parent);
            const auto testModel = MyMoneyModelBase::baseModel(idx);
            if (d->isSpecialDatesModel(testModel)) {
                visible = false;
            }
        }
        return visible;

    } else if (d->isReconciliatonModel(baseModel)) {
        // make sure we only show reconciliation entries that are followed by
        // a regular transaction
        int row = source_row + 1;
        QModelIndex testIdx;
        testIdx = sourceModel()->index(row, 0, source_parent);
        if (MyMoneyModelBase::baseModel(testIdx) == MyMoneyFile::instance()->journalModel()) {
            return true;
        }
        return false;
    }

    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void SpecialDatesFilter::forceReload()
{
    invalidateFilter();
}
