/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "loanamountwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_loanamountwizardpage.h"

LoanAmountWizardPage::LoanAmountWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::LoanAmountWizardPage)
{
    ui->setupUi(this);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("loanAmountEdit", ui->m_loanAmountEdit, "value", SIGNAL(textChanged()));
    registerField("loanAmountEditValid", ui->m_loanAmountEdit, "valid", SIGNAL(textChanged()));

    connect(ui->m_loanAmountEdit, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);

    registerField("loanAmount1", ui->m_loanAmount1, "text");
    registerField("interestRate1", ui->m_interestRate1, "text");
    registerField("duration1", ui->m_duration1, "text");
    registerField("payment1", ui->m_payment1, "text");
    registerField("balloon1", ui->m_balloon1, "text");
}

LoanAmountWizardPage::~LoanAmountWizardPage()
{
    delete ui;
}

/**
 * Update the "Next" button
 */
bool LoanAmountWizardPage::isComplete() const
{
    return !(field("thisYearPaymentButton").toBool()
             && !ui->m_loanAmountEdit->isValid());
}

void LoanAmountWizardPage::initializePage()
{
    if (field("allPaymentsButton").toBool()) {
        ui->m_balanceLabel->setText(
            QString("\n") +
            i18n("Please enter the original loan amount in the field below or leave it "
                 "empty to be calculated."));
    } else if (field("thisYearPaymentButton").toBool()) {
        ui->m_balanceLabel->setText(QString("\n") +
                                    i18n("Please enter the remaining loan amount of last years final "
                                         "statement in the field below. You should not leave this field empty."));

    }
}

void LoanAmountWizardPage::resetCalculator()
{
    ui->m_loanAmount1->setText(QString());
    ui->m_interestRate1->setText(QString());
    ui->m_duration1->setText(QString());
    ui->m_payment1->setText(QString());
    ui->m_balloon1->setText(QString());
}
