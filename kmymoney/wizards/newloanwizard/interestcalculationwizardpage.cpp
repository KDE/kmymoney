/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interestcalculationwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestcalculationwizardpage.h"

InterestCalculationWizardPage::InterestCalculationWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::InterestCalculationWizardPage)
{
    ui->setupUi(this);
    ui->ButtonGroup5->setId(ui->m_interestOnReceptionButton, 0);
    ui->ButtonGroup5->setId(ui->m_interestOnPaymentButton, 1);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("interestOnReceptionButton", ui->m_interestOnReceptionButton);
    registerField("interestOnPaymentButton", ui->m_interestOnPaymentButton);

    ui->m_interestOnReceptionButton->click();
}

InterestCalculationWizardPage::~InterestCalculationWizardPage()
{
    delete ui;
}
