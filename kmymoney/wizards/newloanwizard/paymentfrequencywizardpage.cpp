/***************************************************************************
                         paymentfrequencywizardpage  -  description
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

#include "paymentfrequencywizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyscheduled.h"

PaymentFrequencyWizardPage::PaymentFrequencyWizardPage(QWidget *parent)
    : PaymentFrequencyWizardPageDecl(parent)
{
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_DAILY).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_WEEKLY).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_FORTNIGHTLY).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERWEEK).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYHALFMONTH).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHREEWEEKS).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURWEEKS).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYTHIRTYDAYS).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_MONTHLY).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYEIGHTWEEKS).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYOTHERMONTH).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_QUARTERLY).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_EVERYFOURMONTHS).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_TWICEYEARLY).toLatin1()));
  m_paymentFrequencyUnitEdit->addItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_YEARLY).toLatin1()));

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("paymentFrequencyUnitEdit", m_paymentFrequencyUnitEdit, "currentText", SIGNAL(currentIndexChanged(const QString&)));

  m_paymentFrequencyUnitEdit->setCurrentItem(i18n(MyMoneySchedule::occurrenceToString(MyMoneySchedule::OCCUR_MONTHLY).toLatin1()));

}

#include "paymentfrequencywizardpage.moc"

