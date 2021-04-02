/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interestwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interestwizardpage.h"

InterestWizardPage::InterestWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::InterestWizardPage)
{
    ui->setupUi(this);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("interestRateEdit", ui->m_interestRateEdit, "value", SIGNAL(textChanged()));
    registerField("interestRateEditValid", ui->m_interestRateEdit, "valid", SIGNAL(textChanged()));

    registerField("loanAmount2", ui->m_loanAmount2, "text");
    registerField("interestRate2", ui->m_interestRate2, "text");
    registerField("duration2", ui->m_duration2, "text");
    registerField("payment2", ui->m_payment2, "text");
    registerField("balloon2", ui->m_balloon2, "text");
}

InterestWizardPage::~InterestWizardPage()
{
    delete ui;
}

void InterestWizardPage::initializePage()
{
    ui->m_interestRateEdit->setFocus();
}

void InterestWizardPage::resetCalculator()
{
    ui->m_loanAmount2->setText(QString());
    ui->m_interestRate2->setText(QString());
    ui->m_duration2->setText(QString());
    ui->m_payment2->setText(QString());
    ui->m_balloon2->setText(QString());
}
