/***************************************************************************
                          knewloanwizard.cpp  -  description
                             -------------------
    begin                : Wed Oct 8 2003
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

#include "knewloanwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QList>
#include <qmath.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kpushbutton.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyutils.h"
#include "kmymoneylineedit.h"
#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneyaccountselector.h"
#include "knewaccountdlg.h"
#include "ksplittransactiondlg.h"

#include "mymoneyfinancialcalculator.h"
#include "mymoneyfile.h"

#include "kmymoney.h"

KNewLoanWizard::KNewLoanWizard(QWidget *parent) :
    KNewLoanWizardDecl(parent), m_pages(Page_Summary + 1, true)
{
  setModal(true);

  KMyMoneyMVCCombo::setSubstringSearchForChildren(m_namePage, !KMyMoneySettings::stringMatchFromStart());

  // make sure, the back button does not clear fields
  setOption(QWizard::IndependentPages, true);

  // connect(m_payeeEdit, SIGNAL(newPayee(QString)), this, SLOT(slotNewPayee(QString)));
  connect(m_namePage->m_payeeEdit, SIGNAL(createItem(QString,QString&)), this, SIGNAL(createPayee(QString,QString&)));

  connect(m_additionalFeesPage, SIGNAL(newCategory(MyMoneyAccount&)), this, SIGNAL(newCategory(MyMoneyAccount&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotReloadEditWidgets()));

  resetCalculator();

  slotReloadEditWidgets();

  // As default we assume a liability loan, with fixed interest rate,
  // with a first payment due on the 30th of this month. All payments
  // should be recorded and none have been made so far.

  //FIXME: port
  m_firstPaymentPage->m_firstDueDateEdit->loadDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 30));

  // FIXME: we currently only support interest calculation on reception
  m_pages.clearBit(Page_InterestCalculation);

  // turn off all pages that are contained here for derived classes
  m_pages.clearBit(Page_EditIntro);
  m_pages.clearBit(Page_EditSelection);
  m_pages.clearBit(Page_EffectiveDate);
  m_pages.clearBit(Page_PaymentEdit);
  m_pages.clearBit(Page_InterestEdit);
  m_pages.clearBit(Page_SummaryEdit);

  // for now, we don't have online help :-(
  setOption(QWizard::HaveHelpButton, false);

  // setup a phony transaction for additional fee processing
  m_account = MyMoneyAccount("Phony-ID", MyMoneyAccount());
  m_split.setAccountId(m_account.id());
  m_split.setValue(MyMoneyMoney());
  m_transaction.addSplit(m_split);

  KMyMoneyUtils::updateWizardButtons(this);
}

KNewLoanWizard::~KNewLoanWizard()
{
}

const MyMoneyAccountLoan KNewLoanWizard::account() const
{
  return m_account;
}

int KNewLoanWizard::nextId() const
{
  // Starting from the current page, look for the first enabled page
  // and return that value
  // If the end of the list is encountered first, then return -1.
  for (int i = currentId() + 1; i < m_pages.size() && i < pageIds().size(); ++i) {
    if (m_pages.testBit(i))
      return pageIds()[i];
  }
  return -1;
}

void KNewLoanWizard::resetCalculator()
{
  m_loanAmountPage->resetCalculator();
  m_interestPage->resetCalculator();
  m_durationPage->resetCalculator();
  m_paymentPage->resetCalculator();
  m_finalPaymentPage->resetCalculator();

  setField("additionalCost", MyMoneyMoney().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
}

void KNewLoanWizard::updateLoanAmount()
{
  QString txt;
  //FIXME: port
  if (! field("loanAmountEditValid").toBool()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = field("loanAmountEdit").value<MyMoneyMoney>().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId())));
  }
  setField("loanAmount1", txt);
  setField("loanAmount2", txt);
  setField("loanAmount3", txt);
  setField("loanAmount4", txt);
  setField("loanAmount5", txt);
}

void KNewLoanWizard::updateInterestRate()
{
  QString txt;
  //FIXME: port
  if (! field("interestRateEditValid").toBool()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = field("interestRateEdit").value<MyMoneyMoney>().formatMoney("", 3) + QString("%");
  }
  setField("interestRate1", txt);
  setField("interestRate2", txt);
  setField("interestRate3", txt);
  setField("interestRate4", txt);
  setField("interestRate5", txt);
}

void KNewLoanWizard::updateDuration()
{
  QString txt;
  //FIXME: port
  if (field("durationValueEdit").toInt() == 0) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = QString().sprintf("%d ", field("durationValueEdit").toInt())
          + field("durationUnitEdit").toString();
  }
  setField("duration1", txt);
  setField("duration2", txt);
  setField("duration3", txt);
  setField("duration4", txt);
  setField("duration5", txt);
}

void KNewLoanWizard::updatePayment()
{
  QString txt;
  //FIXME: port
  if (! field("paymentEditValid").toBool()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = field("paymentEdit").value<MyMoneyMoney>().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId())));
  }
  setField("payment1", txt);
  setField("payment2", txt);
  setField("payment3", txt);
  setField("payment4", txt);
  setField("payment5", txt);
  setField("basePayment", txt);
}

void KNewLoanWizard::updateFinalPayment()
{
  QString txt;
  //FIXME: port
  if (! field("finalPaymentEditValid").toBool()) {
    txt = QString("<") + i18n("calculate") + QString(">");
  } else {
    txt = field("finalPaymentEdit").value<MyMoneyMoney>().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId())));
  }
  setField("balloon1", txt);
  setField("balloon2", txt);
  setField("balloon3", txt);
  setField("balloon4", txt);
  setField("balloon5", txt);
}

void KNewLoanWizard::updateLoanInfo()
{
  updateLoanAmount();
  updateInterestRate();
  updateDuration();
  updatePayment();
  updateFinalPayment();
  m_additionalFeesPage->updatePeriodicPayment(m_account);

  QString txt;

  int fraction = m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()));
  setField("loanAmount6", field("loanAmountEdit").value<MyMoneyMoney>().formatMoney(fraction));
  setField("interestRate6", QString(field("interestRateEdit").value<MyMoneyMoney>().formatMoney("", 3) + QString("%")));
  txt = QString().sprintf("%d ", field("durationValueEdit").toInt())
        + field("durationUnitEdit").toString();
  setField("duration6", txt);
  setField("payment6", field("paymentEdit").value<MyMoneyMoney>().formatMoney(fraction));
  setField("balloon6", field("finalPaymentEdit").value<MyMoneyMoney>().formatMoney(fraction));
}

bool KNewLoanWizard::validateCurrentPage()
{
  bool dontLeavePage = false;
  KLocalizedString ks = ki18n(
                          "The loan wizard is unable to calculate two different values for your loan "
                          "at the same time. "
                          "Please enter a value for the %1 on this page or backup to the page where the "
                          "current value to be calculated is defined and fill in a value.");

  if (currentPage() == m_lendBorrowPage) {
    // load the appropriate categories into the list
    loadAccountList();

  } else if (currentPage() == m_interestTypePage) {
    if (field("fixedInterestButton").toBool()) {
      m_pages.setBit(Page_PreviousPayments);
      if (field("previousPaymentButton").toBool())
        m_pages.setBit(Page_RecordPayment);
      else
        m_pages.clearBit(Page_RecordPayment);
      m_pages.clearBit(Page_VariableInterestDate);

    } else {
      m_pages.clearBit(Page_PreviousPayments);
      m_pages.clearBit(Page_RecordPayment);
      m_pages.setBit(Page_VariableInterestDate);
    }

  } else if (currentPage() == m_previousPaymentsPage) {
    if (field("previousPaymentButton").toBool()) {
      m_pages.setBit(Page_RecordPayment);
    } else if (field("noPreviousPaymentButton").toBool()) {
      m_pages.clearBit(Page_RecordPayment);
    }
  } else if (currentPage() == m_loanAmountPage) {
    if (field("thisYearPaymentButton").toBool()
        && !field("loanAmountEditValid").toBool()) {
      dontLeavePage = true;
      KMessageBox::error(nullptr, i18n("You selected, that payments have already been made towards this loan. "
                                 "This requires you to enter the loan amount exactly as found on your "
                                 "last statement."), i18n("Calculation error"));
    } else
      updateLoanAmount();

  } else if (currentPage() == m_interestPage) {

    if (!field("loanAmountEditValid").toBool()
        && !field("interestRateEditValid").toBool()) {
      dontLeavePage = true;
      KMessageBox::error(nullptr, ks.subs(i18n("interest rate")).toString(), i18n("Calculation error"));
    } else
      updateInterestRate();

  } else if (currentPage() == m_durationPage) {
    if ((!field("loanAmountEditValid").toBool()
         || !field("interestRateEditValid").toBool())
        && field("durationValueEdit").toInt() == 0) {
      dontLeavePage = true;
      KMessageBox::error(nullptr, ks.subs(i18n("term")).toString(), i18n("Calculation error"));
    } else
      updateDuration();

  } else if (currentPage() == m_paymentPage) {
    if ((!field("loanAmountEditValid").toBool()
         || !field("interestRateEditValid").toBool()
         || field("durationValueEdit").toInt() == 0)
        && !field("paymentEditValid").toBool()) {
      dontLeavePage = true;
      KMessageBox::error(nullptr, ks.subs(i18n("principal and interest")).toString(), i18n("Calculation error"));
    } else
      updatePayment();

  } else if (currentPage() == m_finalPaymentPage) {
    if ((!field("loanAmountEditValid").toBool()
         || !field("interestRateEditValid").toBool()
         || field("durationValueEdit").toInt() == 0
         || !field("paymentEditValid").toBool())
        && !field("finalPaymentEditValid").toBool()) {
      // if two fields are empty and one of them is the final payment
      // we assume the final payment to be 0 instead of presenting a dialog
      setField("finalPaymentEdit", QVariant::fromValue<MyMoneyMoney>((MyMoneyMoney())));
    }
    updateFinalPayment();
    if (!calculateLoan()) {
      dontLeavePage = true;
    } else
      updateLoanInfo();
  } else if (currentPage() == m_schedulePage) {
    if (field("allPaymentsButton").toBool() || field("noPreviousPaymentButton").toBool()) {
      if (m_assetAccountPage)
        m_pages.setBit(Page_AssetAccount);
    } else {
      if (m_assetAccountPage)
        m_pages.clearBit(Page_AssetAccount);
      m_assetAccountPage->m_assetAccountEdit->slotDeselectAllAccounts();
    }
  }

  if (!dontLeavePage)
    return KNewLoanWizardDecl::validateCurrentPage();
  else
    return false;
}

int KNewLoanWizard::calculateLoan()
{
  MyMoneyFinancialCalculator calc;
  double val;
  int PF;
  QString result;

  // FIXME: for now, we only support interest calculation at the end of the period
  calc.setBep();
  // FIXME: for now, we only support periodic compounding
  calc.setDisc();

  PF = MyMoneySchedule::eventsPerYear(MyMoneySchedule::occurrenceE(field("paymentFrequencyUnitEdit").toInt()));
  if (PF == 0)
    return 0;
  calc.setPF(PF);

  // FIXME: for now we only support compounding frequency == payment frequency
  calc.setCF(PF);

  if (field("loanAmountEditValid").toBool()) {
    val = field("loanAmountEdit").value<MyMoneyMoney>().abs().toDouble();
    if (field("borrowButton").toBool())
      val = -val;
    calc.setPv(val);
  }

  if (field("interestRateEditValid").toBool()) {
    val = field("interestRateEdit").value<MyMoneyMoney>().abs().toDouble();
    calc.setIr(val);
  }

  if (field("paymentEditValid").toBool()) {
    val = field("paymentEdit").value<MyMoneyMoney>().abs().toDouble();
    if (field("lendButton").toBool())
      val = -val;
    calc.setPmt(val);
  }

  if (field("finalPaymentEditValid").toBool()) {
    val = field("finalPaymentEditValid").value<MyMoneyMoney>().abs().toDouble();
    if (field("lendButton").toBool())
      val = -val;
    calc.setFv(val);
  }

  if (field("durationValueEdit").toInt() != 0) {
    calc.setNpp(m_durationPage->term());
  }

  int fraction = m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()));
  // setup of parameters is done, now do the calculation
  try {
    //FIXME: port
    if (!field("loanAmountEditValid").toBool()) {
      // calculate the amount of the loan out of the other information
      val = calc.presentValue();
      m_loanAmountPage->m_loanAmountEdit->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney(fraction));
      result = i18n("KMyMoney has calculated the amount of the loan as %1.", m_loanAmountPage->m_loanAmountEdit->lineedit()->text());

    } else if (!field("interestRateEditValid").toBool()) {
      // calculate the interest rate out of the other information
      val = calc.interestRate();

      m_interestPage->m_interestRateEdit->loadText(MyMoneyMoney(static_cast<double>(val)).abs().formatMoney("", 3));
      result = i18n("KMyMoney has calculated the interest rate to %1%.", m_interestPage->m_interestRateEdit->lineedit()->text());

    } else if (!field("paymentEditValid").toBool()) {
      // calculate the periodical amount of the payment out of the other information
      val = calc.payment();
      setField("paymentEdit", QVariant::fromValue<MyMoneyMoney>(MyMoneyMoney(val).abs()));
      // reset payment as it might have changed due to rounding
      val = field("paymentEdit").value<MyMoneyMoney>().abs().toDouble();
      if (field("lendButton").toBool())
        val = -val;
      calc.setPmt(val);

      result = i18n("KMyMoney has calculated a periodic payment of %1 to cover principal and interest.", m_paymentPage->m_paymentEdit->lineedit()->text());

      val = calc.futureValue();
      if ((field("borrowButton").toBool() && val < 0 && qAbs(val) >= qAbs(calc.payment()))
          || (field("lendButton").toBool() && val > 0 && qAbs(val) >= qAbs(calc.payment()))) {
        calc.setNpp(calc.npp() - 1);
        m_durationPage->updateTermWidgets(calc.npp());
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_finalPaymentPage->m_finalPaymentEdit->loadText(refVal.abs().formatMoney(fraction));
        result += QString(" ");
        result += i18n("The number of payments has been decremented and the final payment has been modified to %1.", m_finalPaymentPage->m_finalPaymentEdit->lineedit()->text());
      } else if ((field("borrowButton").toBool() && val < 0 && qAbs(val) < qAbs(calc.payment()))
                 || (field("lendButton").toBool() && val > 0 && qAbs(val) < qAbs(calc.payment()))) {
        m_finalPaymentPage->m_finalPaymentEdit->loadText(MyMoneyMoney().formatMoney(fraction));
      } else {
        MyMoneyMoney refVal(static_cast<double>(val));
        m_finalPaymentPage->m_finalPaymentEdit->loadText(refVal.abs().formatMoney(fraction));
        result += i18n("The final payment has been modified to %1.", m_finalPaymentPage->m_finalPaymentEdit->lineedit()->text());
      }

    } else if (field("durationValueEdit").toInt() == 0) {
      // calculate the number of payments out of the other information
      val = calc.numPayments();
      if (val == 0)
        throw MYMONEYEXCEPTION("incorrect fincancial calculation");

      // if the number of payments has a fractional part, then we
      // round it to the smallest integer and calculate the balloon payment
      result = i18n("KMyMoney has calculated the term of your loan as %1. ", m_durationPage->updateTermWidgets(qFloor(val)));

      if (val != qFloor(val)) {
        calc.setNpp(qFloor(val));
        val = calc.futureValue();
        MyMoneyMoney refVal(static_cast<double>(val));
        m_finalPaymentPage->m_finalPaymentEdit->loadText(refVal.abs().formatMoney(fraction));
        result += i18n("The final payment has been modified to %1.", m_finalPaymentPage->m_finalPaymentEdit->lineedit()->text());
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

      if ((field("borrowButton").toBool() && val < 0 && qAbs(val) > qAbs(calc.payment()))
          || (field("lendButton").toBool() && val > 0 && qAbs(val) > qAbs(calc.payment()))) {
        // case a)
        qDebug("Future Value is %f", val);
        throw MYMONEYEXCEPTION("incorrect fincancial calculation");

      } else if ((field("borrowButton").toBool() && val < 0 && qAbs(val) <= qAbs(calc.payment()))
                 || (field("lendButton").toBool() && val > 0 && qAbs(val) <= qAbs(calc.payment()))) {
        // case b)
        val = 0;
      }

      MyMoneyMoney refVal(static_cast<double>(val));
      result = i18n("KMyMoney has calculated a final payment of %1 for this loan.", refVal.abs().formatMoney(fraction));

      if (field("finalPaymentEditValid").toBool()) {
        if ((field("finalPaymentEdit").value<MyMoneyMoney>().abs() - refVal.abs()).abs().toDouble() > 1) {
          throw MYMONEYEXCEPTION("incorrect fincancial calculation");
        }
        result = i18n("KMyMoney has successfully verified your loan information.");
      }
      //FIXME: port
      m_finalPaymentPage->m_finalPaymentEdit->loadText(refVal.abs().formatMoney(fraction));
    }

  } catch (const MyMoneyException &) {
    KMessageBox::error(nullptr,
                       i18n("You have entered mis-matching information. Please backup to the "
                            "appropriate page and update your figures or leave one value empty "
                            "to let KMyMoney calculate it for you"),
                       i18n("Calculation error"));
    return 0;
  }

  result += i18n("\n\nAccept this or modify the loan information and recalculate.");

  KMessageBox::information(nullptr, result, i18n("Calculation successful"));
  return 1;
}

void KNewLoanWizard::loadAccountList()
{
  AccountSet interestSet, assetSet;

  if (field("borrowButton").toBool()) {
    interestSet.addAccountType(MyMoneyAccount::Expense);
  } else {
    interestSet.addAccountType(MyMoneyAccount::Income);
  }
  if (m_interestCategoryPage)
    interestSet.load(m_interestCategoryPage->m_interestAccountEdit);

  assetSet.addAccountType(MyMoneyAccount::Checkings);
  assetSet.addAccountType(MyMoneyAccount::Savings);
  assetSet.addAccountType(MyMoneyAccount::Cash);
  assetSet.addAccountType(MyMoneyAccount::Asset);
  assetSet.addAccountType(MyMoneyAccount::Currency);
  if (m_assetAccountPage)
    assetSet.load(m_assetAccountPage->m_assetAccountEdit);

  assetSet.addAccountType(MyMoneyAccount::CreditCard);
  assetSet.addAccountType(MyMoneyAccount::Liability);
  if (m_schedulePage)
    assetSet.load(m_schedulePage->m_paymentAccountEdit);
}

MyMoneyTransaction KNewLoanWizard::transaction() const
{
  MyMoneyTransaction t;
  bool hasInterest = !field("interestRateEdit").value<MyMoneyMoney>().isZero();

  MyMoneySplit sPayment, sInterest, sAmortization;
  // setup accounts. at this point, we cannot fill in the id of the
  // account that the amortization will be performed on, because we
  // create the account. So the id is yet unknown.
  sPayment.setAccountId(field("paymentAccountEdit").toStringList().first());


  //Only create the interest split if not zero
  if (hasInterest) {
    sInterest.setAccountId(field("interestAccountEdit").toStringList().first());
    sInterest.setValue(MyMoneyMoney::autoCalc);
    sInterest.setShares(sInterest.value());
    sInterest.setAction(MyMoneySplit::ActionInterest);
  }

  // values
  if (field("borrowButton").toBool()) {
    sPayment.setValue(-field("paymentEdit").value<MyMoneyMoney>());
  } else {
    sPayment.setValue(field("paymentEdit").value<MyMoneyMoney>());
  }

  sAmortization.setValue(MyMoneyMoney::autoCalc);
  // don't forget the shares
  sPayment.setShares(sPayment.value());

  sAmortization.setShares(sAmortization.value());

  // setup the commodity
  MyMoneyAccount acc = MyMoneyFile::instance()->account(sPayment.accountId());
  t.setCommodity(acc.currencyId());

  // actions
  sPayment.setAction(MyMoneySplit::ActionAmortization);
  sAmortization.setAction(MyMoneySplit::ActionAmortization);

  // payee
  QString payeeId = field("payeeEdit").toString();
  sPayment.setPayeeId(payeeId);
  sAmortization.setPayeeId(payeeId);

  MyMoneyAccount account("Phony-ID", MyMoneyAccount());
  sAmortization.setAccountId(account.id());

  // IMPORTANT: Payment split must be the first one, because
  //            the schedule view expects it this way during display
  t.addSplit(sPayment);
  t.addSplit(sAmortization);

  if (hasInterest) {
    t.addSplit(sInterest);
  }

  // copy the splits from the other costs and update the payment split
  foreach (const MyMoneySplit& it, m_transaction.splits()) {
    if (it.accountId() != account.id()) {
      MyMoneySplit sp = it;
      sp.clearId();
      t.addSplit(sp);
      sPayment.setValue(sPayment.value() - sp.value());
      sPayment.setShares(sPayment.value());
      t.modifySplit(sPayment);
    }
  }
  return t;
}

MyMoneySchedule KNewLoanWizard::schedule() const
{
  MyMoneySchedule sched(field("nameEdit").toString(),
                        MyMoneySchedule::TYPE_LOANPAYMENT,
                        MyMoneySchedule::occurrenceE(field("paymentFrequencyUnitEdit").toInt()), 1,
                        MyMoneySchedule::STYPE_OTHER,
                        QDate(),
                        QDate(),
                        false,
                        false);

  MyMoneyTransaction t = transaction();
  t.setPostDate(field("nextDueDateEdit").toDate());
  sched.setTransaction(t);

  return sched;
}

void KNewLoanWizard::slotReloadEditWidgets()
{
  // load the various account widgets
  loadAccountList();

  // reload payee widget
  QString payeeId = field("payeeEdit").toString();

  //FIXME: port
  m_namePage->m_payeeEdit->loadPayees(MyMoneyFile::instance()->payeeList());

  if (!payeeId.isEmpty()) {
    setField("payeeEdit", payeeId);
  }
}

QString KNewLoanWizard::initialPaymentAccount() const
{
  if (field("dontCreatePayoutCheckBox").toBool()) {
    return QString();
  }
  return field("assetAccountEdit").toStringList().first();
}

QDate KNewLoanWizard::initialPaymentDate() const
{
  if (field("dontCreatePayoutCheckBox").toBool()) {
    return QDate();
  }
  return field("paymentDate").toDate();
}
