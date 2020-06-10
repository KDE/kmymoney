/***************************************************************************
                             kloandetailspage.cpp
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

#include "kloandetailspage.h"
#include "kloandetailspage_p.h"

#include <qmath.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QSpinBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kgeneralloaninfopage.h"
#include "ui_kloandetailspage.h"

#include "kmymoneyfrequencycombo.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kgeneralloaninfopage.h"
#include "kgeneralloaninfopage_p.h"
#include "kloanpaymentpage.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfinancialcalculator.h"
#include "mymoneymoney.h"
#include "wizardpage.h"
#include "kguiutils.h"

using namespace eMyMoney;

namespace NewAccountWizard
{
  LoanDetailsPage::LoanDetailsPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new LoanDetailsPagePrivate(wizard), StepPayments, this, wizard)
  {
    Q_D(LoanDetailsPage);
    d->m_needCalculate = true;
    d->ui->setupUi(this);
    // force the balloon payment to zero (default)
    d->ui->m_balloonAmount->setValue(MyMoneyMoney());
    // allow any precision for the interest rate
    d->ui->m_interestRate->setPrecision(-1);

    connect(d->ui->m_paymentDue, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &LoanDetailsPage::slotValuesChanged);

    connect(d->ui->m_termAmount, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &LoanDetailsPage::slotValuesChanged);
    connect(d->ui->m_termUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::highlighted), this, &LoanDetailsPage::slotValuesChanged);
    connect(d->ui->m_loanAmount, &AmountEdit::textChanged, this, &LoanDetailsPage::slotValuesChanged);
    connect(d->ui->m_interestRate, &AmountEdit::textChanged, this, &LoanDetailsPage::slotValuesChanged);
    connect(d->ui->m_paymentAmount, &AmountEdit::textChanged, this, &LoanDetailsPage::slotValuesChanged);
    connect(d->ui->m_balloonAmount, &AmountEdit::textChanged, this, &LoanDetailsPage::slotValuesChanged);

    connect(d->ui->m_calculateButton, &QAbstractButton::clicked, this, &LoanDetailsPage::slotCalculate);
  }

  LoanDetailsPage::~LoanDetailsPage()
  {
  }

  void LoanDetailsPage::enterPage()
  {
    Q_D(LoanDetailsPage);
    // we need to remove a bunch of entries of the payment frequencies
    d->ui->m_termUnit->clear();

    d->m_mandatoryGroup->clear();
    if (!d->m_wizard->openingBalance().isZero()) {
        d->m_mandatoryGroup->add(d->ui->m_loanAmount);
        if (d->ui->m_loanAmount->text().length() == 0) {
            d->ui->m_loanAmount->setValue(d->m_wizard->openingBalance().abs());
          }
      }

    switch (d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_paymentFrequency->currentItem()) {
      default:
        d->ui->m_termUnit->insertItem(i18n("Payments"), (int)Schedule::Occurrence::Once);
        d->ui->m_termUnit->setCurrentItem((int)Schedule::Occurrence::Once);
        break;
      case Schedule::Occurrence::Monthly:
        d->ui->m_termUnit->insertItem(i18n("Months"), (int)Schedule::Occurrence::Monthly);
        d->ui->m_termUnit->insertItem(i18n("Years"), (int)Schedule::Occurrence::Yearly);
        d->ui->m_termUnit->setCurrentItem((int)Schedule::Occurrence::Monthly);
        break;
      case Schedule::Occurrence::Yearly:
        d->ui->m_termUnit->insertItem(i18n("Years"), (int)Schedule::Occurrence::Yearly);
        d->ui->m_termUnit->setCurrentItem((int)Schedule::Occurrence::Yearly);
        break;
      }
  }

  void LoanDetailsPage::slotValuesChanged()
  {
    Q_D(LoanDetailsPage);
    d->m_needCalculate = true;
    d->m_wizard->completeStateChanged();
  }

  void LoanDetailsPage::slotCalculate()
  {
    Q_D(LoanDetailsPage);
    MyMoneyFinancialCalculator calc;
    MyMoneyMoney val;
    int PF, CF;
    QString result;
    bool moneyBorrowed = d->m_wizard->moneyBorrowed();
    bool moneyLend = !moneyBorrowed;

    // FIXME: for now, we only support interest calculation at the end of the period
    calc.setBep();
    // FIXME: for now, we only support periodic compounding
    calc.setDisc();

    PF = d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_paymentFrequency->eventsPerYear();
    CF = d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_compoundFrequency->eventsPerYear();

    if (PF == 0 || CF == 0)
      return;

    calc.setPF(PF);
    calc.setCF(CF);


    if (!d->ui->m_loanAmount->text().isEmpty()) {
        val = d->ui->m_loanAmount->value().abs();
        if (moneyBorrowed)
          val = -val;
        calc.setPv(val.toDouble());
      }

    if (!d->ui->m_interestRate->text().isEmpty()) {
        val = d->ui->m_interestRate->value().abs();
        calc.setIr(val.toDouble());
      }

    if (!d->ui->m_paymentAmount->text().isEmpty()) {
        val = d->ui->m_paymentAmount->value().abs();
        if (moneyLend)
          val = -val;
        calc.setPmt(val.toDouble());
      }

    if (!d->ui->m_balloonAmount->text().isEmpty()) {
        val = d->ui->m_balloonAmount->value().abs();
        if (moneyLend)
          val = -val;
        calc.setFv(val.toDouble());
      }

    if (d->ui->m_termAmount->value() != 0) {
        calc.setNpp(term());
      }

    // setup of parameters is done, now do the calculation
    try {
      if (d->ui->m_loanAmount->text().isEmpty()) {
          // calculate the amount of the loan out of the other information
          val = MyMoneyMoney(calc.presentValue());
          d->ui->m_loanAmount->setValue(val);
          result = i18n("KMyMoney has calculated the amount of the loan as %1.", d->ui->m_loanAmount->text());

        } else if (d->ui->m_interestRate->text().isEmpty()) {
          // calculate the interest rate out of the other information
          val = MyMoneyMoney(calc.interestRate());

          d->ui->m_interestRate->setValue(val);
          result = i18n("KMyMoney has calculated the interest rate to %1%.", d->ui->m_interestRate->text());

        } else if (d->ui->m_paymentAmount->text().isEmpty()) {
          // calculate the periodical amount of the payment out of the other information
          val = MyMoneyMoney(calc.payment());
          d->ui->m_paymentAmount->setValue(val.abs());
          // reset payment as it might have changed due to rounding
          val = d->ui->m_paymentAmount->value().abs();
          if (moneyLend)
            val = -val;
          calc.setPmt(val.toDouble());

          result = i18n("KMyMoney has calculated a periodic payment of %1 to cover principal and interest.", d->ui->m_paymentAmount->text());

          val = MyMoneyMoney(calc.futureValue());
          if ((moneyBorrowed && val < MyMoneyMoney() && qAbs(val.toDouble()) >= qAbs(calc.payment()))
              || (moneyLend && val > MyMoneyMoney() && qAbs(val.toDouble()) >= qAbs(calc.payment()))) {
              calc.setNpp(calc.npp() - 1);
              // updateTermWidgets(calc.npp());
              val = MyMoneyMoney(calc.futureValue());
              MyMoneyMoney refVal(val);
              d->ui->m_balloonAmount->setValue(refVal);
              result += QString(" ");
              result += i18n("The number of payments has been decremented and the balloon payment has been modified to %1.", d->ui->m_balloonAmount->text());
            } else if ((moneyBorrowed && val < MyMoneyMoney() && qAbs(val.toDouble()) < qAbs(calc.payment()))
                       || (moneyLend && val > MyMoneyMoney() && qAbs(val.toDouble()) < qAbs(calc.payment()))) {
              d->ui->m_balloonAmount->setValue(MyMoneyMoney());
            } else {
              MyMoneyMoney refVal(val);
              d->ui->m_balloonAmount->setValue(refVal);
              result += i18n("The balloon payment has been modified to %1.", d->ui->m_balloonAmount->text());
            }

        } else if (d->ui->m_termAmount->value() == 0) {
          // calculate the number of payments out of the other information
          val = MyMoneyMoney(calc.numPayments());
          if (val == 0)
            throw MYMONEYEXCEPTION_CSTRING("incorrect financial calculation");

          // if the number of payments has a fractional part, then we
          // round it to the smallest integer and calculate the balloon payment
          result = i18n("KMyMoney has calculated the term of your loan as %1. ", updateTermWidgets(qFloor(val.toDouble())));

          if (val.toDouble() != qFloor(val.toDouble())) {
              calc.setNpp(qFloor(val.toDouble()));
              val = MyMoneyMoney(calc.futureValue());
              d->ui->m_balloonAmount->setValue(val);
              result += i18n("The balloon payment has been modified to %1.", d->ui->m_balloonAmount->text());
            }

        } else {
          // calculate the future value of the loan out of the other information
          val = MyMoneyMoney(calc.futureValue());

          // we differentiate between the following cases:
          // a) the future value is greater than a payment
          // b) the future value is less than a payment or the loan is overpaid
          // c) all other cases
          //
          // a) means, we have paid more than we owed. This can't be
          // b) means, we paid more than we owed but the last payment is
          //    less in value than regular payments. That means, that the
          //    future value is to be treated as  (fully payed back)
          // c) the loan is not payed back yet
          if ((moneyBorrowed && val < MyMoneyMoney() && qAbs(val.toDouble()) > qAbs(calc.payment()))
              || (moneyLend && val > MyMoneyMoney() && qAbs(val.toDouble()) > qAbs(calc.payment()))) {
              // case a)
              qDebug("Future Value is %f", val.toDouble());
              throw MYMONEYEXCEPTION_CSTRING("incorrect financial calculation");

            } else if ((moneyBorrowed && val < MyMoneyMoney() && qAbs(val.toDouble()) <= qAbs(calc.payment()))
                       || (moneyLend && val > MyMoneyMoney() && qAbs(val.toDouble()) <= qAbs(calc.payment()))) {
              // case b)
              val = 0;
            }

          result = i18n("KMyMoney has calculated a balloon payment of %1 for this loan.", val.abs().formatMoney(QString(), d->m_wizard->d_func()->precision()));

          if (!d->ui->m_balloonAmount->text().isEmpty()) {
              if ((d->ui->m_balloonAmount->value().abs() - val.abs()).abs().toDouble() > 1) {
                  throw MYMONEYEXCEPTION_CSTRING("incorrect financial calculation");
                }
              result = i18n("KMyMoney has successfully verified your loan information.");
            }
          d->ui->m_balloonAmount->setValue(val);
        }

    } catch (const MyMoneyException &) {
      KMessageBox::error(0,
                         i18n("You have entered mis-matching information. Please modify "
                              "your figures or leave one value empty "
                              "to let KMyMoney calculate it for you"),
                         i18n("Calculation error"));
      return;
    }

    result += i18n("\n\nAccept this or modify the loan information and recalculate.");

    KMessageBox::information(0, result, i18n("Calculation successful"));
    d->m_needCalculate = false;

    // now update change
    d->m_wizard->completeStateChanged();
  }

  int LoanDetailsPage::term() const
  {
    Q_D(const LoanDetailsPage);
    int factor = 0;

    if (d->ui->m_termAmount->value() != 0) {
        factor = 1;
        switch (d->ui->m_termUnit->currentItem()) {
          case Schedule::Occurrence::Yearly: // years
            factor = 12;
            // intentional fall through

          case Schedule::Occurrence::Monthly: // months
            factor *= 30;
            factor *= d->ui->m_termAmount->value();
            // factor now is the duration in days. we divide this by the
            // payment frequency and get the number of payments
            factor /= d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_paymentFrequency->daysBetweenEvents();
            break;

          default:
            qDebug("Unknown term unit %d in LoanDetailsPage::term(). Using payments.", (int)d->ui->m_termUnit->currentItem());
            // intentional fall through

          case Schedule::Occurrence::Once: // payments
            factor = d->ui->m_termAmount->value();
            break;
          }
      }
    return factor;
  }

  QString LoanDetailsPage::updateTermWidgets(const double val)
  {
    Q_D(LoanDetailsPage);
    long vl = qFloor(val);

    QString valString;
    Schedule::Occurrence unit = d->ui->m_termUnit->currentItem();

    if ((unit == Schedule::Occurrence::Monthly)
        && ((vl % 12) == 0)) {
        vl /= 12;
        unit = Schedule::Occurrence::Yearly;
      }

    switch (unit) {
      case Schedule::Occurrence::Monthly:
        valString = i18np("one month", "%1 months", vl);
        d->ui->m_termUnit->setCurrentItem((int)Schedule::Occurrence::Monthly);
        break;
      case Schedule::Occurrence::Yearly:
        valString = i18np("one year", "%1 years", vl);
        d->ui->m_termUnit->setCurrentItem((int)Schedule::Occurrence::Yearly);
        break;
      default:
        valString = i18np("one payment", "%1 payments", vl);
        d->ui->m_termUnit->setCurrentItem((int)Schedule::Occurrence::Once);
        break;
      }
    d->ui->m_termAmount->setValue(vl);
    return valString;
  }

  bool LoanDetailsPage::isComplete() const
  {
    Q_D(const LoanDetailsPage);
    // bool rc = KMyMoneyWizardPage::isComplete();

    int fieldCnt = 0;

    if (d->ui->m_loanAmount->text().length() > 0) {
        fieldCnt++;
      }

    if (d->ui->m_interestRate->text().length() > 0) {
        fieldCnt++;
      }

    if (d->ui->m_termAmount->value() != 0) {
        fieldCnt++;
      }

    if (d->ui->m_paymentAmount->text().length() > 0) {
        fieldCnt++;
      }

    if (d->ui->m_balloonAmount->text().length() > 0) {
        fieldCnt++;
      }

    d->ui->m_calculateButton->setEnabled(fieldCnt == 4 || (fieldCnt == 5 && d->m_needCalculate));

    d->ui->m_calculateButton->setAutoDefault(false);
    d->ui->m_calculateButton->setDefault(false);
    if (d->m_needCalculate && fieldCnt == 4) {
        d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("Press Calculate to verify the values"));
        d->ui->m_calculateButton->setAutoDefault(true);
        d->ui->m_calculateButton->setDefault(true);
      } else if (fieldCnt != 5) {
        d->m_wizard->d_func()->m_nextButton->setToolTip(i18n("Not all details supplied"));
        d->ui->m_calculateButton->setAutoDefault(true);
        d->ui->m_calculateButton->setDefault(true);
      }
    d->m_wizard->d_func()->m_nextButton->setAutoDefault(!d->ui->m_calculateButton->autoDefault());
    d->m_wizard->d_func()->m_nextButton->setDefault(!d->ui->m_calculateButton->autoDefault());

    return (fieldCnt == 5) && !d->m_needCalculate;
  }

  QWidget* LoanDetailsPage::initialFocusWidget() const
  {
    Q_D(const LoanDetailsPage);
    return d->ui->m_paymentDue;
  }

  KMyMoneyWizardPage* LoanDetailsPage::nextPage() const
  {
    Q_D(const LoanDetailsPage);
    return d->m_wizard->d_func()->m_loanPaymentPage;
  }
}
