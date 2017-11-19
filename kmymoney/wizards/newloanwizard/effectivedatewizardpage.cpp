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

#include "ui_effectivedatewizardpage.h"

#include "knewloanwizard.h"
#include "mymoneyaccountloan.h"

EffectiveDateWizardPage::EffectiveDateWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::EffectiveDateWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("effectiveChangeDateEdit", ui->m_effectiveChangeDateEdit, "date", SIGNAL(dateChanged(QDate)));
  connect(ui->m_effectiveChangeDateEdit, &KMyMoneyDateInput::dateChanged, this, &QWizardPage::completeChanged);
}

EffectiveDateWizardPage::~EffectiveDateWizardPage()
{
  delete ui;
}

void EffectiveDateWizardPage::initializePage()
{
  ui->m_effectiveDateLabel->setText(QString("\n") + i18n(
                                  "Please enter the date from which on the following changes will be effective. "
                                  "The date entered must be later than the opening date of this account (%1), but must "
                                  "not be in the future. The default will be today.", QLocale().toString(qobject_cast<KNewLoanWizard*>(wizard())->account().openingDate())));
}

/**
 * Update the "Next" button
 */
bool EffectiveDateWizardPage::isComplete() const
{
  return !(ui->m_effectiveChangeDateEdit->date() < qobject_cast<KNewLoanWizard*>(wizard())->account().openingDate()
           || ui->m_effectiveChangeDateEdit->date() > QDate::currentDate());
  return true;
}
