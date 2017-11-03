/***************************************************************************
                         effectivedatewizardpage  -  description
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

#include "effectivedatewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewloanwizard.h"
#include "mymoneyaccountloan.h"

EffectiveDateWizardPage::EffectiveDateWizardPage(QWidget *parent)
    : EffectiveDateWizardPageDecl(parent)
{
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("effectiveChangeDateEdit", m_effectiveChangeDateEdit, "date", SIGNAL(dateChanged(QDate)));
  connect(m_effectiveChangeDateEdit, SIGNAL(dateChanged(QDate)), this, SIGNAL(completeChanged()));
}

void EffectiveDateWizardPage::initializePage()
{
  m_effectiveDateLabel->setText(QString("\n") + i18n(
                                  "Please enter the date from which on the following changes will be effective. "
                                  "The date entered must be later than the opening date of this account (%1), but must "
                                  "not be in the future. The default will be today.", QLocale().toString(qobject_cast<KNewLoanWizard*>(wizard())->account().openingDate())));
}

/**
 * Update the "Next" button
 */
bool EffectiveDateWizardPage::isComplete() const
{
  return !(m_effectiveChangeDateEdit->date() < qobject_cast<KNewLoanWizard*>(wizard())->account().openingDate()
           || m_effectiveChangeDateEdit->date() > QDate::currentDate());
  return true;
}
