/*
 * SPDX-FileCopyrightText: 2016-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ledgerproxymodel.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "modelenums.h"

using namespace eLedgerModel;
using namespace eMyMoney;

LedgerProxyModel::LedgerProxyModel(QObject* parent)
  : QSortFilterProxyModel(parent)
  , m_showNewTransaction(false)
  , m_accountType(Account::Type::Asset)
  , m_filterRole(Qt::DisplayRole)
{
  setFilterRole((int)Role::AccountId);
  setFilterKeyColumn(0);
  setSortRole((int)Role::PostDate);
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
      case (int)Column::Payment:
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

      case (int)Column::Deposit:
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
  if(left.data((int)Role::TransactionSplitId).toString().isEmpty()) {
    return false;

  } else if(right.data((int)Role::TransactionSplitId).toString().isEmpty()) {
    return true;
  }

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

  // otherwise use normal sorting
  return QSortFilterProxyModel::lessThan(left, right);
}

bool LedgerProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
  bool rc = idx.data(m_filterRole).toString().compare(m_filterId) == 0;
  if(!rc && m_showNewTransaction) {
    rc = idx.data((int)Role::TransactionSplitId).toString().isEmpty();
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

void LedgerProxyModel::setShowNewTransaction(bool show)
{
  m_showNewTransaction = show;
}
