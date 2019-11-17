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

#include "ledgerfilterbase.h"
#include "ledgerfilterbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "journalmodel.h"
#include "accountsmodel.h"
#include "specialdatesmodel.h"

using namespace eMyMoney;

LedgerFilterBase::LedgerFilterBase(LedgerFilterBasePrivate* dd, QObject* parent, QAbstractItemModel* accountsModel, QAbstractItemModel* datesModel)
: QSortFilterProxyModel(parent)
, d_ptr(dd)
{
  Q_D(LedgerFilterBase);
  d->accountsModel = accountsModel;
  d->specialDatesModel = datesModel;
  setFilterRole(eMyMoney::Model::Roles::SplitAccountIdRole);
  setFilterKeyColumn(0);
  setSortRole(eMyMoney::Model::Roles::TransactionPostDateRole);
}

LedgerFilterBase::~LedgerFilterBase()
{
}

void LedgerFilterBase::setAccountType(Account::Type type)
{
  Q_D(LedgerFilterBase);
  d->accountType = type;
}

QVariant LedgerFilterBase::headerData(int section, Qt::Orientation orientation, int role) const
{
  if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    Q_D(const LedgerFilterBase);
    switch(section) {
      case JournalModel::Column::Payment:
        switch(d->accountType) {
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
        switch(d->accountType) {
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

bool LedgerFilterBase::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  Q_D(const LedgerFilterBase);

  // make sure that the dummy transaction is shown last in any case
  if(left.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
    return false;

  } else if(right.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
    return true;
  }

  // make sure that the online balance is the last entry of a day
  // and the date headers are the first
  if (left.data(eMyMoney::Model::TransactionPostDateRole).toDate() == right.data(eMyMoney::Model::TransactionPostDateRole).toDate()) {
    const auto leftModel = MyMoneyModelBase::baseModel(left);
    const auto rightModel = MyMoneyModelBase::baseModel(right);
    if (leftModel != rightModel) {
      if (d->isAccountsModel(leftModel)) {
        return false;
      } else if (d->isAccountsModel(rightModel)) {
        return true;
      } else if(d->isSpecialDatesModel(leftModel)) {
        return true;
      } else if(d->isSpecialDatesModel(rightModel)) {
        return false;
      }
    }
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

bool LedgerFilterBase::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  Q_D(const LedgerFilterBase);
  if (d->filterId.isEmpty())
    return false;

  QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
#if 0

  const auto baseModel = MyMoneyModelBase::baseModel(idx);
  if (d->isSpecialDatesModel(baseModel)) {
    // make sure we don't show trailing special date entries
    const auto rows = sourceModel()->rowCount(source_parent);
    int row = source_row + 1;
    bool visible = false;
    QModelIndex testIdx;
    for (; !visible && row < rows; ++row) {
      testIdx = sourceModel()->index(row, 0, source_parent);
      if(testIdx.data(eMyMoney::Model::IdRole).toString().isEmpty()) {
        // the empty id is the entry for the new transaction entry
        // we're done scanning
        break;
      }
      const auto testModel = MyMoneyModelBase::baseModel(testIdx);
      if (!d->isSpecialDatesModel(testModel)) {
        // we hit a real transaction, we're done and need to display
        visible = true;
        break;
      }
    }

    // in case this is not a trailing date entry, we need to check
    // if it is the last of a row of date entries.
    if (visible && ((source_row + 1) < rows)) {
      // check if the next is also a date entry
      testIdx = sourceModel()->index(source_row+1, 0, source_parent);
      const auto testModel = MyMoneyModelBase::baseModel(testIdx);
      if (d->isSpecialDatesModel(testModel)) {
        visible = false;
      }
    }
    return visible;
  }
#endif
  bool rc = idx.data(filterRole()).toString().compare(d->filterId) == 0;
  // in case a journal entry has no id, it is the new transaction placeholder
  if(!rc) {
    rc = idx.data(eMyMoney::Model::IdRole).toString().isEmpty();
  }
  return rc;
}

void LedgerFilterBase::setFilterFixedString(const QString& id)
{
  Q_D(LedgerFilterBase);
  d->filterId = id;
}

void LedgerFilterBase::setShowEntryForNewTransaction(bool show)
{
  Q_D(LedgerFilterBase);

  if (show && !d->newTransactionPresent) {
    d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel()->newTransaction());
    d->newTransactionPresent = true;
  } else if (!show && d->newTransactionPresent) {
    d->concatModel->removeSourceModel(MyMoneyFile::instance()->journalModel()->newTransaction());
    d->newTransactionPresent = false;
  }
}

