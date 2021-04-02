/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lendborrowwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_lendborrowwizardpage.h"

LendBorrowWizardPage::LendBorrowWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::LendBorrowWizardPage)
{
    ui->setupUi(this);

    ui->ButtonGroup1->setId(ui->m_borrowButton, 0);
    ui->ButtonGroup1->setId(ui->m_lendButton, 1);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("borrowButton", ui->m_borrowButton);
    registerField("lendButton", ui->m_lendButton);

    ui->m_borrowButton->click();
}

LendBorrowWizardPage::~LendBorrowWizardPage()
{
    delete ui;
}
