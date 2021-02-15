/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif


// QBanking includes
#include "kbmapaccount.h"
#include "../widgets/kbaccountlist.h"
#include "../kbanking.h"

// QT includes
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QLayout>
#include <QLineEdit>


#include "ui_kbmapaccount.h"

struct KBMapAccount::Private {
  Ui::KBMapAccount ui;
  KBankingExt *banking;
  AB_ACCOUNT_SPEC *account;
};

KBMapAccount::KBMapAccount(KBankingExt *kb,
                           const char *bankCode,
                           const char *accountId,
                           QWidget* parent,
                           Qt::WindowFlags fl) :
    QDialog(parent, fl),
    d(new Private)
{
  d->banking = kb;
  d->account = 0;
  d->ui.setupUi(this);

  d->ui.accountList->setSelectionMode(QAbstractItemView::SingleSelection);

  if (bankCode)
    d->ui.bankCodeEdit->setText(QString::fromUtf8(bankCode));
  else
    d->ui.bankCodeEdit->setEnabled(false);
  if (accountId)
    d->ui.accountIdEdit->setText(QString::fromUtf8(accountId));
  else
    d->ui.accountIdEdit->setEnabled(false);

  QObject::connect(d->ui.accountList, SIGNAL(itemSelectionChanged()),
                   this, SLOT(slotSelectionChanged()));
  QObject::connect(d->ui.helpButton, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));

  d->ui.accountList->addAccounts(d->banking->getAccounts());
}

KBMapAccount::~KBMapAccount()
{
  delete d;
}

AB_ACCOUNT_SPEC *KBMapAccount::getAccount()
{
  return d->account;
}

void KBMapAccount::accept()
{
  if (d->account)
    QDialog::accept();
}

void KBMapAccount::slotSelectionChanged()
{
  std::list<AB_ACCOUNT_SPEC*> al;
  AB_ACCOUNT_SPEC *a;

  al = d->ui.accountList->getSelectedAccounts();
  if (al.empty()) {
    d->ui.assignButton->setEnabled(false);
    d->account = 0;
    return;
  }
  a = al.front();
  if (AB_AccountSpec_GetUniqueId(a) != 0) {
    d->account = a;
    d->ui.assignButton->setEnabled(true);
  } else
    d->ui.assignButton->setEnabled(false);
}

void KBMapAccount::slotHelpClicked()
{
}

