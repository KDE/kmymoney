/***************************************************************************
                         interesttypewizardpage  -  description
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

#include "interesttypewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interesttypewizardpage.h"

InterestTypeWizardPage::InterestTypeWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::InterestTypeWizardPage)
{
  ui->setupUi(this);

  ui->ButtonGroup2->setId(ui->m_fixedInterestButton, 0);
  ui->ButtonGroup2->setId(ui->m_variableInterestButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("fixedInterestButton", ui->m_fixedInterestButton);
  registerField("variableInterestButton", ui->m_variableInterestButton);

  ui->m_fixedInterestButton->click();
}

InterestTypeWizardPage::~InterestTypeWizardPage()
{
  delete ui;
}
