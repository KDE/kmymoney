/***************************************************************************
                         finalpaymentwizardpage  -  description
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

#include "finalpaymentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_finalpaymentwizardpage.h"

FinalPaymentWizardPage::FinalPaymentWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::FinalPaymentWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("finalPaymentEdit", ui->m_finalPaymentEdit, "value", SIGNAL(textChanged()));
  registerField("finalPaymentEditValid", ui->m_finalPaymentEdit, "valid", SIGNAL(textChanged()));

  registerField("loanAmount5", ui->m_loanAmount5, "text");
  registerField("interestRate5", ui->m_interestRate5, "text");
  registerField("duration5", ui->m_duration5, "text");
  registerField("payment5", ui->m_payment5, "text");
  registerField("balloon5", ui->m_balloon5, "text");

  ui->m_finalPaymentEdit->setAllowEmpty(true);
}

FinalPaymentWizardPage::~FinalPaymentWizardPage()
{
  delete ui;
}

void FinalPaymentWizardPage::resetCalculator()
{
  ui->m_loanAmount5->setText(QString());
  ui->m_interestRate5->setText(QString());
  ui->m_duration5->setText(QString());
  ui->m_payment5->setText(QString());
  ui->m_balloon5->setText(QString());
}
