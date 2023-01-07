/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "ledgeraccountfilter.h"
#include "ledgerfilterbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "journalmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "onlinebalanceproxymodel.h"
#include "schedulesjournalmodel.h"
#include "specialdatesmodel.h"

class LedgerAccountFilterPrivate : public LedgerFilterBasePrivate
{
public:
    explicit LedgerAccountFilterPrivate(LedgerAccountFilter* qq)
        : LedgerFilterBasePrivate(qq)
        , onlinebalanceproxymodel(nullptr)
    {}

    ~LedgerAccountFilterPrivate()
    {
    }

    OnlineBalanceProxyModel*    onlinebalanceproxymodel;
    MyMoneyAccount              account;
};


LedgerAccountFilter::LedgerAccountFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels)
    : LedgerFilterBase(new LedgerAccountFilterPrivate(this), parent)
{
    Q_D(LedgerAccountFilter);
    d->onlinebalanceproxymodel = new OnlineBalanceProxyModel(parent);
    setMaintainBalances(true);

    setObjectName("LedgerAccountFilter");

    d->concatModel->setObjectName("LedgerView concatModel");
    d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());

    d->onlinebalanceproxymodel->setObjectName("OnlineBalanceProxyModel");
    d->onlinebalanceproxymodel->setSourceModel(MyMoneyFile::instance()->accountsModel());
    d->concatModel->addSourceModel(d->onlinebalanceproxymodel);

    for (const auto model : specialJournalModels) {
        d->concatModel->addSourceModel(model);
    }

    setFilterRole(eMyMoney::Model::SplitAccountIdRole);

    setSourceModel(d->concatModel);
}

LedgerAccountFilter::~LedgerAccountFilter()
{
}

void LedgerAccountFilter::setShowBalanceInverted(bool inverted)
{
    Q_D(LedgerAccountFilter);
    d->showValuesInverted = inverted;
}

void LedgerAccountFilter::recalculateBalancesOnIdle(const QString& accountId)
{
    Q_D(LedgerAccountFilter);
    // only start recalc if the caller means us
    bool updateCausedBySubAccount = false;
    if (d->account.accountType() == eMyMoney::Account::Type::Investment) {
        updateCausedBySubAccount = d->account.accountList().contains(accountId);
    }
    if ((accountId.compare(d->account.id()) == 0) || updateCausedBySubAccount) {
        // make sure the balances are recalculated but trigger only once
        // if sorting is pending, we don't trigger recalc as it is part of sorting
        if(!d->balanceCalculationPending && !d->sortPending) {
            d->balanceCalculationPending = true;
            QMetaObject::invokeMethod(this, &LedgerAccountFilter::recalculateBalances, Qt::QueuedConnection);
        }
    }
}

void LedgerAccountFilter::recalculateBalances()
{
    Q_D(LedgerAccountFilter);

    // false alert. we could end up here in case a recalc is triggered
    // and turned off by sorting afterwards. In this case, we simply
    // skip the calculation as it is in vain.
    if(!d->balanceCalculationPending) {
        return;
    }

    if (sourceModel() == nullptr || d->account.id().isEmpty())
        return;

    // we need to operate on our own source model (not filtered,
    // sorted by date and including schedules)
    // and update only the selected account(s). In case of investment
    // accounts, we also update the balance of the underlying stock
    // accounts.
    bool isInvestmentAccount = false;
    QStringList accountIds;
    accountIds << d->account.id();
    if (d->account.accountType() == eMyMoney::Account::Type::Investment) {
        isInvestmentAccount = true;
        accountIds << d->account.accountList();
    }

    QHash<QString, MyMoneyMoney> balances;
    const auto rows = sourceModel()->rowCount();
    QModelIndex idx;
    QString accountId;
    const auto file = MyMoneyFile::instance();

    for (int row = 0; row < rows; ++row) {
        idx = sourceModel()->index(row, 0);
        const auto baseModel = MyMoneyModelBase::baseModel(idx);
        if (baseModel) {
            if (baseModel == file->journalModel() || baseModel == file->schedulesJournalModel()) {
                accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
                if (accountIds.contains(accountId)) {
                    if (isInvestmentAccount) {
                        if (idx.data(eMyMoney::Model::TransactionIsStockSplitRole).toBool()) {
                            balances[accountId] =
                                MyMoneyFile::instance()->journalModel()->stockSplitBalance(accountId,
                                                                                           balances[accountId],
                                                                                           idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
                        } else {
                            balances[accountId] += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                        }

                    } else {
                        if (d->showValuesInverted) {
                            balances[accountId] -= idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                        } else {
                            balances[accountId] += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                        }
                    }
                    const auto dispIndex = sourceModel()->index(row, JournalModel::Column::Balance);
                    sourceModel()->setData(dispIndex, QVariant::fromValue(balances[accountId]), Qt::DisplayRole);
                }
            }
        }
    }
    Q_EMIT dataChanged(index(0, JournalModel::Column::Balance), index(rows - 1, JournalModel::Column::Balance));
    d->balanceCalculationPending = false;
}

void LedgerAccountFilter::setAccount(const MyMoneyAccount& acc)
{
    Q_D(LedgerAccountFilter);

    d->account = acc;

    d->showValuesInverted = false;
    if(d->account.accountGroup() == eMyMoney::Account::Type::Liability
            || d->account.accountGroup() == eMyMoney::Account::Type::Income) {
        d->showValuesInverted = true;
    }

    setAccountType(d->account.accountType());
    setFilterFixedString(d->account.id());

    // if balance calculation has not been triggered, then run it immediately
    recalculateBalancesOnIdle(d->account.id());
}

bool LedgerAccountFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const LedgerAccountFilter);
    auto rc = LedgerFilterBase::filterAcceptsRow(source_row, source_parent);

    // in case we don't have a match and the current account is an investment account
    // we check if the journal entry references a child account of the investment account
    // if so, we need to display the transaction
    if (!rc && d->account.accountType() == eMyMoney::Account::Type::Investment) {
        const auto idx = sourceModel()->index(source_row, 0, source_parent);
        rc = d->account.accountList().contains(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
    }
    return rc;
}

void LedgerAccountFilter::doSort()
{
    Q_D(LedgerAccountFilter);

    LedgerFilterBase::doSort();
    // trigger a recalculation of the balances after sorting
    recalculateBalancesOnIdle(d->account.id());
}

QVariant LedgerAccountFilter::data(const QModelIndex& index, int role) const
{
    Q_D(const LedgerAccountFilter);
    if (role == eMyMoney::Model::ShowValueInvertedRole) {
        return d->showValuesInverted;
    }

    if (index.column() == JournalModel::Balance) {
        switch (role) {
        case Qt::DisplayRole:
            if (index.row() < d->balances.size()) {
                // only report a balance for transactions and schedules but
                // not for the empty (new) transaction
                if (!index.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                    return d->balances.at(index.row()).formatMoney(d->account.fraction());
                }
            }
            return {};

        case Qt::TextAlignmentRole:
            return QVariant(Qt::AlignRight | Qt::AlignTop);

        default:
            break;
        }
    }
    return LedgerFilterBase::data(index, role);
}
