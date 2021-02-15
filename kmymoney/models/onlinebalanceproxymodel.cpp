/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "onlinebalanceproxymodel.h"
#include "accountsmodel.h"
#include "mymoneyfile.h"
#include "securitiesmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KDescendantsProxyModel>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "journalmodel.h"
#include "accountsmodel.h"
#include "mymoneyenums.h"

class OnlineBalanceProxyModelPrivate
{
  Q_DECLARE_PUBLIC(OnlineBalanceProxyModel);
  OnlineBalanceProxyModel* q_ptr;

public:
  OnlineBalanceProxyModelPrivate(OnlineBalanceProxyModel* qq)
    : q_ptr(qq)
    , accountsListModel(new KDescendantsProxyModel(qq))
  {
  }

  ~OnlineBalanceProxyModelPrivate() {}

  KDescendantsProxyModel*   accountsListModel;
};

OnlineBalanceProxyModel::OnlineBalanceProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
  , d_ptr(new OnlineBalanceProxyModelPrivate(this))
{
  Q_D(OnlineBalanceProxyModel);
  QSortFilterProxyModel::setSourceModel(d->accountsListModel);
}

OnlineBalanceProxyModel::~OnlineBalanceProxyModel()
{
  Q_D(OnlineBalanceProxyModel);
  delete d;
}

void OnlineBalanceProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  Q_D(OnlineBalanceProxyModel);
  d->accountsListModel->setSourceModel(sourceModel);
}

int OnlineBalanceProxyModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  return MyMoneyFile::instance()->journalModel()->columnCount();
}

QModelIndex OnlineBalanceProxyModel::index(int row, int column, const QModelIndex& parent) const
{
  switch(column) {
    case JournalModel::Column::Balance:
      column = AccountsModel::Column::Balance;
      break;
    default:
      break;
  }
  return QSortFilterProxyModel::index(row, column, parent);
}

QVariant OnlineBalanceProxyModel::data(const QModelIndex& idx, int role) const
{
  if (idx.isValid()) {
    switch(role) {
      case Qt::DisplayRole:
        switch (idx.column()) {
          case JournalModel::Column::Date:
            return QSortFilterProxyModel::data(idx, eMyMoney::Model::AccountOnlineBalanceDateRole);
          case JournalModel::Column::Balance:
            return QSortFilterProxyModel::data(idx, eMyMoney::Model::AccountBalanceRole);
        }
        break;

      case eMyMoney::Model::JournalSplitIdRole:
      case eMyMoney::Model::SplitAccountIdRole:
        return QSortFilterProxyModel::data(idx, eMyMoney::Model::IdRole);

      case eMyMoney::Model::TransactionPostDateRole:
        return QSortFilterProxyModel::data(idx, eMyMoney::Model::AccountOnlineBalanceDateRole);

      case Qt::ForegroundRole:
        return QVariant();

      case Qt::FontRole:
        return QVariant();

      case Qt::TextAlignmentRole:
        switch(idx.column()) {
          case AccountsModel::Column::Balance:
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);

          default:
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        }
        break;
    }
  }

#if 0
  if (idx.column() == JournalModel::Column::Balance) {
    const QModelIndex baseIdx = idx.model()->index(idx.row(), 0, idx.parent());
    return QSortFilterProxyModel::data(baseIdx, role);
  }
#endif
  return QSortFilterProxyModel::data(idx, role);
}

Qt::ItemFlags OnlineBalanceProxyModel::flags(const QModelIndex& index) const
{
  return Qt::NoItemFlags;
}

bool OnlineBalanceProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
  // never show a favorite entry
  auto idx = sourceModel()->index(source_row, 0, source_parent);
  if (idx.data(eMyMoney::Model::AccountIsFavoriteIndexRole).toBool() == true)
    return false;

  // don't show an online balance if there isn't any for the account
  const auto dateValue = idx.data(eMyMoney::Model::AccountOnlineBalanceDateRole);
  if (!dateValue.isValid()) {
    return false;
  }

  return dateValue.toDate().isValid();
}
