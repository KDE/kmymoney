/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interesteditwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_interesteditwizardpage.h"

InterestEditWizardPage::InterestEditWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::InterestEditWizardPage)
{
    ui->setupUi(this);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("newInterestRateEdit", ui->m_newInterestRateEdit, "value", SIGNAL(textChanged(QString)));
    connect(ui->m_newInterestRateEdit, &AmountEdit::textChanged, this, &QWizardPage::completeChanged);

    ui->m_newInterestRateEdit->setAllowEmpty(true);
}

InterestEditWizardPage::~InterestEditWizardPage()
{
    delete ui;
}

/**
 * Update the "Next" button
 */
bool InterestEditWizardPage::isComplete() const
{
    //FIXME: this only exists in the EditLoanWizard subclass
    return field("newPaymentEditValid").toBool()
           || ui->m_newInterestRateEdit->isValid();
}
