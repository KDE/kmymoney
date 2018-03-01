/***************************************************************************
                          kfindtransactiondlg.cpp
                             -------------------
    copyright            : (C) 2003, 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017, 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kfindtransactiondlg.h"
#include "kfindtransactiondlg_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QKeyEvent>
#include <QList>
#include <QEvent>
#include <QPushButton>
#include <QDialogButtonBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>
#include <KComboBox>
#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "kmymoneyedit.h"
#include "kmymoneysettings.h"
#include "register.h"
#include "transaction.h"
#include "daterangedlg.h"

#include "ui_kfindtransactiondlg.h"
#include "ui_ksortoptiondlg.h"

KSortOptionDlg::KSortOptionDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::KSortOptionDlg)
{
  ui->setupUi(this);
}

KSortOptionDlg::~KSortOptionDlg()
{
  delete ui;
}

void KSortOptionDlg::setSortOption(const QString& option, const QString& def)
{
  if (option.isEmpty()) {
    ui->m_sortOption->setSettings(def);
    ui->m_useDefault->setChecked(true);
  } else {
    ui->m_sortOption->setSettings(option);
    ui->m_useDefault->setChecked(false);
  }
}

QString KSortOptionDlg::sortOption() const
{
  QString rc;
  if (!ui->m_useDefault->isChecked()) {
    rc = ui->m_sortOption->settings();
  }
  return rc;
}

void KSortOptionDlg::hideDefaultButton()
{
  ui->m_useDefault->hide();
}

KFindTransactionDlg::KFindTransactionDlg(QWidget *parent, bool withEquityAccounts) :
  QDialog(parent),
  d_ptr(new KFindTransactionDlgPrivate(this))
{
  Q_D(KFindTransactionDlg);
  d->init(withEquityAccounts);
}

KFindTransactionDlg::KFindTransactionDlg(KFindTransactionDlgPrivate &dd, QWidget *parent, bool withEquityAccounts) :
  QDialog(parent),
  d_ptr(&dd)
{
  Q_D(KFindTransactionDlg);
  d->init(withEquityAccounts);
}

KFindTransactionDlg::~KFindTransactionDlg()
{
  Q_D(KFindTransactionDlg);
  delete d;
}

void KFindTransactionDlg::slotReset()
{
  Q_D(KFindTransactionDlg);
  d->m_tabFilters->slotReset();
}

void KFindTransactionDlg::slotSearch()
{
  Q_D(KFindTransactionDlg);
  // perform the search only if the button is enabled
  if (!d->ui->buttonBox->button(QDialogButtonBox::Apply)->isEnabled())
    return;

  // setup the filter from the dialog widgets
  d->m_filter = d->m_tabFilters->setupFilter();

  // filter is setup, now fill the register
  slotRefreshView();

  d->ui->m_register->setFocus();
}

void KFindTransactionDlg::slotRefreshView()
{
  Q_D(KFindTransactionDlg);
  d->m_needReload = true;
  if (isVisible()) {
    d->loadView();
    d->m_needReload = false;
  }
}

void KFindTransactionDlg::showEvent(QShowEvent* event)
{
  Q_D(KFindTransactionDlg);
  if (d->m_needReload) {
    d->loadView();
    d->m_needReload = false;
  }
  QDialog::showEvent(event);
}

void KFindTransactionDlg::slotRightSize()
{
  Q_D(KFindTransactionDlg);
  d->ui->m_register->update();
}

void KFindTransactionDlg::resizeEvent(QResizeEvent* ev)
{
  Q_D(KFindTransactionDlg);
  // Columns
  // 1 = Date
  // 2 = Account
  // 4 = Detail
  // 5 = C
  // 6 = Payment
  // 7 = Deposit

  // don't forget the resizer
  QDialog::resizeEvent(ev);

  if (!d->ui->m_register->isVisible())
    return;

  // resize the register
  int w = d->ui->m_register->contentsRect().width();

  int m_debitWidth = 80;
  int m_creditWidth = 80;

  d->ui->m_register->adjustColumn(1);
  d->ui->m_register->adjustColumn(2);
  d->ui->m_register->adjustColumn(5);

  d->ui->m_register->setColumnWidth(6, m_debitWidth);
  d->ui->m_register->setColumnWidth(7, m_creditWidth);

  for (auto i = 0; i < d->ui->m_register->columnCount(); ++i) {
    switch (i) {
      case 4:     // skip the one, we want to set
        break;
      default:
        w -= d->ui->m_register->columnWidth(i);
        break;
    }
  }

  d->ui->m_register->setColumnWidth(4, w);
}

void KFindTransactionDlg::slotSelectTransaction()
{
  Q_D(KFindTransactionDlg);
  auto list = d->ui->m_register->selectedItems();
  if (!list.isEmpty()) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (t) {
      emit transactionSelected(t->split().accountId(), t->transaction().id());
      hide();
    }
  }
}

bool KFindTransactionDlg::eventFilter(QObject* o, QEvent* e)
{
  Q_D(KFindTransactionDlg);
  auto rc = false;

  if (o->isWidgetType()) {
    if (e->type() == QEvent::KeyPress) {
      const QWidget* w = dynamic_cast<const QWidget*>(o);
      QKeyEvent *k = static_cast<QKeyEvent *>(e);
      if (w == d->ui->m_register) {
        switch (k->key()) {
          default:
            break;

          case Qt::Key_Return:
          case Qt::Key_Enter:
            rc = true;
            slotSelectTransaction();
            break;
        }
      }
    }
  }
  return rc;
}

void KFindTransactionDlg::slotShowHelp()
{
  Q_D(KFindTransactionDlg);
  if (d->ui->m_tabWidget->currentIndex() == 0)
    d->m_tabFilters->slotShowHelp();
}

void KFindTransactionDlg::slotSortOptions()
{
  QPointer<KSortOptionDlg> dlg = new KSortOptionDlg(this);

  dlg->setSortOption(KMyMoneySettings::sortSearchView(), QString());
  dlg->hideDefaultButton();

  if (dlg->exec() == QDialog::Accepted) {
    QString sortOrder = dlg->sortOption();
    if (sortOrder != KMyMoneySettings::sortSearchView()) {
      KMyMoneySettings::setSortSearchView(sortOrder);
      slotRefreshView();
    }
  }
  delete dlg;
}
