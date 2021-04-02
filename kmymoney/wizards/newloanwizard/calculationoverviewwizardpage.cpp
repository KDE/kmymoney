/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "calculationoverviewwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_calculationoverviewwizardpage.h"

CalculationOverviewWizardPage::CalculationOverviewWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::CalculationOverviewWizardPage)
{
    ui->setupUi(this);
    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("loanAmount6", ui->m_loanAmount6, "text");
    registerField("interestRate6", ui->m_interestRate6, "text");
    registerField("duration6", ui->m_duration6, "text");
    registerField("payment6", ui->m_payment6, "text");
    registerField("balloon6", ui->m_balloon6, "text");
}

CalculationOverviewWizardPage::~CalculationOverviewWizardPage()
{
    delete ui;
}
