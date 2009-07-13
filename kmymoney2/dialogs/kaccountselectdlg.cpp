/***************************************************************************
                          kaccountselectdlg.cpp  -  description
                             -------------------
    begin                : Mon Feb 10 2003
    copyright            : (C) 2000-2003 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <k3activelabel.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountselectdlg.h"
#include <mymoneyinstitution.h>
#include <mymoneyfile.h>
#include <kmymoneycategory.h>
#include "kmymoneyaccountselector.h"

#include <../kmymoney2.h>

KAccountSelectDlg::KAccountSelectDlg(const KMyMoneyUtils::categoryTypeE accountType, const QString& purpose, QWidget *parent )
 : KAccountSelectDlgDecl(parent),
   m_purpose(purpose),
   m_accountType(accountType),
   m_aborted(false)
{
  // Hide the abort button. It needs to be shown on request by the caller
  // using showAbortButton()
  m_kButtonAbort->hide();

  slotReloadWidget();

  KGuiItem skipButtonItem( i18n( "&Skip" ),
                    KIcon("redo"),
                    i18n("Skip this transaction"),
                    i18n("Use this to skip importing this transaction and proceed with the next one."));
  m_qbuttonCancel->setGuiItem(skipButtonItem);

  KGuiItem createButtenItem( i18n( "&Create..." ),
                      KIcon("document-new"),
                      i18n("Create a new account/category"),
                      i18n("Use this to add a new account/category to the file"));
  m_createButton->setGuiItem(createButtenItem);
  m_qbuttonOk->setGuiItem(KStandardGuiItem::ok());

  KGuiItem abortButtenItem( i18n("&Abort" ),
                    KIcon("stop"),
                    i18n("Abort the import operation and dismiss all changes"),
                    i18n("Use this to abort the import. Your financial data will be in the state before you started the QIF import."));
  m_kButtonAbort->setGuiItem(abortButtenItem);


  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadWidget()));

  connect(m_createButton, SIGNAL(clicked()), this, SLOT(slotCreateAccount()));
  connect(m_qbuttonOk, SIGNAL(clicked()), this, SLOT(accept()));
  connect(m_qbuttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_kButtonAbort, SIGNAL(clicked()), this, SLOT(abort()));
}

KAccountSelectDlg::~KAccountSelectDlg()
{
}

void KAccountSelectDlg::slotReloadWidget(void)
{
  AccountSet set;
  if(m_accountType & KMyMoneyUtils::asset)
    set.addAccountGroup(MyMoneyAccount::Asset);
  if(m_accountType & KMyMoneyUtils::liability)
    set.addAccountGroup(MyMoneyAccount::Liability);
  if(m_accountType & KMyMoneyUtils::income)
    set.addAccountGroup(MyMoneyAccount::Income);
  if(m_accountType & KMyMoneyUtils::expense)
    set.addAccountGroup(MyMoneyAccount::Expense);
  if(m_accountType & KMyMoneyUtils::equity)
    set.addAccountGroup(MyMoneyAccount::Equity);

  set.load(m_accountSelector->selector());
}

void KAccountSelectDlg::setDescription(const QString& msg)
{
  m_descLabel->setText(msg);
}

void KAccountSelectDlg::setHeader(const QString& msg)
{
  m_headerLabel->setText(msg);
}

void KAccountSelectDlg::setAccount(const MyMoneyAccount& account, const QString& id)
{
  m_account = account;
  m_accountSelector->setSelectedItem(id);
}

void KAccountSelectDlg::slotCreateInstitution(void)
{
  kmymoney2->slotInstitutionNew();
}

void KAccountSelectDlg::slotCreateAccount(void)
{
  if(!(m_accountType & (KMyMoneyUtils::expense | KMyMoneyUtils::income))) {
    kmymoney2->slotAccountNew(m_account);
    if(!m_account.id().isEmpty()) {
      slotReloadWidget();
      m_accountSelector->setSelectedItem(m_account.id());
      accept();
    }
  } else {
    if(m_account.accountType() == MyMoneyAccount::Expense)
      kmymoney2->createCategory(m_account, MyMoneyFile::instance()->expense());
    else
      kmymoney2->createCategory(m_account, MyMoneyFile::instance()->income());
    if(!m_account.id().isEmpty()) {
      slotReloadWidget();
      m_accountSelector->setSelectedItem(m_account.id());
      accept();
    }
  }
}

void KAccountSelectDlg::abort(void)
{
  m_aborted = true;
  reject();
}

void KAccountSelectDlg::setMode(const int mode)
{
  m_mode = mode ? 1 : 0;
}

void KAccountSelectDlg::showAbortButton(const bool visible)
{
  m_kButtonAbort->setVisible(visible);
}

int KAccountSelectDlg::exec(void)
{
  int rc = Rejected;

  if(m_mode == 1) {
    slotCreateAccount();
    rc = result();
  }
  if(rc != Accepted) {
    m_createButton->setFocus();
    rc = KAccountSelectDlgDecl::exec();
  }
  return rc;
}

const QString& KAccountSelectDlg::selectedAccount(void) const
{
  return m_accountSelector->selectedItem();
}

#include "kaccountselectdlg.moc"
