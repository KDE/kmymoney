/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checkingstartwizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_checkingstartwizardpage.h"

CheckingStartWizardPage::CheckingStartWizardPage(QWidget *parent) :
  QWizardPage(parent),
  ui(new Ui::CheckingStartWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly
}

CheckingStartWizardPage::~CheckingStartWizardPage()
{
  delete ui;
}
