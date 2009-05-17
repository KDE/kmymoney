/***************************************************************************
                          keditloanwizard.cpp  -  description
                             -------------------
    begin                : Wed Nov 12 2003
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3ValueList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <../dialogs/knewloanwizard.h>
#include "keditloanwizard.h"
#include "../widgets/kmymoneylineedit.h"
#include "../widgets/kmymoneyedit.h"
#include "../widgets/kmymoneycombo.h"
#include "../widgets/kmymoneyaccountselector.h"
#include "../widgets/kmymoneydateinput.h"
#include "../mymoney/mymoneyfile.h"
#include "../kmymoneyutils.h"

KEditLoanWizard::KEditLoanWizard(const MyMoneyAccount& account, QWidget *parent ) :
  KNewLoanWizard(parent)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  setCaption(i18n("Edit loan wizard"));
  m_effectiveDateLabel->setText(QString("\n") + i18n(
            "Please enter the date from which on the following changes will be effective. "
            "The date entered must be later than the opening date of this account (%1), but must "
            "not be in the future. The default will be today.",KGlobal::locale()->formatDate(account.openingDate())));
  m_account = account;
  try {
    QString id = m_account.value("schedule");
    m_schedule = file->schedule(id);
  } catch (MyMoneyException *e) {
    delete e;
  }

  m_lastSelection = -1;
  m_editInterestRateButton->animateClick();

  loadWidgets(m_account);

  if(m_account.openingDate() > QDate::currentDate()) {
    m_effectiveDateNoteLabel->setText(QString("\n") + i18n(
            "Note: you will not be able to modify this account today, because the opening date \"%1\" is in the future. "
            "Please revisit this dialog when the time has come.",KGlobal::locale()->formatDate(m_account.openingDate())));
  } else {
    m_effectiveDateNoteLabel->hide();
  }
  // turn off all pages that are contained here for derived classes
  setAppropriate(m_newIntroPage, false);
  setAppropriate(m_newGeneralInfoPage, false);
  setAppropriate(m_lendBorrowPage, false);
  setAppropriate(m_namePage, false);
  setAppropriate(m_newCalculateLoanPage, false);
  setAppropriate(m_newPaymentsPage, false);
  removePage(m_assetAccountPage);
  m_assetAccountPage = 0;

  // turn on all pages that are contained here for derived classes
  setAppropriate(m_editIntroPage, true);
  setAppropriate(m_editSelectionPage, true);

  // setup connections
  connect(m_effectiveChangeDateEdit, SIGNAL(dateChanged(const QDate&)), this, SLOT(slotCheckPageFinished()));
  connect(m_newPaymentEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));
  connect(m_newInterestRateEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotCheckPageFinished()));

  // make sure, we show the correct start page
  showPage(m_editIntroPage);
}

KEditLoanWizard::~KEditLoanWizard()
{
}

void KEditLoanWizard::loadWidgets(const MyMoneyAccount& /* account */)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString paymentAccountId, interestAccountId;

  m_nameEdit->loadText(m_account.name());
  m_loanAmountEdit->loadText(m_account.loanAmount().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
  m_finalPaymentEdit->loadText(m_account.finalPayment().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
  m_firstDueDateEdit->setDate(m_account.openingDate());

  if(m_account.fixedInterestRate()) {
    m_fixedInterestButton->animateClick();
  } else {
    m_variableInterestButton->animateClick();
  }

  MyMoneyMoney ir;
  if(m_schedule.startDate() > QDate::currentDate()) {
    ir = m_account.interestRate(m_schedule.startDate());
  } else {
    ir = m_account.interestRate(QDate::currentDate());
  }
  m_interestRateEdit->loadText(ir.formatMoney("", 3));
  m_newInterestRateEdit->loadText(ir.formatMoney("", 3));
  m_newInterestRateEdit->setPrecision(3);
  m_interestRateLabel->setText(QString(" ") + ir.formatMoney("", 3) + QString("%"));

  m_paymentFrequencyUnitEdit->setCurrentItem(i18n(m_schedule.occurenceToString().latin1()));
  updateTermWidgets(m_account.term());

  // the base payment (amortization and interest) is determined
  // by adding all splits that are not automatically calculated.
  // If the loan is a liability, we reverse the sign at the end
  MyMoneyMoney basePayment;
  MyMoneyMoney addPayment;

  m_transaction = m_schedule.transaction();

  Q3ValueList<MyMoneySplit>::ConstIterator it_s;
  for(it_s = m_schedule.transaction().splits().begin();
      it_s != m_schedule.transaction().splits().end();
      ++it_s) {
    MyMoneyAccount acc = file->account((*it_s).accountId());
    // if it's the split that references the source/dest
    // of the money, we check if we borrow or loan money
    if(paymentAccountId.isEmpty()
    && acc.isAssetLiability() && !acc.isLoan()
    && (*it_s).value() != MyMoneyMoney::autoCalc) {
      if((*it_s).value().isNegative()) {
        m_lendButton->setChecked(false);
        m_borrowButton->setChecked(true);
      } else {
        m_lendButton->setChecked(true);
        m_borrowButton->setChecked(false);
      }
      // we keep the amount of the full payment and subtract the
      // base payment later to get information about the additional payment
      addPayment = (*it_s).value();
      paymentAccountId = (*it_s).accountId();
      MyMoneyPayee payee;
      if(!(*it_s).payeeId().isEmpty()) {
        try {
          payee = file->payee((*it_s).payeeId());
          m_payeeEdit->setSelectedItem(payee.id());
        } catch(MyMoneyException *e) {
          delete e;
          qWarning("Payee for schedule has been deleted");
        }
      }

      // remove this split with one that will be replaced
      // later and has a phony id
      m_transaction.removeSplit(*it_s);
      m_split.clearId();
      m_transaction.addSplit(m_split);
    }

    if((*it_s).action() == MyMoneySplit::ActionInterest) {
      interestAccountId = (*it_s).accountId();
    }

    if((*it_s).value() != MyMoneyMoney::autoCalc) {
      basePayment += (*it_s).value();
    } else {
      // remove the splits which should not show up
      // for additional fees
      m_transaction.removeSplit(*it_s);
    }

  }
  if(m_borrowButton->isChecked()) {
    basePayment = -basePayment;
    addPayment = -addPayment;
  }
  // now make adjustment to get the amount of the additional fees
  addPayment -= basePayment;

  // load account selection widgets now that we know if
  // we borrow or lend money
  loadAccountList();

  int fraction = m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()));
  m_paymentEdit->loadText(basePayment.formatMoney(fraction));
  m_newPaymentEdit->loadText(basePayment.formatMoney(fraction));
  m_paymentLabel->setText(QString(" ") + basePayment.formatMoney(fraction));

  m_additionalCost->setText(addPayment.formatMoney(fraction));
  m_interestAccountEdit->setSelected(interestAccountId);
  m_paymentAccountEdit->setSelected(paymentAccountId);
  m_nextDueDateEdit->setDate(m_schedule.nextPayment());

  int changeFrequencyUnit;
  int amt = m_account.interestChangeFrequency(&changeFrequencyUnit);
  if(amt != -1) {
    m_interestFrequencyAmountEdit->setValue(amt);
    m_interestFrequencyUnitEdit->setCurrentItem(changeFrequencyUnit);
  }

  // keep track, if the loan should be fully repayed
  m_fullyRepayLoan = m_account.finalPayment() < basePayment;

  updateLoanInfo();
  updateSummary();
}

void KEditLoanWizard::next()
{
  bool dontLeavePage = false;
  QAbstractButton* button = m_selectionButtonGroup->find(m_lastSelection);

  if(currentPage() == m_editSelectionPage) {

    if(button != 0
    && m_lastSelection != m_selectionButtonGroup->id(m_selectionButtonGroup->selected())) {
      QString errMsg = i18n(
            "Your previous selection was \"%1\". If you select another option, "
            "KMyMoney will dismiss the changes you have just entered. "
            "Do you wish to proceed?").arg(button->text());

      if(KMessageBox::questionYesNo(this, errMsg) == KMessageBox::No) {
        dontLeavePage = true;
      } else {
        loadWidgets(m_account);
      }
    }

    if(!dontLeavePage) {
      button = m_selectionButtonGroup->selected();

      // turn off all pages except the summary at the end
      // and the one's we need for the selected option
      // and load the widgets with the current values

      // general info
      setAppropriate(m_namePage, false);
      setAppropriate(m_interestTypePage, false);
      setAppropriate(m_previousPaymentsPage, false);
      setAppropriate(m_recordPaymentPage, false);
      setAppropriate(m_variableInterestDatePage, false);
      setAppropriate(m_firstPaymentPage, false);
      // loan calculation
      setAppropriate(m_paymentEditPage, false);
      setAppropriate(m_interestEditPage, false);
      setAppropriate(m_paymentFrequencyPage, false);
      setAppropriate(m_interestCalculationPage, false);
      setAppropriate(m_loanAmountPage, false);
      setAppropriate(m_interestPage, false);
      setAppropriate(m_durationPage, false);
      setAppropriate(m_paymentPage, false);
      setAppropriate(m_finalPaymentPage, false);
      setAppropriate(m_calculationOverviewPage, false);
      // payment
      setAppropriate(m_interestCategoryPage, false);
      setAppropriate(m_additionalFeesPage, false);
      setAppropriate(m_schedulePage, false);
      setAppropriate(m_summaryPage, true);

      setAppropriate(m_effectiveDatePage, true);
      if(indexOf(m_summaryPage) != -1) {
        removePage(m_summaryPage);
        setFinishEnabled(m_summaryEditPage, true);
      }

      if(button == m_editInterestRateButton) {
        setAppropriate(m_interestTypePage, true);
        setAppropriate(m_variableInterestDatePage, true);
        setAppropriate(m_paymentEditPage, true);
        setAppropriate(m_interestEditPage, true);
        setAppropriate(m_summaryEditPage, true);

      } else if(button == m_editOtherCostButton) {
        setAppropriate(m_additionalFeesPage, true);
        setAppropriate(m_summaryEditPage, true);

      } else if(button == m_editOtherInfoButton) {
        setAppropriate(m_namePage, true);
        setAppropriate(m_interestCalculationPage, true);
        setAppropriate(m_interestPage, true);
        setAppropriate(m_durationPage, true);
        setAppropriate(m_paymentPage, true);
        setAppropriate(m_finalPaymentPage, true);
        setAppropriate(m_calculationOverviewPage, true);
        setAppropriate(m_interestCategoryPage, true);
        setAppropriate(m_additionalFeesPage, true);
        setAppropriate(m_schedulePage, true);
        setAppropriate(m_summaryEditPage, false);
        addPage(m_summaryPage, i18n("Summary"));
        setAppropriate(m_summaryPage, true);
        setFinishEnabled(m_summaryEditPage, false);
        setFinishEnabled(m_summaryPage, true);

      } else {
        qFatal("%s,%d: This should never happen", __FILE__, __LINE__);
      }

      m_lastSelection = m_selectionButtonGroup->id(m_selectionButtonGroup->selected());
    } // if(!dontLeavePage)

  } else if(currentPage() == m_additionalFeesPage) {
    button = m_selectionButtonGroup->selected();
    if(button == m_editOtherCostButton) {
      updateLoanInfo();
      updateEditSummary();
    }

  } else if(currentPage() == m_interestEditPage) {
    // copy the necessary data to the widgets used for calculation
    m_interestRateEdit->setValue(m_newInterestRateEdit->value());
    m_paymentEdit->setValue(m_newPaymentEdit->value());

    // if interest rate and payment amount is given, then force
    // the term to be recalculated. The final payment is adjusted to
    // 0 if the loan was ment to be fully repayed
    updateTermWidgets(m_account.term());
    if(!m_interestRateEdit->lineedit()->text().isEmpty()
    && !m_paymentEdit->lineedit()->text().isEmpty()) {
      // if there's an amortization going on, we can evaluate
      // the new term. If the amortization is 0 (interest only
      // payments) then we keep the term as entered by the user.
      if(m_loanAmountEdit->value() != m_finalPaymentEdit->value()) {
        m_durationValueEdit->setValue(0);
      }
      if(m_fullyRepayLoan)
        m_finalPaymentEdit->loadText(MyMoneyMoney(0).formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
    }

/*
    // we need to calculate the balance at the time of the change
    // in order to accurately recalculate the term. A special
    // situation arises, when we keep track of all payments and
    // the full loan is not yet paid out. In this case, we take the
    // the loan amount minus all amortization payments as the current
    // balance.
    // FIXME: This needs some more thoughts. We leave it out for
    //        now and always calculate with the full loan amount.
    MyMoneyMoney balance = m_account.openingBalance();

    QValueList<MyMoneyTransaction> list;
    QValueList<MyMoneyTransaction>::ConstIterator it;
    MyMoneySplit split;
    MyMoneyTransactionFilter filter(m_account.id());

    filter.setDateFilter(QDate(), m_effectiveChangeDateEdit->date().addDays(-1));
    list = MyMoneyFile::instance()->transactionList(filter);

    for(it = list.begin(); it != list.end(); ++it) {
      try {
        split = (*it).splitByAccount(m_account.id());
        balance += split.value();

      } catch(MyMoneyException *e) {
        // account is not referenced within this transaction
        delete e;
      }
    }
    m_loanAmountEdit->setText(balance.formatMoney());
*/

    // now re-calculate the figures
    dontLeavePage = !calculateLoan();

    // reset the original loan amount to the widget
    m_loanAmountEdit->setValue(m_account.loanAmount());

    if(!dontLeavePage) {
      updateLoanInfo();
      updateEditSummary();
    }
  }

  if(!dontLeavePage)
    KNewLoanWizard::next();

  // These might have been set by KNewLoanWizard::next()
  setAppropriate(m_previousPaymentsPage, false);
  setAppropriate(m_recordPaymentPage, false);
  // we never need to show this page
  if(currentPage() == m_previousPaymentsPage)
    KNewLoanWizard::next();
}

void KEditLoanWizard::slotCheckPageFinished(void)
{
  KNewLoanWizard::slotCheckPageFinished();

  // if we're on one of the specific edit pages, the next button
  // is enabled. If the values in the edit widgets are not
  // appropriate, we just have to disable it.

  if(currentPage() == m_effectiveDatePage) {
    if(m_effectiveChangeDateEdit->date() < m_account.openingDate()
    || m_effectiveChangeDateEdit->date() > QDate::currentDate())
      nextButton()->setEnabled(false);

  } else if(currentPage() == m_interestEditPage) {
    if(!m_newPaymentEdit->isValid()
    && !m_newInterestRateEdit->isValid())
      nextButton()->setEnabled(false);
  }
}

void KEditLoanWizard::updateEditSummary(void)
{
  updateSummary();
  m_payment7->setText(m_summaryPeriodicPayment->text());
  m_additionalFees7->setText(m_summaryAdditionalFees->text());
  m_totalPayment7->setText(m_summaryTotalPeriodicPayment->text());
  m_interestRate7->setText(m_summaryInterestRate->text());
  m_startDateChanges->setText(KGlobal::locale()->formatDate(m_effectiveChangeDateEdit->date()));

  // calculate the number of affected transactions
  MyMoneyTransactionFilter filter(m_account.id());
  filter.setDateFilter(m_effectiveChangeDateEdit->date(), QDate());

  int count = 0;
  Q3ValueList<MyMoneyTransaction> list;
  Q3ValueList<MyMoneyTransaction>::ConstIterator it;
  list = MyMoneyFile::instance()->transactionList(filter);

  for(it = list.begin(); it != list.end(); ++it) {
    Q3ValueList<MyMoneySplit>::ConstIterator it_s;
    int match = 0;
    for(it_s = (*it).splits().begin(); it_s != (*it).splits().end(); ++it_s) {
      // we only count those transactions that have an interest
      // and amortization part
      if((*it_s).action() == MyMoneySplit::ActionInterest)
        match |= 0x01;
      if((*it_s).action() == MyMoneySplit::ActionAmortization)
        match |= 0x02;
    }
    if(match == 0x03)
      count++;
  }

  m_affectedPayments->setText(QString().sprintf("%d", count));
}

const MyMoneySchedule KEditLoanWizard::schedule(void) const
{
  MyMoneySchedule sched = m_schedule;
  sched.setTransaction(transaction());
  sched.setOccurence(MyMoneySchedule::stringToOccurence(m_paymentFrequencyUnitEdit->currentText()));
  if(m_nextDueDateEdit->date() < m_schedule.startDate())
    sched.setStartDate(m_nextDueDateEdit->date());

  return sched;
}

const MyMoneyAccount KEditLoanWizard::account(void) const
{
  MyMoneyAccountLoan acc(m_account);

  if(m_interestOnReceptionButton->isChecked())
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentReceived);
  else
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentDue);

  acc.setFixedInterestRate(m_fixedInterestButton->isChecked());
  acc.setFinalPayment(MyMoneyMoney(m_finalPaymentEdit->text()));
  acc.setTerm(term());
  acc.setPeriodicPayment(m_paymentEdit->value());
  acc.setInterestRate(m_effectiveChangeDateEdit->date(), m_interestRateEdit->value());

  acc.setPayee(m_payeeEdit->selectedItem());

  if(m_variableInterestButton->isChecked()) {
    acc.setNextInterestChange(m_interestChangeDateEdit->date());
    acc.setInterestChangeFrequency(m_interestFrequencyAmountEdit->value(),
                                   m_interestFrequencyUnitEdit->currentItem());
  }

  return acc;
}

const MyMoneyTransaction KEditLoanWizard::transaction() const
{
  MyMoneyTransaction t = KNewLoanWizard::transaction();
  MyMoneySplit s = t.splitByAccount(QString("Phony-ID"));

  s.setAccountId(m_account.id());
  t.modifySplit(s);

  return t;
}

#include "keditloanwizard.moc"
