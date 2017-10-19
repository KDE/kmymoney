/***************************************************************************
                         firstpaymentwizardpage  -  description
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

#include "firstpaymentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


FirstPaymentWizardPage::FirstPaymentWizardPage(QWidget *parent)
    : FirstPaymentWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("firstDueDateEdit", m_firstDueDateEdit, "date");
  connect(m_firstDueDateEdit, SIGNAL(dateChanged(QDate)), this, SIGNAL(completeChanged()));

}

/**
 * Update the "Next" button
 */
bool FirstPaymentWizardPage::isComplete() const
{
  return m_firstDueDateEdit->date().isValid();
}

void FirstPaymentWizardPage::initializePage()
{
  if (field("allPaymentsButton").toBool()) {
    m_firstPaymentLabel->setText(
      QString("\n") +
      i18n("Please enter the date, the first payment for this loan was/is due."));
    m_firstPaymentNote->setText(
      i18n("Note: Consult the loan contract for details of the first due date. "
           "Keep in mind, that the first due date usually differs from the date "
           "the contract was signed"));
  } else if (field("thisYearPaymentButton").toBool()) {
    m_firstPaymentLabel->setText(
      QString("\n") +
      i18n("Please enter the date, the first payment for this loan was/is due this year."));
    m_firstPaymentNote->setText(
      i18n("Note: You can easily figure out the date of the first payment "
           "if you consult the last statement of last year."));
  }
}
