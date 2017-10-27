/***************************************************************************
                          ledgerviewpage.cpp
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

#include "ledgerviewpage.h"
#include "mymoneyaccount.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "newtransactionform.h"
#include "models.h"
#include "ledgermodel.h"
#include "ui_ledgerviewpage.h"

class LedgerViewPage::Private
{
public:
  Private(QWidget* parent)
  : ui(new Ui_LedgerViewPage)
  , form(0)
  {
    ui->setupUi(parent);

    // make sure, we can disable the detail form but not the ledger view
    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, true);

    // make sure the ledger gets all the stretching
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setSizes(QList<int>() << 10000 << ui->formWidget->sizeHint().height());
  }
  Ui_LedgerViewPage*  ui;
  NewTransactionForm* form;
  QSet<QString>       hideFormReasons;
};

LedgerViewPage::LedgerViewPage(QWidget* parent)
  : QWidget(parent)
  , d(new Private(this))
{
  connect(d->ui->ledgerView, &LedgerView::transactionSelected, this, &LedgerViewPage::transactionSelected);
  connect(d->ui->ledgerView, &LedgerView::aboutToStartEdit, this, &LedgerViewPage::aboutToStartEdit);
  connect(d->ui->ledgerView, &LedgerView::aboutToFinishEdit, this, &LedgerViewPage::aboutToFinishEdit);
  connect(d->ui->ledgerView, &LedgerView::aboutToStartEdit, this, &LedgerViewPage::startEdit);
  connect(d->ui->ledgerView, &LedgerView::aboutToFinishEdit, this, &LedgerViewPage::finishEdit);
  connect(d->ui->splitter, &QSplitter::splitterMoved, this, &LedgerViewPage::splitterChanged);
}

LedgerViewPage::~LedgerViewPage()
{
  delete d;
}

QString LedgerViewPage::accountId() const
{
  return d->ui->ledgerView->accountId();
}

void LedgerViewPage::setAccount(const MyMoneyAccount& acc)
{
  // get rid of current form
  delete d->form;
  d->form = 0;
  d->hideFormReasons.insert(QLatin1String("FormAvailable"));

  switch(acc.accountType()) {
    case eMyMoney::Account::Investment:
      break;

    default:
      d->form = new NewTransactionForm(d->ui->formWidget);
      break;
  }

  if(d->form) {
    d->hideFormReasons.remove(QLatin1String("FormAvailable"));
    // make sure we have a layout
    if(!d->ui->formWidget->layout()) {
      d->ui->formWidget->setLayout(new QHBoxLayout(d->ui->formWidget));
    }
    d->ui->formWidget->layout()->addWidget(d->form);
    connect(d->ui->ledgerView, &LedgerView::transactionSelected,
            d->form, &NewTransactionForm::showTransaction);
    connect(Models::instance()->ledgerModel(), &LedgerModel::dataChanged,
            d->form, &NewTransactionForm::modelDataChanged);
  }
  d->ui->formWidget->setVisible(d->hideFormReasons.isEmpty());
  d->ui->ledgerView->setAccount(acc);
}

void LedgerViewPage::showTransactionForm(bool show)
{
  if(show) {
    d->hideFormReasons.remove(QLatin1String("General"));
  } else {
    d->hideFormReasons.insert(QLatin1String("General"));
  }
  d->ui->formWidget->setVisible(d->hideFormReasons.isEmpty());
}

void LedgerViewPage::startEdit()
{
  d->hideFormReasons.insert(QLatin1String("Edit"));
  d->ui->formWidget->hide();
}

void LedgerViewPage::finishEdit()
{
  d->hideFormReasons.remove(QLatin1String("Edit"));
  d->ui->formWidget->setVisible(d->hideFormReasons.isEmpty());
  // the focus should be on the ledger view once editing ends
  d->ui->ledgerView->setFocus();
}

void LedgerViewPage::splitterChanged(int pos, int index)
{
  Q_UNUSED(pos);
  Q_UNUSED(index);

  d->ui->ledgerView->ensureCurrentItemIsVisible();
}

void LedgerViewPage::setShowEntryForNewTransaction(bool show)
{
  d->ui->ledgerView->setShowEntryForNewTransaction(show);
}
