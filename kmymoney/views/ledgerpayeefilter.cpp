/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ledgerpayeefilter.h"
#include "ledgerfilterbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "journalmodel.h"
#include "accountsmodel.h"

class LedgerPayeeFilterPrivate : public LedgerFilterBasePrivate
{
public:
    explicit LedgerPayeeFilterPrivate(LedgerPayeeFilter* qq)
        : LedgerFilterBasePrivate(qq)
        , balanceCalculationPending(false)
    {}

    ~LedgerPayeeFilterPrivate()
    {
    }

    bool                        balanceCalculationPending;
};


LedgerPayeeFilter::LedgerPayeeFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels)
    : LedgerFilterBase(new LedgerPayeeFilterPrivate(this), parent)
{
    Q_D(LedgerPayeeFilter);

    setFilterRole(eMyMoney::Model::SplitPayeeIdRole);
    setObjectName("LedgerPayeeFilter");
    setFilterKeyColumn(0);

    d->concatModel->setObjectName("LedgerView concatModel");
    d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());

    for (const auto model : specialJournalModels) {
        d->concatModel->addSourceModel(model);
    }

    setSourceModel(d->concatModel);
}

LedgerPayeeFilter::~LedgerPayeeFilter()
{
}

void LedgerPayeeFilter::setShowBalanceInverted(bool inverted)
{
    Q_D(LedgerPayeeFilter);
    d->showValuesInverted = inverted;
}

void LedgerPayeeFilter::recalculateBalancesOnIdle(const QString& accountId)
{
    Q_UNUSED(accountId);
    Q_D(LedgerPayeeFilter);

    // make sure the balances are recalculated but trigger only once
    if(!d->balanceCalculationPending) {
        d->balanceCalculationPending = true;
        QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
    }
}

void LedgerPayeeFilter::recalculateBalances()
{
    Q_D(LedgerPayeeFilter);

    if (sourceModel() == nullptr || d->filterIds.isEmpty())
        return;

    /// @todo port to new model code
#if 0
    const auto start = index(0, 0);
    const auto indexes = match(start, eMyMoney::Model::SplitAccountIdRole, d->payeeId, -1);
    MyMoneyMoney balance;
    for(const auto &idx : indexes) {
        if(d->showValuesInverted) {
            balance -= idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        } else {
            balance += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        }
        const auto dispIndex = index(idx.row(), JournalModel::Column::Balance);
        setData(dispIndex, QVariant::fromValue(balance), Qt::DisplayRole);
    }
#endif

    // filterModel->invalidate();
    const QModelIndex top = index(0, JournalModel::Column::Balance);
    const QModelIndex bottom = index(rowCount()-1, JournalModel::Column::Balance);

    dataChanged(top, bottom);
    d->balanceCalculationPending = false;
}

void LedgerPayeeFilter::setPayeeIdList(const QStringList& payeeIds)
{
    Q_D(LedgerPayeeFilter);

    setFilterFixedStrings(payeeIds);

    /// @todo sorting - move to new sort definition
    setSortRole(eMyMoney::Model::TransactionPostDateRole);
    sort(JournalModel::Column::Date, sortOrder());

    // if balance calculation has not been triggered, then run it immediately
    if(!d->balanceCalculationPending) {
        recalculateBalances();
    }

    invalidateFilter();
}

bool LedgerPayeeFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const LedgerPayeeFilter);

    bool rc = LedgerFilterBase::filterAcceptsRow(source_row,  source_parent);

    if (rc) {
        QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);

        // special dates are shown when sorted by date
        const auto baseModel = MyMoneyFile::baseModel()->baseModel(idx);
        if (d->isSpecialDatesModel(baseModel)) {
            return (sortRole() == eMyMoney::Model::TransactionPostDateRole);
        }

        // only display splits that reference an asset or liability account
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        idx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
        const auto accountGroup = static_cast<eMyMoney::Account::Type>(idx.data(eMyMoney::Model::AccountGroupRole).toInt());
        switch(accountGroup) {
        case eMyMoney::Account::Type::Asset:
        case eMyMoney::Account::Type::Liability:
            break;
        default:
            return false;
        }
    }
    return rc;
}
