/***************************************************************************
                         variableinterestdatewizardpage  -  description
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

#include "variableinterestdatewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccountloan.h"

VariableInterestDateWizardPage::VariableInterestDateWizardPage(QWidget *parent)
    : VariableInterestDateWizardPageDecl(parent)
{

  m_interestFrequencyUnitEdit->insertItem(i18nc("Occurrence period 'days'", "Days"), static_cast<int>(MyMoneyAccountLoan::changeDaily));
  m_interestFrequencyUnitEdit->insertItem(i18n("Weeks"), static_cast<int>(MyMoneyAccountLoan::changeWeekly));
  m_interestFrequencyUnitEdit->insertItem(i18n("Months"), static_cast<int>(MyMoneyAccountLoan::changeMonthly));
  m_interestFrequencyUnitEdit->insertItem(i18n("Years"), static_cast<int>(MyMoneyAccountLoan::changeYearly));

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  //registerField("onlineFactor", m_onlineFactor, "value");
  registerField("interestChangeDateEdit", m_interestChangeDateEdit, "date");
  registerField("interestFrequencyAmountEdit", m_interestFrequencyAmountEdit, "value");
  registerField("interestFrequencyUnitEdit", m_interestFrequencyUnitEdit, "currentItem");

  m_interestFrequencyAmountEdit->setValue(1);
  m_interestFrequencyUnitEdit->setCurrentItem(static_cast<int>(MyMoneyAccountLoan::changeYearly));

}
