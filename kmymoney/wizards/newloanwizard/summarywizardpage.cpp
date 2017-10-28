/***************************************************************************
                         summarywizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "summarywizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneypayee.h"
#include "mymoneyschedule.h"

SummaryWizardPage::SummaryWizardPage(QWidget *parent)
    : SummaryWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
}

void SummaryWizardPage::initializePage()
{
  // General
  if (field("borrowButton").toBool())
    m_summaryLoanType->setText(i18n("borrowed"));
  else
    m_summaryLoanType->setText(i18n("lend"));

  m_summaryFirstPayment->setText(QLocale().toString(field("firstDueDateEdit").toDate()));

  const QString &payeeId = field("payeeEdit").toString();
  if (!payeeId.isEmpty()) {
    try {
      const MyMoneyPayee &payee = MyMoneyFile::instance()->payee(payeeId);
      m_summaryPayee->setText(payee.name());
    } catch (const MyMoneyException &) {
      qWarning("Unable to load the payee name from the id");
    }
  } else {
    m_summaryPayee->setText(i18n("not assigned"));
  }

  // Calculation
  if (field("interestOnReceptionButton").toBool())
    m_summaryInterestDue->setText(i18n("on reception"));
  else
    m_summaryInterestDue->setText(i18n("on due date"));
  m_summaryPaymentFrequency->setText(MyMoneySchedule::occurrenceToString(eMyMoney::Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt())));
  m_summaryAmount->setText(field("loanAmount6").toString());
  m_summaryInterestRate->setText(field("interestRate6").toString());
  m_summaryTerm->setText(field("duration6").toString());
  m_summaryPeriodicPayment->setText(field("payment6").toString());
  m_summaryBalloonPayment->setText(field("balloon6").toString());

  // Payment
  try {
    QStringList sel = field("interestAccountEdit").toStringList();
    if (sel.count() != 1)
      throw MYMONEYEXCEPTION("Need a single selected interest category");
    MyMoneyAccount acc = MyMoneyFile::instance()->account(sel.first());
    m_summaryInterestCategory->setText(acc.name());
  } catch (const MyMoneyException &) {
    qWarning("Unable to determine interest category for loan account creation");
  }
  m_summaryAdditionalFees->setText(field("additionalCost").toString());
  m_summaryTotalPeriodicPayment->setText(field("periodicPayment").toString());
  m_summaryNextPayment->setText(QLocale().toString(field("nextDueDateEdit").toDate()));

  try {
    QStringList sel = field("paymentAccountEdit").toStringList();
    if (sel.count() != 1)
      throw MYMONEYEXCEPTION("Need a single selected payment account");
    MyMoneyAccount acc = MyMoneyFile::instance()->account(sel.first());
    m_summaryPaymentAccount->setText(acc.name());
  } catch (const MyMoneyException &) {
    qWarning("Unable to determine payment account for loan account creation");
  }

}
