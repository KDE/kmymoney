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

#include <KDescendantsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "journalmodel.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "onlinebalanceproxymodel.h"
#include "reconciliationmodel.h"
#include "schedulesjournalmodel.h"
#include "securitiesmodel.h"
#include "securityaccountsproxymodel.h"
#include "specialdatesmodel.h"

class LedgerAccountFilterPrivate : public LedgerFilterBasePrivate
{
public:
    explicit LedgerAccountFilterPrivate(LedgerAccountFilter* qq)
        : LedgerFilterBasePrivate(qq)
        , onlinebalanceproxymodel(nullptr)
        , securityAccountsProxyModel(nullptr)
    {
    }

    ~LedgerAccountFilterPrivate()
    {
    }

    OnlineBalanceProxyModel*    onlinebalanceproxymodel;
    SecurityAccountsProxyModel* securityAccountsProxyModel;

    MyMoneyAccount              account;
};


LedgerAccountFilter::LedgerAccountFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels)
    : LedgerFilterBase(new LedgerAccountFilterPrivate(this), parent)
{
    Q_D(LedgerAccountFilter);
    setMaintainBalances(true);
    setObjectName("LedgerAccountFilter");

    d->onlinebalanceproxymodel = new OnlineBalanceProxyModel(parent);
    d->securityAccountsProxyModel = new SecurityAccountsProxyModel(parent);

    const auto accountsModel = MyMoneyFile::instance()->flatAccountsModel();
    d->onlinebalanceproxymodel->setObjectName("OnlineBalanceProxyModel");
    d->onlinebalanceproxymodel->setSourceModel(accountsModel);
    d->securityAccountsProxyModel->setObjectName("SecurityAccountsProxyModel");
    d->securityAccountsProxyModel->setSourceModel(accountsModel);

    d->concatModel->setObjectName("LedgerView concatModel");
    d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());
    d->concatModel->addSourceModel(d->onlinebalanceproxymodel);
    d->concatModel->addSourceModel(d->securityAccountsProxyModel);

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

    connect(d->concatModel, &QAbstractItemModel::modelReset, this, [&]() {
        Q_D(LedgerAccountFilter);
        const auto newAccount = MyMoneyFile::instance()->accountsModel()->itemById(d->account.id());
        if (d->account.accountList() != newAccount.accountList()) {
            d->account = newAccount;
            invalidate();
        }
    });
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
    // we don't do any sorting on this model by design
}

QVariant LedgerAccountFilter::data(const QModelIndex& index, int role) const
{
    Q_D(const LedgerAccountFilter);
    if (role == eMyMoney::Model::ShowValueInvertedRole) {
        return d->showValuesInverted;
    }

    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == JournalModel::Balance) {
            if (index.row() < d->balances.size()) {
                // only report a balance for transactions and schedules but
                // not for the empty (new) transaction
                if (!index.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
                    const auto file = MyMoneyFile::instance();
                    const auto accountId = index.data(eMyMoney::Model::SplitAccountIdRole).toString();
                    const auto acc = file->accountsModel()->itemById(accountId);
                    const auto security = file->securitiesModel()->itemById(acc.currencyId());
                    const auto fraction =
                        (acc.accountType() == eMyMoney::Account::Type::Cash) ? security.smallestCashFraction() : security.smallestAccountFraction();
                    return d->balances.at(index.row()).formatMoney(fraction);
                }
            }
            return {};
        }
        break;

    case eMyMoney::Model::SplitReconcileDateRole:
        // we're asking for the reconciliation date of a special date
        // which maps to the next younger reconciliation date of that
        // account
        if (d->isSpecialDatesModel(index)) {
            QDate result;
            const auto entryDate = index.data(eMyMoney::Model::TransactionPostDateRole).toDate();
            const auto reconciliationModel = MyMoneyFile::instance()->reconciliationModel();
            const auto reconciliations =
                reconciliationModel->match(reconciliationModel->index(0, 0),
                                           eMyMoney::Model::SplitAccountIdRole,
                                           d->account.id(),
                                           -1,
                                           Qt::MatchFlags(Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive)));
            for (const auto& idx : reconciliations) {
                const auto date = idx.data(role).toDate();
                if (date >= entryDate) {
                    if (!result.isValid() || (date < result)) {
                        result = date;
                    }
                }
            }
            return result;
        }
        break;

    default:
        break;
    }

    return LedgerFilterBase::data(index, role);
}
