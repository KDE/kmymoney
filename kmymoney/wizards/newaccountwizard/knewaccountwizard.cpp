/***************************************************************************
                             knewaccountwizard.cpp
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

#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountsummarypage.h"
#include "kaccounttypepage.h"
#include "kbrokeragepage.h"
#include "kcreditcardschedulepage.h"
#include "kgeneralloaninfopage.h"
#include "khierarchypage.h"
#include "kinstitutionpage.h"
#include "kloandetailspage.h"
#include "kloanpaymentpage.h"
#include "kloanpayoutpage.h"
#include "kloanschedulepage.h"

#include "kaccounttypepage_p.h"
#include "kbrokeragepage_p.h"
#include "kcreditcardschedulepage_p.h"
#include "kgeneralloaninfopage_p.h"
#include "kinstitutionpage_p.h"
#include "kloandetailspage_p.h"
#include "kloanpayoutpage_p.h"
#include "kloanschedulepage_p.h"

#include "kmymoneycurrencyselector.h"
#include "kcurrencycalculator.h"

#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccountloan.h"
#include "mymoneyprice.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"

using namespace NewAccountWizard;
using namespace Icons;
using namespace eMyMoney;

namespace NewAccountWizard
{
  Wizard::Wizard(QWidget *parent, bool modal, Qt::WindowFlags flags)
    : KMyMoneyWizard(*new WizardPrivate(this), parent, modal, flags)
  {
    Q_D(Wizard);
    setTitle(i18n("KMyMoney New Account Setup"));
    addStep(i18n("Institution"));
    addStep(i18n("Account"));
    addStep(i18n("Broker"));
    addStep(i18n("Details"));
    addStep(i18n("Payments"));
    addStep(i18n("Fees"));
    addStep(i18n("Schedule"));
    addStep(i18n("Payout"));
    addStep(i18n("Parent Account"));
    addStep(i18nc("Finish the wizard", "Finish"));
    d->setStepHidden(StepBroker);
    d->setStepHidden(StepSchedule);
    d->setStepHidden(StepPayout);
    d->setStepHidden(StepDetails);
    d->setStepHidden(StepPayments);
    d->setStepHidden(StepFees);

    d->m_institutionPage = new InstitutionPage(this);
    d->m_accountTypePage = new AccountTypePage(this);
    // Investment Pages
    d->m_brokeragepage = new BrokeragePage(this);
    // Credit Card Pages
    d->m_schedulePage = new CreditCardSchedulePage(this);
    // Loan Pages
    d->m_generalLoanInfoPage = new GeneralLoanInfoPage(this);
    d->m_loanDetailsPage = new LoanDetailsPage(this);
    d->m_loanPaymentPage = new LoanPaymentPage(this);
    d->m_loanSchedulePage = new LoanSchedulePage(this);
    d->m_loanPayoutPage = new LoanPayoutPage(this);
    // Not a loan page
    d->m_hierarchyPage = new HierarchyPage(this);
    // Finish
    d->m_accountSummaryPage = new AccountSummaryPage(this);

    d->setFirstPage(d->m_institutionPage);
  }

  Wizard::~Wizard()
  {
  }

  void Wizard::setAccount(const MyMoneyAccount& acc)
  {
    Q_D(Wizard);
    d->m_account = acc;
    d->m_accountTypePage->setAccount(d->m_account);

    if (!acc.institutionId().isEmpty()) {
      d->m_institutionPage->selectExistingInstitution(acc.institutionId());
    }
  }

  MyMoneyMoney Wizard::interestRate() const
  {
    Q_D(const Wizard);
    return d->m_loanDetailsPage->d_func()->ui->m_interestRate->value() / MyMoneyMoney(100, 1);
  }



  const MyMoneyAccount& Wizard::account()
  {
    Q_D(Wizard);
    d->m_account = MyMoneyAccountLoan();
    d->m_account.setName(d->m_accountTypePage->d_func()->ui->m_accountName->text());
    d->m_account.setOpeningDate(d->m_accountTypePage->d_func()->ui->m_openingDate->date());
    d->m_account.setAccountType(d->m_accountTypePage->accountType());
    d->m_account.setInstitutionId(d->m_institutionPage->institution().id());
    d->m_account.setNumber(d->m_institutionPage->d_func()->ui->m_accountNumber->text());
    d->m_account.setValue("iban", d->m_institutionPage->d_func()->ui->m_iban->text());
    if (d->m_accountTypePage->d_func()->ui->m_preferredAccount->isChecked())
      d->m_account.setValue("PreferredAccount", "Yes");
    else
      d->m_account.deletePair("PreferredAccount");

    d->m_account.setCurrencyId(d->currency().id());
    if (d->m_account.isLoan()) {
      // in case we lend the money we adjust the account type
      if (!moneyBorrowed())
        d->m_account.setAccountType(Account::Type::AssetLoan);
      d->m_account.setLoanAmount(d->m_loanDetailsPage->d_func()->ui->m_loanAmount->value());
      d->m_account.setInterestRate(d->m_loanSchedulePage->firstPaymentDueDate(), d->m_loanDetailsPage->d_func()->ui->m_interestRate->value());
      d->m_account.setInterestCalculation(d->m_loanDetailsPage->d_func()->ui->m_paymentDue->currentIndex() == 0 ? MyMoneyAccountLoan::paymentReceived : MyMoneyAccountLoan::paymentDue);
      d->m_account.setFixedInterestRate(d->m_generalLoanInfoPage->d_func()->ui->m_interestType->currentIndex() == 0);
      d->m_account.setFinalPayment(d->m_loanDetailsPage->d_func()->ui->m_balloonAmount->value());
      d->m_account.setTerm(d->m_loanDetailsPage->term());
      d->m_account.setPeriodicPayment(d->m_loanDetailsPage->d_func()->ui->m_paymentAmount->value());
      d->m_account.setPayee(d->m_generalLoanInfoPage->d_func()->ui->m_payee->selectedItem());
      d->m_account.setInterestCompounding((int)d->m_generalLoanInfoPage->d_func()->ui->m_compoundFrequency->currentItem());

      if (!d->m_account.fixedInterestRate()) {
        d->m_account.setNextInterestChange(d->m_generalLoanInfoPage->d_func()->ui->m_interestChangeDateEdit->date());
        d->m_account.setInterestChangeFrequency(d->m_generalLoanInfoPage->d_func()->ui->m_interestFrequencyAmountEdit->value(), d->m_generalLoanInfoPage->d_func()->ui->m_interestFrequencyUnitEdit->currentIndex());
      }
    }
    return d->m_account;
  }

  MyMoneyTransaction Wizard::payoutTransaction()
  {
    Q_D(Wizard);
    MyMoneyTransaction t;
    if (d->m_account.isLoan()                                      // we're creating a loan
        && openingBalance().isZero()                                // and don't have an opening balance
        && !d->m_loanPayoutPage->d_func()->ui->m_noPayoutTransaction->isChecked()) { // and the user wants to have a payout transaction
      t.setPostDate(d->m_loanPayoutPage->d_func()->ui->m_payoutDate->date());
      t.setCommodity(d->m_account.currencyId());
      MyMoneySplit s;
      s.setAccountId(d->m_account.id());
      s.setShares(d->m_loanDetailsPage->d_func()->ui->m_loanAmount->value());
      if (moneyBorrowed())
        s.setShares(-s.shares());
      s.setValue(s.shares());
      t.addSplit(s);

      s.clearId();
      s.setValue(-s.value());
      s.setAccountId(d->m_loanPayoutPage->payoutAccountId());
      MyMoneyMoney shares;
      KCurrencyCalculator::setupSplitPrice(shares, t, s, QMap<QString, MyMoneyMoney>(), this);
      s.setShares(shares);
      t.addSplit(s);
    }
    return t;
  }

  MyMoneyAccount Wizard::parentAccount()
  {
    Q_D(Wizard);
    return d->m_accountTypePage->allowsParentAccount()
        ? d->m_hierarchyPage->parentAccount()
        : (d->m_accountTypePage->accountType() == Account::Type::Loan
           ? d->m_generalLoanInfoPage->parentAccount()
           : d->m_accountTypePage->parentAccount());
  }

  MyMoneyAccount Wizard::brokerageAccount() const
  {
    Q_D(const Wizard);
    MyMoneyAccount account;
    if (d->m_account.accountType() == Account::Type::Investment
        && d->m_brokeragepage->d_func()->ui->m_createBrokerageButton->isChecked()) {
      account.setName(d->m_account.brokerageName());
      account.setAccountType(Account::Type::Checkings);
      account.setInstitutionId(d->m_account.institutionId());
      account.setOpeningDate(d->m_account.openingDate());
      account.setCurrencyId(d->m_brokeragepage->d_func()->ui->m_brokerageCurrency->security().id());
      if (d->m_brokeragepage->d_func()->ui->m_accountNumber->isEnabled() && !d->m_brokeragepage->d_func()->ui->m_accountNumber->text().isEmpty())
        account.setNumber(d->m_brokeragepage->d_func()->ui->m_accountNumber->text());
      if (d->m_brokeragepage->d_func()->ui->m_iban->isEnabled() && !d->m_brokeragepage->d_func()->ui->m_iban->text().isEmpty())
        account.setValue("iban", d->m_brokeragepage->d_func()->ui->m_iban->text());
    }
    return account;
  }

  const MyMoneySchedule& Wizard::schedule()
  {
    Q_D(Wizard);
    d->m_schedule = MyMoneySchedule();

    if (!d->m_account.id().isEmpty()) {
      if (d->m_schedulePage->d_func()->ui->m_reminderCheckBox->isChecked() && (d->m_account.accountType() == Account::Type::CreditCard)) {
        d->m_schedule.setName(d->m_schedulePage->d_func()->ui->m_name->text());
        d->m_schedule.setType(Schedule::Type::Transfer);
        d->m_schedule.setPaymentType(static_cast<Schedule::PaymentType>(d->m_schedulePage->d_func()->ui->m_method->currentItem()));
        d->m_schedule.setFixed(false);
        d->m_schedule.setOccurrencePeriod(Schedule::Occurrence::Monthly);
        d->m_schedule.setOccurrenceMultiplier(1);
        MyMoneyTransaction t;
        MyMoneySplit s;
        s.setPayeeId(d->m_schedulePage->d_func()->ui->m_payee->selectedItem());
        s.setAccountId(d->m_schedulePage->d_func()->ui->m_paymentAccount->selectedItem());
        s.setMemo(i18n("Credit card payment"));
        s.setShares(-d->m_schedulePage->d_func()->ui->m_amount->value());
        s.setValue(s.shares());
        t.addSplit(s);

        s.clearId();
        s.setAccountId(d->m_account.id());
        s.setShares(d->m_schedulePage->d_func()->ui->m_amount->value());
        s.setValue(s.shares());
        t.addSplit(s);

        // setup the next due date
        t.setPostDate(d->m_schedulePage->d_func()->ui->m_date->date());
        d->m_schedule.setTransaction(t);

      } else if (d->m_account.isLoan()) {
        d->m_schedule.setName(i18n("Loan payment for %1", d->m_accountTypePage->d_func()->ui->m_accountName->text()));
        d->m_schedule.setType(Schedule::Type::LoanPayment);
        if (moneyBorrowed())
          d->m_schedule.setPaymentType(Schedule::PaymentType::DirectDebit);
        else
          d->m_schedule.setPaymentType(Schedule::PaymentType::DirectDeposit);

        d->m_schedule.setFixed(true);
        d->m_schedule.setOccurrence(d->m_generalLoanInfoPage->d_func()->ui->m_paymentFrequency->currentItem());

        MyMoneyTransaction t;
        t.setCommodity(d->m_account.currencyId());
        MyMoneySplit s;
        // payment split
        s.setPayeeId(d->m_generalLoanInfoPage->d_func()->ui->m_payee->selectedItem());
        s.setAccountId(d->m_loanSchedulePage->d_func()->ui->m_paymentAccount->selectedItem());
        s.setMemo(i18n("Loan payment"));
        s.setValue(d->m_loanPaymentPage->basePayment() + d->m_loanPaymentPage->additionalFees());
        if (moneyBorrowed()) {
          s.setValue(-s.value());
        }
        s.setShares(s.value());
        if (d->m_account.id() != QLatin1String("Phony-ID")) {
          // if the real account is already set perform the currency conversion if it's necessary
          MyMoneyMoney paymentShares;
          KCurrencyCalculator::setupSplitPrice(paymentShares, t, s, QMap<QString, MyMoneyMoney>(), this);
          s.setShares(paymentShares);
        }
        t.addSplit(s);

        // principal split
        s.clearId();
        s.setAccountId(d->m_account.id());
        s.setShares(MyMoneyMoney::autoCalc);
        s.setValue(MyMoneyMoney::autoCalc);
        s.setMemo(i18n("Amortization"));
        s.setAction(MyMoneySplit::ActionAmortization);
        t.addSplit(s);

        // interest split
        //only add if interest is above zero
        if (!d->m_loanDetailsPage->d_func()->ui->m_interestRate->value().isZero()) {
          s.clearId();
          s.setAccountId(d->m_loanSchedulePage->d_func()->ui->m_interestCategory->selectedItem());
          s.setShares(MyMoneyMoney::autoCalc);
          s.setValue(MyMoneyMoney::autoCalc);
          s.setMemo(i18n("Interest"));
          s.setAction(MyMoneySplit::ActionInterest);
          t.addSplit(s);
        }

        // additional fee splits
        QList<MyMoneySplit> additionalSplits;
        d->m_loanPaymentPage->additionalFeesSplits(additionalSplits);
        QList<MyMoneySplit>::const_iterator it;
        MyMoneyMoney factor(moneyBorrowed() ? 1 : -1, 1);

        for (it = additionalSplits.constBegin(); it != additionalSplits.constEnd(); ++it) {
          s = (*it);
          s.clearId();
          s.setShares(s.shares() * factor);
          s.setValue(s.value() * factor);
          t.addSplit(s);
        }

        // setup the next due date
        t.setPostDate(d->m_loanSchedulePage->firstPaymentDueDate());
        d->m_schedule.setTransaction(t);
      }
    }
    return d->m_schedule;
  }

  MyMoneyMoney Wizard::openingBalance() const
  {
    Q_D(const Wizard);
    // equity accounts don't have an opening balance
    if (d->m_accountTypePage->accountType() == Account::Type::Equity)
      return MyMoneyMoney();

    if (d->m_accountTypePage->accountType() == Account::Type::Loan) {
      if (d->m_generalLoanInfoPage->recordAllPayments())
        return MyMoneyMoney();
      if (moneyBorrowed())
        return -(d->m_generalLoanInfoPage->d_func()->ui->m_openingBalance->value());
      return d->m_generalLoanInfoPage->d_func()->ui->m_openingBalance->value();
    }
    return d->m_accountTypePage->d_func()->ui->m_openingBalance->value();
  }

  MyMoneyPrice Wizard::conversionRate() const
  {
    Q_D(const Wizard);
    if (MyMoneyFile::instance()->baseCurrency().id() == d->m_accountTypePage->d_func()->ui->m_currencyComboBox->security().id())
      return MyMoneyPrice(MyMoneyFile::instance()->baseCurrency().id(),
                          d->m_accountTypePage->d_func()->ui->m_currencyComboBox->security().id(),
                          d->m_accountTypePage->d_func()->ui->m_openingDate->date(),
                          MyMoneyMoney::ONE,
                          i18n("User"));
    return MyMoneyPrice(MyMoneyFile::instance()->baseCurrency().id(),
                        d->m_accountTypePage->d_func()->ui->m_currencyComboBox->security().id(),
                        d->m_accountTypePage->d_func()->ui->m_openingDate->date(),
                        d->m_accountTypePage->d_func()->ui->m_conversionRate->value(),
                        i18n("User"));
  }

  bool Wizard::moneyBorrowed() const
  {
    Q_D(const Wizard);
    return d->m_generalLoanInfoPage->d_func()->ui->m_loanDirection->currentIndex() == 0;
  }
}
