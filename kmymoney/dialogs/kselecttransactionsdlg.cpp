/***************************************************************************
                          kselecttransactionsdlg.cpp
                             -------------------
    begin                : Wed May 16 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kselecttransactionsdlg.h"
#include "kselecttransactionsdlg_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QEvent>
#include <QList>
#include <QKeyEvent>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KStandardGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kselecttransactionsdlg.h"

#include "mymoneyaccount.h"
#include "selectedtransactions.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "transaction.h"
#include "kmymoneyglobalsettings.h"
#include "widgetenums.h"

KSelectTransactionsDlg::KSelectTransactionsDlg(const MyMoneyAccount& _account, QWidget* parent) :
  QDialog(parent),
  d_ptr(new KSelectTransactionsDlgPrivate)
{
  Q_D(KSelectTransactionsDlg);
  d->m_account = _account;
  d->ui->setupUi(this);
  // setup descriptive texts
  setWindowTitle(i18n("Select Transaction"));
  d->ui->m_description->setText(i18n("Select a transaction and press the OK button or use Cancel to select none."));

  // clear current register contents
  d->ui->m_register->clear();

  // no selection possible
  d->ui->m_register->setSelectionMode(QTableWidget::SingleSelection);

  // setup header font
  auto font = KMyMoneyGlobalSettings::listHeaderFont();
  QFontMetrics fm(font);
  auto height = fm.lineSpacing() + 6;
  d->ui->m_register->horizontalHeader()->setMinimumHeight(height);
  d->ui->m_register->horizontalHeader()->setMaximumHeight(height);
  d->ui->m_register->horizontalHeader()->setFont(font);

  // setup cell font
  font = KMyMoneyGlobalSettings::listCellFont();
  d->ui->m_register->setFont(font);

  // ... setup the register columns ...
  d->ui->m_register->setupRegister(d->m_account);

  // setup buttons

  // default is to need at least one transaction selected
  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);

  // catch some events from the register
  d->ui->m_register->installEventFilter(this);

  connect(d->ui->m_register, &KMyMoneyRegister::Register::transactionsSelected, this, &KSelectTransactionsDlg::slotEnableOk);
  connect(d->ui->m_register, &KMyMoneyRegister::Register::editTransaction, this, &QDialog::accept);

  connect(d->ui->buttonBox, &QDialogButtonBox::helpRequested, this, &KSelectTransactionsDlg::slotHelp);
}

KSelectTransactionsDlg::~KSelectTransactionsDlg()
{
  Q_D(KSelectTransactionsDlg);
  delete d;
}

void KSelectTransactionsDlg::slotEnableOk(const KMyMoneyRegister::SelectedTransactions& list)
{
  Q_D(KSelectTransactionsDlg);
  d->ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(list.count() != 0);
}

void KSelectTransactionsDlg::addTransaction(const MyMoneyTransaction& t)
{
  Q_D(KSelectTransactionsDlg);
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if ((*it_s).accountId() == d->m_account.id()) {
      KMyMoneyRegister::Transaction* tr = KMyMoneyRegister::Register::transactionFactory(d->ui->m_register, t, (*it_s), 0);
      // force full detail display
      tr->setNumRowsRegister(tr->numRowsRegister(true));
      break;
    }
  }
}

int KSelectTransactionsDlg::exec()
{
  Q_D(KSelectTransactionsDlg);
  d->ui->m_register->updateRegister(true);
  d->ui->m_register->update();

  d->ui->m_register->setFocus();

  return QDialog::exec();
}

void KSelectTransactionsDlg::slotHelp()
{
  // KHelpClient::invokeHelp("details.ledgers.match");
}

void KSelectTransactionsDlg::showEvent(QShowEvent* event)
{
  Q_D(KSelectTransactionsDlg);
  QDialog::showEvent(event);
  d->ui->m_register->resize((int)eWidgets::eTransaction::Column::Detail, true);
}

void KSelectTransactionsDlg::resizeEvent(QResizeEvent* ev)
{
  Q_D(KSelectTransactionsDlg);
  // don't forget the resizer
  QDialog::resizeEvent(ev);

  // resize the register
  d->ui->m_register->resize((int)eWidgets::eTransaction::Column::Detail, true);
}

MyMoneyTransaction KSelectTransactionsDlg::transaction() const
{
  Q_D(const KSelectTransactionsDlg);
  MyMoneyTransaction t;

  QList<KMyMoneyRegister::RegisterItem*> list;
  list = d->ui->m_register->selectedItems();
  if (list.count()) {
    KMyMoneyRegister::Transaction* _t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (_t)
      t = _t->transaction();
  }
  return t;
}

KMyMoneyRegister::Register* KSelectTransactionsDlg::getRegister()
{
  Q_D(KSelectTransactionsDlg);
  return d->ui->m_register;
}

bool KSelectTransactionsDlg::eventFilter(QObject* o, QEvent* e)
{
  Q_D(KSelectTransactionsDlg);
  auto rc = false;
  QKeyEvent* k;

  if (o == d->ui->m_register) {
    switch (e->type()) {
      case QEvent::KeyPress:
        k = dynamic_cast<QKeyEvent*>(e);
        if ((k->modifiers() & Qt::KeyboardModifierMask) == 0
            || (k->modifiers() & Qt::KeypadModifier) != 0) {
          switch (k->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
              if (d->ui->buttonBox->button(QDialogButtonBox::Ok)->isEnabled()) {
                accept();
                rc = true;
              }
              // tricky fall through here
            default:
              break;
          }
        }
        // tricky fall through here
      default:
        break;
    }
  }
  return rc;
}
