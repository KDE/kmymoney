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

#include "keditloanwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QButtonGroup>
#include <QRadioButton>
#include <QLabel>
#include <QList>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interesteditwizardpage.h"
#include "ui_paymentfrequencywizardpage.h"
#include "ui_paymenteditwizardpage.h"
#include "ui_loanattributeswizardpage.h"
#include "ui_effectivedatewizardpage.h"
#include "ui_interesttypewizardpage.h"
#include "ui_editselectionwizardpage.h"
#include "ui_finalpaymentwizardpage.h"

#include "knewloanwizard.h"
#include "knewloanwizard_p.h"
#include "kmymoneylineedit.h"
#include "kmymoneyedit.h"
#include "kmymoneyaccountselector.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "mymoneyaccountloan.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"
#include "mymoneytransactionfilter.h"

class KEditLoanWizardPrivate : public KNewLoanWizardPrivate
{
  Q_DISABLE_COPY(KEditLoanWizardPrivate)

public:
  KEditLoanWizardPrivate(KEditLoanWizard *qq) :
    KNewLoanWizardPrivate(qq),
    m_lastSelection(0),
    m_fullyRepayLoan(false)
  {
  }

  MyMoneySchedule     m_schedule;
  int                 m_lastSelection;
  bool                m_fullyRepayLoan;
};

KEditLoanWizard::KEditLoanWizard(const MyMoneyAccount& account, QWidget *parent) :
    KNewLoanWizard(*new KEditLoanWizardPrivate(this), parent)
{
  Q_D(KEditLoanWizard);
  auto file = MyMoneyFile::instance();

  setWindowTitle(i18n("Edit loan wizard"));

  d->m_account = account;
  try {
    QString id = d->m_account.value("schedule");
    d->m_schedule = file->schedule(id);
  } catch (const MyMoneyException &) {
  }
  d->m_lastSelection = -1;

  loadWidgets(d->m_account);

  if (d->m_account.openingDate() > QDate::currentDate()) {
    //FIXME: port
    d->ui->m_effectiveDatePage->ui->m_effectiveDateNoteLabel->setText(QString("\n") + i18n(
          "Note: you will not be able to modify this account today, because the opening date \"%1\" is in the future. "
          "Please revisit this dialog when the time has come.", QLocale().toString(d->m_account.openingDate())));
  } else {
    d->ui->m_effectiveDatePage->ui->m_effectiveDateNoteLabel->hide();
  }
  // turn off all pages that are contained here for derived classes
  d->m_pages.clearBit(Page_Intro);
  d->m_pages.clearBit(Page_NewGeneralInfo);
  d->m_pages.clearBit(Page_LendBorrow);
  d->m_pages.clearBit(Page_Name);
  d->m_pages.clearBit(Page_NewCalculateLoan);
  d->m_pages.clearBit(Page_NewPayments);
  removePage(Page_AssetAccount);
  d->ui->m_assetAccountPage = 0;

  // turn on all pages that are contained here for derived classes
  d->m_pages.setBit(Page_EditIntro);
  d->m_pages.setBit(Page_EditSelection);

  // make sure, we show the correct start page
  setStartId(Page_EditIntro);
}

KEditLoanWizard::~KEditLoanWizard()
{
}

void KEditLoanWizard::loadWidgets(const MyMoneyAccount& /* account */)
{
  Q_D(KEditLoanWizard);
  auto file = MyMoneyFile::instance();
  QString paymentAccountId, interestAccountId;

  //FIXME: port
  d->ui->m_namePage->ui->m_nameEdit->loadText(d->m_account.name());
  d->ui->m_loanAmountPage->ui->m_loanAmountEdit->loadText(d->m_account.loanAmount().formatMoney(d->m_account.fraction(MyMoneyFile::instance()->security(d->m_account.currencyId()))));
  d->ui->m_finalPaymentPage->ui->m_finalPaymentEdit->loadText(d->m_account.finalPayment().formatMoney(d->m_account.fraction(MyMoneyFile::instance()->security(d->m_account.currencyId()))));
  setField("firstDueDateEdit", d->m_account.openingDate());

  //FIXME: port
  if (d->m_account.fixedInterestRate()) {
    d->ui->m_interestTypePage->ui->m_fixedInterestButton->click();
  } else {
    d->ui->m_interestTypePage->ui->m_variableInterestButton->click();
  }

  QString institutionName = file->institution(d->m_account.institutionId()).name();
  d->ui->m_loanAttributesPage->setInstitution(institutionName);

  MyMoneyMoney ir;
  if (d->m_schedule.startDate() > QDate::currentDate()) {
    ir = d->m_account.interestRate(d->m_schedule.startDate());
  } else {
    ir = d->m_account.interestRate(QDate::currentDate());
  }
  //FIXME: port
  d->ui->m_interestPage->ui->m_interestRateEdit->loadText(ir.formatMoney(QString(), 3));
  d->ui->m_interestPage->ui->m_interestRateEdit->setPrecision(3);
  d->ui->m_interestEditPage->ui->m_newInterestRateEdit->loadText(ir.formatMoney(QString(), 3));
  d->ui->m_interestEditPage->ui->m_newInterestRateEdit->setPrecision(3);
  d->ui->m_interestEditPage->ui->m_interestRateLabel->setText(QString(" ") + ir.formatMoney(QString(), 3) + QString("%"));

  d->ui->m_paymentFrequencyPage->ui->m_paymentFrequencyUnitEdit->setCurrentIndex(d->ui->m_paymentFrequencyPage->ui->m_paymentFrequencyUnitEdit->findData(QVariant((int)d->m_schedule.occurrencePeriod()), Qt::UserRole, Qt::MatchExactly));
  d->ui->m_durationPage->updateTermWidgets(d->m_account.term());

  // the base payment (amortization and interest) is determined
  // by adding all splits that are not automatically calculated.
  // If the loan is a liability, we reverse the sign at the end
  MyMoneyMoney basePayment;
  MyMoneyMoney addPayment;

  d->m_transaction = d->m_schedule.transaction();

  foreach (const MyMoneySplit& it_s, d->m_schedule.transaction().splits()) {
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
      d->m_transaction.removeSplit(it_s);
      d->m_split.clearId();
      d->m_transaction.addSplit(d->m_split);
    }

    if (it_s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
      interestAccountId = it_s.accountId();
    }

    if (it_s.value() != MyMoneyMoney::autoCalc) {
      basePayment += it_s.value();
    } else {
      // remove the splits which should not show up
      // for additional fees
      d->m_transaction.removeSplit(it_s);
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
  d->loadAccountList();

  int fraction = d->m_account.fraction(MyMoneyFile::instance()->security(d->m_account.currencyId()));
  //FIXME: port
  d->ui->m_paymentPage->ui->m_paymentEdit->loadText(basePayment.formatMoney(fraction));
  d->ui->m_paymentEditPage->ui->m_newPaymentEdit->loadText(basePayment.formatMoney(fraction));
  d->ui->m_paymentEditPage->ui->m_paymentLabel->setText(QString(" ") + basePayment.formatMoney(fraction));

  setField("additionalCost", addPayment.formatMoney(fraction));
  d->ui->m_interestCategoryPage->ui->m_interestAccountEdit->setSelected(interestAccountId);
  d->ui->m_schedulePage->ui->m_paymentAccountEdit->setSelected(paymentAccountId);
  setField("nextDueDateEdit", d->m_schedule.nextPayment());

  int changeFrequencyUnit;
  int amt = d->m_account.interestChangeFrequency(&changeFrequencyUnit);
  if (amt != -1) {
    setField("interestFrequencyAmountEdit", amt);
    setField("interestFrequencyUnitEdit", changeFrequencyUnit);
  }

  // keep track, if the loan should be fully repayed
  d->m_fullyRepayLoan = d->m_account.finalPayment() < basePayment;

  d->updateLoanInfo();
}

bool KEditLoanWizard::validateCurrentPage()
{
  Q_D(KEditLoanWizard);
  auto dontLeavePage = false;
  //FIXME: port m_lastSelection
  QAbstractButton* button = d->ui->m_editSelectionPage->ui->m_selectionButtonGroup->button(d->m_lastSelection);

  if (currentPage() == d->ui->m_editSelectionPage) {

    if (button != 0
        && d->m_lastSelection != d->ui->m_editSelectionPage->ui->m_selectionButtonGroup->checkedId()) {

      QString errMsg = i18n(
                         "Your previous selection was \"%1\". If you select another option, "
                         "KMyMoney will dismiss the changes you have just entered. "
                         "Do you wish to proceed?", button->text());

      if (KMessageBox::questionYesNo(this, errMsg) == KMessageBox::No) {
        dontLeavePage = true;
      } else {
        loadWidgets(d->m_account);
      }
    }

    if (!dontLeavePage) {
      // turn off all pages except the summary at the end
      // and the one's we need for the selected option
      // and load the widgets with the current values

      // general info
      d->m_pages.clearBit(Page_Name);
      d->m_pages.clearBit(Page_InterestType);
      d->m_pages.clearBit(Page_PreviousPayments);
      d->m_pages.clearBit(Page_RecordPayment);
      d->m_pages.clearBit(Page_VariableInterestDate);
      d->m_pages.clearBit(Page_FirstPayment);

      // loan calculation
      d->m_pages.clearBit(Page_PaymentEdit);
      d->m_pages.clearBit(Page_InterestEdit);
      d->m_pages.clearBit(Page_PaymentFrequency);
      d->m_pages.clearBit(Page_InterestCalculation);
      d->m_pages.clearBit(Page_LoanAmount);
      d->m_pages.clearBit(Page_Interest);
      d->m_pages.clearBit(Page_Duration);
      d->m_pages.clearBit(Page_Payment);
      d->m_pages.clearBit(Page_FinalPayment);
      d->m_pages.clearBit(Page_CalculationOverview);

      // payment
      d->m_pages.clearBit(Page_InterestCategory);
      d->m_pages.clearBit(Page_AdditionalFees);
      d->m_pages.clearBit(Page_Schedule);
      d->m_pages.setBit(Page_Summary);

      // Attributes
      d->m_pages.clearBit(Page_LoanAttributes);

      d->m_pages.setBit(Page_EffectiveDate);

      if (page(Page_Summary) != 0) {
        removePage(Page_Summary);
      }

      if (field("editInterestRateButton").toBool()) {
        d->m_pages.setBit(Page_PaymentFrequency);
        d->m_pages.setBit(Page_InterestType);
        d->m_pages.setBit(Page_VariableInterestDate);
        d->m_pages.setBit(Page_PaymentEdit);
        d->m_pages.setBit(Page_InterestEdit);
        d->m_pages.setBit(Page_InterestCategory);
        d->m_pages.setBit(Page_Schedule);
        d->m_pages.setBit(Page_SummaryEdit);

      } else if (field("editOtherCostButton").toBool()) {
        d->m_pages.setBit(Page_PaymentFrequency);
        d->m_pages.setBit(Page_AdditionalFees);
        d->m_pages.setBit(Page_InterestCategory);
        d->m_pages.setBit(Page_Schedule);
        d->m_pages.setBit(Page_SummaryEdit);

      } else if (field("editOtherInfoButton").toBool()) {
        d->m_pages.setBit(Page_Name);
        d->m_pages.setBit(Page_InterestCalculation);
        d->m_pages.setBit(Page_Interest);
        d->m_pages.setBit(Page_Duration);
        d->m_pages.setBit(Page_Payment);
        d->m_pages.setBit(Page_FinalPayment);
        d->m_pages.setBit(Page_CalculationOverview);
        d->m_pages.setBit(Page_InterestCategory);
        d->m_pages.setBit(Page_AdditionalFees);
        d->m_pages.setBit(Page_Schedule);
        d->m_pages.clearBit(Page_SummaryEdit);
        setPage(Page_Summary, d->ui->m_summaryPage);
        d->m_pages.setBit(Page_Summary);

      } else if (field("editAttributesButton").toBool()) {
        d->m_pages.setBit(Page_LoanAttributes);
        d->m_pages.clearBit(Page_EffectiveDate);
      } else {
        qWarning("%s,%d: This should never happen", __FILE__, __LINE__);
      }

      d->m_lastSelection = d->ui->m_editSelectionPage->ui->m_selectionButtonGroup->checkedId();
    } // if(!dontLeavePage)

  } else if (currentPage() == d->ui->m_additionalFeesPage) {
    if (field("editOtherCostButton").toBool()) {
      d->updateLoanInfo();
      updateEditSummary();
    }

  } else if (currentPage() == d->ui->m_interestEditPage) {
    // copy the necessary data to the widgets used for calculation
    //FIXME: port to fields
    d->ui->m_interestPage->ui->m_interestRateEdit->setValue(field("newInterestRateEdit").value<MyMoneyMoney>());
    d->ui->m_paymentPage->ui->m_paymentEdit->setValue(field("newPaymentEdit").value<MyMoneyMoney>());

    // if interest rate and payment amount is given, then force
    // the term to be recalculated. The final payment is adjusted to
    // 0 if the loan was ment to be fully repayed
    d->ui->m_durationPage->updateTermWidgets(d->m_account.term());
    if (field("interestRateEditValid").toBool()
        && field("paymentEditValid").toBool()) {
      // if there's an amortization going on, we can evaluate
      // the new term. If the amortization is 0 (interest only
      // payments) then we keep the term as entered by the user.
      if (field("loanAmountEdit").value<MyMoneyMoney>() != field("finalPaymentEdit").value<MyMoneyMoney>()) {
        setField("durationValueEdit", 0);
      }
      if (d->m_fullyRepayLoan)
        d->ui->m_finalPaymentPage->ui->m_finalPaymentEdit->loadText(MyMoneyMoney().formatMoney(d->m_account.fraction(MyMoneyFile::instance()->security(d->m_account.currencyId()))));
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
    dontLeavePage = !d->calculateLoan();

    // reset the original loan amount to the widget
    //FIXME: port to fields
    d->ui->m_loanAmountPage->ui->m_loanAmountEdit->setValue(d->m_account.loanAmount());

    if (!dontLeavePage) {
      d->updateLoanInfo();
      updateEditSummary();
    }
  }

  if (!dontLeavePage)
    dontLeavePage = ! KNewLoanWizard::validateCurrentPage();

  // These might have been set by KNewLoanWizard
  d->m_pages.clearBit(Page_PreviousPayments);
  d->m_pages.clearBit(Page_RecordPayment);

  if (dontLeavePage)
    return false;

  // we never need to show this page
  if (currentPage() == d->ui->m_previousPaymentsPage)
    dontLeavePage = KNewLoanWizard::validateCurrentPage();

  return ! dontLeavePage;
}

void KEditLoanWizard::updateEditSummary()
{
  Q_D(KEditLoanWizard);
  // calculate the number of affected transactions
  MyMoneyTransactionFilter filter(d->m_account.id());
  filter.setDateFilter(field("effectiveChangeDateEdit").toDate(), QDate());

  int count = 0;
  QList<MyMoneyTransaction> list;
  list = MyMoneyFile::instance()->transactionList(filter);

  foreach (const MyMoneyTransaction& it, list) {
    int match = 0;
    foreach (const MyMoneySplit& it_s, it.splits()) {
      // we only count those transactions that have an interest
      // and amortization part
      if (it_s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest))
        match |= 0x01;
      if (it_s.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization))
        match |= 0x02;
    }
    if (match == 0x03)
      ++count;
  }

  setField("affectedPayments", QString().sprintf("%d", count));
}

const MyMoneySchedule KEditLoanWizard::schedule() const
{
  Q_D(const KEditLoanWizard);
  MyMoneySchedule sched = d->m_schedule;
  sched.setTransaction(transaction());
  sched.setOccurrence(eMyMoney::Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt()));
  if (field("nextDueDateEdit").toDate() < d->m_schedule.startDate())
    sched.setStartDate(field("nextDueDateEdit").toDate());

  return sched;
}

const MyMoneyAccount KEditLoanWizard::account() const
{
  Q_D(const KEditLoanWizard);
  MyMoneyAccountLoan acc(d->m_account);

  if (field("interestOnReceptionButton").toBool())
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentReceived);
  else
    acc.setInterestCalculation(MyMoneyAccountLoan::paymentDue);

  auto file = MyMoneyFile::instance();

  QString institution = d->ui->m_loanAttributesPage->ui->m_qcomboboxInstitutions->currentText();
  if (institution != i18n("(No Institution)")) {
    const auto list = file->institutionList();
    for (const auto& testInstitution : list) {
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
  acc.setTerm(d->ui->m_durationPage->term());
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
  Q_D(const KEditLoanWizard);
  auto t = d->transaction();
  auto s = t.splitByAccount(QString("Phony-ID"));

  s.setAccountId(d->m_account.id());
  t.modifySplit(s);

  return t;
}
