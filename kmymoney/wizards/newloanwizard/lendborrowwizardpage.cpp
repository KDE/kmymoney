/***************************************************************************
                         lendborrowwizardpage  -  description
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

#include "lendborrowwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


LendBorrowWizardPage::LendBorrowWizardPage(QWidget *parent)
    : LendBorrowWizardPageDecl(parent)
{

  ButtonGroup1->setId(m_borrowButton, 0);
  ButtonGroup1->setId(m_lendButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("borrowButton", m_borrowButton);
  registerField("lendButton", m_lendButton);

  m_borrowButton->animateClick();
}

#include "lendborrowwizardpage.moc"

