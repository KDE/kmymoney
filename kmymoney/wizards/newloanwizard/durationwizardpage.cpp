/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "durationwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qmath.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_durationwizardpage.h"

#include "mymoneyschedule.h"
#include "mymoneyenums.h"

using namespace eMyMoney;

DurationWizardPage::DurationWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::DurationWizardPage)
{
  ui->setupUi(this);

  ui->m_durationUnitEdit->insertItem(i18n("Months"), static_cast<int>(Schedule::Occurrence::Monthly));
  ui->m_durationUnitEdit->insertItem(i18n("Years"), static_cast<int>(Schedule::Occurrence::Yearly));
  ui->m_durationUnitEdit->insertItem(i18n("Payments"), static_cast<int>(Schedule::Occurrence::Once));

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("durationValueEdit", ui->m_durationValueEdit, "value");
  registerField("durationUnitEdit", ui->m_durationUnitEdit, "currentText", SIGNAL(currentIndexChanged(QString)));

  registerField("loanAmount3", ui->m_loanAmount3, "text");
  registerField("interestRate3", ui->m_interestRate3, "text");
  registerField("duration3", ui->m_duration3, "text");
  registerField("payment3", ui->m_payment3, "text");
  registerField("balloon3", ui->m_balloon3, "text");
}

DurationWizardPage::~DurationWizardPage()
{
  delete ui;
}

void DurationWizardPage::resetCalculator()
{
  ui->m_loanAmount3->setText(QString());
  ui->m_interestRate3->setText(QString());
  ui->m_duration3->setText(QString());
  ui->m_payment3->setText(QString());
  ui->m_balloon3->setText(QString());
}

int DurationWizardPage::term() const
{
  int factor = 0;

  if (ui->m_durationValueEdit->value() != 0) {
    factor = 1;
    switch (ui->m_durationUnitEdit->currentItem()) {
      case (int)Schedule::Occurrence::Yearly: // years
        factor = 12;
        // intentional fall through

      case (int)Schedule::Occurrence::Monthly: // months
        factor *= 30;
        factor *= ui->m_durationValueEdit->value();
        // factor now is the duration in days. we divide this by the
        // payment frequency and get the number of payments
        factor /= MyMoneySchedule::daysBetweenEvents(Schedule::Occurrence(field("paymentFrequencyUnitEdit").toInt()));
        break;

      case (int)Schedule::Occurrence::Once: // payments
        factor = ui->m_durationValueEdit->value();
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
      ui->m_durationUnitEdit->setCurrentItem(static_cast<int>(Schedule::Occurrence::Monthly));
      break;
    case Schedule::Occurrence::Yearly:
      valString = i18np("one year", "%1 years", vl);
      ui->m_durationUnitEdit->setCurrentItem(static_cast<int>(Schedule::Occurrence::Yearly));
      break;
    default:
      valString = i18np("one payment", "%1 payments", vl);
      ui->m_durationUnitEdit->setCurrentItem(static_cast<int>(Schedule::Occurrence::Once));
      break;
  }
  ui->m_durationValueEdit->setValue(vl);
  return valString;
}
