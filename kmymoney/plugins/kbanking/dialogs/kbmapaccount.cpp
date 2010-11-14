/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif


// QBanking includes
#include "kbmapaccount.h"
#include "kbaccountlist.h"
#include "mymoneybanking.h"

// QT includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlineedit.h>




KBMapAccount::KBMapAccount(KMyMoneyBanking *kb,
                           const char *bankCode,
                           const char *accountId,
                           QWidget* parent,
                           Qt::WFlags fl)
    : QDialog(parent, fl)
    , _banking(kb)
    , _account(0)
{
  _ui.setupUi(this);

  _ui.accountList->setSelectionMode(QAbstractItemView::SingleSelection);

  if (bankCode)
    _ui.bankCodeEdit->setText(QString::fromUtf8(bankCode));
  else
    _ui.bankCodeEdit->setEnabled(false);
  if (accountId)
    _ui.accountIdEdit->setText(QString::fromUtf8(accountId));
  else
    _ui.accountIdEdit->setEnabled(false);

  QObject::connect(_ui.accountList, SIGNAL(itemSelectionChanged()),
                   this, SLOT(slotSelectionChanged()));
  QObject::connect(_ui.helpButton, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));

  _ui.accountList->addAccounts(_banking->getAccounts());
}


KBMapAccount::~KBMapAccount()
{
}



AB_ACCOUNT *KBMapAccount::getAccount()
{
  return _account;
}



void KBMapAccount::accept()
{
  if (_account)
    QDialog::accept();
}



void KBMapAccount::slotSelectionChanged()
{
  std::list<AB_ACCOUNT*> al;
  AB_ACCOUNT *a;

  al = _ui.accountList->getSelectedAccounts();
  if (al.empty()) {
    _ui.assignButton->setEnabled(false);
    _account = 0;
    return;
  }
  a = al.front();
  if (AB_Account_GetUniqueId(a) != 0) {
    _account = a;
    _ui.assignButton->setEnabled(true);
  } else
    _ui.assignButton->setEnabled(false);
}



void KBMapAccount::slotHelpClicked()
{
}




