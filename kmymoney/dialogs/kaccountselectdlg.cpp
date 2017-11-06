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

#include "kaccountselectdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KStandardGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountselectdlg.h"

#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "kmymoneycategory.h"
#include "kmymoneyaccountselector.h"

#include <../kmymoney.h>
#include "dialogenums.h"
#include "icons/icons.h"

using namespace Icons;

class KAccountSelectDlgPrivate
{
  Q_DISABLE_COPY(KAccountSelectDlgPrivate)

public:
  KAccountSelectDlgPrivate() :
    ui(new Ui::KAccountSelectDlg),
    m_aborted(false)
  {
  }

  ~KAccountSelectDlgPrivate()
  {
    delete ui;
  }

  Ui::KAccountSelectDlg *ui;
  QString                m_purpose;
  MyMoneyAccount         m_account;
  int                    m_mode;       // 0 - select or create, 1 - create only
  eDialogs::Category     m_accountType;
  bool                   m_aborted;
};

KAccountSelectDlg::KAccountSelectDlg(const eDialogs::Category accountType, const QString& purpose, QWidget *parent) :
  QDialog(parent),
  d_ptr(new KAccountSelectDlgPrivate)
{
  Q_D(KAccountSelectDlg);
  d->ui->setupUi(this);
  d->m_purpose = purpose;
  d->m_accountType = accountType;
  // Hide the abort button. It needs to be shown on request by the caller
  // using showAbortButton()
  d->ui->m_kButtonAbort->hide();

  slotReloadWidget();

  KGuiItem skipButtonItem(i18n("&Skip"),
                          QIcon::fromTheme(g_Icons[Icon::MediaSkipForward]),
                          i18n("Skip this transaction"),
                          i18n("Use this to skip importing this transaction and proceed with the next one."));
  KGuiItem::assign(d->ui->m_qbuttonCancel, skipButtonItem);

  KGuiItem createButtenItem(i18n("&Create..."),
                            QIcon::fromTheme(g_Icons[Icon::DocumentNew]),
                            i18n("Create a new account/category"),
                            i18n("Use this to add a new account/category to the file"));
  KGuiItem::assign(d->ui->m_createButton, createButtenItem);
  KGuiItem::assign(d->ui->m_qbuttonOk, KStandardGuiItem::ok());

  KGuiItem abortButtenItem(i18n("&Abort"),
                           QIcon::fromTheme(g_Icons[Icon::DialogCancel]),
                           i18n("Abort the import operation and dismiss all changes"),
                           i18n("Use this to abort the import. Your financial data will be in the state before you started the QIF import."));
  KGuiItem::assign(d->ui->m_kButtonAbort, abortButtenItem);

  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KAccountSelectDlg::slotReloadWidget);

  connect(d->ui->m_createButton,  &QAbstractButton::clicked, this, &KAccountSelectDlg::slotCreateAccount);
  connect(d->ui->m_qbuttonOk,     &QAbstractButton::clicked, this, &QDialog::accept);
  connect(d->ui->m_qbuttonCancel, &QAbstractButton::clicked, this, &QDialog::reject);
  connect(d->ui->m_kButtonAbort,  &QAbstractButton::clicked, this, &KAccountSelectDlg::abort);
}

KAccountSelectDlg::~KAccountSelectDlg()
{
  Q_D(KAccountSelectDlg);
  delete d;
}

void KAccountSelectDlg::slotReloadWidget()
{
  Q_D(KAccountSelectDlg);
  AccountSet set;
  if (d->m_accountType & eDialogs::Category::asset)
    set.addAccountGroup(eMyMoney::Account::Asset);
  if (d->m_accountType & eDialogs::Category::liability)
    set.addAccountGroup(eMyMoney::Account::Liability);
  if (d->m_accountType & eDialogs::Category::income)
    set.addAccountGroup(eMyMoney::Account::Income);
  if (d->m_accountType & eDialogs::Category::expense)
    set.addAccountGroup(eMyMoney::Account::Expense);
  if (d->m_accountType & eDialogs::Category::equity)
    set.addAccountGroup(eMyMoney::Account::Equity);
  if (d->m_accountType & eDialogs::Category::checking)
    set.addAccountType(eMyMoney::Account::Checkings);
  if (d->m_accountType & eDialogs::Category::savings)
    set.addAccountType(eMyMoney::Account::Savings);
  if (d->m_accountType & eDialogs::Category::investment)
    set.addAccountType(eMyMoney::Account::Investment);
  if (d->m_accountType & eDialogs::Category::creditCard)
    set.addAccountType(eMyMoney::Account::CreditCard);

  set.load(d->ui->m_accountSelector->selector());
}

void KAccountSelectDlg::setDescription(const QString& msg)
{
  Q_D(KAccountSelectDlg);
  d->ui->m_descLabel->setText(msg);
}

void KAccountSelectDlg::setHeader(const QString& msg)
{
  Q_D(KAccountSelectDlg);
  d->ui->m_headerLabel->setText(msg);
}

void KAccountSelectDlg::setAccount(const MyMoneyAccount& account, const QString& id)
{
  Q_D(KAccountSelectDlg);
  d->m_account = account;
  d->ui->m_accountSelector->setSelectedItem(id);
}

void KAccountSelectDlg::slotCreateInstitution()
{
  kmymoney->slotInstitutionNew();
}

void KAccountSelectDlg::slotCreateAccount()
{
  Q_D(KAccountSelectDlg);
  if (!((int)d->m_accountType & ((int)eDialogs::Category::expense | (int)eDialogs::Category::income))) {
    kmymoney->slotAccountNew(d->m_account);
    if (!d->m_account.id().isEmpty()) {
      slotReloadWidget();
      d->ui->m_accountSelector->setSelectedItem(d->m_account.id());
      accept();
    }
  } else {
    if (d->m_account.accountType() == eMyMoney::Account::Expense)
      kmymoney->createCategory(d->m_account, MyMoneyFile::instance()->expense());
    else
      kmymoney->createCategory(d->m_account, MyMoneyFile::instance()->income());
    if (!d->m_account.id().isEmpty()) {
      slotReloadWidget();
      d->ui->m_accountSelector->setSelectedItem(d->m_account.id());
      accept();
    }
  }
}

void KAccountSelectDlg::abort()
{
  Q_D(KAccountSelectDlg);
  d->m_aborted = true;
  reject();
}

void KAccountSelectDlg::setMode(const int mode)
{
  Q_D(KAccountSelectDlg);
  d->m_mode = mode ? 1 : 0;
}

void KAccountSelectDlg::showAbortButton(const bool visible)
{
  Q_D(KAccountSelectDlg);
  d->ui->m_kButtonAbort->setVisible(visible);
}

bool KAccountSelectDlg::aborted() const
{
  Q_D(const KAccountSelectDlg);
  return d->m_aborted;
}

void KAccountSelectDlg::hideQifEntry()
{
  Q_D(KAccountSelectDlg);
  d->ui->m_qifEntry->hide();
}

int KAccountSelectDlg::exec()
{
  Q_D(KAccountSelectDlg);
  int rc = Rejected;

  if (d->m_mode == 1) {
    slotCreateAccount();
    rc = result();
  }
  if (rc != Accepted) {
    d->ui->m_createButton->setFocus();
    rc = QDialog::exec();
  }
  return rc;
}

QString KAccountSelectDlg::selectedAccount() const
{
  Q_D(const KAccountSelectDlg);
  return d->ui->m_accountSelector->selectedItem();
}
