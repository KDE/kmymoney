/***************************************************************************
                          newtransactioneform.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "newtransactionform.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>
#include <QDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocale>

// ----------------------------------------------------------------------------
// Project Includes

#include "models.h"
#include "ledgermodel.h"
#include "ui_newtransactionform.h"

class NewTransactionForm::Private
{
public:
  Private(NewTransactionForm* p)
  : parent(p)
  , filterModel(new LedgerSortFilterProxyModel(parent))
  , ui(new Ui_NewTransactionForm)
  {
    filterModel->setDynamicSortFilter(true);
    filterModel->setFilterRole(LedgerModel::TransactionSplitIdRole);
    filterModel->setSourceModel(Models::instance()->ledgerModel());
  }
  NewTransactionForm*         parent;
  LedgerSortFilterProxyModel* filterModel;
  Ui_NewTransactionForm*      ui;
  QString                     transactionSplitId;
};







NewTransactionForm::NewTransactionForm(QWidget* parent)
  : QFrame(parent)
  , d(new Private(this))
{
  d->ui->setupUi(this);
}

NewTransactionForm::~NewTransactionForm()
{
  delete d;
}

void NewTransactionForm::showTransaction(const QString& transactionSplitId)
{
  d->transactionSplitId = transactionSplitId;
  d->filterModel->setFilterFixedString(transactionSplitId);

  qDebug() << "NewTransactionForm::showTransaction:" << transactionSplitId;

  QModelIndex index = d->filterModel->index(0, 0);
  if(index.isValid()) {
    d->ui->dateEdit->setText(KLocale::global()->formatDate(d->filterModel->data(index, LedgerModel::PostDateRole).toDate(),
                                                           KLocale::ShortDate));
    d->ui->payeeEdit->setText(d->filterModel->data(index, LedgerModel::PayeeNameRole).toString());
    d->ui->memoEdit->clear();
    d->ui->memoEdit->insertPlainText(d->filterModel->data(index, LedgerModel::MemoRole).toString());
    d->ui->memoEdit->moveCursor(QTextCursor::Start);
    d->ui->memoEdit->ensureCursorVisible();
    d->ui->accountEdit->setText(d->filterModel->data(index, LedgerModel::CounterAccountRole).toString());
    d->ui->statusEdit->setText(d->filterModel->data(index, LedgerModel::ReconciliationRoleLong).toString());
    QString amount = QString("%1 %2").arg(d->filterModel->data(index, LedgerModel::ShareAmountRole).toString())
                                     .arg(d->filterModel->data(index, LedgerModel::ShareAmountSuffixRole).toString());
    d->ui->amountEdit->setText(amount);
    d->ui->numberEdit->setText(d->filterModel->data(index, LedgerModel::NumberRole).toString());
  }
}

void NewTransactionForm::modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  const QAbstractItemModel * const model = topLeft.model();
  const int startRow = topLeft.row();
  const int lastRow = bottomRight.row();
  qDebug() << "startRow" << startRow << ", lastRow" << lastRow;
  for(int row = startRow; row <= lastRow; ++row) {
    QModelIndex index = model->index(row, 0);
    if(model->data(index, LedgerModel::TransactionSplitIdRole).toString() == d->transactionSplitId) {
      showTransaction(d->transactionSplitId);
      break;
    }
  }
}
