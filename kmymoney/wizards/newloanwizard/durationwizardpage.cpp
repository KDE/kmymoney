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

#include "mymoneyschedule.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

DurationWizardPage::DurationWizardPage(QWidget *parent)
    : DurationWizardPageDecl(parent)
{
  m_durationUnitEdit->insertItem(i18n("Months"), static_cast<int>(Schedule::Occurrence::Monthly));
  m_durationUnitEdit->insertItem(i18n("Years"), static_cast<int>(Schedule::Occurrence::Yearly));
  m_durationUnitEdit->insertItem(i18n("Payments"), static_cast<int>(Schedule::Occurrence::Once));

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
      case (int)Schedule::Occurrence::Yearly: // years
        factor = 12;
        // intentional fall through

      case (int)Schedule::Occurrence::Monthly: // months
        factor *= 30;
        factor *= m_durationValueEdit->value();
        // factor now is the duration in days. we divide this by the
        // payment frequency and get the number of payments
        factor /= MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt()));
        break;

      case (int)Schedule::Occurrence::Once: // payments
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
  Schedule::Occurrence unit;
  unit = Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt());

  if ((unit == Schedule::Occurrence::Monthly)
      && ((vl % 12) == 0)) {
    vl /= 12;
    unit = Schedule::Occurrence::Yearly;
  }

  switch (unit) {
    case Schedule::Occurrence::Monthly:
      valString = i18np("one month", "%1 months", vl);
      m_durationUnitEdit->setCurrentItem(static_cast<int>(Schedule::Occurrence::Monthly));
      break;
    case Schedule::Occurrence::Yearly:
      valString = i18np("one year", "%1 years", vl);
      m_durationUnitEdit->setCurrentItem(static_cast<int>(Schedule::Occurrence::Yearly));
      break;
    default:
      valString = i18np("one payment", "%1 payments", vl);
      m_durationUnitEdit->setCurrentItem(static_cast<int>(Schedule::Occurrence::Once));
      break;
  }
  m_durationValueEdit->setValue(vl);
  return valString;
}
