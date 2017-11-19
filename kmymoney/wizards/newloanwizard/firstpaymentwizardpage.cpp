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

#include "ui_firstpaymentwizardpage.h"

FirstPaymentWizardPage::FirstPaymentWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::FirstPaymentWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("firstDueDateEdit", ui->m_firstDueDateEdit, "date");
  connect(ui->m_firstDueDateEdit, &KMyMoneyDateInput::dateChanged, this, &QWizardPage::completeChanged);
}

FirstPaymentWizardPage::~FirstPaymentWizardPage()
{
  delete ui;
}

/**
 * Update the "Next" button
 */
bool FirstPaymentWizardPage::isComplete() const
{
  return ui->m_firstDueDateEdit->date().isValid();
}

void FirstPaymentWizardPage::initializePage()
{
  if (field("allPaymentsButton").toBool()) {
    ui->m_firstPaymentLabel->setText(
      QString("\n") +
      i18n("Please enter the date, the first payment for this loan was/is due."));
    ui->m_firstPaymentNote->setText(
      i18n("Note: Consult the loan contract for details of the first due date. "
           "Keep in mind, that the first due date usually differs from the date "
           "the contract was signed"));
  } else if (field("thisYearPaymentButton").toBool()) {
    ui->m_firstPaymentLabel->setText(
      QString("\n") +
      i18n("Please enter the date, the first payment for this loan was/is due this year."));
    ui->m_firstPaymentNote->setText(
      i18n("Note: You can easily figure out the date of the first payment "
           "if you consult the last statement of last year."));
  }
}
