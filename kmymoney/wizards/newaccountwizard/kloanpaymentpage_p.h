/***************************************************************************
                             kloanpaymentpage.cpp
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
    LoanPaymentPagePrivate(QObject* parent) :
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
