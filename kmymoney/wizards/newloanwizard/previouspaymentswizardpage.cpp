/***************************************************************************
                         previouspaymentswizardpage  -  description
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

#include "previouspaymentswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_previouspaymentswizardpage.h"

PreviousPaymentsWizardPage::PreviousPaymentsWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::PreviousPaymentsWizardPage)
{
  ui->setupUi(this);

  ui->ButtonGroup3->setId(ui->m_noPreviousPaymentButton, 0);
  ui->ButtonGroup3->setId(ui->m_previousPaymentButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("noPreviousPaymentButton", ui->m_noPreviousPaymentButton);
  registerField("previousPaymentButton", ui->m_previousPaymentButton);

  ui->m_noPreviousPaymentButton->click();
}

PreviousPaymentsWizardPage::~PreviousPaymentsWizardPage()
{
  delete ui;
}
