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


InterestTypeWizardPage::InterestTypeWizardPage(QWidget *parent)
    : InterestTypeWizardPageDecl(parent)
{
  ButtonGroup2->setId(m_fixedInterestButton, 0);
  ButtonGroup2->setId(m_variableInterestButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("fixedInterestButton", m_fixedInterestButton);
  registerField("variableInterestButton", m_variableInterestButton);

  m_fixedInterestButton->animateClick();
}
