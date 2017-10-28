/***************************************************************************
 *   Copyright 2010  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2017  Łukasz Wojniłowicz lukasz.wojnilowicz@gmail.com       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/

#include "accountsproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "modelenums.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"

using namespace eAccountsModel;

/**
  * The pimpl.
  */
class MyMoneyInstitution;
class AccountsProxyModel::Private
{
public:
  Private() :
      m_hideClosedAccounts(true),
      m_hideEquityAccounts(true),
      m_hideUnusedIncomeExpenseAccounts(false),
      m_haveHiddenUnusedIncomeExpenseAccounts(false) {
  }

  ~Private() {
  }

  QList<eMyMoney::Account> m_typeList;
  bool m_hideClosedAccounts;
  bool m_hideEquityAccounts;
  bool m_hideUnusedIncomeExpenseAccounts;
  bool m_haveHiddenUnusedIncomeExpenseAccounts;
};

AccountsProxyModel::AccountsProxyModel(QObject *parent)
  : KRecursiveFilterProxyModel(parent), d(new Private)
{
  setDynamicSortFilter(true);
  setSortLocaleAware(true);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

AccountsProxyModel::~AccountsProxyModel()
{
  delete d;
}

/**
  * This function was re-implemented so we could have a special display order (favorites first)
  */
bool AccountsProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  if (!left.isValid() || !right.isValid())
    return false;
  // different sorting based on the column which is being sorted
  switch (m_mdlColumns->at(left.column())) {
      // for the accounts column sort based on the DisplayOrderRole
    case Column::Account: {
        const auto leftData = sourceModel()->data(left, (int)Role::DisplayOrder);
        const auto rightData = sourceModel()->data(right, (int)Role::DisplayOrder);

        if (leftData.toInt() == rightData.toInt()) {
          // sort items of the same display order alphabetically
          return QSortFilterProxyModel::lessThan(left, right);
        }
        return leftData.toInt() < rightData.toInt();
      }
      // the total balance and value columns are sorted based on the value of the account
    case Column::TotalBalance:
    case Column::TotalValue: {
        const auto leftData = sourceModel()->data(sourceModel()->index(left.row(), (int)Column::Account, left.parent()), (int)Role::TotalValue);
        const auto rightData = sourceModel()->data(sourceModel()->index(right.row(), (int)Column::Account, right.parent()), (int)Role::TotalValue);
        return leftData.value<MyMoneyMoney>() < rightData.value<MyMoneyMoney>();
      }
    default:
      break;
  }
  return QSortFilterProxyModel::lessThan(left, right);
}

/**
  * This function was re-implemented to consider all the filtering aspects that we need in the application.
  */
bool AccountsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  const auto index = sourceModel()->index(source_row, (int)Column::Account, source_parent);
  return acceptSourceItem(index) && filterAcceptsRowOrChildRows(source_row, source_parent);
}

/**
  * This function implements a recursive matching. It is used to match a row even if it's values
  * don't match the current filtering criteria but it has at least one child row that does match.
  */
bool AccountsProxyModel::filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const
{
  if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
    return true;

  const auto index = sourceModel()->index(source_row, (int)Column::Account, source_parent);
  for (auto i = 0; i < sourceModel()->rowCount(index); ++i) {
    if (filterAcceptsRowOrChildRows(i, index))
      return true;
  }
  return false;
}

/**
  * Add the given account group to the filter.
  * @param group The account group to be added.
  * @see eMyMoney::Account
  */
void AccountsProxyModel::addAccountGroup(const QVector<eMyMoney::Account> &groups)
{
  foreach (const auto group, groups) {
    switch (group) {
      case eMyMoney::Account::Asset:
        d->m_typeList << eMyMoney::Account::Checkings;
        d->m_typeList << eMyMoney::Account::Savings;
        d->m_typeList << eMyMoney::Account::Cash;
        d->m_typeList << eMyMoney::Account::AssetLoan;
        d->m_typeList << eMyMoney::Account::CertificateDep;
        d->m_typeList << eMyMoney::Account::Investment;
        d->m_typeList << eMyMoney::Account::Stock;
        d->m_typeList << eMyMoney::Account::MoneyMarket;
        d->m_typeList << eMyMoney::Account::Asset;
        d->m_typeList << eMyMoney::Account::Currency;
        break;
      case eMyMoney::Account::Liability:
        d->m_typeList << eMyMoney::Account::CreditCard;
        d->m_typeList << eMyMoney::Account::Loan;
        d->m_typeList << eMyMoney::Account::Liability;
        break;
      case eMyMoney::Account::Income:
        d->m_typeList << eMyMoney::Account::Income;
        break;
      case eMyMoney::Account::Expense:
        d->m_typeList << eMyMoney::Account::Expense;
        break;
      case eMyMoney::Account::Equity:
        d->m_typeList << eMyMoney::Account::Equity;
        break;
      default:
        break;
    }
  }
  invalidateFilter();
}

/**
  * Add the given account type to the filter.
  * @param type The account type to be added.
  * @see eMyMoney::Account
  */
void AccountsProxyModel::addAccountType(eMyMoney::Account type)
{
  d->m_typeList << type;
  invalidateFilter();
}

/**
  * Remove the given account type from the filter.
  * @param type The account type to be removed.
  * @see eMyMoney::Account
  */
void AccountsProxyModel::removeAccountType(eMyMoney::Account type)
{
  if (d->m_typeList.removeAll(type) > 0) {
    invalidateFilter();
  }
}

/**
  * Use this to reset the filter.
  */
void AccountsProxyModel::clear()
{
  d->m_typeList.clear();
  invalidateFilter();
}

/**
  * Implementation function that performs the actual filtering.
  */
bool AccountsProxyModel::acceptSourceItem(const QModelIndex &source) const
{
  if (source.isValid()) {
    const auto data = sourceModel()->data(source, (int)Role::Account);
    if (data.isValid()) {
      if (data.canConvert<MyMoneyAccount>()) {
        const auto account = data.value<MyMoneyAccount>();
        if ((hideClosedAccounts() && account.isClosed()))
          return false;

        // we hide stock accounts if not in expert mode
        if (account.isInvest() && hideEquityAccounts())
          return false;

        // we hide equity accounts if not in expert mode
        if (account.accountType() == eMyMoney::Account::Equity && hideEquityAccounts())
          return false;

        // we hide unused income and expense accounts if the specific flag is set
        if ((account.accountType() == eMyMoney::Account::Income || account.accountType() == eMyMoney::Account::Expense) && hideUnusedIncomeExpenseAccounts()) {
          const auto totalValue = sourceModel()->data(source, (int)Role::TotalValue);
          if (totalValue.isValid() && totalValue.value<MyMoneyMoney>().isZero()) {
            emit unusedIncomeExpenseAccountHidden();
            return false;
          }
        }

        if (d->m_typeList.contains(account.accountType()))
          return true;
      } else if (data.canConvert<MyMoneyInstitution>() && sourceModel()->rowCount(source) == 0) {
        // if this is an institution that has no children show it only if hide unused institutions (hide closed accounts for now) is not checked
        return !hideClosedAccounts();
      }
      // let the visibility of all other institutions (the ones with children) be controlled by the visibility of their children
    }

    // all parents that have at least one visible child must be visible
    const auto rowCount = sourceModel()->rowCount(source);
    for (auto i = 0; i < rowCount; ++i) {
      const auto index = sourceModel()->index(i, (int)(int)Column::Account, source);
      if (acceptSourceItem(index))
        return true;
    }
  }
  return false;
}

/**
  * Set if closed accounts should be hidden or not.
  * @param hideClosedAccounts
  */
void AccountsProxyModel::setHideClosedAccounts(bool hideClosedAccounts)
{
  if (d->m_hideClosedAccounts != hideClosedAccounts) {
    d->m_hideClosedAccounts = hideClosedAccounts;
    invalidateFilter();
  }
}

/**
  * Check if closed accounts are hidden or not.
  */
bool AccountsProxyModel::hideClosedAccounts() const
{
  return d->m_hideClosedAccounts;
}

/**
  * Set if equity and investment accounts should be hidden or not.
  * @param hideEquityAccounts
  */
void AccountsProxyModel::setHideEquityAccounts(bool hideEquityAccounts)
{
  if (d->m_hideEquityAccounts != hideEquityAccounts) {
    d->m_hideEquityAccounts = hideEquityAccounts;
    invalidateFilter();
  }
}

/**
  * Check if equity and investment accounts are hidden or not.
  */
bool AccountsProxyModel::hideEquityAccounts() const
{
  return d->m_hideEquityAccounts;
}

/**
  * Set if empty categories should be hidden or not.
  * @param hideUnusedIncomeExpenseAccounts
  */
void AccountsProxyModel::setHideUnusedIncomeExpenseAccounts(bool hideUnusedIncomeExpenseAccounts)
{
  if (d->m_hideUnusedIncomeExpenseAccounts != hideUnusedIncomeExpenseAccounts) {
    d->m_hideUnusedIncomeExpenseAccounts = hideUnusedIncomeExpenseAccounts;
    invalidateFilter();
  }
}

/**
  * Check if empty categories are hidden or not.
  */
bool AccountsProxyModel::hideUnusedIncomeExpenseAccounts() const
{
  return d->m_hideUnusedIncomeExpenseAccounts;
}

/**
  * Returns the number of visible items after filtering. In case @a includeBaseAccounts
  * is set to @c true, the 5 base accounts (asset, liability, income, expense and equity)
  * will also be counted. The default is @c false.
  */
int AccountsProxyModel::visibleItems(bool includeBaseAccounts) const
{
  auto rows = 0;
  for (auto i = 0; i < rowCount(QModelIndex()); ++i) {
    if(includeBaseAccounts) {
      ++rows;
    }
    const auto childIndex = index(i, 0);
    if (hasChildren(childIndex)) {
      rows += visibleItems(childIndex);
    }
  }
  return rows;
}

/**
  * Returns the number of visible items under the given @a index.
  * The column of the @a index must be 0, otherwise no count will
  * be returned (returns 0).
  */
int AccountsProxyModel::visibleItems(const QModelIndex& index) const
{
  auto rows = 0;
  if (index.isValid() && index.column() == (int)Column::Account) { // CAUTION! Assumption is being made that Account column number is always 0
    const auto *model = index.model();
    const auto rowCount = model->rowCount(index);
    for (auto i = 0; i < rowCount; ++i) {
      ++rows;
      const auto childIndex = model->index(i, index.column(), index);
      if (model->hasChildren(childIndex))
        rows += visibleItems(childIndex);
    }
  }
  return rows;
}

void AccountsProxyModel::setSourceColumns(QList<eAccountsModel::Column> *columns)
{
  m_mdlColumns = columns;
}
