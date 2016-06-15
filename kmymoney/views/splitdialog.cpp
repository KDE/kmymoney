/***************************************************************************
                          splitdialog.cpp
                             -------------------
    begin                : Sun Apr 3 2016
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

#include "splitdialog.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QHeaderView>
#include <QPainter>
#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_splitdialog.h"
#include "ledgermodel.h"
#include "models.h"
#include "accountsmodel.h"
#include "splitdelegate.h"
#include "newtransactioneditor.h"

class SplitDialog::Private
{
public:
  Private(SplitDialog* p)
  : parent(p)
  , ui(new Ui_SplitDialog)
  {
  }
  SplitDialog*                parent;
  Ui_SplitDialog*             ui;
  SplitDelegate*              splitDelegate;

  /**
   * The account in which this split editor was opened
   */
  MyMoneyAccount              account;

  /**
   * The parent transaction editor which opened the split editor
   */
  NewTransactionEditor*       transactionEditor;
};



SplitDialog::SplitDialog(const MyMoneyAccount& account, NewTransactionEditor* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , d(new Private(this))
{
  d->transactionEditor = parent;
  d->account = account;
  d->ui->setupUi(this);

  d->ui->splitView->setColumnHidden(LedgerModel::DateColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::NumberColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::SecurityColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::ReconciliationColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::PaymentColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::DepositColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::QuantityColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::DepositColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::PriceColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::ValueColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::BalanceColumn, true);

  // setup the item delegate
  d->splitDelegate = new SplitDelegate(d->ui->splitView);
  d->ui->splitView->setItemDelegate(d->splitDelegate);

  // setup some connections
  connect(d->ui->splitView, SIGNAL(aboutToStartEdit()), this, SLOT(disableButtons()));
  connect(d->ui->splitView, SIGNAL(aboutToFinishEdit()), this, SLOT(enableButtons()));
  connect(d->ui->newSplitButton, SIGNAL(pressed()), this, SLOT(newSplit()));

  // finish polishing the widgets
  QMetaObject::invokeMethod(this, "adjustSummary", Qt::QueuedConnection);
}

SplitDialog::~SplitDialog()
{
}

void SplitDialog::enableButtons()
{
  d->ui->buttonContainer->setEnabled(true);
}

void SplitDialog::disableButtons()
{
  d->ui->buttonContainer->setEnabled(false);
}

void SplitDialog::setModel(QAbstractItemModel* model)
{
  d->ui->splitView->setModel(model);

  if(model->rowCount() > 0) {
    QModelIndex index = model->index(0, 0);
    d->ui->splitView->setCurrentIndex(index);
  }

  adjustSummary();

  // force an update of the summary if data changes in the model
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(adjustSummary()));
}

void SplitDialog::adjustSummary()
{
  const int SumRow = 0;
  const int DiffRow = 1;
  const int AmountRow = 2;
  const int HeaderCol = 0;
  const int ValueCol = 1;

  MyMoneyMoney sum;
  for(int row = 0; row < d->ui->splitView->model()->rowCount(); ++row) {
    QModelIndex index = d->ui->splitView->model()->index(row, 0);
    if(index.isValid()) {
      sum += d->ui->splitView->model()->data(index, SplitModel::SplitValueRole).value<MyMoneyMoney>();
    }
  }
  QString formattedValue = sum.formatMoney(d->account.fraction());
  d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);

  MyMoneyMoney amount;
  if(d->transactionEditor) {
    amount = d->transactionEditor->transactionAmount();
    d->splitDelegate->setInversedViewOfAmounts(amount.isNegative());

    amount = amount.abs();
    formattedValue = amount.formatMoney(d->account.fraction());
    d->ui->summaryView->item(AmountRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);
    if((amount - sum).isNegative()) {
      d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Assigned too much"));
    } else {
      d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Unassigned"));
    }
    formattedValue = (amount - sum).abs().formatMoney(d->account.fraction());
    d->ui->summaryView->item(DiffRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);
  } else {
    d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, QString());
    d->ui->summaryView->item(AmountRow, ValueCol)->setData(Qt::DisplayRole, QString());
  }

  d->ui->summaryView->resizeColumnToContents(1);
  d->ui->summaryView->horizontalHeader()->resizeSection(0, d->ui->summaryView->width() - d->ui->summaryView->horizontalHeader()->sectionSize(1) - 10);
}

void SplitDialog::newSplit()
{
  // creating a new split is easy, because we simply
  // need to select the last entry in the view. If we
  // are on this row already with the editor closed things
  // are a bit more complicated.
  QModelIndex index = d->ui->splitView->currentIndex();
  if(index.isValid()) {
    int row = index.row();
    if(row != d->ui->splitView->model()->rowCount()-1) {
      d->ui->splitView->selectRow(d->ui->splitView->model()->rowCount()-1);
    } else {
      d->ui->splitView->edit(index);
    }
  } else {
    d->ui->splitView->selectRow(d->ui->splitView->model()->rowCount()-1);
  }
}
