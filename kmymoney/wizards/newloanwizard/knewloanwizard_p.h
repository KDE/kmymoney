/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNEWLOANWIZARD_P_H
#define KNEWLOANWIZARD_P_H

#include "knewloanwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QBitArray>
#include <qmath.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewloanwizard.h"
#include "ui_namewizardpage.h"
#include "ui_firstpaymentwizardpage.h"
#include "ui_loanamountwizardpage.h"
#include "ui_interestwizardpage.h"
#include "ui_paymenteditwizardpage.h"
#include "ui_finalpaymentwizardpage.h"
#include "ui_interestcategorywizardpage.h"
#include "ui_assetaccountwizardpage.h"
#include "ui_schedulewizardpage.h"
#include "ui_paymentwizardpage.h"

#include "kmymoneyutils.h"
#include "kmymoneysettings.h"

#include "mymoneyfinancialcalculator.h"
#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "mymoneysecurity.h"
#include "mymoneyaccountloan.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "mymoneyenums.h"

namespace Ui { class KNewLoanWizard; }

class KNewLoanWizard;
class KNewLoanWizardPrivate
{
  Q_DISABLE_COPY(KNewLoanWizardPrivate)
  Q_DECLARE_PUBLIC(KNewLoanWizard)

public:
  explicit KNewLoanWizardPrivate(KNewLoanWizard *qq) :
    q_ptr(qq),
    ui(new Ui::KNewLoanWizard)
  {
  }

  ~KNewLoanWizardPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KNewLoanWizard);
    ui->setupUi(q);
    m_pages = QBitArray(KNewLoanWizard::Page_Summary + 1, true);
    q->setModal(true);

    KMyMoneyMVCCombo::setSubstringSearchForChildren(ui->m_namePage, !KMyMoneySettings::stringMatchFromStart());

    // make sure, the back button does not clear fields
    q->setOption(QWizard::IndependentPages, true);

    // connect(m_payeeEdit, SIGNAL(newPayee(QString)), this, SLOT(slotNewPayee(QString)));
    q->connect(ui->m_namePage->ui->m_payeeEdit, &KMyMoneyMVCCombo::createItem, q, &KNewLoanWizard::slotNewPayee);

    q->connect(ui->m_additionalFeesPage, &AdditionalFeesWizardPage::newCategory, q, &KNewLoanWizard::slotNewCategory);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KNewLoanWizard::slotReloadEditWidgets);

    resetCalculator();

    q->slotReloadEditWidgets();

    // As default we assume a liability loan, with fixed interest rate,
    // with a first payment due on the 30th of this month. All payments
    // should be recorded and none have been made so far.

    //FIXME: port
    ui->m_firstPaymentPage->ui->m_firstDueDateEdit->loadDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 30));

    // FIXME: we currently only support interest calculation on reception
    m_pages.clearBit(KNewLoanWizard::Page_InterestCalculation);

    // turn off all pages that are contained here for derived classes
    m_pages.clearBit(KNewLoanWizard::Page_EditIntro);
    m_pages.clearBit(KNewLoanWizard::Page_EditSelection);
    m_pages.clearBit(KNewLoanWizard::Page_EffectiveDate);
    m_pages.clearBit(KNewLoanWizard::Page_PaymentEdit);
    m_pages.clearBit(KNewLoanWizard::Page_InterestEdit);
    m_pages.clearBit(KNewLoanWizard::Page_SummaryEdit);

    // for now, we don't have online help :-(
    q->setOption(QWizard::HaveHelpButton, false);

    // setup a phony transaction for additional fee processing
    m_account = MyMoneyAccount("Phony-ID", MyMoneyAccount());
    m_split.setAccountId(m_account.id());
    m_split.setValue(MyMoneyMoney());
    m_transaction.addSplit(m_split);

    KMyMoneyUtils::updateWizardButtons(q);
  }

  void resetCalculator()
  {
    Q_Q(KNewLoanWizard);
    ui->m_loanAmountPage->resetCalculator();
    ui->m_interestPage->resetCalculator();
    ui->m_durationPage->resetCalculator();
    ui->m_paymentPage->resetCalculator();
    ui->m_finalPaymentPage->resetCalculator();

    q->setField("additionalCost", MyMoneyMoney().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
  }

  void updateLoanAmount()
  {
    Q_Q(KNewLoanWizard);
    QString txt;
    //FIXME: port
    if (! q->field("loanAmountEditValid").toBool()) {
      txt = QString("<") + i18n("calculate") + QString(">");
    } else {
      txt = q->field("loanAmountEdit").value<MyMoneyMoney>().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId())));
    }
    q->setField("loanAmount1", txt);
    q->setField("loanAmount2", txt);
    q->setField("loanAmount3", txt);
    q->setField("loanAmount4", txt);
    q->setField("loanAmount5", txt);
  }

  void updateInterestRate()
  {
    Q_Q(KNewLoanWizard);
    QString txt;
    //FIXME: port
    if (! q->field("interestRateEditValid").toBool()) {
      txt = QString("<") + i18n("calculate") + QString(">");
    } else {
      txt = q->field("interestRateEdit").value<MyMoneyMoney>().formatMoney(QString(), 3) + QString("%");
    }
    q->setField("interestRate1", txt);
    q->setField("interestRate2", txt);
    q->setField("interestRate3", txt);
    q->setField("interestRate4", txt);
    q->setField("interestRate5", txt);
  }

  void updateDuration()
  {
    Q_Q(KNewLoanWizard);
    QString txt;
    //FIXME: port
    if (q->field("durationValueEdit").toInt() == 0) {
      txt = QString("<") + i18n("calculate") + QString(">");
    } else {
      txt = QString().sprintf("%d ", q->field("durationValueEdit").toInt())
            + q->field("durationUnitEdit").toString();
    }
    q->setField("duration1", txt);
    q->setField("duration2", txt);
    q->setField("duration3", txt);
    q->setField("duration4", txt);
    q->setField("duration5", txt);
  }

  void updatePayment()
  {
    Q_Q(KNewLoanWizard);
    QString txt;
    //FIXME: port
    if (! q->field("paymentEditValid").toBool()) {
      txt = QString("<") + i18n("calculate") + QString(">");
    } else {
      txt = q->field("paymentEdit").value<MyMoneyMoney>().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId())));
    }
    q->setField("payment1", txt);
    q->setField("payment2", txt);
    q->setField("payment3", txt);
    q->setField("payment4", txt);
    q->setField("payment5", txt);
    q->setField("basePayment", txt);
  }

  void updateFinalPayment()
  {
    Q_Q(KNewLoanWizard);
    QString txt;
    //FIXME: port
    if (! q->field("finalPaymentEditValid").toBool()) {
      txt = QString("<") + i18n("calculate") + QString(">");
    } else {
      txt = q->field("finalPaymentEdit").value<MyMoneyMoney>().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId())));
    }
    q->setField("balloon1", txt);
    q->setField("balloon2", txt);
    q->setField("balloon3", txt);
    q->setField("balloon4", txt);
    q->setField("balloon5", txt);
  }

  void updateLoanInfo()
  {
    Q_Q(KNewLoanWizard);
    updateLoanAmount();
    updateInterestRate();
    updateDuration();
    updatePayment();
    updateFinalPayment();
    ui->m_additionalFeesPage->updatePeriodicPayment(m_account);

    QString txt;

    int fraction = m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()));
    q->setField("loanAmount6", q->field("loanAmountEdit").value<MyMoneyMoney>().formatMoney(fraction));
    q->setField("interestRate6", QString(q->field("interestRateEdit").value<MyMoneyMoney>().formatMoney("", 3) + QString("%")));
    txt = QString().sprintf("%d ", q->field("durationValueEdit").toInt())
          + q->field("durationUnitEdit").toString();
    q->setField("duration6", txt);
    q->setField("payment6", q->field("paymentEdit").value<MyMoneyMoney>().formatMoney(fraction));
    q->setField("balloon6", q->field("finalPaymentEdit").value<MyMoneyMoney>().formatMoney(fraction));
  }

  int calculateLoan()
  {
    Q_Q(KNewLoanWizard);
    MyMoneyFinancialCalculator calc;
    double val;
    int PF;
    QString result;

    // FIXME: for now, we only support interest calculation at the end of the period
    calc.setBep();
    // FIXME: for now, we only support periodic compounding
    calc.setDisc();

    PF = MyMoneySchedule::eventsPerYear(eMyMoney::Schedule::Occurrence(q->field("paymentFrequencyUnitEdit").toInt()));
    if (PF == 0)
      return 0;
    calc.setPF(PF);

    // FIXME: for now we only support compounding frequency == payment frequency
    calc.setCF(PF);

    if (q->field("loanAmountEditValid").toBool()) {
      val = q->field("loanAmountEdit").value<MyMoneyMoney>().abs().toDouble();
      if (q->field("borrowButton").toBool())
        val = -val;
      calc.setPv(val);
    }

    if (q->field("interestRateEditValid").toBool()) {
      val = q->field("interestRateEdit").value<MyMoneyMoney>().abs().toDouble();
      calc.setIr(val);
    }

    if (q->field("paymentEditValid").toBool()) {
      val = q->field("paymentEdit").value<MyMoneyMoney>().abs().toDouble();
      if (q->field("lendButton").toBool())
        val = -val;
      calc.setPmt(val);
    }

    if (q->field("finalPaymentEditValid").toBool()) {
      val = q->field("finalPaymentEditValid").value<MyMoneyMoney>().abs().toDouble();
      if (q->field("lendButton").toBool())
        val = -val;
      calc.setFv(val);
    }

    if (q->field("durationValueEdit").toInt() != 0) {
      calc.setNpp(ui->m_durationPage->term());
    }

    int fraction = m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()));
    // setup of parameters is done, now do the calculation
    try {
      //FIXME: port
      if (!q->field("loanAmountEditValid").toBool()) {
        // calculate the amount of the loan out of the other information
        val = calc.presentValue();
        ui->m_loanAmountPage->ui->m_loanAmountEdit->setText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney(fraction));
        result = i18n("KMyMoney has calculated the amount of the loan as %1.", ui->m_loanAmountPage->ui->m_loanAmountEdit->text());

      } else if (!q->field("interestRateEditValid").toBool()) {
        // calculate the interest rate out of the other information
        val = calc.interestRate();

        ui->m_interestPage->ui->m_interestRateEdit->setText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", 3));
        result = i18n("KMyMoney has calculated the interest rate to %1%.", ui->m_interestPage->ui->m_interestRateEdit->text());

      } else if (!q->field("paymentEditValid").toBool()) {
        // calculate the periodical amount of the payment out of the other information
        val = calc.payment();
        q->setField("paymentEdit", QVariant::fromValue<MyMoneyMoney>(MyMoneyMoney(val).abs()));
        // reset payment as it might have changed due to rounding
        val = q->field("paymentEdit").value<MyMoneyMoney>().abs().toDouble();
        if (q->field("lendButton").toBool())
          val = -val;
        calc.setPmt(val);

        result = i18n("KMyMoney has calculated a periodic payment of %1 to cover principal and interest.", ui->m_paymentPage->ui->m_paymentEdit->text());

        val = calc.futureValue();
        if ((q->field("borrowButton").toBool() && val < 0 && qAbs(val) >= qAbs(calc.payment()))
            || (q->field("lendButton").toBool() && val > 0 && qAbs(val) >= qAbs(calc.payment()))) {
          calc.setNpp(calc.npp() - 1);
          ui->m_durationPage->updateTermWidgets(calc.npp());
          val = calc.futureValue();
          MyMoneyMoney refVal(static_cast<double>(val));
          ui->m_finalPaymentPage->ui->m_finalPaymentEdit->setText(refVal.abs().formatMoney(fraction));
          result += QString(" ");
          result += i18n("The number of payments has been decremented and the final payment has been modified to %1.", ui->m_finalPaymentPage->ui->m_finalPaymentEdit->text());
        } else if ((q->field("borrowButton").toBool() && val < 0 && qAbs(val) < qAbs(calc.payment()))
                   || (q->field("lendButton").toBool() && val > 0 && qAbs(val) < qAbs(calc.payment()))) {
          ui->m_finalPaymentPage->ui->m_finalPaymentEdit->setText(MyMoneyMoney().formatMoney(fraction));
        } else {
          MyMoneyMoney refVal(static_cast<double>(val));
          ui->m_finalPaymentPage->ui->m_finalPaymentEdit->setText(refVal.abs().formatMoney(fraction));
          result += i18n("The final payment has been modified to %1.", ui->m_finalPaymentPage->ui->m_finalPaymentEdit->text());
        }

      } else if (q->field("durationValueEdit").toInt() == 0) {
        // calculate the number of payments out of the other information
        val = calc.numPayments();
        if (val == 0)
          throw MYMONEYEXCEPTION_CSTRING("incorrect financial calculation");

        // if the number of payments has a fractional part, then we
        // round it to the smallest integer and calculate the balloon payment
        result = i18n("KMyMoney has calculated the term of your loan as %1. ", ui->m_durationPage->updateTermWidgets(qFloor(val)));

        if (val != qFloor(val)) {
          calc.setNpp(qFloor(val));
          val = calc.futureValue();
          MyMoneyMoney refVal(static_cast<double>(val));
          ui->m_finalPaymentPage->ui->m_finalPaymentEdit->setText(refVal.abs().formatMoney(fraction));
          result += i18n("The final payment has been modified to %1.", ui->m_finalPaymentPage->ui->m_finalPaymentEdit->text());
        }

      } else {
        // calculate the future value of the loan out of the other information
        val = calc.futureValue();

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

        if ((q->field("borrowButton").toBool() && val < 0 && qAbs(val) > qAbs(calc.payment()))
            || (q->field("lendButton").toBool() && val > 0 && qAbs(val) > qAbs(calc.payment()))) {
          // case a)
          qDebug("Future Value is %f", val);
          throw MYMONEYEXCEPTION_CSTRING("incorrect financial calculation");

        } else if ((q->field("borrowButton").toBool() && val < 0 && qAbs(val) <= qAbs(calc.payment()))
                   || (q->field("lendButton").toBool() && val > 0 && qAbs(val) <= qAbs(calc.payment()))) {
          // case b)
          val = 0;
        }

        MyMoneyMoney refVal(static_cast<double>(val));
        result = i18n("KMyMoney has calculated a final payment of %1 for this loan.", refVal.abs().formatMoney(fraction));

        if (q->field("finalPaymentEditValid").toBool()) {
          if ((q->field("finalPaymentEdit").value<MyMoneyMoney>().abs() - refVal.abs()).abs().toDouble() > 1) {
            throw MYMONEYEXCEPTION_CSTRING("incorrect financial calculation");
          }
          result = i18n("KMyMoney has successfully verified your loan information.");
        }
        //FIXME: port
        ui->m_finalPaymentPage->ui->m_finalPaymentEdit->setText(refVal.abs().formatMoney(fraction));
      }

    } catch (const MyMoneyException &) {
      KMessageBox::error(0,
                         i18n("You have entered mis-matching information. Please backup to the "
                              "appropriate page and update your figures or leave one value empty "
                              "to let KMyMoney calculate it for you"),
                         i18n("Calculation error"));
      return 0;
    }

    result += i18n("\n\nAccept this or modify the loan information and recalculate.");

    KMessageBox::information(0, result, i18n("Calculation successful"));
    return 1;
  }

  /**
    * This method returns the transaction that is stored within
    * the schedule. See schedule().
    *
    * @return MyMoneyTransaction object to be used within the schedule
    */
  MyMoneyTransaction transaction() const
  {
    Q_Q(const KNewLoanWizard);
    MyMoneyTransaction t;
    bool hasInterest = !q->field("interestRateEdit").value<MyMoneyMoney>().isZero();

    MyMoneySplit sPayment, sInterest, sAmortization;
    // setup accounts. at this point, we cannot fill in the id of the
    // account that the amortization will be performed on, because we
    // create the account. So the id is yet unknown. But all others
    // must exist, otherwise we cannot create a schedule.
    sPayment.setAccountId(q->field("paymentAccountEdit").toStringList().first());
    if (!sPayment.accountId().isEmpty()) {

      //Only create the interest split if not zero
      if (hasInterest) {
        sInterest.setAccountId(q->field("interestAccountEdit").toStringList().first());
        sInterest.setValue(MyMoneyMoney::autoCalc);
        sInterest.setShares(sInterest.value());
        sInterest.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
      }

      // values
      if (q->field("borrowButton").toBool()) {
        sPayment.setValue(-q->field("paymentEdit").value<MyMoneyMoney>());
      } else {
        sPayment.setValue(q->field("paymentEdit").value<MyMoneyMoney>());
      }

      sAmortization.setValue(MyMoneyMoney::autoCalc);
      // don't forget the shares
      sPayment.setShares(sPayment.value());

      sAmortization.setShares(sAmortization.value());

      // setup the commodity
      MyMoneyAccount acc = MyMoneyFile::instance()->account(sPayment.accountId());
      t.setCommodity(acc.currencyId());

      // actions
      sPayment.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));
      sAmortization.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization));

      // payee
      QString payeeId = q->field("payeeEdit").toString();
      sPayment.setPayeeId(payeeId);
      sAmortization.setPayeeId(payeeId);

      sAmortization.setAccountId(QStringLiteral("Phony-ID"));

      // IMPORTANT: Payment split must be the first one, because
      //            the schedule view expects it this way during display
      t.addSplit(sPayment);
      t.addSplit(sAmortization);

      if (hasInterest) {
        t.addSplit(sInterest);
      }

      // copy the splits from the other costs and update the payment split
      foreach (const MyMoneySplit& it, m_transaction.splits()) {
        if (it.accountId() != QStringLiteral("Phony-ID")) {
          MyMoneySplit sp = it;
          sp.clearId();
          t.addSplit(sp);
          sPayment.setValue(sPayment.value() - sp.value());
          sPayment.setShares(sPayment.value());
          t.modifySplit(sPayment);
        }
      }
    }
    return t;
  }

  void loadAccountList()
  {
    Q_Q(KNewLoanWizard);
    AccountSet interestSet, assetSet;

    if (q->field("borrowButton").toBool()) {
      interestSet.addAccountType(eMyMoney::Account::Type::Expense);
    } else {
      interestSet.addAccountType(eMyMoney::Account::Type::Income);
    }
    if (ui->m_interestCategoryPage)
      interestSet.load(ui->m_interestCategoryPage->ui->m_interestAccountEdit);

    assetSet.addAccountType(eMyMoney::Account::Type::Checkings);
    assetSet.addAccountType(eMyMoney::Account::Type::Savings);
    assetSet.addAccountType(eMyMoney::Account::Type::Cash);
    assetSet.addAccountType(eMyMoney::Account::Type::Asset);
    assetSet.addAccountType(eMyMoney::Account::Type::Currency);
    if (ui->m_assetAccountPage)
      assetSet.load(ui->m_assetAccountPage->ui->m_assetAccountEdit);

    assetSet.addAccountType(eMyMoney::Account::Type::CreditCard);
    assetSet.addAccountType(eMyMoney::Account::Type::Liability);
    if (ui->m_schedulePage)
      assetSet.load(ui->m_schedulePage->ui->m_paymentAccountEdit);
  }

  KNewLoanWizard       *q_ptr;
  Ui::KNewLoanWizard   *ui;
  MyMoneyAccountLoan    m_account;
  MyMoneyTransaction    m_transaction;
  MyMoneySplit          m_split;
  QBitArray             m_pages;
};

#endif
