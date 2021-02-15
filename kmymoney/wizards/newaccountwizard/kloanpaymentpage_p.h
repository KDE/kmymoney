/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLOANPAYMENTPAGE_P_H
#define KLOANPAYMENTPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloanpaymentpage.h"

#include "wizardpage_p.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"

namespace NewAccountWizard
{
  class Wizard;

  class LoanPaymentPagePrivate : public WizardPagePrivate<Wizard>
  {
    Q_DISABLE_COPY(LoanPaymentPagePrivate)

  public:
    explicit LoanPaymentPagePrivate(QObject* parent) :
      WizardPagePrivate<Wizard>(parent),
      ui(new Ui::KLoanPaymentPage)
    {
    }

    ~LoanPaymentPagePrivate()
    {
      delete ui;
    }

    Ui::KLoanPaymentPage *ui;
    MyMoneyAccount        phonyAccount;
    MyMoneySplit          phonySplit;
    MyMoneyTransaction    additionalFeesTransaction;
    MyMoneyMoney          additionalFees;
  };
}

#endif
