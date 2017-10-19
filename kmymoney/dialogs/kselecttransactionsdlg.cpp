/***************************************************************************
                          kselecttransactionsdlg.cpp
                             -------------------
    begin                : Wed May 16 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

#include "mymoneytransaction.h"
#include "kmymoneyglobalsettings.h"
#include <KGuiItem>
#include <KStandardGuiItem>

KSelectTransactionsDlg::KSelectTransactionsDlg(const MyMoneyAccount& _account, QWidget* parent) :
    KSelectTransactionsDlgDecl(parent),
    m_account(_account)
{
  // setup descriptive texts
  setWindowTitle(i18n("Select Transaction"));
  m_description->setText(i18n("Select a transaction and press the OK button or use Cancel to select none."));

  // clear current register contents
  m_register->clear();

  // no selection possible
  m_register->setSelectionMode(QTableWidget::SingleSelection);

  // setup header font
  QFont font = KMyMoneyGlobalSettings::listHeaderFont();
  QFontMetrics fm(font);
  int height = fm.lineSpacing() + 6;
  m_register->horizontalHeader()->setMinimumHeight(height);
  m_register->horizontalHeader()->setMaximumHeight(height);
  m_register->horizontalHeader()->setFont(font);

  // setup cell font
  font = KMyMoneyGlobalSettings::listCellFont();
  m_register->setFont(font);

  // ... setup the register columns ...
  m_register->setupRegister(m_account);

  // setup buttons
  KGuiItem::assign(m_helpButton, KStandardGuiItem::help());
  KGuiItem::assign(buttonOk, KStandardGuiItem::ok());
  KGuiItem::assign(buttonCancel, KStandardGuiItem::cancel());

  // default is to need at least one transaction selected
  buttonOk->setDisabled(true);

  // catch some events from the register
  m_register->installEventFilter(this);

  connect(m_register, SIGNAL(transactionsSelected(KMyMoneyRegister::SelectedTransactions)), this, SLOT(slotEnableOk(KMyMoneyRegister::SelectedTransactions)));
  connect(m_register, SIGNAL(editTransaction()), this, SLOT(accept()));

  connect(m_helpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
}

void KSelectTransactionsDlg::slotEnableOk(const KMyMoneyRegister::SelectedTransactions& list)
{
  buttonOk->setEnabled(list.count() != 0);
}

void KSelectTransactionsDlg::addTransaction(const MyMoneyTransaction& t)
{
  QList<MyMoneySplit>::const_iterator it_s;
  for (it_s = t.splits().begin(); it_s != t.splits().end(); ++it_s) {
    if ((*it_s).accountId() == m_account.id()) {
      KMyMoneyRegister::Transaction* tr = KMyMoneyRegister::Register::transactionFactory(m_register, t, (*it_s), 0);
      // force full detail display
      tr->setNumRowsRegister(tr->numRowsRegister(true));
      break;
    }
  }
}

int KSelectTransactionsDlg::exec()
{
  m_register->updateRegister(true);
  m_register->update();

  m_register->setFocus();

  return KSelectTransactionsDlgDecl::exec();
}

void KSelectTransactionsDlg::slotHelp()
{
  // KHelpClient::invokeHelp("details.ledgers.match");
}

void KSelectTransactionsDlg::showEvent(QShowEvent* event)
{
  KSelectTransactionsDlgDecl::showEvent(event);
  m_register->resize(KMyMoneyRegister::DetailColumn, true);
}

void KSelectTransactionsDlg::resizeEvent(QResizeEvent* ev)
{
  // don't forget the resizer
  KSelectTransactionsDlgDecl::resizeEvent(ev);

  // resize the register
  m_register->resize(KMyMoneyRegister::DetailColumn, true);
}

MyMoneyTransaction KSelectTransactionsDlg::transaction() const
{
  MyMoneyTransaction t;

  QList<KMyMoneyRegister::RegisterItem*> list;
  list = m_register->selectedItems();
  if (list.count()) {
    KMyMoneyRegister::Transaction* _t = dynamic_cast<KMyMoneyRegister::Transaction*>(list[0]);
    if (_t)
      t = _t->transaction();
  }
  return t;
}

bool KSelectTransactionsDlg::eventFilter(QObject* o, QEvent* e)
{
  bool rc = false;
  QKeyEvent* k;

  if (o == m_register) {
    switch (e->type()) {
      case QEvent::KeyPress:
        k = dynamic_cast<QKeyEvent*>(e);
        if ((k->modifiers() & Qt::KeyboardModifierMask) == 0
            || (k->modifiers() & Qt::KeypadModifier) != 0) {
          switch (k->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
              if (buttonOk->isEnabled()) {
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
