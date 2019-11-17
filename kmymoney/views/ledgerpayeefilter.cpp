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


#include "ledgerpayeefilter.h"
#include "ledgerfilterbase_p.h"

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
#include "accountsmodel.h"

class LedgerPayeeFilterPrivate : public LedgerFilterBasePrivate
{
public:
  explicit LedgerPayeeFilterPrivate(LedgerPayeeFilter* qq)
  : LedgerFilterBasePrivate(qq)
  , view(nullptr)
  , showValuesInverted(false)
  , balanceCalculationPending(false)
  {}

  ~LedgerPayeeFilterPrivate()
  {
  }

  LedgerView*                 view;
  QStringList                 payeeIdList;
  bool                        showValuesInverted;
  bool                        balanceCalculationPending;
};


LedgerPayeeFilter::LedgerPayeeFilter(LedgerView* parent)
  : LedgerFilterBase(new LedgerPayeeFilterPrivate(this), parent)
{
  Q_D(LedgerPayeeFilter);

  d->view = parent;
  setFilterRole(eMyMoney::Model::SplitPayeeIdRole);
  setObjectName("LedgerPayeeFilter");

  connect(parent, &LedgerView::requestBottomHalfSetup, this, &LedgerPayeeFilter::setupBottomHalf);
  connect(parent, &LedgerView::requestBalanceRecalculation, this, &LedgerPayeeFilter::recalculateBalancesOnIdle);
}

LedgerPayeeFilter::~LedgerPayeeFilter()
{
}

void LedgerPayeeFilter::setupBottomHalf()
{
  Q_D(LedgerPayeeFilter);
  d->view->horizontalHeader()->resizeSection(JournalModel::Column::Reconciliation, 15);

  d->showValuesInverted = false;

  setFilterKeyColumn(0);

  setSortRole(eMyMoney::Model::TransactionPostDateRole);
  sort(JournalModel::Column::Date);

  d->view->setModel(this);
  d->concatModel->setObjectName("LedgerView concatModel");
  d->concatModel->addSourceModel(MyMoneyFile::instance()->journalModel());
  setSourceModel(d->concatModel);

  // we need to do this only once
  disconnect(d->view, &LedgerView::requestBottomHalfSetup, this, &LedgerPayeeFilter::setupBottomHalf);
}


void LedgerPayeeFilter::setShowBalanceInverted(bool inverted)
{
  Q_D(LedgerPayeeFilter);
  d->showValuesInverted = inverted;
}

void LedgerPayeeFilter::recalculateBalancesOnIdle()
{
  Q_D(LedgerPayeeFilter);

  // make sure the balances are recalculated but trigger only once
  if(!d->balanceCalculationPending) {
    d->balanceCalculationPending = true;
    QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
  }
}

void LedgerPayeeFilter::recalculateBalances()
{
  Q_D(LedgerPayeeFilter);

  if (sourceModel() == nullptr || d->payeeIdList.isEmpty())
    return;

  const auto start = index(0, 0);
  /// @todo port to new model code
#if 0
  const auto indexes = match(start, eMyMoney::Model::SplitAccountIdRole, d->payeeId, -1);
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
#endif

  // filterModel->invalidate();
  const QModelIndex top = index(0, JournalModel::Column::Balance);
  const QModelIndex bottom = index(rowCount()-1, JournalModel::Column::Balance);

  dataChanged(top, bottom);
  d->balanceCalculationPending = false;
}

void LedgerPayeeFilter::setPayeeIdList(const QStringList& payeeIds)
{
  Q_D(LedgerPayeeFilter);

  d->payeeIdList = payeeIds;
  setFilterFixedString(QString());
  #if 0
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
#endif
  invalidateFilter();
}

bool LedgerPayeeFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  Q_D(const LedgerPayeeFilter);

  QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
  const auto payeeId = idx.data(eMyMoney::Model::SplitPayeeIdRole).toString();
  // qDebug << payeeId << d->payeeIdList;
  if (!d->payeeIdList.contains(payeeId)) {
    return false;
  }

  const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
  idx = MyMoneyFile::instance()->accountsModel()->indexById(accountId);
  const auto accountGroup = static_cast<eMyMoney::Account::Type>(idx.data(eMyMoney::Model::AccountGroupRole).toInt());
  switch(accountGroup) {
    case eMyMoney::Account::Type::Asset:
    case eMyMoney::Account::Type::Liability:
      break;
    default:
      return false;
  }
  return true;
}
