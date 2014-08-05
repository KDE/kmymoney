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


InterestCalculationWizardPage::InterestCalculationWizardPage(QWidget *parent)
    : InterestCalculationWizardPageDecl(parent)
{
  ButtonGroup5->setId(m_interestOnReceptionButton, 0);
  ButtonGroup5->setId(m_interestOnPaymentButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestOnReceptionButton", m_interestOnReceptionButton);
  registerField("interestOnPaymentButton", m_interestOnPaymentButton);

  m_interestOnReceptionButton->animateClick();
}
