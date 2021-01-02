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
  if (source_parent.isValid()) {
    // if the entry has a valid parent it is an account
    return AccountsProxyModel::filterAcceptsRow(source_row, source_parent);
  } else {
    return filterAcceptsRowOrChildRows(source_row, source_parent);
  }
}
