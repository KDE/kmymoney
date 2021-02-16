/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "variableinterestdatewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_variableinterestdatewizardpage.h"

#include "mymoneyaccountloan.h"

VariableInterestDateWizardPage::VariableInterestDateWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::VariableInterestDateWizardPage)
{
  ui->setupUi(this);
  ui->m_interestFrequencyUnitEdit->insertItem(i18nc("Occurrence period 'days'", "Days"), static_cast<int>(MyMoneyAccountLoan::changeDaily));
  ui->m_interestFrequencyUnitEdit->insertItem(i18n("Weeks"), static_cast<int>(MyMoneyAccountLoan::changeWeekly));
  ui->m_interestFrequencyUnitEdit->insertItem(i18n("Months"), static_cast<int>(MyMoneyAccountLoan::changeMonthly));
  ui->m_interestFrequencyUnitEdit->insertItem(i18n("Years"), static_cast<int>(MyMoneyAccountLoan::changeYearly));

  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
  //registerField("onlineFactor", m_onlineFactor, "value");
  registerField("interestChangeDateEdit", ui->m_interestChangeDateEdit, "date");
  registerField("interestFrequencyAmountEdit", ui->m_interestFrequencyAmountEdit, "value");
  registerField("interestFrequencyUnitEdit", ui->m_interestFrequencyUnitEdit, "currentItem");

  ui->m_interestFrequencyAmountEdit->setValue(1);
  ui->m_interestFrequencyUnitEdit->setCurrentItem(static_cast<int>(MyMoneyAccountLoan::changeYearly));
}

VariableInterestDateWizardPage::~VariableInterestDateWizardPage()
{
  delete ui;
}
