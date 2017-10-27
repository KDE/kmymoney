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

#include "keditloanwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewloanwizard.h"
#include "kmymoneylineedit.h"
#include "kmymoneyedit.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"

KEditLoanWizard::KEditLoanWizard(const MyMoneyAccount& account, QWidget *parent) :
    KNewLoanWizard(parent)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  setWindowTitle(i18n("Edit loan wizard"));

  m_account = account;
  try {
    QString id = m_account.value("schedule");
    m_schedule = file->schedule(id);
  } catch (const MyMoneyException &) {
  }
  m_lastSelection = -1;

  loadWidgets(m_account);

  if (m_account.openingDate() > QDate::currentDate()) {
    //FIXME: port
    m_effectiveDatePage->m_effectiveDateNoteLabel->setText(QString("\n") + i18n(
          "Note: you will not be able to modify this account today, because the opening date \"%1\" is in the future. "
          "Please revisit this dialog when the time has come.", QLocale().toString(m_account.openingDate())));
  } else {
    m_effectiveDatePage->m_effectiveDateNoteLabel->hide();
  }
  // turn off all pages that are contained here for derived classes
  m_pages.clearBit(Page_Intro);
  m_pages.clearBit(Page_NewGeneralInfo);
  m_pages.clearBit(Page_LendBorrow);
  m_pages.clearBit(Page_Name);
  m_pages.clearBit(Page_NewCalculateLoan);
  m_pages.clearBit(Page_NewPayments);
  removePage(Page_AssetAccount);
  m_assetAccountPage = 0;

  // turn on all pages that are contained here for derived classes
  m_pages.setBit(Page_EditIntro);
  m_pages.setBit(Page_EditSelection);

  // make sure, we show the correct start page
  setStartId(Page_EditIntro);
}

KEditLoanWizard::~KEditLoanWizard()
{
}

void KEditLoanWizard::loadWidgets(const MyMoneyAccount& /* account */)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  QString paymentAccountId, interestAccountId;

  //FIXME: port
  m_namePage->m_nameEdit->loadText(m_account.name());
  m_loanAmountPage->m_loanAmountEdit->loadText(m_account.loanAmount().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
  m_finalPaymentPage->m_finalPaymentEdit->loadText(m_account.finalPayment().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
  setField("firstDueDateEdit", m_account.openingDate());

  //FIXME: port
  if (m_account.fixedInterestRate()) {
    m_interestTypePage->m_fixedInterestButton->click();
  } else {
    m_interestTypePage->m_variableInterestButton->click();
  }

  QString institutionName = file->institution(m_account.institutionId()).name();
  m_loanAttributesPage->setInstitution(institutionName);

  MyMoneyMoney ir;
  if (m_schedule.startDate() > QDate::currentDate()) {
    ir = m_account.interestRate(m_schedule.startDate());
  } else {
    ir = m_account.interestRate(QDate::currentDate());
  }
  //FIXME: port
  m_interestPage->m_interestRateEdit->loadText(ir.formatMoney("", 3));
  m_interestPage->m_interestRateEdit->setPrecision(3);
  m_interestEditPage->m_newInterestRateEdit->loadText(ir.formatMoney("", 3));
  m_interestEditPage->m_newInterestRateEdit->setPrecision(3);
  m_interestEditPage->m_interestRateLabel->setText(QString(" ") + ir.formatMoney("", 3) + QString("%"));

  m_paymentFrequencyPage->m_paymentFrequencyUnitEdit->setCurrentIndex(m_paymentFrequencyPage->m_paymentFrequencyUnitEdit->findData(QVariant((int)m_schedule.occurrencePeriod()), Qt::UserRole, Qt::MatchExactly));
  m_durationPage->updateTermWidgets(m_account.term());

  // the base payment (amortization and interest) is determined
  // by adding all splits that are not automatically calculated.
  // If the loan is a liability, we reverse the sign at the end
  MyMoneyMoney basePayment;
  MyMoneyMoney addPayment;

  m_transaction = m_schedule.transaction();

  foreach (const MyMoneySplit& it_s, m_schedule.transaction().splits()) {
    MyMoneyAccount acc = file->account(it_s.accountId());
    // if it's the split that references the source/dest
    // of the money, we check if we borrow or loan money
    if (paymentAccountId.isEmpty()
        && acc.isAssetLiability() && !acc.isLoan()
        && it_s.value() != MyMoneyMoney::autoCalc) {
      if (it_s.value().isNegative()) {
        setField("lendButton", false);
        setField("borrowButton", true);
      } else {
        setField("lendButton", true);
        setField("borrowButton", false);
      }
      // we keep the amount of the full payment and subtract the
      // base payment later to get information about the additional payment
      addPayment = it_s.value();
      paymentAccountId = it_s.accountId();
      MyMoneyPayee payee;
      if (!it_s.payeeId().isEmpty()) {
        try {
          payee = file->payee(it_s.payeeId());
          setField("payeeEdit", payee.id());
        } catch (const MyMoneyException &) {
          qWarning("Payee for schedule has been deleted");
        }
      }

      // remove this split with one that will be replaced
      // later and has a phony id
      m_transaction.removeSplit(it_s);
      m_split.clearId();
      m_transaction.addSplit(m_split);
    }

    if (it_s.action() == MyMoneySplit::ActionInterest) {
      interestAccountId = it_s.accountId();
    }

    if (it_s.value() != MyMoneyMoney::autoCalc) {
      basePayment += it_s.value();
    } else {
      // remove the splits which should not show up
      // for additional fees
      m_transaction.removeSplit(it_s);
    }

  }
  if (field("borrowButton").toBool()) {
    basePayment = -basePayment;
    addPayment = -addPayment;
  }
  // now make adjustment to get the amount of the additional fees
  addPayment -= basePayment;

  // load account selection widgets now that we know if
  // we borrow or lend money
  loadAccountList();

  int fraction = m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()));
  //FIXME: port
  m_paymentPage->m_paymentEdit->loadText(basePayment.formatMoney(fraction));
  m_paymentEditPage->m_newPaymentEdit->loadText(basePayment.formatMoney(fraction));
  m_paymentEditPage->m_paymentLabel->setText(QString(" ") + basePayment.formatMoney(fraction));

  setField("additionalCost", addPayment.formatMoney(fraction));
  m_interestCategoryPage->m_interestAccountEdit->setSelected(interestAccountId);
  m_schedulePage->m_paymentAccountEdit->setSelected(paymentAccountId);
  setField("nextDueDateEdit", m_schedule.nextPayment());

  int changeFrequencyUnit;
  int amt = m_account.interestChangeFrequency(&changeFrequencyUnit);
  if (amt != -1) {
    setField("interestFrequencyAmountEdit", amt);
    setField("interestFrequencyUnitEdit", changeFrequencyUnit);
  }

  // keep track, if the loan should be fully repayed
  m_fullyRepayLoan = m_account.finalPayment() < basePayment;

  updateLoanInfo();
}

bool KEditLoanWizard::validateCurrentPage()
{
  bool dontLeavePage = false;
  //FIXME: port m_lastSelection
  QAbstractButton* button = m_editSelectionPage->m_selectionButtonGroup->button(m_lastSelection);

  if (currentPage() == m_editSelectionPage) {

    if (button != 0
        && m_lastSelection != m_editSelectionPage->m_selectionButtonGroup->checkedId()) {

      QString errMsg = i18n(
                         "Your previous selection was \"%1\". If you select another option, "
                         "KMyMoney will dismiss the changes you have just entered. "
                         "Do you wish to proceed?", button->text());

      if (KMessageBox::questionYesNo(this, errMsg) == KMessageBox::No) {
        dontLeavePage = true;
      } else {
        loadWidgets(m_account);
      }
    }

    if (!dontLeavePage) {
      // turn off all pages except the summary at the end
      // and the one's we need for the selected option
      // and load the widgets with the current values

      // general info
      m_pages.clearBit(Page_Name);
      m_pages.clearBit(Page_InterestType);
      m_pages.clearBit(Page_PreviousPayments);
      m_pages.clearBit(Page_RecordPayment);
      m_pages.clearBit(Page_VariableInterestDate);
      m_pages.clearBit(Page_FirstPayment);

      // loan calculation
      m_pages.clearBit(Page_PaymentEdit);
      m_pages.clearBit(Page_InterestEdit);
      m_pages.clearBit(Page_PaymentFrequency);
      m_pages.clearBit(Page_InterestCalculation);
      m_pages.clearBit(Page_LoanAmount);
      m_pages.clearBit(Page_Interest);
      m_pages.clearBit(Page_Duration);
      m_pages.clearBit(Page_Payment);
      m_pages.clearBit(Page_FinalPayment);
      m_pages.clearBit(Page_CalculationOverview);

      // payment
      m_pages.clearBit(Page_InterestCategory);
      m_pages.clearBit(Page_AdditionalFees);
      m_pages.clearBit(Page_Schedule);
      m_pages.setBit(Page_Summary);

      // Attributes
      m_pages.clearBit(Page_LoanAttributes);

      m_pages.setBit(Page_EffectiveDate);

      if (page(Page_Summary) != 0) {
        removePage(Page_Summary);
      }

      if (field("editInterestRateButton").toBool()) {
        m_pages.setBit(Page_PaymentFrequency);
        m_pages.setBit(Page_InterestType);
        m_pages.setBit(Page_VariableInterestDate);
        m_pages.setBit(Page_PaymentEdit);
        m_pages.setBit(Page_InterestEdit);
        m_pages.setBit(Page_InterestCategory);
        m_pages.setBit(Page_Schedule);
        m_pages.setBit(Page_SummaryEdit);

      } else if (field("editOtherCostButton").toBool()) {
        m_pages.setBit(Page_PaymentFrequency);
        m_pages.setBit(Page_AdditionalFees);
        m_pages.setBit(Page_InterestCategory);
        m_pages.setBit(Page_Schedule);
        m_pages.setBit(Page_SummaryEdit);

      } else if (field("editOtherInfoButton").toBool()) {
        m_pages.setBit(Page_Name);
        m_pages.setBit(Page_InterestCalculation);
        m_pages.setBit(Page_Interest);
        m_pages.setBit(Page_Duration);
        m_pages.setBit(Page_Payment);
        m_pages.setBit(Page_FinalPayment);
        m_pages.setBit(Page_CalculationOverview);
        m_pages.setBit(Page_InterestCategory);
        m_pages.setBit(Page_AdditionalFees);
        m_pages.setBit(Page_Schedule);
        m_pages.clearBit(Page_SummaryEdit);
        setPage(Page_Summary, m_summaryPage);
        m_pages.setBit(Page_Summary);

      } else if (field("editAttributesButton").toBool()) {
        m_pages.setBit(Page_LoanAttributes);
        m_pages.clearBit(Page_EffectiveDate);
      } else {
        qWarning("%s,%d: This should never happen", __FILE__, __LINE__);
      }

      m_lastSelection = m_editSelectionPage->m_selectionButtonGroup->checkedId();
    } // if(!dontLeavePage)

  } else if (currentPage() == m_additionalFeesPage) {
    if (field("editOtherCostButton").toBool()) {
      updateLoanInfo();
      updateEditSummary();
    }

  } else if (currentPage() == m_interestEditPage) {
    // copy the necessary data to the widgets used for calculation
    //FIXME: port to fields
    m_interestPage->m_interestRateEdit->setValue(field("newInterestRateEdit").value<MyMoneyMoney>());
    m_paymentPage->m_paymentEdit->setValue(field("newPaymentEdit").value<MyMoneyMoney>());

    // if interest rate and payment amount is given, then force
    // the term to be recalculated. The final payment is adjusted to
    // 0 if the loan was ment to be fully repayed
    m_durationPage->updateTermWidgets(m_account.term());
    if (field("interestRateEditValid").toBool()
        && field("paymentEditValid").toBool()) {
      // if there's an amortization going on, we can evaluate
      // the new term. If the amortization is 0 (interest only
      // payments) then we keep the term as entered by the user.
      if (field("loanAmountEdit").value<MyMoneyMoney>() != field("finalPaymentEdit").value<MyMoneyMoney>()) {
        setField("durationValueEdit", 0);
      }
      if (m_fullyRepayLoan)
        m_finalPaymentPage->m_finalPaymentEdit->loadText(MyMoneyMoney().formatMoney(m_account.fraction(MyMoneyFile::instance()->security(m_account.currencyId()))));
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

        QList<MyMoneyTransaction> list;
        QList<MyMoneyTransaction>::ConstIterator it;
        MyMoneySplit split;
        MyMoneyTransactionFilter filter(m_account.id());

        filter.setDateFilter(QDate(), m_effectiveChangeDateEdit->date().addDays(-1));
        list = MyMoneyFile::instance()->transactionList(filter);

        for(it = list.begin(); it != list.end(); ++it) {
          try {
            split = (*it).splitByAccount(m_account.id());
            balance += split.value();

          } catch(const MyMoneyException &e) {
            // account is not referenced within this transaction
          }
        }
        m_loanAmountEdit->setText(balance.formatMoney());
    */
    // now re-calculate the figures
    dontLeavePage = !calculateLoan();

    // reset the original loan amount to the widget
    //FIXME: port to fields
    m_loanAmountPage->m_loanAmountEdit->setValue(m_account.loanAmount());

    if (!dontLeavePage) {
      updateLoanInfo();
      updateEditSummary();
    }
  }

  if (!dontLeavePage)
    dontLeavePage = ! KNewLoanWizard::validateCurrentPage();

  // These might have been set by KNewLoanWizard
  m_pages.clearBit(Page_PreviousPayments);
  m_pages.clearBit(Page_RecordPayment);

  if (dontLeavePage)
    return false;

  // we never need to show this page
  if (currentPage() == m_previousPaymentsPage)
    dontLeavePage = KNewLoanWizard::validateCurrentPage();

  return ! dontLeavePage;
}

void KEditLoanWizard::updateEditSummary()
{
  // calculate the number of affected transactions
  MyMoneyTransactionFilter filter(m_account.id());
  filter.setDateFilter(field("effectiveChangeDateEdit").toDate(), QDate());

  int count = 0;
  QList<MyMoneyTransaction> list;
  list = MyMoneyFile::instance()->transactionList(filter);

  foreach (const MyMoneyTransaction& it, list) {
    int match = 0;
    foreach (const MyMoneySplit& it_s, it.splits()) {
      // we only count those transactions that have an interest
      // and amortization part
      if (it_s.action() == MyMoneySplit::ActionInterest)
        match |= 0x01;
      if (it_s.action() == MyMoneySplit::ActionAmortization)
        match |= 0x02;
    }
    if (match == 0x03)
      ++count;
  }

  setField("affectedPayments", QString().sprintf("%d", count));
}

const MyMoneySchedule KEditLoanWizard::schedule() const
{
  MyMoneySchedule sched = m_schedule;
  sched.setTransaction(transaction());
  sched.setOccurrence(eMyMoney::Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt()));
  if (field("nextDueDateEdit").toDate() < m_schedule.startDate())
    sched.setStartDate(field("nextDueDateEdit").toDate());

  return sched;
}

const MyMoneyAccount KEditLoanWizard::account() const
{
  MyMoneyAccountLoan acc(m_account);

  if (field("interestOnReceptionButton").toBool())
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentReceived);
  else
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentDue);

  MyMoneyFile *file = MyMoneyFile::instance();

  QString institution = m_loanAttributesPage->m_qcomboboxInstitutions->currentText();
  if (institution != i18n("(No Institution)")) {
    QList<MyMoneyInstitution> list;
    file->institutionList(list);
    Q_FOREACH(const MyMoneyInstitution& testInstitution, list) {
      if (testInstitution.name() == institution) {
        acc.setInstitutionId(testInstitution.id());
        break;
      }
    }
  } else {
    acc.setInstitutionId(QString());
  }

  acc.setFixedInterestRate(field("fixedInterestButton").toBool());
  acc.setFinalPayment(field("finalPaymentEdit").value<MyMoneyMoney>());
  acc.setTerm(m_durationPage->term());
  acc.setPeriodicPayment(field("paymentEdit").value<MyMoneyMoney>());
  acc.setInterestRate(field("effectiveChangeDateEdit").toDate(), field("interestRateEdit").value<MyMoneyMoney>());

  acc.setPayee(field("payeeEdit").toString());

  if (field("variableInterestButton").toBool()) {
    acc.setNextInterestChange(field("interestChangeDateEdit").toDate());
    acc.setInterestChangeFrequency(field("interestFrequencyAmountEdit").toInt(),
                                   field("interestFrequencyUnitEdit").toInt());
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
