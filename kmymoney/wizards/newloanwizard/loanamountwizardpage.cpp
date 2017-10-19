/***************************************************************************
                         loanamountwizardpage  -  description
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

#include "loanamountwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


LoanAmountWizardPage::LoanAmountWizardPage(QWidget *parent)
    : LoanAmountWizardPageDecl(parent)
{

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("loanAmountEdit", m_loanAmountEdit, "value", SIGNAL(textChanged()));
  registerField("loanAmountEditValid", m_loanAmountEdit, "valid", SIGNAL(textChanged()));

  connect(m_loanAmountEdit, SIGNAL(textChanged(QString)), this, SIGNAL(completeChanged()));

  registerField("loanAmount1", m_loanAmount1, "text");
  registerField("interestRate1", m_interestRate1, "text");
  registerField("duration1", m_duration1, "text");
  registerField("payment1", m_payment1, "text");
  registerField("balloon1", m_balloon1, "text");
}

/**
 * Update the "Next" button
 */
bool LoanAmountWizardPage::isComplete() const
{
  return !(field("thisYearPaymentButton").toBool()
           && !m_loanAmountEdit->isValid());
}

void LoanAmountWizardPage::initializePage()
{
  if (field("allPaymentsButton").toBool()) {
    m_balanceLabel->setText(
      QString("\n") +
      i18n("Please enter the original loan amount in the field below or leave it "
           "empty to be calculated."));
  } else if (field("thisYearPaymentButton").toBool()) {
    m_balanceLabel->setText(QString("\n") +
                            i18n("Please enter the remaining loan amount of last years final "
                                 "statement in the field below. You should not leave this field empty."));

  }
}

void LoanAmountWizardPage::resetCalculator()
{
  m_loanAmount1->setText(QString());
  m_interestRate1->setText(QString());
  m_duration1->setText(QString());
  m_payment1->setText(QString());
  m_balloon1->setText(QString());
}
