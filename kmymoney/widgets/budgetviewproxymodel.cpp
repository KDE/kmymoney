/*
 * Copyright 2006       Darren Gould <darren_gould@gmx.de>
 * Copyright 2009-2014  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "budgetviewproxymodel.h"
#include "accountsviewproxymodel_p.h"

// ----------------------------------------------------------------------------
// QT Includes

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
#include "models.h"
#include "accountsmodel.h"
#include "modelenums.h"

using namespace eAccountsModel;

class BudgetViewProxyModelPrivate : public AccountsViewProxyModelPrivate
{
  Q_DISABLE_COPY(BudgetViewProxyModelPrivate)

public:
  BudgetViewProxyModelPrivate() :
    AccountsViewProxyModelPrivate()
  {
  }

  ~BudgetViewProxyModelPrivate() override
  {
  }

  MyMoneyBudget m_budget;
  MyMoneyMoney m_lastBalance;
};

BudgetViewProxyModel::BudgetViewProxyModel(QObject *parent) :
  AccountsViewProxyModel(*new BudgetViewProxyModelPrivate, parent)
{
}

BudgetViewProxyModel::~BudgetViewProxyModel()
{
}

/**
  * This function was reimplemented to add the data needed by the other columns that this model
  * is adding besides the columns of the @ref AccountsModel.
  */
QVariant BudgetViewProxyModel::data(const QModelIndex &index, int role) const
{
  Q_D(const BudgetViewProxyModel);
  if (!MyMoneyFile::instance()->storageAttached())
    return QVariant();
  const auto sourceColumn = d->m_mdlColumns->at(mapToSource(index).column());
  auto const file = MyMoneyFile::instance();
  const auto ixAccount = mapToSource(BudgetViewProxyModel::index(index.row(), static_cast<int>(Column::Account), index.parent()));
  const auto account = ixAccount.data((int)Role::Account).value<MyMoneyAccount>();

  static QVector<Column> columnsToProcess {Column::TotalBalance, Column::TotalValue/*, AccountsModel::PostedValue, Column::Account*/};
  if (columnsToProcess.contains(sourceColumn)) {
        switch (role) {
          case Qt::DisplayRole:
            {
              switch (sourceColumn) {
                case Column::TotalBalance:
                  if (file->security(account.currencyId()) != file->baseCurrency())
                    return QVariant(MyMoneyUtils::formatMoney(accountBalance(account.id()), file->security(account.currencyId())));
                  else
                    return QVariant();
                case Column::TotalValue:
                  return QVariant(MyMoneyUtils::formatMoney(computeTotalValue(ixAccount), file->baseCurrency()));
                  // FIXME: Posted value doesn't correspond with total value without below code. Investigate why and wheather it matters.
                  //              case AccountsModel::PostedValue:
                  //                return QVariant(MyMoneyUtils::formatMoney(accountValue(account, accountBalance(account.id())), file->baseCurrency()));
                default:
                  break;
              }
              break;
            }
          default:
            break;
        }
  }
  switch (role) {
    case (int)Role::Balance:
      if (file->security(account.currencyId()) != file->baseCurrency())
        return QVariant::fromValue(accountBalance(account.id()));
      else
        return QVariant();
    case (int)Role::TotalValue:
      return QVariant::fromValue(computeTotalValue(ixAccount));
    case (int)Role::Value:
      return QVariant::fromValue(accountValue(account, accountBalance(account.id())));
    default:
      break;
  }
  return AccountsViewProxyModel::data(index, role);
}

Qt::ItemFlags BudgetViewProxyModel::flags(const QModelIndex &index) const
{
  Q_D(const BudgetViewProxyModel);
  Qt::ItemFlags flags = AccountsViewProxyModel::flags(index);
  if (!index.parent().isValid())
    return flags & ~Qt::ItemIsSelectable;

  // check if any of the parent accounts has the 'include subaccounts'
  // flag set. If so, we don't allow selecting this account
  QModelIndex idx = index.parent();
  while (idx.isValid()) {
    QModelIndex source_idx = mapToSource(idx);
    QVariant accountData = sourceModel()->data(source_idx, (int)Role::Account);
    if (accountData.canConvert<MyMoneyAccount>()) {
      MyMoneyAccount account = accountData.value<MyMoneyAccount>();
      // find out if the account is budgeted
      MyMoneyBudget::AccountGroup budgetAccount = d->m_budget.account(account.id());
      if (budgetAccount.id() == account.id()) {
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

bool BudgetViewProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  Q_D(const BudgetViewProxyModel);
  const auto index = sourceModel()->index(source_row, static_cast<int>(Column::Account), source_parent);
  const auto accountData = sourceModel()->data(index, (int)Role::Account);
  if (accountData.canConvert<MyMoneyAccount>()) {
    const auto account = accountData.value<MyMoneyAccount>();
    if (!index.parent().isValid()) {
      if (!account.isIncomeExpense()) {
        return false;
      }
    }
    if (hideUnusedIncomeExpenseAccounts()) {
      MyMoneyMoney balance;
      // find out if the account is budgeted
      const auto budgetAccount = d->m_budget.account(account.id());
      if (budgetAccount.id() == account.id()) {
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
        return true;
      for (auto i = 0; i < sourceModel()->rowCount(index); ++i) {
        if (filterAcceptsRow(i, index))
          return true;
      }
      return false;
    }
    return true;
  }
  return false;
}

MyMoneyMoney BudgetViewProxyModel::accountBalance(const QString &accountId) const
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

MyMoneyMoney BudgetViewProxyModel::accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance) const
{
  return Models::instance()->accountsModel()->accountValue(account, balance);
}

MyMoneyMoney BudgetViewProxyModel::computeTotalValue(const QModelIndex &source_index) const
{
  auto model = sourceModel();
  auto account = model->data(source_index, (int)Role::Account).value<MyMoneyAccount>();
  auto totalValue = accountValue(account, accountBalance(account.id()));
  for (auto i = 0; i < model->rowCount(source_index); ++i)
    totalValue += computeTotalValue(model->index(i, static_cast<int>(Column::Account), source_index));
  return totalValue;
}

void BudgetViewProxyModel::checkBalance()
{
  Q_D(BudgetViewProxyModel);
  // compute the balance
  QModelIndexList incomeList = match(index(0, 0),
                                     (int)Role::ID,
                                     MyMoneyFile::instance()->income().id(),
                                     1,
                                     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchWrap));

  QModelIndexList expenseList = match(index(0, 0),
                                      (int)Role::ID,
                                      MyMoneyFile::instance()->expense().id(),
                                      1,
                                      Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchWrap));

  MyMoneyMoney balance;
  if (!incomeList.isEmpty() && !expenseList.isEmpty()) {
    QVariant incomeValue = data(incomeList.front(), (int)Role::TotalValue);
    QVariant expenseValue = data(expenseList.front(), (int)Role::TotalValue);

    if (incomeValue.isValid() && expenseValue.isValid()) {
      balance = incomeValue.value<MyMoneyMoney>() - expenseValue.value<MyMoneyMoney>();
    }
  }
  if (d->m_lastBalance != balance) {
    d->m_lastBalance = balance;
    emit balanceChanged(d->m_lastBalance);
  }
}
