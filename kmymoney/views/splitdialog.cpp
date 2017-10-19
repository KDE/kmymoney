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
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_splitdialog.h"
#include "mymoneyaccount.h"
#include "ledgermodel.h"
#include "splitdelegate.h"
#include "newtransactioneditor.h"
#include "splitadjustdialog.h"
#include "icons/icons.h"

using namespace Icons;

class SplitDialog::Private
{
public:
  Private(SplitDialog* p)
  : parent(p)
  , ui(new Ui_SplitDialog)
  {
  }

  void deleteSplits(QModelIndexList indexList);

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

  MyMoneyMoney                transactionTotal;
  MyMoneyMoney                splitsTotal;
};

static const int SumRow = 0;
static const int DiffRow = 1;
static const int AmountRow = 2;
static const int HeaderCol = 0;
static const int ValueCol = 1;


void SplitDialog::Private::deleteSplits(QModelIndexList indexList)
{
  if (indexList.isEmpty()) {
    return;
  }

  // remove from the end so that the row information stays
  // consistent and is not changed due to index changes
  QMap<int, int> sortedList;
  foreach(auto index, indexList) {
    sortedList[index.row()] = index.row();
  }

  QMap<int, int>::const_iterator it = sortedList.constEnd();
  do {
    --it;
    ui->splitView->model()->removeRow(*it);
  } while(it != sortedList.constBegin());
}


SplitDialog::SplitDialog(const MyMoneyAccount& account, const MyMoneyMoney& amount, NewTransactionEditor* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
  , d(new Private(this))
{
  d->transactionEditor = parent;
  d->account = account;
  d->transactionTotal = amount;
  d->ui->setupUi(this);

  d->ui->splitView->setColumnHidden(LedgerModel::DateColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::NumberColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::SecurityColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::ReconciliationColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::PaymentColumn, false);
  d->ui->splitView->setColumnHidden(LedgerModel::DepositColumn, false);
  d->ui->splitView->setColumnHidden(LedgerModel::QuantityColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::PriceColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::AmountColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::ValueColumn, true);
  d->ui->splitView->setColumnHidden(LedgerModel::BalanceColumn, true);
  d->ui->splitView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  d->ui->splitView->setSelectionBehavior(QAbstractItemView::SelectRows);

  // setup the item delegate
  d->splitDelegate = new SplitDelegate(d->ui->splitView);
  d->ui->splitView->setItemDelegate(d->splitDelegate);

  d->ui->okButton->setIcon(QIcon::fromTheme(g_Icons[Icon::DialogOK]));
  d->ui->cancelButton->setIcon(QIcon::fromTheme(g_Icons[Icon::DialogCancel]));

  // setup some connections
  connect(d->ui->splitView, SIGNAL(aboutToStartEdit()), this, SLOT(disableButtons()));
  connect(d->ui->splitView, SIGNAL(aboutToFinishEdit()), this, SLOT(enableButtons()));

  connect(d->ui->deleteAllButton, SIGNAL(pressed()), this, SLOT(deleteAllSplits()));
  connect(d->ui->deleteButton, SIGNAL(pressed()), this, SLOT(deleteSelectedSplits()));
  connect(d->ui->deleteZeroButton, SIGNAL(pressed()), this, SLOT(deleteZeroSplits()));
  connect(d->ui->mergeButton, SIGNAL(pressed()), this, SLOT(mergeSplits()));
  connect(d->ui->newSplitButton, SIGNAL(pressed()), this, SLOT(newSplit()));

  // finish polishing the widgets
  QMetaObject::invokeMethod(this, "adjustSummary", Qt::QueuedConnection);
}

SplitDialog::~SplitDialog()
{
}

int SplitDialog::exec()
{
  if(!d->ui->splitView->model()) {
    qWarning() << "SplitDialog::exec() executed without a model. Use setModel() before calling exec().";
    return QDialog::Rejected;
  }
  return QDialog::exec();
}

void SplitDialog::accept()
{
  adjustSummary();
  bool accept = true;
  if(d->transactionTotal != d->splitsTotal) {
    QPointer<SplitAdjustDialog> dlg = new SplitAdjustDialog(this);
    dlg->setValues(d->ui->summaryView->item(AmountRow, ValueCol)->data(Qt::DisplayRole).toString(),
                  d->ui->summaryView->item(SumRow, ValueCol)->data(Qt::DisplayRole).toString(),
                  d->ui->summaryView->item(DiffRow, ValueCol)->data(Qt::DisplayRole).toString(),
                  d->ui->splitView->model()->rowCount());
    accept = false;
    if(dlg->exec() == QDialog::Accepted && dlg) {
      switch(dlg->selectedOption()) {
        case SplitAdjustDialog::SplitAdjustContinue:
          break;
        case SplitAdjustDialog::SplitAdjustChange:
          d->transactionTotal = d->splitsTotal;
          accept = true;
          break;
        case SplitAdjustDialog::SplitAdjustDistribute:
          qWarning() << "SplitDialog::accept needs to implement the case SplitAdjustDialog::SplitAdjustDistribute";
          accept = true;
          break;
        case SplitAdjustDialog::SplitAdjustLeaveAsIs:
          accept = true;
          break;
      }
    }
    delete dlg;
    updateButtonState();
  }
  if(accept)
    QDialog::accept();
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
  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(adjustSummary()));
  connect(d->ui->splitView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(selectionChanged()));
}

void SplitDialog::adjustSummary()
{
  d->splitsTotal = 0;
  for(int row = 0; row < d->ui->splitView->model()->rowCount(); ++row) {
    QModelIndex index = d->ui->splitView->model()->index(row, 0);
    if(index.isValid()) {
      d->splitsTotal += d->ui->splitView->model()->data(index, LedgerRole::SplitValueRole).value<MyMoneyMoney>();
    }
  }
  QString formattedValue = d->splitsTotal.formatMoney(d->account.fraction());
  d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);

  if(d->transactionEditor) {
    d->transactionTotal = d->transactionEditor->transactionAmount();
    formattedValue = d->transactionTotal.formatMoney(d->account.fraction());
    d->ui->summaryView->item(AmountRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);
    if((d->transactionTotal - d->splitsTotal).isNegative()) {
      d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Assigned too much"));
    } else {
      d->ui->summaryView->item(DiffRow, HeaderCol)->setData(Qt::DisplayRole, i18nc("Split editor summary", "Unassigned"));
    }
    formattedValue = (d->transactionTotal - d->splitsTotal).abs().formatMoney(d->account.fraction());
    d->ui->summaryView->item(DiffRow, ValueCol)->setData(Qt::DisplayRole, formattedValue);
  } else {
    d->ui->summaryView->item(SumRow, ValueCol)->setData(Qt::DisplayRole, QString());
    d->ui->summaryView->item(AmountRow, ValueCol)->setData(Qt::DisplayRole, QString());
  }

  adjustSummaryWidth();
  updateButtonState();
}

void SplitDialog::resizeEvent(QResizeEvent* ev)
{
  QDialog::resizeEvent(ev);
  adjustSummaryWidth();
}

void SplitDialog::adjustSummaryWidth()
{
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

MyMoneyMoney SplitDialog::transactionAmount() const
{
  return d->transactionTotal;
}

void SplitDialog::selectionChanged()
{
  updateButtonState();
}

void SplitDialog::updateButtonState()
{
  d->ui->deleteButton->setEnabled(false);
  d->ui->deleteAllButton->setEnabled(false);
  d->ui->mergeButton->setEnabled(false);
  d->ui->deleteZeroButton->setEnabled(false);

  if(d->ui->splitView->selectionModel()->selectedRows().count() >= 1) {
    d->ui->deleteButton->setEnabled(true);
  }

  if(d->ui->splitView->model()->rowCount() > 2) {
    d->ui->deleteAllButton->setEnabled(true);
  }

  QAbstractItemModel* model = d->ui->splitView->model();
  QSet<QString> accountIDs;
  for(int row = 0; row < model->rowCount(); ++row) {
    const QModelIndex index = model->index(row,0);
    // don't check the empty line at the end
    if(model->data(index, LedgerRole::SplitIdRole).toString().isEmpty())
      continue;

    const QString accountID = model->data(index, LedgerRole::AccountIdRole).toString();
    const MyMoneyMoney value = model->data(index, LedgerRole::SplitSharesRole).value<MyMoneyMoney>();
    if(accountIDs.contains(accountID)) {
      d->ui->mergeButton->setEnabled(true);
    }
    if(value.isZero()) {
      d->ui->deleteZeroButton->setEnabled(true);
    }
  }
}

void SplitDialog::deleteSelectedSplits()
{
  d->deleteSplits(d->ui->splitView->selectionModel()->selectedRows());
  adjustSummary();
}

void SplitDialog::deleteAllSplits()
{
  QAbstractItemModel* model = d->ui->splitView->model();
  QModelIndexList list = model->match(model->index(0,0),
                                      LedgerRole::SplitIdRole,
                                      QLatin1String(".+"),
                                      -1,
                                      Qt::MatchRegExp
                                     );
  d->deleteSplits(list);
  adjustSummary();
}

void SplitDialog::deleteZeroSplits()
{
  QAbstractItemModel* model = d->ui->splitView->model();
  QModelIndexList list = model->match(model->index(0,0),
                                      LedgerRole::SplitIdRole,
                                      QLatin1String(".+"),
                                      -1,
                                      Qt::MatchRegExp
                                     );
  for(int idx = 0; idx < list.count();) {
    QModelIndex index = list.at(idx);
    if(!model->data(index, LedgerRole::SplitSharesRole).value<MyMoneyMoney>().isZero()) {
      list.removeAt(idx);
    } else {
      ++idx;
    }
  }
  d->deleteSplits(list);
  adjustSummary();
}

void SplitDialog::mergeSplits()
{
  qDebug() << "Merge splits not yet implemented.";
  adjustSummary();
}
