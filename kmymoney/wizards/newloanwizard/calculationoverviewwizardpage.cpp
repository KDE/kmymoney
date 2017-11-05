/***************************************************************************
                         calculationoverviewwizardpage  -  description
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

#include "calculationoverviewwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_calculationoverviewwizardpage.h"

CalculationOverviewWizardPage::CalculationOverviewWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::CalculationOverviewWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("loanAmount6", ui->m_loanAmount6, "text");
  registerField("interestRate6", ui->m_interestRate6, "text");
  registerField("duration6", ui->m_duration6, "text");
  registerField("payment6", ui->m_payment6, "text");
  registerField("balloon6", ui->m_balloon6, "text");
}

CalculationOverviewWizardPage::~CalculationOverviewWizardPage()
{
  delete ui;
}
