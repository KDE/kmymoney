/*
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include "institutionsproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "modelenums.h"
#include "mymoneyenums.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "accountsmodel.h"

InstitutionsProxyModel::InstitutionsProxyModel(QObject *parent)
  : AccountsProxyModel(parent)
{
  setDynamicSortFilter(true);
  setSortLocaleAware(true);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

InstitutionsProxyModel::~InstitutionsProxyModel()
{
}

/**
  * This function was re-implemented so we could have a special display order (favorites first)
  */
bool InstitutionsProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
  if (!left.isValid() || !right.isValid())
    return false;
  // different sorting based on the column which is being sorted
  switch (left.column()) {
      // for the accounts column sort based on the DisplayOrderRole
    case AccountsModel::Column::AccountName: {
      const auto leftData = sourceModel()->data(left, eMyMoney::Model::Roles::AccountDisplayOrderRole);
      const auto rightData = sourceModel()->data(right, eMyMoney::Model::Roles::AccountDisplayOrderRole);

        if (leftData.toInt() == rightData.toInt()) {
          // sort items of the same display order alphabetically
          return QSortFilterProxyModel::lessThan(left, right);
        }
        return leftData.toInt() < rightData.toInt();
      }
      // the total balance and value columns are sorted based on the value of the account
    case AccountsModel::Column::TotalBalance:
    case AccountsModel::Column::TotalPostedValue: {
      const auto leftData = sourceModel()->data(sourceModel()->index(left.row(), AccountsModel::Column::AccountName, left.parent()), eMyMoney::Model::Roles::AccountTotalValueRole);
      const auto rightData = sourceModel()->data(sourceModel()->index(right.row(), AccountsModel::Column::AccountName, right.parent()), eMyMoney::Model::Roles::AccountTotalValueRole);
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
bool InstitutionsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  const auto index = sourceModel()->index(source_row, AccountsModel::Column::AccountName, source_parent);
  return acceptSourceItem(index) && filterAcceptsRowOrChildRows(source_row, source_parent);
}

/**
  * This function implements a recursive matching. It is used to match a row even if it's values
  * don't match the current filtering criteria but it has at least one child row that does match.
  */
bool InstitutionsProxyModel::filterAcceptsRowOrChildRows(int source_row, const QModelIndex &source_parent) const
{
  if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
    return true;

  const auto index = sourceModel()->index(source_row, AccountsModel::Column::AccountName, source_parent);
  for (auto i = 0; i < sourceModel()->rowCount(index); ++i) {
    if (filterAcceptsRowOrChildRows(i, index))
      return true;
  }
  return false;
}

/**
  * Implementation function that performs the actual filtering.
  */
bool InstitutionsProxyModel::acceptSourceItem(const QModelIndex &source) const
{
  if (source.isValid()) {
    const auto accountTypeValue = sourceModel()->data(source, eMyMoney::Model::Roles::AccountTypeRole);
    const bool isValidAccountEntry = accountTypeValue.isValid();
    const bool isValidInstititonEntry = sourceModel()->data(source, eMyMoney::Model::Roles::InstitutionBankCodeRole).isValid();

    if (isValidAccountEntry) {
      const auto accountType = static_cast<eMyMoney::Account::Type>(accountTypeValue.toInt());

      if (hideClosedAccounts() && sourceModel()->data(source, eMyMoney::Model::Roles::AccountIsClosedRole).toBool())
        return false;

      // we hide stock accounts if not in expert mode
      // we hide equity accounts if not in expert mode
      if (hideEquityAccounts()) {
        if (accountType == eMyMoney::Account::Type::Equity)
          return false;

        if (sourceModel()->data(source, eMyMoney::Model::Roles::AccountIsInvestRole).toBool())
          return false;
      }
      return true;

    } else if (isValidInstititonEntry) {
      return true;
    }

    // all parents that have at least one visible child must be visible
    const auto rowCount = sourceModel()->rowCount(source);
    for (auto i = 0; i < rowCount; ++i) {
      const auto index = sourceModel()->index(i, AccountsModel::Column::AccountName, source);
      if (acceptSourceItem(index))
        return true;
    }
  }
  return false;
}
