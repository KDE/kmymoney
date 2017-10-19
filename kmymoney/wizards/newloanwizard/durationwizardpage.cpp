/***************************************************************************
                         durationwizardpage  -  description
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

#include "durationwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qmath.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


DurationWizardPage::DurationWizardPage(QWidget *parent)
    : DurationWizardPageDecl(parent)
{
  m_durationUnitEdit->insertItem(i18n("Months"), static_cast<int>(MyMoneySchedule::OCCUR_MONTHLY));
  m_durationUnitEdit->insertItem(i18n("Years"), static_cast<int>(MyMoneySchedule::OCCUR_YEARLY));
  m_durationUnitEdit->insertItem(i18n("Payments"), static_cast<int>(MyMoneySchedule::OCCUR_ONCE));

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("durationValueEdit", m_durationValueEdit, "value");
  registerField("durationUnitEdit", m_durationUnitEdit, "currentText", SIGNAL(currentIndexChanged(QString)));

  registerField("loanAmount3", m_loanAmount3, "text");
  registerField("interestRate3", m_interestRate3, "text");
  registerField("duration3", m_duration3, "text");
  registerField("payment3", m_payment3, "text");
  registerField("balloon3", m_balloon3, "text");
}

void DurationWizardPage::resetCalculator()
{
  m_loanAmount3->setText(QString());
  m_interestRate3->setText(QString());
  m_duration3->setText(QString());
  m_payment3->setText(QString());
  m_balloon3->setText(QString());
}

int DurationWizardPage::term() const
{
  int factor = 0;

  if (m_durationValueEdit->value() != 0) {
    factor = 1;
    switch (m_durationUnitEdit->currentItem()) {
      case MyMoneySchedule::OCCUR_YEARLY: // years
        factor = 12;
        // intentional fall through

      case MyMoneySchedule::OCCUR_MONTHLY: // months
        factor *= 30;
        factor *= m_durationValueEdit->value();
        // factor now is the duration in days. we divide this by the
        // payment frequency and get the number of payments
        factor /= MyMoneySchedule::daysBetweenEvents(MyMoneySchedule::occurrenceE(field("paymentFrequencyUnitEdit").toInt()));
        break;

      case MyMoneySchedule::OCCUR_ONCE: // payments
        factor = m_durationValueEdit->value();
        break;
    }
  }
  return factor;
}

QString DurationWizardPage::updateTermWidgets(const double val)
{
  long vl = qFloor(val);

  QString valString;
  MyMoneySchedule::occurrenceE unit;
  unit = MyMoneySchedule::occurrenceE(field("paymentFrequencyUnitEdit").toInt());

  if ((unit == MyMoneySchedule::OCCUR_MONTHLY)
      && ((vl % 12) == 0)) {
    vl /= 12;
    unit = MyMoneySchedule::OCCUR_YEARLY;
  }

  switch (unit) {
    case MyMoneySchedule::OCCUR_MONTHLY:
      valString = i18np("one month", "%1 months", vl);
      m_durationUnitEdit->setCurrentItem(static_cast<int>(MyMoneySchedule::OCCUR_MONTHLY));
      break;
    case MyMoneySchedule::OCCUR_YEARLY:
      valString = i18np("one year", "%1 years", vl);
      m_durationUnitEdit->setCurrentItem(static_cast<int>(MyMoneySchedule::OCCUR_YEARLY));
      break;
    default:
      valString = i18np("one payment", "%1 payments", vl);
      m_durationUnitEdit->setCurrentItem(static_cast<int>(MyMoneySchedule::OCCUR_ONCE));
      break;
  }
  m_durationValueEdit->setValue(vl);
  return valString;
}
