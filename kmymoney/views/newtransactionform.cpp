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

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "models.h"
#include "ledgermodel.h"
#include "ledgerproxymodel.h"
#include "modelenums.h"
#include "ui_newtransactionform.h"

using namespace eLedgerModel;

class NewTransactionForm::Private
{
public:
  Private(NewTransactionForm* p)
  : parent(p)
  , filterModel(new LedgerProxyModel(parent))
  , ui(new Ui_NewTransactionForm)
  {
    filterModel->setDynamicSortFilter(true);
    filterModel->setFilterRole((int)eLedgerModel::Role::TransactionSplitId);
    filterModel->setSourceModel(Models::instance()->ledgerModel());
  }
  NewTransactionForm*         parent;
  LedgerProxyModel* filterModel;
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

  QModelIndex index = d->filterModel->index(0, 0);
  if(index.isValid()) {
    d->ui->dateEdit->setText(QLocale().toString(d->filterModel->data(index, (int)Role::PostDate).toDate(),
                                                           QLocale::ShortFormat));
    d->ui->payeeEdit->setText(d->filterModel->data(index, (int)Role::PayeeName).toString());
    d->ui->memoEdit->clear();
    d->ui->memoEdit->insertPlainText(d->filterModel->data(index, (int)Role::Memo).toString());
    d->ui->memoEdit->moveCursor(QTextCursor::Start);
    d->ui->memoEdit->ensureCursorVisible();
    d->ui->accountEdit->setText(d->filterModel->data(index, (int)Role::CounterAccount).toString());
    d->ui->statusEdit->setText(d->filterModel->data(index, (int)Role::ReconciliationLong).toString());
    QString amount = QString("%1 %2").arg(d->filterModel->data(index, (int)Role::ShareAmount).toString())
    .arg(d->filterModel->data(index, (int)Role::ShareAmountSuffix).toString());
    d->ui->amountEdit->setText(amount);
    d->ui->numberEdit->setText(d->filterModel->data(index, (int)Role::Number).toString());
  }
}

void NewTransactionForm::modelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
  const QAbstractItemModel * const model = topLeft.model();
  const int startRow = topLeft.row();
  const int lastRow = bottomRight.row();
  for(int row = startRow; row <= lastRow; ++row) {
    QModelIndex index = model->index(row, 0);
    if(model->data(index, (int)eLedgerModel::Role::TransactionSplitId).toString() == d->transactionSplitId) {
      showTransaction(d->transactionSplitId);
      break;
    }
  }
}
