/***************************************************************************
                         previouspaymentswizardpage  -  description
                            -------------------
   begin                : Sun Jul 4 2010
   copyright            : (C) 2010 by Fernando Vilas
   email                : kmymoney-devel@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
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
