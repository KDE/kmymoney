/*
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
  , sortPending(false)
  {}

  ~LedgerAccountFilterPrivate()
  {
  }

  OnlineBalanceProxyModel*    onlinebalanceproxymodel;
  MyMoneyAccount              account;
  bool                        balanceCalculationPending;
  bool                        sortPending;
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

  connect(d->concatModel, &QAbstractItemModel::rowsInserted, this, [&](const QModelIndex &parent, int first, int last) {
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)

    Q_D(LedgerAccountFilter);
    // mark this view for sorting but don't actually start sorting
    // until we come back to the main event loop. This allows to collect
    // multiple row insertions into the model into a single sort run.
    // This is important during import of multiple transactions.
    if (!d->sortPending) {
      d->sortPending = true;
      // in case a recalc operation is pending, we turn it off
      // since we need to sort first. Once sorting is done,
      // the recalc will be triggered again
      d->balanceCalculationPending = false;
      QMetaObject::invokeMethod(this, &LedgerAccountFilter::sortView, Qt::QueuedConnection);
    }
  });
}

LedgerAccountFilter::~LedgerAccountFilter()
{
}

void LedgerAccountFilter::sortView()
{
  Q_D(LedgerAccountFilter);
  sort(0);
  d->sortPending = false;

  // trigger a recalculation of the balances after sorting
  recalculateBalancesOnIdle(d->account.id());
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
    // if sorting is pending, we don't trigger recalc as it is part of sorting
    if(!d->balanceCalculationPending && !d->sortPending) {
      d->balanceCalculationPending = true;
      QMetaObject::invokeMethod(this, "recalculateBalances", Qt::QueuedConnection);
    }
  }
}

void LedgerAccountFilter::recalculateBalances()
{
  Q_D(LedgerAccountFilter);

  // false alert. we could end up here in case a recalc is triggered
  // and turned off by sorting afterwards. In this case, we simply
  // skip the calculation as it is in vain.
  if(!d->balanceCalculationPending) {
    return;
  }

  if (sourceModel() == nullptr || d->account.id().isEmpty())
    return;

  // we need to operate on our own model (filtered by account and
  // sorted by date including schedules)
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
  const auto rows = rowCount();
  QModelIndex idx;
  QString accountId;
  for (int row = 0; row < rows; ++row) {
    idx = index(row, 0);
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
      const auto dispIndex = index(row, JournalModel::Column::Balance);
      setData(dispIndex, QVariant::fromValue(balances[accountId]), Qt::DisplayRole);
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
  recalculateBalancesOnIdle(d->account.id());
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
