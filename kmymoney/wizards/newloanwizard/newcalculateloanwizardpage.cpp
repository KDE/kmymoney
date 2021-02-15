/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newcalculateloanwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newcalculateloanwizardpage.h"

NewCalculateLoanWizardPage::NewCalculateLoanWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::NewCalculateLoanWizardPage)
{
  ui->setupUi(this);
}

NewCalculateLoanWizardPage::~NewCalculateLoanWizardPage()
{
  delete ui;
}
