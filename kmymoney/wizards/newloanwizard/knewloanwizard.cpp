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
#include "knewloanwizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneydateinput.h"
#include "kmymoneyedit.h"
#include "kmymoneyaccountselector.h"

#include "mymoneypayee.h"

KNewLoanWizard::KNewLoanWizard(QWidget *parent) :
    QWizard(parent),
    d_ptr(new KNewLoanWizardPrivate(this))
{
  Q_D(KNewLoanWizard);
  d->init();
}

KNewLoanWizard::KNewLoanWizard(KNewLoanWizardPrivate &dd, QWidget *parent) :
  QWizard(parent),
  d_ptr(&dd)
{
  Q_D(KNewLoanWizard);
  d->init();
}

KNewLoanWizard::~KNewLoanWizard()
{
}

const MyMoneyAccountLoan KNewLoanWizard::account() const
{
  Q_D(const KNewLoanWizard);
  return d->m_account;
}

int KNewLoanWizard::nextId() const
{
  Q_D(const KNewLoanWizard);
  // Starting from the current page, look for the first enabled page
  // and return that value
  // If the end of the list is encountered first, then return -1.
  for (int i = currentId() + 1; i < d->m_pages.size() && i < pageIds().size(); ++i) {
    if (d->m_pages.testBit(i))
      return pageIds()[i];
  }
  return -1;
}

bool KNewLoanWizard::validateCurrentPage()
{
  Q_D(KNewLoanWizard);
  auto dontLeavePage = false;
  KLocalizedString ks = ki18n(
                          "The loan wizard is unable to calculate two different values for your loan "
                          "at the same time. "
                          "Please enter a value for the %1 on this page or backup to the page where the "
                          "current value to be calculated is defined and fill in a value.");

  if (currentPage() == d->ui->m_lendBorrowPage) {
    // load the appropriate categories into the list
    d->loadAccountList();

  } else if (currentPage() == d->ui->m_interestTypePage) {
    if (field("fixedInterestButton").toBool()) {
      d->m_pages.setBit(Page_PreviousPayments);
      if (field("previousPaymentButton").toBool())
        d->m_pages.setBit(Page_RecordPayment);
      else
        d->m_pages.clearBit(Page_RecordPayment);
      d->m_pages.clearBit(Page_VariableInterestDate);

    } else {
      d->m_pages.clearBit(Page_PreviousPayments);
      d->m_pages.clearBit(Page_RecordPayment);
      d->m_pages.setBit(Page_VariableInterestDate);
    }

  } else if (currentPage() == d->ui->m_previousPaymentsPage) {
    if (field("previousPaymentButton").toBool()) {
      d->m_pages.setBit(Page_RecordPayment);
    } else if (field("noPreviousPaymentButton").toBool()) {
      d->m_pages.clearBit(Page_RecordPayment);
    }
  } else if (currentPage() == d->ui->m_loanAmountPage) {
    if (field("thisYearPaymentButton").toBool()
        && !field("loanAmountEditValid").toBool()) {
      dontLeavePage = true;
      KMessageBox::error(0, i18n("You selected, that payments have already been made towards this loan. "
                                 "This requires you to enter the loan amount exactly as found on your "
                                 "last statement."), i18n("Calculation error"));
    } else
      d->updateLoanAmount();

  } else if (currentPage() == d->ui->m_interestPage) {

    if (!field("loanAmountEditValid").toBool()
        && !field("interestRateEditValid").toBool()) {
      dontLeavePage = true;
      KMessageBox::error(0, ks.subs(i18n("interest rate")).toString(), i18n("Calculation error"));
    } else
      d->updateInterestRate();

  } else if (currentPage() == d->ui->m_durationPage) {
    if ((!field("loanAmountEditValid").toBool()
         || !field("interestRateEditValid").toBool())
        && field("durationValueEdit").toInt() == 0) {
      dontLeavePage = true;
      KMessageBox::error(0, ks.subs(i18n("term")).toString(), i18n("Calculation error"));
    } else
      d->updateDuration();

  } else if (currentPage() == d->ui->m_paymentPage) {
    if ((!field("loanAmountEditValid").toBool()
         || !field("interestRateEditValid").toBool()
         || field("durationValueEdit").toInt() == 0)
        && !field("paymentEditValid").toBool()) {
      dontLeavePage = true;
      KMessageBox::error(0, ks.subs(i18n("principal and interest")).toString(), i18n("Calculation error"));
    } else
      d->updatePayment();

  } else if (currentPage() == d->ui->m_finalPaymentPage) {
    if ((!field("loanAmountEditValid").toBool()
         || !field("interestRateEditValid").toBool()
         || field("durationValueEdit").toInt() == 0
         || !field("paymentEditValid").toBool())
        && !field("finalPaymentEditValid").toBool()) {
      // if two fields are empty and one of them is the final payment
      // we assume the final payment to be 0 instead of presenting a dialog
      setField("finalPaymentEdit", QVariant::fromValue<MyMoneyMoney>((MyMoneyMoney())));
    }
    d->updateFinalPayment();
    if (!d->calculateLoan()) {
      dontLeavePage = true;
    } else
      d->updateLoanInfo();
  } else if (currentPage() == d->ui->m_schedulePage) {
    if (field("allPaymentsButton").toBool() || field("noPreviousPaymentButton").toBool()) {
      if (d->ui->m_assetAccountPage)
        d->m_pages.setBit(Page_AssetAccount);
    } else {
      if (d->ui->m_assetAccountPage)
        d->m_pages.clearBit(Page_AssetAccount);
      d->ui->m_assetAccountPage->m_assetAccountEdit->slotDeselectAllAccounts();
    }
  }

  if (!dontLeavePage)
    return true;
  else
    return false;
}

MyMoneySchedule KNewLoanWizard::schedule() const
{
  MyMoneySchedule sched(field("nameEdit").toString(),
                        eMyMoney::Schedule::Type::LoanPayment,
                        eMyMoney::Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt()), 1,
                        eMyMoney::Schedule::PaymentType::Other,
                        QDate(),
                        QDate(),
                        false,
                        false);

  Q_D(const KNewLoanWizard);
  MyMoneyTransaction t = d->transaction();
  t.setPostDate(field("nextDueDateEdit").toDate());
  sched.setTransaction(t);

  return sched;
}

void KNewLoanWizard::slotReloadEditWidgets()
{
  Q_D(KNewLoanWizard);
  // load the various account widgets
  d->loadAccountList();

  // reload payee widget
  auto payeeId = field("payeeEdit").toString();

  //FIXME: port
  d->ui->m_namePage->m_payeeEdit->loadPayees(MyMoneyFile::instance()->payeeList());

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
