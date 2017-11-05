/***************************************************************************
                         interestwizardpage  -  description
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

#include "interestwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestwizardpage.h"

InterestWizardPage::InterestWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::InterestWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestRateEdit", ui->m_interestRateEdit, "value", SIGNAL(textChanged()));
  registerField("interestRateEditValid", ui->m_interestRateEdit, "valid", SIGNAL(textChanged()));

  registerField("loanAmount2", ui->m_loanAmount2, "text");
  registerField("interestRate2", ui->m_interestRate2, "text");
  registerField("duration2", ui->m_duration2, "text");
  registerField("payment2", ui->m_payment2, "text");
  registerField("balloon2", ui->m_balloon2, "text");
}

InterestWizardPage::~InterestWizardPage()
{
  delete ui;
}

void InterestWizardPage::initializePage()
{
  ui->m_interestRateEdit->setFocus();
}

void InterestWizardPage::resetCalculator()
{
  ui->m_loanAmount2->setText(QString());
  ui->m_interestRate2->setText(QString());
  ui->m_duration2->setText(QString());
  ui->m_payment2->setText(QString());
  ui->m_balloon2->setText(QString());
}
