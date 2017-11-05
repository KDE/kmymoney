/***************************************************************************
                         recordpaymentwizardpage  -  description
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

#include "recordpaymentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_recordpaymentwizardpage.h"

RecordPaymentWizardPage::RecordPaymentWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::RecordPaymentWizardPage)
{
  ui->setupUi(this);

  ui->ButtonGroup4->setId(ui->m_allPaymentsButton, 0);
  ui->ButtonGroup4->setId(ui->m_thisYearPaymentButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("allPaymentsButton", ui->m_allPaymentsButton);
  registerField("thisYearPaymentButton", ui->m_thisYearPaymentButton);

  ui->m_allPaymentsButton->click();
}

RecordPaymentWizardPage::~RecordPaymentWizardPage()
{
  delete ui;
}

void RecordPaymentWizardPage::initializePage()
{
  if (field("noPreviousPaymentButton").toBool())
    ui->m_allPaymentsButton->click();
}
