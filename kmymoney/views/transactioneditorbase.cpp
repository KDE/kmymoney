/*
 * SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "transactioneditorbase.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QModelIndex>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneytransaction.h"
#include "mymoneyexception.h"
#include "splitmodel.h"

TransactionEditorBase::TransactionEditorBase(QWidget* parent, const QString& accountId)
  : QFrame(parent, Qt::FramelessWindowHint /* | Qt::X11BypassWindowManagerHint */)
{
  Q_UNUSED(accountId)
}

TransactionEditorBase::~TransactionEditorBase()
{
}

void TransactionEditorBase::addSplitsFromModel(QList<MyMoneySplit>& splits, const SplitModel* model)
{
  const auto rows = model->rowCount();
  for (int row = 0; row < rows; ++row) {
    const auto idx = model->index(row, 0);
    MyMoneySplit s;
    s.setNumber(idx.data(eMyMoney::Model::SplitNumberRole).toString());
    s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
    s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
    s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
    s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());
    s.setCostCenterId(idx.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
    s.setPayeeId(idx.data(eMyMoney::Model::SplitPayeeIdRole).toString());
    s.setTagIdList(idx.data(eMyMoney::Model::SplitTagIdRole).toStringList());
    splits.append(s);
  }
}

void TransactionEditorBase::addSplitsFromModel(MyMoneyTransaction& t, const SplitModel* model)
{
  // now update and add what we have in the model
  const auto rows = model->rowCount();
  for (int row = 0; row < rows; ++row) {
    const auto idx = model->index(row, 0);
    MyMoneySplit s;
    const QString splitId = idx.data(eMyMoney::Model::IdRole).toString();
    // Extract the split from the transaction if
    // it already exists. Otherwise it remains
    // an empty split and will be added later.
    try {
      s = t.splitById(splitId);
    } catch(const MyMoneyException&) {
    }
    s.setNumber(idx.data(eMyMoney::Model::SplitNumberRole).toString());
    s.setMemo(idx.data(eMyMoney::Model::SplitMemoRole).toString());
    s.setAccountId(idx.data(eMyMoney::Model::SplitAccountIdRole).toString());
    s.setShares(idx.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>());
    s.setValue(idx.data(eMyMoney::Model::SplitValueRole).value<MyMoneyMoney>());
    s.setCostCenterId(idx.data(eMyMoney::Model::SplitCostCenterIdRole).toString());
    s.setPayeeId(idx.data(eMyMoney::Model::SplitPayeeIdRole).toString());
    s.setTagIdList(idx.data(eMyMoney::Model::SplitTagIdRole).toStringList());

    if (s.id().isEmpty()) {
      t.addSplit(s);
    } else {
      t.modifySplit(s);
    }
  }
}
