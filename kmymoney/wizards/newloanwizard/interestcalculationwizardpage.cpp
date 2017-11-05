/***************************************************************************
                         interestcalculationwizardpage  -  description
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

#include "interestcalculationwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestcalculationwizardpage.h"

InterestCalculationWizardPage::InterestCalculationWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::InterestCalculationWizardPage)
{
  ui->setupUi(this);
  ui->ButtonGroup5->setId(ui->m_interestOnReceptionButton, 0);
  ui->ButtonGroup5->setId(ui->m_interestOnPaymentButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestOnReceptionButton", ui->m_interestOnReceptionButton);
  registerField("interestOnPaymentButton", ui->m_interestOnPaymentButton);

  ui->m_interestOnReceptionButton->click();
}

InterestCalculationWizardPage::~InterestCalculationWizardPage()
{
  delete ui;
}
