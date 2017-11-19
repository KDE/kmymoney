/***************************************************************************
                             kloanschedulepage.cpp
                             -------------------
    begin                : Tue Sep 25 2006
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kloanschedulepage.h"
#include "kloanschedulepage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kgeneralloaninfopage.h"
#include "ui_kloanschedulepage.h"

#include <kguiutils.h>
#include "kmymoneyaccountselector.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccountsummarypage.h"
#include "kgeneralloaninfopage.h"
#include "kgeneralloaninfopage_p.h"
#include "kloanpayoutpage.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "wizardpage.h"
#include "mymoneyenums.h"

using namespace NewAccountWizard;
using namespace Icons;
using namespace eMyMoney;

namespace NewAccountWizard
{
  LoanSchedulePage::LoanSchedulePage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new LoanSchedulePagePrivate(wizard), StepSchedule, this, wizard)
  {
    Q_D(const LoanSchedulePage);
    d->ui->setupUi(this);
    d->m_mandatoryGroup->add(d->ui->m_interestCategory->lineEdit());
    d->m_mandatoryGroup->add(d->ui->m_paymentAccount->lineEdit());
    connect(d->ui->m_interestCategory, &KMyMoneyCombo::createItem, this, &LoanSchedulePage::slotCreateCategory);
    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &LoanSchedulePage::slotLoadWidgets);
  }

  LoanSchedulePage::~LoanSchedulePage()
  {
  }

  void LoanSchedulePage::slotCreateCategory(const QString& name, QString& id)
  {
    Q_D(LoanSchedulePage);
    MyMoneyAccount acc, parent;
    acc.setName(name);

    if (d->m_wizard->moneyBorrowed())
      parent = MyMoneyFile::instance()->expense();
    else
      parent = MyMoneyFile::instance()->income();

    emit d->m_wizard->createCategory(acc, parent);

    // return id
    id = acc.id();
  }

  QDate LoanSchedulePage::firstPaymentDueDate() const
  {
    Q_D(const LoanSchedulePage);
    if (d->ui->m_firstPaymentDueDate->isEnabled())
      return d->ui->m_firstPaymentDueDate->date();
    return d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_firstPaymentDate->date();
  }

  QWidget* LoanSchedulePage::initialFocusWidget() const
  {
    Q_D(const LoanSchedulePage);
    return d->ui->m_interestCategory;
  }

  void LoanSchedulePage::enterPage()
  {
    Q_D(LoanSchedulePage);
    d->ui->m_interestCategory->setFocus();
    d->ui->m_firstPaymentDueDate->setDisabled(d->m_wizard->d_func()->m_generalLoanInfoPage->recordAllPayments());
    slotLoadWidgets();
  }

  void LoanSchedulePage::slotLoadWidgets()
  {
    Q_D(LoanSchedulePage);
    AccountSet set;
    if (d->m_wizard->moneyBorrowed())
      set.addAccountGroup(Account::Expense);
    else
      set.addAccountGroup(Account::Income);
    set.load(d->ui->m_interestCategory->selector());

    set.clear();
    set.addAccountGroup(Account::Asset);
    set.load(d->ui->m_paymentAccount->selector());
  }

  KMyMoneyWizardPage* LoanSchedulePage::nextPage() const
  {
    Q_D(const LoanSchedulePage);
    // if the balance widget of the general loan info page is enabled and
    // the value is not zero, then the payout already happened and we don't
    // aks for it.
    if (d->m_wizard->openingBalance().isZero())
      return d->m_wizard->d_func()->m_loanPayoutPage;
    return d->m_wizard->d_func()->m_accountSummaryPage;
  }
}
