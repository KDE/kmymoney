/***************************************************************************
                         editselectionwizardpage  -  description
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

#include "editselectionwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


EditSelectionWizardPage::EditSelectionWizardPage(QWidget *parent)
    : EditSelectionWizardPageDecl(parent)
{
  m_selectionButtonGroup->setId(m_editInterestRateButton, 0);
  m_selectionButtonGroup->setId(m_editOtherCostButton, 1);
  m_selectionButtonGroup->setId(m_editOtherInfoButton, 2);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("editInterestRateButton", m_editInterestRateButton);
  registerField("editOtherCostButton", m_editOtherCostButton);
  registerField("editOtherInfoButton", m_editOtherInfoButton);

  m_editInterestRateButton->animateClick();
}

#include "editselectionwizardpage.moc"

