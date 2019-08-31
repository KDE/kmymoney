/*
 * Copyright 2016-2018  Thomas Baumgart <tbaumgart@kde.org>
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

#include "ledgerproxymodel.h"
#include "journalmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
// #include "modelenums.h"

// using namespace eLedgerModel;
using namespace eMyMoney;

LedgerProxyModel::LedgerProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
  , m_showNewTransaction(false)
  , m_accountType(Account::Type::Asset)
  , m_filterRole(Qt::DisplayRole)
{
  setFilterRole(eMyMoney::Model::Roles::SplitAccountIdRole);
  setFilterKeyColumn(0);
  setSortRole(eMyMoney::Model::Roles::TransactionPostDateRole);
}

LedgerProxyModel::~LedgerProxyModel()
{
}

void LedgerProxyModel::setAccountType(Account::Type type)
{
  m_accountType = type;
}

QVariant LedgerProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case JournalModel::Column::Payment:
        switch(m_accountType) {
          case Account::Type::CreditCard:
            return i18nc("Payment made with credit card", "Charge");

          case Account::Type::Asset:
          case Account::Type::AssetLoan:
            return i18nc("Decrease of asset/liability value", "Decrease");

          case Account::Type::Liability:
          case Account::Type::Loan:
            return i18nc("Increase of asset/liability value", "Increase");

          case Account::Type::Income:
          case Account::Type::Expense:
            return i18n("Income");

          default:
            break;
        }
        break;

      case JournalModel::Column::Deposit:
        switch(m_accountType) {
          case Account::Type::CreditCard:
            return i18nc("Payment towards credit card", "Payment");

          case Account::Type::Asset:
          case Account::Type::AssetLoan:
            return i18nc("Increase of asset/liability value", "Increase");

          case Account::Type::Liability:
          case Account::Type::Loan:
            return i18nc("Decrease of asset/liability value", "Decrease");

          case Account::Type::Income:
          case Account::Type::Expense:
            return i18n("Expense");

          default:
            break;
        }
        break;
    }
  }
  return QSortFilterProxyModel::headerData(section, orientation, role);
}

bool LedgerProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  // make sure that the dummy transaction is shown last in any case
  if(left.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
    return false;

  } else if(right.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
    return true;
  }

  // sort the schedules always after the real transactions
  /// @todo port to new model code, make sure to support display within correct date
#if 0
  const QString leftString(left.data((int)Role::ScheduleId).toString());
  const QString rightString(right.data((int)Role::ScheduleId).toString());

  // make sure schedules are shown past real transactions
  if(!leftString.isEmpty() && rightString.isEmpty()) {
    // left is schedule, right is not
    return false;

  } else if(leftString.isEmpty() && !rightString.isEmpty()) {
    // right is schedule, left is not
    return true;
  }
#endif

  // otherwise use normal sorting
  return QSortFilterProxyModel::lessThan(left, right);
}

bool LedgerProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  if (m_filterId.isEmpty())
    return false;

  QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
  bool rc = idx.data(m_filterRole).toString().compare(m_filterId) == 0;
  // in case a journal entry has no id, it is the new transaction placeholder
  if(!rc) {
    rc = idx.data(eMyMoney::Model::IdRole).toString().isEmpty();
  }
  return rc;
}

void LedgerProxyModel::setFilterFixedString(const QString& id)
{
  m_filterId = id;
}

int LedgerProxyModel::filterRole() const
{
  return m_filterRole;
}

void LedgerProxyModel::setFilterRole(int role)
{
  m_filterRole = role;
}

bool LedgerProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  QModelIndex sourceIndex = mapToSource(index);
  return sourceModel()->setData(sourceIndex, value, role);
}
