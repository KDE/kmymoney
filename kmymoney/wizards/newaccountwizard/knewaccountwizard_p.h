/***************************************************************************
                             knewaccountwizard_p.h
                             -------------------
    begin                : Tue Sep 25 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWACCOUNTWIZARD_P_H
#define KNEWACCOUNTWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneywizard_p.h"
#include "knewaccountwizard.h"
#include "kaccounttypepage.h"
#include "mymoneyaccountloan.h"
#include "mymoneymoney.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"

namespace NewAccountWizard
{
  class AccountTypePage;
  class InstitutionPage;
  class BrokeragePage;
  class CreditCardSchedulePage;
  class GeneralLoanInfoPage;
  class LoanDetailsPage;
  class LoanPaymentPage;
  class LoanSchedulePage;
  class LoanPayoutPage;
  class HierarchyPage;
  class AccountSummaryPage;

  class WizardPrivate : public KMyMoneyWizardPrivate
  {
    Q_DISABLE_COPY(WizardPrivate)

  public:
    explicit WizardPrivate(Wizard *qq):
      KMyMoneyWizardPrivate(qq),
      m_institutionPage(nullptr),
      m_accountTypePage(nullptr),
      m_brokeragepage(nullptr),
      m_schedulePage(nullptr),
      m_generalLoanInfoPage(nullptr),
      m_loanDetailsPage(nullptr),
      m_loanPaymentPage(nullptr),
      m_loanSchedulePage(nullptr),
      m_loanPayoutPage(nullptr),
      m_hierarchyPage(nullptr),
      m_accountSummaryPage(nullptr)
    {
    }

    ~WizardPrivate()
    {
    }

    /**
   * This method returns the currently selected currency for the account
   */
    const MyMoneySecurity& currency() const
    {
      return m_accountTypePage->currency();
    }

    /**
   * This method returns the precision in digits for the selected currency.
   * @sa currency()
   */
    int precision() const
    {
      return MyMoneyMoney::denomToPrec(currency().smallestAccountFraction());
    }

    InstitutionPage*         m_institutionPage;
    AccountTypePage*         m_accountTypePage;
    BrokeragePage*           m_brokeragepage;
    CreditCardSchedulePage*  m_schedulePage;
    GeneralLoanInfoPage*     m_generalLoanInfoPage;
    LoanDetailsPage*         m_loanDetailsPage;
    LoanPaymentPage*         m_loanPaymentPage;
    LoanSchedulePage*        m_loanSchedulePage;
    LoanPayoutPage*          m_loanPayoutPage;
    HierarchyPage*           m_hierarchyPage;
    AccountSummaryPage*      m_accountSummaryPage;

    MyMoneyAccountLoan       m_account;
    MyMoneySchedule          m_schedule;
  };
} // namespace

#endif
