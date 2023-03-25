/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "summaryeditwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_summaryeditwizardpage.h"

#include "mymoneyutils.h"

SummaryEditWizardPage::SummaryEditWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::SummaryEditWizardPage)
{
    ui->setupUi(this);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("affectedPayments", ui->m_affectedPayments, "text");
}

SummaryEditWizardPage::~SummaryEditWizardPage()
{
    delete ui;
}

void SummaryEditWizardPage::initializePage()
{
    ui->m_payment7->setText(field("payment6").toString());
    ui->m_additionalFees7->setText(field("additionalCost").toString());
    ui->m_totalPayment7->setText(field("periodicPayment").toString());
    ui->m_interestRate7->setText(field("interestRate6").toString());
    ui->m_startDateChanges->setText(MyMoneyUtils::formatDate(field("effectiveChangeDateEdit").toDate(), QLocale::LongFormat));
}
