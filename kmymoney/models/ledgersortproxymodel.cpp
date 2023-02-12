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
    setDynamicSortFilter(false);
}

LedgerSortProxyModel::~LedgerSortProxyModel()
{
}

void LedgerSortProxyModel::setSourceModel(QAbstractItemModel* model)
{
    if (sourceModel()) {
        disconnect(model, &QAbstractItemModel::rowsInserted, this, &LedgerSortProxyModel::sortOnIdle);
    }
    if (model) {
        connect(model, &QAbstractItemModel::rowsInserted, this, &LedgerSortProxyModel::sortOnIdle);
    }
    QSortFilterProxyModel::setSourceModel(model);
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
    for (const auto sortOrderItem : d->ledgerSortOrder) {
        //                   SortOrder of item
        //                ascending    descending
        // trueValue        true         false
        // falseValue       false        true
        const auto trueValue = sortOrderItem.lessThanIs(true);
        const auto falseValue = sortOrderItem.lessThanIs(false);

        switch (sortOrderItem.sortRole) {
        case eMyMoney::Model::TransactionPostDateRole:
        case eMyMoney::Model::TransactionEntryDateRole:
        case eMyMoney::Model::SplitReconcileDateRole: {
            const auto leftDate = left.data(sortOrderItem.sortRole).toDate();
            const auto rightDate = right.data(sortOrderItem.sortRole).toDate();

            // in case of sorting by reconciliation date, the date
            // may be invalid and we have to react a bit different.
            if ((leftDate == rightDate) && leftDate.isValid()) {
                const auto leftModel = model->baseModel(left);
                const auto rightModel = model->baseModel(right);
                if (leftModel != rightModel) {
                    // schedules will always be presented last on the same day
                    // before that the online balance is shown
                    // before that the reconciliation records are displayed
                    // special date records are shown on top
                    // account names are shown on top
                    if (d->isSchedulesJournalModel(leftModel)) {
                        return falseValue;
                    } else if (d->isSchedulesJournalModel(rightModel)) {
                        return trueValue;
                    } else if (left.data(eMyMoney::Model::OnlineBalanceEntryRole).toBool()) {
                        return falseValue;
                    } else if (right.data(eMyMoney::Model::OnlineBalanceEntryRole).toBool()) {
                        return trueValue;
                    } else if (d->isSpecialDatesModel(leftModel)) {
                        return trueValue;
                    } else if (d->isSpecialDatesModel(rightModel)) {
                        return falseValue;
                    } else if (d->isReconciliationModel(leftModel)) {
                        return falseValue;
                    } else if (d->isReconciliationModel(rightModel)) {
                        return trueValue;
                    }
                    // if we get here, both are transaction entries
                }

                // same date and same model means that the next item
                // in the sortOrderList needs to be evaluated
                break;

            } else if (sortOrderItem.sortRole == eMyMoney::Model::SplitReconcileDateRole) {
                // special handling for reconciliation date because it
                // might be invalid and has to be sorted to the end in
                // this case
                if (leftDate.isValid() && !rightDate.isValid()) {
                    return trueValue;
                }
                if (!leftDate.isValid() && rightDate.isValid()) {
                    return falseValue;
                }
                if (leftDate.isValid()) {
                    // actually, both dates are valid here but testing
                    // one for validity is enough
                    return sortOrderItem.lessThanIs(leftDate < rightDate);
                }

                // in case both are invalid, we continue with the
                // next item in the sortOrderList
                break;
            }

            return sortOrderItem.lessThanIs(leftDate < rightDate);
        }
        case eMyMoney::Model::SplitSharesRole: {
            const auto lValue = left.data(sortOrderItem.sortRole).value<MyMoneyMoney>();
            const auto rValue = right.data(sortOrderItem.sortRole).value<MyMoneyMoney>();
            if (lValue != rValue) {
                return sortOrderItem.lessThanIs(lValue < rValue);
            }
            break;
        }
        case eMyMoney::Model::SplitPayeeRole:
        case eMyMoney::Model::TransactionCounterAccountRole:
        case eMyMoney::Model::SplitSharesSuffixRole:
        case eMyMoney::Model::IdRole: {
            const auto lValue = left.data(sortOrderItem.sortRole).toString();
            const auto rValue = right.data(sortOrderItem.sortRole).toString();
            if (lValue != rValue) {
                return sortOrderItem.lessThanIs(QString::localeAwareCompare(lValue, rValue) == -1);
            }
            break;
        }

        case eMyMoney::Model::JournalSplitSecurityNameRole: {
            const auto leftSecurity = left.data(sortOrderItem.sortRole).toString();
            const auto rightSecurity = right.data(sortOrderItem.sortRole).toString();
            if (leftSecurity == rightSecurity) {
                const auto leftModel = model->baseModel(left);
                const auto rightModel = model->baseModel(right);
                if (leftModel != rightModel) {
                    if (left.data(eMyMoney::Model::SecurityAccountNameEntryRole).toBool()) {
                        return trueValue;
                    } else if (right.data(eMyMoney::Model::SecurityAccountNameEntryRole).toBool()) {
                        return falseValue;
                    }
                    // if we get here, both are transaction entries
                }
                // same security and same model means that the next item
                // in the sortOrderList needs to be evaluated
                break;
            }
            return sortOrderItem.lessThanIs(leftSecurity < rightSecurity);
        }

        case eMyMoney::Model::SplitReconcileFlagRole: {
            const auto lValue = left.data(sortOrderItem.sortRole).toInt();
            const auto rValue = right.data(sortOrderItem.sortRole).toInt();
            if (lValue != rValue) {
                return sortOrderItem.lessThanIs(lValue < rValue);
            }
            break;
        }

        case eMyMoney::Model::SplitNumberRole: {
            const auto lValue = left.data(sortOrderItem.sortRole).toString();
            const auto rValue = right.data(sortOrderItem.sortRole).toString();
            if (lValue != rValue) {
                // convert both values to numbers
                bool ok1(false);
                bool ok2(false);
                const auto n1 = lValue.toULongLong(&ok1);
                const auto n2 = rValue.toULongLong(&ok2);
                // the following four cases exist:
                // a) both are converted correct
                //    compare them directly
                // b) n1 is numeric, n2 is not
                //    numbers come first, so trueValue
                // c) n1 is not numeric, n2 is
                //    numbers come first, so falseValue
                // d) both are non numbers
                //    compare using localeAwareCompare
                if (ok1 && ok2) { // case a)
                    return sortOrderItem.lessThanIs(n1 < n2);

                } else if (ok1 && !ok2) { // case b)
                    return trueValue;

                } else if (!ok1 && ok2) { // case c)
                    return falseValue;

                } else { // case d)
                    return sortOrderItem.lessThanIs(QString::localeAwareCompare(lValue, rValue) == -1);
                }
            }
            break;
        }
        default:
            break;
        }
    }

    // same everything, let the id decide
    return left.data(eMyMoney::Model::IdRole).toString() < right.data(eMyMoney::Model::IdRole).toString();
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

void LedgerSortProxyModel::setSortingEnabled(bool enable)
{
    Q_D(LedgerSortProxyModel);
    if (d->sortEnabled != enable) {
        d->sortEnabled = enable;
        if (enable && d->sortPending) {
            doSort();
        }
    }
}
void LedgerSortProxyModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)
    Q_D(LedgerSortProxyModel);

    // call the actual sorting only if we really need to sort
    if (sortRole() >= 0) {
        if (d->sortEnabled) {
            // LedgerSortProxyModel::lessThan takes care of the sort
            // order and is based on a general ascending order
            QSortFilterProxyModel::sort(0, Qt::AscendingOrder);
            d->sortPending = false;
        } else {
            d->sortPending = true;
        }
        d->sortPostponed = false;
    }
}

void LedgerSortProxyModel::sortOnIdle()
{
    Q_D(LedgerSortProxyModel);
    if (!d->sortPostponed) {
        d->sortPostponed = true;
        // in case a recalc operation is pending, we turn it off
        // since we need to sort first. Once sorting is done,
        // the recalc will be triggered again
        d->balanceCalculationPending = false;
        QMetaObject::invokeMethod(this, &LedgerSortProxyModel::doSortOnIdle, Qt::QueuedConnection);
    }
}

void LedgerSortProxyModel::doSort()
{
    sort(0, Qt::AscendingOrder);
}

void LedgerSortProxyModel::doSortOnIdle()
{
    Q_D(LedgerSortProxyModel);
    if (d->sortPostponed) {
        doSort();
    }
}

void LedgerSortProxyModel::setLedgerSortOrder(LedgerSortOrder sortOrder)
{
    Q_D(LedgerSortProxyModel);
    if (sortOrder != d->ledgerSortOrder) {
        // the next line will turn on sorting for this model
        // but is otherwise not used (see lessThan())
        setSortRole(eMyMoney::Model::TransactionPostDateRole);
        d->ledgerSortOrder = sortOrder;
        doSort();
    }
}

LedgerSortOrder LedgerSortProxyModel::ledgerSortOrder() const
{
    Q_D(const LedgerSortProxyModel);
    return d->ledgerSortOrder;
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
