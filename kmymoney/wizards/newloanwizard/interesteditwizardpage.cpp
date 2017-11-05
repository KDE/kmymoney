/***************************************************************************
                         interesteditwizardpage  -  description
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

#include "interesteditwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interesteditwizardpage.h"

InterestEditWizardPage::InterestEditWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::InterestEditWizardPage)
{
  ui->setupUi(this);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("newInterestRateEdit", ui->m_newInterestRateEdit, "value", SIGNAL(textChanged(QString)));
  connect(ui->m_newInterestRateEdit, SIGNAL(textChanged(QString)), this, SIGNAL(completeChanged()));
}

InterestEditWizardPage::~InterestEditWizardPage()
{
  delete ui;
}

/**
 * Update the "Next" button
 */
bool InterestEditWizardPage::isComplete() const
{
  //FIXME: this only exists in the EditLoanWizard subclass
  return field("newPaymentEditValid").toBool()
         || ui->m_newInterestRateEdit->isValid();
}
