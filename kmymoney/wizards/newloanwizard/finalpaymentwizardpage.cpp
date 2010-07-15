/***************************************************************************
                         finalpaymentwizardpage  -  description
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

#include "finalpaymentwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


FinalPaymentWizardPage::FinalPaymentWizardPage(QWidget *parent)
    : FinalPaymentWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("finalPaymentEdit", m_finalPaymentEdit, "value", SIGNAL(textChanged()));
  registerField("finalPaymentEditValid", m_finalPaymentEdit, "valid", SIGNAL(textChanged()));

  registerField("loanAmount5", m_loanAmount5, "text");
  registerField("interestRate5", m_interestRate5, "text");
  registerField("duration5", m_duration5, "text");
  registerField("payment5", m_payment5, "text");
  registerField("balloon5", m_balloon5, "text");
}

void FinalPaymentWizardPage::resetCalculator()
{
  m_loanAmount5->setText(QString());
  m_interestRate5->setText(QString());
  m_duration5->setText(QString());
  m_payment5->setText(QString());
  m_balloon5->setText(QString());
}

#include "finalpaymentwizardpage.moc"

