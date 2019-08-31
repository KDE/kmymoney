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


#include "ledgeraccountfilter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QHeaderView>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConcatenateRowsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerview.h"
#include "mymoneyenums.h"
#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "journalmodel.h"

class LedgerAccountFilterPrivate
{
  Q_DECLARE_PUBLIC(LedgerAccountFilter)

public:
  explicit LedgerAccountFilterPrivate(LedgerAccountFilter* qq)
  : q_ptr(qq)
  , view(nullptr)
  , concatModel(new KConcatenateRowsProxyModel(qq))
  , showValuesInverted(false)
  , balanceCalculationPending(false)
  , newTransactionPresent(false)
  {}

  ~LedgerAccountFilterPrivate()
  {
  }

  LedgerAccountFilter*        q_ptr;
  LedgerView*                 view;
  KConcatenateRowsProxyModel* concatModel;
  MyMoneyAccount              account;
  bool                        showValuesInverted;
  bool                        balanceCalculationPending;
  bool                        newTransactionPresent;
};


LedgerAccountFilter::LedgerAccountFilter(LedgerView* parent)
  : LedgerProxyModel(parent)
  , d_ptr(new LedgerAccountFilterPrivate(this))
{
  Q_D(LedgerAccountFilter);
  d->view = parent;
  setFilterRole(eMyMoney::Model::SplitAccountIdRole);
  setObjectName("LedgerAccountFilter");

  connect(parent, &LedgerView::requestBottomHalfSetup, this, &LedgerAccountFilter::setupBottomHalf);
  connect(parent, &LedgerView::requestBalanceRecalculation, this, &LedgerAccountFilter::recalculateBalancesOnIdle);
}

LedgerAccountFilter::~LedgerAccountFilter()
{
}

void LedgerAccountFilter::setupBottomHalf()
{
  Q_D(LedgerAccountFilter);
  switch(d->account.accountType()) {
    case eMyMoney::Account::Type::Investment:
      break;

    default:
      d->view->horizontalHeader()->resizeSection(JournalModel::Column::Reconciliation, 15);
      break;
  }

  d->showValuesInverted = false;
  if(d->account.accountGroup() == eMyMoney::Account::Type::Liability
    || d->account.accountGroup() == eMyMoney::Account::Type::Income) {
    d->showValuesInverted = true;
  }

  setFilterKeyColumn(0);
  setFilterFixedString(d->account.id());
  setAccountType(d->account.accountType());

  setSortRole(eMyMoney::Model::TransactionPostDateRole);
  sort(JournalModel::Column::Date);

  d->view->setModel(this);
  d->concatModel->setObjectName("LedgerView concatModel");
  d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());
  setSourceModel(d->concatModel);

  // if balance calculation has not been triggered, then run it immediately
  if(!d->balanceCalculationPending) {
    recalculateBalances();
  }

  /// @todo port to new model code
  if(rowCount() > 0) {
    // we need to check that the last row may contain a scheduled transaction or
    // the row that is shown for new transacations.
    // in that case, we need to go back to find the actual last transaction
    int row = rowCount()-1;
    while(row >= 0) {
      const QModelIndex idx = index(row, 0);
      if(!idx.data(eMyMoney::Model::IdRole).toString().isEmpty()
        // && !d->filterModel->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString().isEmpty()
      ) {
        d->view->setCurrentIndex(idx);
        d->view->selectRow(idx.row());
        d->view->scrollTo(idx, QAbstractItemView::PositionAtBottom);
        break;
      }
      row--;
    }
  }

  // we need to do this only once
  disconnect(d->view, &LedgerView::requestBottomHalfSetup, this, &LedgerAccountFilter::setupBottomHalf);
}


void LedgerAccountFilter::setShowBalanceInverted(bool inverted)
{
  Q_D(LedgerAccountFilter);
  d->showValuesInverted = inverted;
}

void LedgerAccountFilter::recalculateBalancesOnIdle()
{
  Q_D(LedgerAccountFilter);

  // make sure the balances are recalculated but trigger only once
  if(!d->balanceCalculationPending) {
    d->balanceCalculationPending = true;
    QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
  }
}

void LedgerAccountFilter::recalculateBalances()
{
  Q_D(LedgerAccountFilter);

  if (sourceModel() == nullptr || d->account.id().isEmpty())
    return;

  const auto start = index(0, 0);
  /// @note No idea if additional filtering is necessary, as we filter on an account anyway
  /// upon no account filter active, we should not show a balance anyway
  const auto indexes = match(start, eMyMoney::Model::SplitAccountIdRole, d->account.id(), -1);
  MyMoneyMoney balance;
  for(const auto &idx : indexes) {
    if(d->showValuesInverted) {
      balance -= idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
    } else {
      balance += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
    }
    const auto dispIndex = index(idx.row(), JournalModel::Column::Balance);
    setData(dispIndex, QVariant::fromValue(balance), Qt::DisplayRole);
  }

  // filterModel->invalidate();
  const QModelIndex top = index(0, JournalModel::Column::Balance);
  const QModelIndex bottom = index(rowCount()-1, JournalModel::Column::Balance);

  dataChanged(top, bottom);
  d->balanceCalculationPending = false;
}

void LedgerAccountFilter::setAccount(const MyMoneyAccount& acc)
{
  Q_D(LedgerAccountFilter);

  d->account = acc;
}

void LedgerAccountFilter::setShowEntryForNewTransaction(bool show)
{
  Q_D(LedgerAccountFilter);

  if (show && !d->newTransactionPresent) {
    d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel()->newTransaction());
    d->newTransactionPresent = true;
  } else if (!show && d->newTransactionPresent) {
    d->concatModel->removeSourceModel(MyMoneyFile::instance()->journalModel()->newTransaction());
    d->newTransactionPresent = false;
  }
}

