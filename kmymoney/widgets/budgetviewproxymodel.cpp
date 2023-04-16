/*
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2009-2014 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "budgetviewproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QColor>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneymoney.h"
#include "mymoneybudget.h"
#include "accountsproxymodel.h"
#include "accountsproxymodel_p.h"
#include "budgetviewproxymodel.h"
#include "mymoneyenums.h"
#include "mymoneymodelbase.h"


class BudgetViewProxyModelPrivate : public AccountsProxyModelPrivate
{
    Q_DISABLE_COPY(BudgetViewProxyModelPrivate)

public:
    BudgetViewProxyModelPrivate() :
        AccountsProxyModelPrivate()
    {
    }

    ~BudgetViewProxyModelPrivate() override
    {
    }

    QString accountName(const QModelIndex& idx, const QString& basename) const
    {
        // only return a modified name for asset or liability accounts
        if (idx.data(eMyMoney::Model::AccountIsAssetLiabilityRole).toBool()) {
            const auto accountId = idx.data(eMyMoney::Model::IdRole).toString();
            eMyMoney::Account::Type budgetType;
            // in case we have an entry for that account in the budget
            // we use that type, if not, we add the default found in
            // with the account. If the default is not to include the
            // account in the budget, we should never come around here
            // because BudgetViewProxyModel::filterAcceptsRow() should
            // filter out the entry
            if (m_budget.account(accountId).id() == accountId) {
                budgetType = m_budget.account(accountId).budgetType();
            } else {
                budgetType = idx.data(eMyMoney::Model::AccountTypeInBudgetRole).value<eMyMoney::Account::Type>();
            }
            // only append the type if it is income or expense
            if (MyMoneyAccount::isIncomeExpense(budgetType)) {
                return QStringLiteral("%1 (%2)").arg(basename, MyMoneyAccount::accountTypeToString(budgetType));
            }
        }
        return basename;
    }

    MyMoneyBudget   m_budget;
    MyMoneyMoney    m_lastBalance;
    QColor          positiveScheme;
    QColor          negativeScheme;
};

BudgetViewProxyModel::BudgetViewProxyModel(QObject *parent) :
    AccountsProxyModel(*new BudgetViewProxyModelPrivate, parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

BudgetViewProxyModel::~BudgetViewProxyModel()
{
}

void BudgetViewProxyModel::setColorScheme(AccountsModel::ColorScheme scheme, const QColor& color)
{
    Q_D(BudgetViewProxyModel);
    switch(scheme) {
    case AccountsModel::Positive:
        d->positiveScheme = color;
        break;
    case AccountsModel::Negative:
        d->negativeScheme = color;
        break;
    }
}


/**
  * This function was reimplemented to add the data needed by the other columns that this model
  * is adding besides the columns of the @ref AccountsModel.
  */
QVariant BudgetViewProxyModel::data(const QModelIndex & idx, int role) const
{
    Q_D(const BudgetViewProxyModel);

#if 0
    static QVector<Column> columnsToProcess {Column::TotalBalance, Column::TotalValue/*, AccountsModel::PostedValue, Column::Account*/};
    if (columnsToProcess.contains(sourceColumn)) {
#endif

    // get index in base model
    const auto accountIdx = MyMoneyModelBase::mapToBaseSource(idx);

    switch (role) {
    case Qt::DisplayRole: {
        const auto file = MyMoneyFile::instance();
        const auto accountId = accountIdx.data(eMyMoney::Model::IdRole).toString();
        const auto currencyId = accountIdx.data(eMyMoney::Model::AccountCurrencyIdRole).toString();
        const auto baseCurrency = file->baseCurrency();

        switch (idx.column()) {
        case AccountsModel::Column::AccountName:
            return d->accountName(idx, AccountsProxyModel::data(idx, role).toString());

        case AccountsModel::Column::Balance:
            if (currencyId != baseCurrency.id())
                return MyMoneyUtils::formatMoney(accountBalance(accountId).abs(), file->security(currencyId));
            else
                return QVariant();
        case AccountsModel::Column::TotalPostedValue:
            return MyMoneyUtils::formatMoney(computeTotalValue(accountIdx).abs(), baseCurrency);
        // FIXME: Posted value doesn't correspond with total value without below code. Investigate why and whether it matters.
        //              case AccountsModel::PostedValue:
        //                return QVariant(MyMoneyUtils::formatMoney(accountValue(account, accountBalance(account.id())), file->baseCurrency()));
        default:
            break;
        }
    } break;

    case eMyMoney::Model::AccountTotalValueRole:
        return QVariant::fromValue(computeTotalValue(accountIdx));

    case Qt::ForegroundRole:
        // show all numbers in positive scheme
        switch (idx.column()) {
        case AccountsModel::Column::Balance:
        case AccountsModel::Column::PostedValue:
        case AccountsModel::Column::TotalPostedValue:
            return d->positiveScheme;

        default:
            break;
        }
        break;

#if 0
    case (int)Role::Balance:
        if (account.currencyId() != file->baseCurrency().id())
            return QVariant::fromValue(accountBalance(account.id()));
        else
            return QVariant();
    case (int)Role::Value:
        return QVariant::fromValue(accountValue(account, accountBalance(account.id())));
#endif
    default:
        break;
    }
    return AccountsProxyModel::data(idx, role);
}

Qt::ItemFlags BudgetViewProxyModel::flags(const QModelIndex& index) const
{
    Q_D(const BudgetViewProxyModel);
    Qt::ItemFlags flags = AccountsProxyModel::flags(index);
    if (!index.parent().isValid())
        return flags & ~Qt::ItemIsSelectable;

    // check if any of the parent accounts has the 'include subaccounts'
    // flag set. If so, we don't allow selecting this account
    QModelIndex idx = index.parent();
    while (idx.isValid()) {
        const auto accountIdx = MyMoneyFile::baseModel()->mapToBaseSource(index);
        const auto accountId = accountIdx.data(eMyMoney::Model::IdRole).toString();
        if (!accountId.isEmpty()) {
            // find out if the account is budgeted
            MyMoneyBudget::AccountGroup budgetAccount = d->m_budget.account(accountId);
            if (budgetAccount.id() == accountId) {
                if (budgetAccount.budgetSubaccounts()) {
                    return flags & ~Qt::ItemIsEnabled;
                }
            }
        }
        idx = idx.parent();
    }
    return flags;
}

void BudgetViewProxyModel::setBudget(const MyMoneyBudget& budget)
{
    Q_D(BudgetViewProxyModel);
    d->m_budget = budget;
    invalidate();
    checkBalance();
}

bool BudgetViewProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const BudgetViewProxyModel);
    // we want to include
    // a) all income and expense accounts
    // b) all asset and liability accounts that are marked to be included in budget
    // c) all asset and liability accounts that are contained in budget
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    auto accountType = idx.data(eMyMoney::Model::AccountTypeInBudgetRole).value<eMyMoney::Account::Type>();
    // accountType is either income or expense for cases a) and b)

    // find out if the account is budgeted
    const auto accountId = idx.data(eMyMoney::Model::IdRole).toString();
    const auto budgetAccount = d->m_budget.account(accountId);

    if (!idx.data(eMyMoney::Model::AccountIsIncomeExpenseRole).toBool()) {
        if (budgetAccount.id() == accountId) {
            // case c)
            accountType = budgetAccount.budgetType();
        }
    }

    if (MyMoneyAccount::isIncomeExpense(accountType)) {
        if (hideUnusedIncomeExpenseAccounts()) {
            MyMoneyMoney balance;
            if (budgetAccount.id() == accountId) {
                balance = budgetAccount.balance();
                switch (budgetAccount.budgetLevel()) {
                case eMyMoney::Budget::Level::Monthly:
                    balance *= MyMoneyMoney(12);
                    break;
                default:
                    break;
                }
            }
            if (!balance.isZero())
                return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);

            for (auto i = 0; i < sourceModel()->rowCount(idx); ++i) {
                if (filterAcceptsRow(i, idx))
                    return true;
            }
            return false;
        }
        return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);
    }
    return false;
}

MyMoneyMoney BudgetViewProxyModel::accountBalance(const QString& accountId) const
{
    Q_D(const BudgetViewProxyModel);
    MyMoneyMoney balance;
    // find out if the account is budgeted
    MyMoneyBudget::AccountGroup budgetAccount = d->m_budget.account(accountId);
    if (budgetAccount.id() == accountId) {
        balance = budgetAccount.balance();
        switch (budgetAccount.budgetLevel()) {
        case eMyMoney::Budget::Level::Monthly:
            balance *= MyMoneyMoney(12);
            break;
        default:
            break;
        }
    }
    return balance;
}

MyMoneyMoney BudgetViewProxyModel::accountBalance(const QModelIndex& idx) const
{
    Q_D(const BudgetViewProxyModel);
    MyMoneyMoney balance;
    const auto accountId = idx.data(eMyMoney::Model::IdRole).toString();
    // find out if the account is budgeted
    MyMoneyBudget::AccountGroup budgetAccount = d->m_budget.account(accountId);
    if (budgetAccount.id() == accountId) {
        balance = budgetAccount.balance();
        switch (budgetAccount.budgetLevel()) {
        case eMyMoney::Budget::Level::Monthly:
            balance *= MyMoneyMoney(12);
            break;
        default:
            break;
        }
        // revert sign if it is an expense account or is treated as an expense
        if ((idx.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>() == eMyMoney::Account::Type::Expense)
            || (budgetAccount.budgetType() == eMyMoney::Account::Type::Expense)) {
            balance = -balance;
        }
    }
    return balance;
}

MyMoneyMoney BudgetViewProxyModel::computeTotalValue(const QModelIndex& source_index) const
{
    // get balance of this account
    const auto accountId = source_index.data(eMyMoney::Model::IdRole).toString();
    auto totalValue = MyMoneyFile::instance()->accountsModel()->balanceToValue(accountId, accountBalance(source_index)).first;

    // and all children
    auto model = sourceModel();
    const auto rows = model->rowCount(source_index);
    for (auto i = 0; i < rows; ++i)
        totalValue += computeTotalValue(model->index(i, 0, source_index));
    return totalValue;
}

void BudgetViewProxyModel::checkBalance()
{
    Q_D(BudgetViewProxyModel);
    const auto file = MyMoneyFile::instance();

    const auto incomeIdx = MyMoneyModelBase::mapFromBaseSource(this, file->accountsModel()->incomeIndex());
    const auto expenseIdx = MyMoneyModelBase::mapFromBaseSource(this, file->accountsModel()->expenseIndex());
    const auto assetIdx = MyMoneyModelBase::mapFromBaseSource(this, file->accountsModel()->assetIndex());
    const auto liabilityIdx = MyMoneyModelBase::mapFromBaseSource(this, file->accountsModel()->liabilityIndex());

    const MyMoneyMoney balance = incomeIdx.data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>()
        + expenseIdx.data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>()
        + assetIdx.data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>()
        + liabilityIdx.data(eMyMoney::Model::AccountTotalValueRole).value<MyMoneyMoney>();

    if (d->m_lastBalance != balance) {
        d->m_lastBalance = balance;
        Q_EMIT balanceChanged(d->m_lastBalance);
    }
}
