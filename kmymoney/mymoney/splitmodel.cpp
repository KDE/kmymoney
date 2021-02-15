/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "splitmodel.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStandardItem>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneyexception.h"

struct SplitModel::Private
{
  Private(SplitModel* qq)
  : q(qq)
  , currentSplitCount(-1)
  , headerData(QHash<Column, QString> ({
    { Category, i18nc("Split header", "Category") },
    { Memo, i18nc("Split header", "Memo") },
    { Payment, i18nc("Split header", "Payment") },
    { Deposit, i18nc("Split header", "Deposit") },
  }))
  {
  }

  void copyFrom(const SplitModel& right)
  {
    q->unload();
    headerData = right.d->headerData;
    const auto rows = right.rowCount();
    for (int row = 0; row < rows; ++row) {
      const auto idx = right.index(row, 0);
      const auto split = right.itemByIndex(idx);
      q->appendSplit(split);
    }
        updateItemCount();
  }

  int splitCount() const
  {
    int count = 0;
    const auto rows = q->rowCount();
    for (auto row = 0; row < rows; ++row) {
      const auto idx = q->index(row, 0);
      if (!idx.data(eMyMoney::Model::SplitAccountIdRole).toString().isEmpty()) {
        ++count;
      }
    }
    return count;
  }

  void updateItemCount()
  {
    const auto count = splitCount();
    if (count != currentSplitCount) {
            currentSplitCount = count;
      emit q->itemCountChanged(currentSplitCount);
    }
  }


  QString counterAccount() const
  {
    // A transaction can have more than 2 splits ...
    if(splitCount() > 1) {
      return i18n("Split transaction");

      // ... exactly two splits ...
    } else if(splitCount() == 1) {
      // we have to check which one is filled and which one
      // could be an empty (new) split
      const auto rows = q->rowCount();
      for (auto row = 0; row < rows; ++row) {
        const auto idx = q->index(row, 0);
        const auto accountId = idx.data(eMyMoney::Model::SplitAccountIdRole).toString();
        if (!accountId.isEmpty()) {
          return MyMoneyFile::instance()->accountsModel()->accountIdToHierarchicalName(accountId);
        }
      }

      // ... or a single split
#if 0
    } else if(!idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>().isZero()) {
      return i18n("*** UNASSIGNED ***");
#endif
    }
    return QString();
  }

  int currencyPrecision(const MyMoneySplit& split) const
  {
    const auto account = MyMoneyFile::instance()->accountsModel()->itemById(split.accountId());
    if (!account.id().isEmpty()) {
      try {
        const auto currency = MyMoneyFile::instance()->currency(account.currencyId());
        if (Q_UNLIKELY(account.accountType() == eMyMoney::Account::Type::Cash)) {
          return currency.smallestCashFraction();
        }
        return currency.smallestAccountFraction();
      } catch(MyMoneyException& e) {
      }
    }
    return 100;
  }

  SplitModel*   q;
  int                       currentSplitCount;
  QHash<Column, QString>    headerData;
};

SplitModel::SplitModel(QObject* parent, QUndoStack* undoStack)
  : MyMoneyModel<MyMoneySplit>(parent, QStringLiteral("S"), 4, undoStack)
  , d(new Private(this))
{
  // new splits in the split model start with 2 instead of 1
  // since the first split id is assigned by the transaction
  // editor when the transaction is created. (see
  // NewTransactionEditor::saveTransaction() )
  ++m_nextId;
  connect(this, &SplitModel::modelReset, this, [&] { d->updateItemCount(); });
}

SplitModel::SplitModel(QObject* parent, QUndoStack* undoStack, const SplitModel& right)
  : MyMoneyModel<MyMoneySplit>(parent, QStringLiteral("S"), 4, undoStack)
  , d(new Private(this))
{
  d->copyFrom(right);
}

SplitModel& SplitModel::operator=(const SplitModel& right)
{
  d->copyFrom(right);
  return *this;
}

SplitModel::~SplitModel()
{
}

int SplitModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent);
  Q_ASSERT(d->headerData.count() == MaxColumns);
  return MaxColumns;
}

QString SplitModel::newSplitId()
{
  return QStringLiteral("New-ID");
}

bool SplitModel::isNewSplitId(const QString& id)
{
  return id.compare(newSplitId()) == 0;
}

QVariant SplitModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal) {
    switch (role) {
      case Qt::DisplayRole:
        return d->headerData.value(static_cast<Column>(section));

      case Qt::SizeHintRole:
        return QSize(20, 20);;
    }
    return {};
  }
  if (orientation == Qt::Vertical && role == Qt::SizeHintRole) {
    return QSize(10, 10);
  }

  return QAbstractItemModel::headerData(section, orientation, role);
}

Qt::ItemFlags SplitModel::flags(const QModelIndex& index) const
{
  Q_UNUSED(index);
  return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
}

QVariant SplitModel::data(const QModelIndex& idx, int role) const
{
  if (!idx.isValid())
    return QVariant();
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent()))
    return QVariant();

  const MyMoneySplit& split = static_cast<TreeItem<MyMoneySplit>*>(idx.internalPointer())->constDataRef();
  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch(idx.column()) {
        case Column::Category:
          return MyMoneyFile::instance()->accountsModel()->itemById(split.accountId()).name();

        case Column::Memo:
          {
            QString rc(split.memo());
            // remove empty lines
            rc.replace("\n\n", "\n");
            // replace '\n' with ", "
            rc.replace('\n', ", ");
            return rc;
          }

        case Column::Payment:
          if (!split.id().isEmpty()) {
            const auto value = split.value();
            if (value.isPositive()) {
              return value.formatMoney(d->currencyPrecision(split));
            }
          }
          return {};

        case Column::Deposit:
          if (!split.id().isEmpty()) {
            const auto value = split.value();
            if (value.isNegative() || value.isZero()) {
              return (-value).formatMoney(d->currencyPrecision(split));
            }
          }
          return {};

        default:
          break;
      }
      break;

    case Qt::TextAlignmentRole:
      switch (idx.column()) {
        case Payment:
        case Deposit:
          return QVariant(Qt::AlignRight | Qt::AlignTop);

        default:
          break;
      }
      return QVariant(Qt::AlignLeft | Qt::AlignTop);

    case eMyMoney::Model::IdRole:
      return split.id();

    case eMyMoney::Model::SplitMemoRole:
      return split.memo();

    case eMyMoney::Model::SplitAccountIdRole:
      return split.accountId();

    case eMyMoney::Model::SplitSharesRole:
      return QVariant::fromValue<MyMoneyMoney>(split.shares());

    case eMyMoney::Model::SplitValueRole:
      return QVariant::fromValue<MyMoneyMoney>(split.value());

    case eMyMoney::Model::SplitCostCenterIdRole:
      return split.costCenterId();

    case eMyMoney::Model::SplitNumberRole:
      return split.number();

    case eMyMoney::Model::SplitPayeeIdRole:
      return split.payeeId();

    case eMyMoney::Model::TransactionCounterAccountRole:
      break;

    default:
      break;
  }
  return {};
}

bool SplitModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
  if(!idx.isValid()) {
    return false;
  }
  if (idx.row() < 0 || idx.row() >= rowCount(idx.parent())) {
    return false;
  }

  const auto startIdx = idx.model()->index(idx.row(), 0);
  const auto endIdx = idx.model()->index(idx.row(), idx.model()->columnCount()-1);
  MyMoneySplit& split = static_cast<TreeItem<MyMoneySplit>*>(idx.internalPointer())->dataRef();

  // in case we modify the data of a new split, we need to setup an id
  // this will be updated once we add the split to the transaction
  if (split.id().isEmpty()) {
    split = MyMoneySplit(newSplitId(), split);
  }

  switch(role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      break;

    case eMyMoney::Model::SplitNumberRole:
      split.setNumber(value.toString());
      emit dataChanged(startIdx, endIdx);
      return true;

    case eMyMoney::Model::SplitMemoRole:
      split.setMemo(value.toString());
      emit dataChanged(startIdx, endIdx);
      return true;

    case eMyMoney::Model::SplitAccountIdRole:
      split.setAccountId(value.toString());
      emit dataChanged(startIdx, endIdx);
      return true;

    case eMyMoney::Model::SplitCostCenterIdRole:
      split.setCostCenterId(value.toString());
      emit dataChanged(startIdx, endIdx);
      return true;

    case eMyMoney::Model::SplitSharesRole:
      split.setShares(value.value<MyMoneyMoney>());
      emit dataChanged(startIdx, endIdx);
      return true;

    case eMyMoney::Model::SplitValueRole:
      split.setValue(value.value<MyMoneyMoney>());
      emit dataChanged(startIdx, endIdx);
      return true;

    case eMyMoney::Model::SplitPayeeIdRole:
      split.setPayeeId(value.toString());
      emit dataChanged(startIdx, endIdx);
      return true;

  }
  return QAbstractItemModel::setData(idx, value, role);
}

void SplitModel::appendSplit(const MyMoneySplit& split)
{
  doAddItem(split);
}

void SplitModel::appendEmptySplit()
{
  const QModelIndexList list = match(index(0, 0), eMyMoney::Model::IdRole, QString(), -1, Qt::MatchExactly);
  if(list.isEmpty()) {
    doAddItem(MyMoneySplit());
  }
}

void SplitModel::removeEmptySplit()
{
  const QModelIndexList list = match(index(0, 0), eMyMoney::Model::IdRole, QString(), -1, Qt::MatchExactly);
  if(!list.isEmpty()) {
    removeRow(list.first().row(), list.first().parent());
  }
}

void SplitModel::doAddItem(const MyMoneySplit& item, const QModelIndex& parentIdx)
{
  MyMoneyModel::doAddItem(item, parentIdx);
  d->updateItemCount();
}

void SplitModel::doRemoveItem(const MyMoneySplit& before)
{
  MyMoneyModel::doRemoveItem(before);
  d->updateItemCount();
}

MyMoneyMoney SplitModel::valueSum() const
{
  MyMoneyMoney sum;
  const auto rows = rowCount();
  for (int row = 0; row < rows; ++row) {
    const auto idx = index(row, 0);
        sum += idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>();
  }
  return sum;
}
