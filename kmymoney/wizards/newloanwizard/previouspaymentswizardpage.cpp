/***************************************************************************
                         previouspaymentswizardpage  -  description
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

#include "previouspaymentswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


PreviousPaymentsWizardPage::PreviousPaymentsWizardPage(QWidget *parent)
    : PreviousPaymentsWizardPageDecl(parent)
{
  ButtonGroup3->setId(m_noPreviousPaymentButton, 0);
  ButtonGroup3->setId(m_previousPaymentButton, 1);

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("noPreviousPaymentButton", m_noPreviousPaymentButton);
  registerField("previousPaymentButton", m_previousPaymentButton);

  m_noPreviousPaymentButton->click();
}
