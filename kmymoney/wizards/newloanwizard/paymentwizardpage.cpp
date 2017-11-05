/***************************************************************************
                         paymentwizardpage  -  description
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

#include "paymentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_paymentwizardpage.h"

PaymentWizardPage::PaymentWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::PaymentWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("paymentEdit", ui->m_paymentEdit, "value", SIGNAL(textChanged()));
  registerField("paymentEditValid", ui->m_paymentEdit, "valid", SIGNAL(textChanged()));

  registerField("loanAmount4", ui->m_loanAmount4, "text");
  registerField("interestRate4", ui->m_interestRate4, "text");
  registerField("duration4", ui->m_duration4, "text");
  registerField("payment4", ui->m_payment4, "text");
  registerField("balloon4", ui->m_balloon4, "text");
}

PaymentWizardPage::~PaymentWizardPage()
{
  delete ui;
}

void PaymentWizardPage::resetCalculator()
{
  ui->m_loanAmount4->setText(QString());
  ui->m_interestRate4->setText(QString());
  ui->m_duration4->setText(QString());
  ui->m_payment4->setText(QString());
  ui->m_balloon4->setText(QString());
}
