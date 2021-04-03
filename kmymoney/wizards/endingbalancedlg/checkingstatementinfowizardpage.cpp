/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkingstatementinfowizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_checkingstatementinfowizardpage.h"

CheckingStatementInfoWizardPage::CheckingStatementInfoWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::CheckingStatementInfoWizardPage)
{
    ui->setupUi(this);
    ui->m_statementDate->setDate(QDate::currentDate());

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("statementDate", ui->m_statementDate, "date", SIGNAL(dateChanged(QDate)));

    registerField("endingBalance", ui->m_endingBalance, "value", SIGNAL(textChanged()));
    registerField("endingBalanceValid", ui->m_endingBalance, "valid", SIGNAL(textChanged()));
    registerField("previousBalance", ui->m_previousBalance, "value", SIGNAL(textChanged()));
    registerField("previousBalanceValid", ui->m_previousBalance, "valid", SIGNAL(textChanged()));
}

CheckingStatementInfoWizardPage::~CheckingStatementInfoWizardPage()
{
    delete ui;
}

