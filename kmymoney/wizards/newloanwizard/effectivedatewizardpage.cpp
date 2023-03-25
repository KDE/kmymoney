/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
#include "mymoneyutils.h"

EffectiveDateWizardPage::EffectiveDateWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::EffectiveDateWizardPage)
{
    ui->setupUi(this);
    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("effectiveChangeDateEdit", ui->m_effectiveChangeDateEdit, "date", SIGNAL(dateChanged(QDate)));
    connect(ui->m_effectiveChangeDateEdit, &KMyMoneyDateEdit::dateChanged, this, &QWizardPage::completeChanged);
}

EffectiveDateWizardPage::~EffectiveDateWizardPage()
{
    delete ui;
}

void EffectiveDateWizardPage::initializePage()
{
    ui->m_effectiveDateLabel->setText(QString("\n")
                                      + i18n("Please enter the date from which on the following changes will be effective. "
                                             "The date entered must be later than the opening date of this account (%1), but must "
                                             "not be in the future. The default will be today.",
                                             MyMoneyUtils::formatDate(qobject_cast<KNewLoanWizard*>(wizard())->account().openingDate(), QLocale::LongFormat)));
}

/**
 * Update the "Next" button
 */
bool EffectiveDateWizardPage::isComplete() const
{
    if (!ui->m_effectiveChangeDateEdit->isValid())
        return false;
    return !(ui->m_effectiveChangeDateEdit->date() < qobject_cast<KNewLoanWizard*>(wizard())->account().openingDate()
             || ui->m_effectiveChangeDateEdit->date() > QDate::currentDate());
    return true;
}
