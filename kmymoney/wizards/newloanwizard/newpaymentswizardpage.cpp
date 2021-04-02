/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newpaymentswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newpaymentswizardpage.h"

NewPaymentsWizardPage::NewPaymentsWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::NewPaymentsWizardPage)
{
    ui->setupUi(this);
}

NewPaymentsWizardPage::~NewPaymentsWizardPage()
{
    delete ui;
}
