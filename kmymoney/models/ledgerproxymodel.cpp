/***************************************************************************
                          ledgerproxymodel.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
  , m_accountType(Account::Asset)
{
  setFilterRole((int)Role::AccountId);
  setFilterKeyColumn(0);
  setSortRole((int)Role::PostDate);
  setDynamicSortFilter(true);
}

LedgerProxyModel::~LedgerProxyModel()
{
}

void LedgerProxyModel::setAccountType(Account type)
{
  m_accountType = type;
}

QVariant LedgerProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch(section) {
      case (int)Column::Payment:
        switch(m_accountType) {
          case Account::CreditCard:
            return i18nc("Payment made with credit card", "Charge");

          case Account::Asset:
          case Account::AssetLoan:
            return i18nc("Decrease of asset/liability value", "Decrease");

          case Account::Liability:
          case Account::Loan:
            return i18nc("Increase of asset/liability value", "Increase");

          case Account::Income:
          case Account::Expense:
            return i18n("Income");

          default:
            break;
        }
        break;

      case (int)Column::Deposit:
        switch(m_accountType) {
          case Account::CreditCard:
            return i18nc("Payment towards credit card", "Payment");

          case Account::Asset:
          case Account::AssetLoan:
            return i18nc("Increase of asset/liability value", "Increase");

          case Account::Liability:
          case Account::Loan:
            return i18nc("Decrease of asset/liability value", "Decrease");

          case Account::Income:
          case Account::Expense:
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

  // make sure schedules are shown past real transactions
  if(!left.data((int)Role::ScheduleId).toString().isEmpty()
  && right.data((int)Role::ScheduleId).toString().isEmpty()) {
    // left is schedule, right is not
    return false;

  } else if(left.data((int)Role::ScheduleId).toString().isEmpty()
         && !right.data((int)Role::ScheduleId).toString().isEmpty()) {
    // right is schedule, left is not
    return true;
  }

  // otherwise use normal sorting
  return QSortFilterProxyModel::lessThan(left, right);
}

bool LedgerProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  if(m_showNewTransaction) {
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if(idx.data((int)Role::TransactionSplitId).toString().isEmpty()) {
      return true;
    }
  }
  return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool LedgerProxyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  QModelIndex sourceIndex = mapToSource(index);
  return sourceModel()->setData(sourceIndex, value, role);
}

void LedgerProxyModel::setShowNewTransaction(bool show)
{
  const bool changed = show != m_showNewTransaction;
  m_showNewTransaction = show;
  if(changed) {
    invalidate();
  }
}
