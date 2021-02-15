/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
