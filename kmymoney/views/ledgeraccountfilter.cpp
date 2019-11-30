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
#include "ledgerfilterbase_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "journalmodel.h"
#include "accountsmodel.h"
#include "specialdatesmodel.h"
#include "onlinebalanceproxymodel.h"

class LedgerAccountFilterPrivate : public LedgerFilterBasePrivate
{
public:
  explicit LedgerAccountFilterPrivate(LedgerAccountFilter* qq)
  : LedgerFilterBasePrivate(qq)
  , onlinebalanceproxymodel(nullptr)
  , balanceCalculationPending(false)
  {}

  ~LedgerAccountFilterPrivate()
  {
  }

  OnlineBalanceProxyModel*    onlinebalanceproxymodel;
  MyMoneyAccount              account;
  bool                        balanceCalculationPending;
};


LedgerAccountFilter::LedgerAccountFilter(QObject* parent, QVector<QAbstractItemModel*> specialJournalModels)
  : LedgerFilterBase(new LedgerAccountFilterPrivate(this), parent)
{
  Q_D(LedgerAccountFilter);
  d->onlinebalanceproxymodel = new OnlineBalanceProxyModel(parent);

  setFilterKeyColumn(0);
  setFilterRole(eMyMoney::Model::SplitAccountIdRole);
  setObjectName("LedgerAccountFilter");

  d->concatModel->setObjectName("LedgerView concatModel");
  d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());

  d->onlinebalanceproxymodel->setObjectName("OnlineBalanceProxyModel");
  d->onlinebalanceproxymodel->setSourceModel(MyMoneyFile::instance()->accountsModel());
  d->concatModel->addSourceModel(d->onlinebalanceproxymodel);

  for (const auto model : specialJournalModels) {
    d->concatModel->addSourceModel(model);
  }

  setSortRole(eMyMoney::Model::TransactionPostDateRole);
  setSourceModel(d->concatModel);
}

LedgerAccountFilter::~LedgerAccountFilter()
{
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

  const QModelIndex top = index(0, JournalModel::Column::Balance);
  const QModelIndex bottom = index(rowCount()-1, JournalModel::Column::Balance);

  dataChanged(top, bottom);
  d->balanceCalculationPending = false;
}

void LedgerAccountFilter::setAccount(const MyMoneyAccount& acc)
{
  Q_D(LedgerAccountFilter);

  d->account = acc;

  d->showValuesInverted = false;
  if(d->account.accountGroup() == eMyMoney::Account::Type::Liability
    || d->account.accountGroup() == eMyMoney::Account::Type::Income) {
    d->showValuesInverted = true;
  }

  setAccountType(d->account.accountType());
  setFilterFixedString(d->account.id());

  invalidateFilter();
  setSortRole(eMyMoney::Model::TransactionPostDateRole);
  sort(JournalModel::Column::Date);

  // if balance calculation has not been triggered, then run it immediately
  if(!d->balanceCalculationPending) {
    recalculateBalances();
  }

#if 0
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
#endif
}
