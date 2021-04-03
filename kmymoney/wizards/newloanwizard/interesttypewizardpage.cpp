/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interesttypewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interesttypewizardpage.h"

InterestTypeWizardPage::InterestTypeWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::InterestTypeWizardPage)
{
    ui->setupUi(this);

    ui->ButtonGroup2->setId(ui->m_fixedInterestButton, 0);
    ui->ButtonGroup2->setId(ui->m_variableInterestButton, 1);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("fixedInterestButton", ui->m_fixedInterestButton);
    registerField("variableInterestButton", ui->m_variableInterestButton);

    ui->m_fixedInterestButton->click();
}

InterestTypeWizardPage::~InterestTypeWizardPage()
{
    delete ui;
}
