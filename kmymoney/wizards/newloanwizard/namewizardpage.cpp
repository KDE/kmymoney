/***************************************************************************
                         namewizardpage  -  description
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

#include "namewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes


NameWizardPage::NameWizardPage(QWidget *parent)
    : NameWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("payeeEdit", m_payeeEdit, "selectedItem", SIGNAL(itemSelected(QString)));
  connect(m_nameEdit, SIGNAL(textChanged(QString)), this, SIGNAL(completeChanged()));
}

/**
 * Update the "Next" button
 */
bool NameWizardPage::isComplete() const
{
  return !m_nameEdit->text().isEmpty();
}

void NameWizardPage::initializePage()
{
  if (field("borrowButton").toBool()) {
    m_generalReceiverText->setText(i18n("To whom do you make payments?"));
    m_receiverLabel->setText(i18n("Payments to"));
  } else if (field("lendButton").toBool()) {
    m_generalReceiverText->setText(i18n("From whom do you expect payments?"));
    m_receiverLabel->setText(i18n("Payments from"));
  }
  m_nameEdit->setFocus();
}
