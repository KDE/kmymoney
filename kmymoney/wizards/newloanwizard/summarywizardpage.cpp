/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "summarywizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_summarywizardpage.h"

#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"

SummaryWizardPage::SummaryWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::SummaryWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
}

SummaryWizardPage::~SummaryWizardPage()
{
  delete ui;
}

void SummaryWizardPage::initializePage()
{
  // General
  if (field("borrowButton").toBool())
    ui->m_summaryLoanType->setText(i18n("borrowed"));
  else
    ui->m_summaryLoanType->setText(i18n("lend"));

  ui->m_summaryFirstPayment->setText(QLocale().toString(field("firstDueDateEdit").toDate()));

  const QString &payeeId = field("payeeEdit").toString();
  if (!payeeId.isEmpty()) {
    try {
      const MyMoneyPayee &payee = MyMoneyFile::instance()->payee(payeeId);
      ui->m_summaryPayee->setText(payee.name());
    } catch (const MyMoneyException &) {
      qWarning("Unable to load the payee name from the id");
    }
  } else {
    ui->m_summaryPayee->setText(i18n("not assigned"));
  }

  // Calculation
  if (field("interestOnReceptionButton").toBool())
    ui->m_summaryInterestDue->setText(i18n("on reception"));
  else
    ui->m_summaryInterestDue->setText(i18n("on due date"));
  ui->m_summaryPaymentFrequency->setText(MyMoneySchedule::occurrenceToString(eMyMoney::Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt())));
  ui->m_summaryAmount->setText(field("loanAmount6").toString());
  ui->m_summaryInterestRate->setText(field("interestRate6").toString());
  ui->m_summaryTerm->setText(field("duration6").toString());
  ui->m_summaryPeriodicPayment->setText(field("payment6").toString());
  ui->m_summaryBalloonPayment->setText(field("balloon6").toString());

  // Payment
  try {
    QStringList sel = field("interestAccountEdit").toStringList();
    if (sel.count() != 1)
      throw MYMONEYEXCEPTION_CSTRING("Need a single selected interest category");
    MyMoneyAccount acc = MyMoneyFile::instance()->account(sel.first());
    ui->m_summaryInterestCategory->setText(acc.name());
  } catch (const MyMoneyException &) {
    qWarning("Unable to determine interest category for loan account creation");
  }
  ui->m_summaryAdditionalFees->setText(field("additionalCost").toString());
  ui->m_summaryTotalPeriodicPayment->setText(field("periodicPayment").toString());
  ui->m_summaryNextPayment->setText(QLocale().toString(field("nextDueDateEdit").toDate()));

  try {
    QStringList sel = field("paymentAccountEdit").toStringList();
    if (sel.count() != 1)
      throw MYMONEYEXCEPTION_CSTRING("Need a single selected payment account");
    MyMoneyAccount acc = MyMoneyFile::instance()->account(sel.first());
    ui->m_summaryPaymentAccount->setText(acc.name());
  } catch (const MyMoneyException &) {
    qWarning("Unable to determine payment account for loan account creation");
  }

}
