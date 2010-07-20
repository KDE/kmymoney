/***************************************************************************
                        interestchargecheckingswizardpage.cpp - description
                            -------------------
   begin                : Sun Jul 18 2010
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

#include "interestchargecheckingswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes


InterestChargeCheckingsWizardPage::InterestChargeCheckingsWizardPage(QWidget *parent)
    : InterestChargeCheckingsWizardPageDecl(parent)
{

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  registerField("interestDateEdit", m_interestDateEdit, "date", SIGNAL(dateChanged(const QDate&)));
  registerField("chargesDateEdit", m_chargesDateEdit, "date", SIGNAL(dateChanged(const QDate&)));

  registerField("interestEdit", m_interestEdit, "value", SIGNAL(textChanged()));
  registerField("interestEditValid", m_interestEdit, "valid", SIGNAL(textChanged()));
  registerField("chargesEdit", m_chargesEdit, "value", SIGNAL(textChanged()));
  registerField("chargesEditValid", m_chargesEdit, "valid", SIGNAL(textChanged()));

  registerField("interestCategoryEdit", m_interestCategoryEdit, "selectedItem", SIGNAL(itemSelected(const QString&)));
  registerField("chargesCategoryEdit", m_chargesCategoryEdit, "selectedItem", SIGNAL(itemSelected(const QString&)));

  registerField("payeeEdit", m_payeeEdit, "selectedItem", SIGNAL(itemSelected(const QString&)));

  connect(m_interestEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(completeChanged()));
  connect(m_interestCategoryEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(completeChanged()));
  connect(m_chargesEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(completeChanged()));
  connect(m_chargesCategoryEdit, SIGNAL(textChanged(const QString&)), this, SIGNAL(completeChanged()));


}

bool InterestChargeCheckingsWizardPage::isComplete() const
{
  int cnt1, cnt2;
  cnt1 = !m_interestEdit->value().isZero() + !m_interestCategoryEdit->selectedItem().isEmpty();
  cnt2 = !m_chargesEdit->value().isZero() + !m_chargesCategoryEdit->selectedItem().isEmpty();
  if (cnt1 == 1 || cnt2 == 1)
    return false;

  return true;
}

#include "interestchargecheckingswizardpage.moc"

