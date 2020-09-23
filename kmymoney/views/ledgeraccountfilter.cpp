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

void LedgerAccountFilter::recalculateBalancesOnIdle(const QString& accountId)
{
  Q_D(LedgerAccountFilter);
  // only start recalc if the caller means us
  if (!accountId.compare(d->account.id())) {
    // make sure the balances are recalculated but trigger only once
    if(!d->balanceCalculationPending) {
      d->balanceCalculationPending = true;
      QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
    }
  }
}

void LedgerAccountFilter::recalculateBalances()
{
  Q_D(LedgerAccountFilter);

  if (sourceModel() == nullptr || d->account.id().isEmpty())
    return;

  // we need to operate on the base model, not the filtered one
  // and update only the selected account(s). In case of investment
  // accounts, we also update the balance of the underlying stock
  // accounts.
  bool isInvestmentAccount = false;
  QStringList accountIds;
  accountIds << d->account.id();
  if (d->account.accountType() == eMyMoney::Account::Type::Investment) {
    isInvestmentAccount = true;
    accountIds << d->account.accountList();
  }

  QHash<QString, MyMoneyMoney> balances;
  const auto model = MyMoneyFile::instance()->journalModel();
  const auto rows = model->rowCount();
  QModelIndex idx;
  QString accountId;
  for (int row = 0; row < rows; ++row) {
    idx = model->index(row, 0);
    accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
    if (accountIds.contains(accountId)) {
      if (isInvestmentAccount) {
        if (idx.data(eMyMoney::Model::TransactionIsStockSplitRole).toBool()) {
          balances[accountId] *= idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        } else {
          balances[accountId] += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        }

      } else {
        if(d->showValuesInverted) {
          balances[accountId] -= idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        } else {
          balances[accountId] += idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
        }
      }
      const auto dispIndex = model->index(idx.row(), JournalModel::Column::Balance);
      model->setData(dispIndex, QVariant::fromValue(balances[accountId]), Qt::DisplayRole);

    }
  }

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
}

bool LedgerAccountFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  Q_D(const LedgerAccountFilter);
  auto rc = LedgerFilterBase::filterAcceptsRow(source_row, source_parent);

  // in case we don't have a match and the current account is an investment account
  // we check if the journal entry references a child account of the investment account
  // if so, we need to display the transaction
  if (!rc && d->account.accountType() == eMyMoney::Account::Type::Investment) {
    const auto idx = sourceModel()->index(source_row, 0, source_parent);
    rc = d->account.accountList().contains(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
  }
  return rc;
}
