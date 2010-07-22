/***************************************************************************
                         schedulewizardpage  -  description
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

#include "schedulewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


ScheduleWizardPage::ScheduleWizardPage(QWidget *parent)
    : ScheduleWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("nextDueDateEdit", m_nextDueDateEdit, "date", SIGNAL(dateChanged(const QDate&)));
  registerField("paymentAccountEdit", m_paymentAccountEdit, "selectedItems");

  connect(m_nextDueDateEdit, SIGNAL(dateChanged(const QDate&)), this, SIGNAL(completeChanged()));
  connect(m_paymentAccountEdit,  SIGNAL(stateChanged()), this, SIGNAL(completeChanged()));

  m_paymentAccountEdit->removeButtons();
}

/**
 * Update the "Next" button
 */
bool ScheduleWizardPage::isComplete() const
{
  return m_nextDueDateEdit->date().isValid()
         && m_nextDueDateEdit->date() >= field("firstDueDateEdit").toDate()
         && m_paymentAccountEdit->selectedItems().count() > 0;
}

void ScheduleWizardPage::initializePage()
{
  m_nextDueDateEdit->setEnabled(true);
  if (field("allPaymentsButton").toBool() || field("noPreviousPaymentButton").toBool()) {
    setField("nextDueDateEdit", field("firstDueDateEdit").toDate());
    m_nextDueDateEdit->setEnabled(false);
  } else {
    QDate nextPayment(QDate::currentDate().year(), 1, field("firstDueDateEdit").toDate().day());
    setField("nextDueDateEdit", nextPayment);
  }
  if (field("nextDueDateEdit").toDate() < field("firstDueDateEdit").toDate()) {
    setField("nextDueDateEdit", field("firstDueDateEdit").toDate());
  }
}


#include "schedulewizardpage.moc"

