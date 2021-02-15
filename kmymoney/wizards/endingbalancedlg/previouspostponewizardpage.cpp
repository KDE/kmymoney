/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "previouspostponewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_previouspostponewizardpage.h"

PreviousPostponeWizardPage::PreviousPostponeWizardPage(QWidget *parent) :
  QWizardPage(parent),
  ui(new Ui::PreviousPostponeWizardPage)
{
  ui->setupUi(this);
  // Register the fields with the QWizard and connect the
  // appropriate signals to update the "Next" button correctly

}

PreviousPostponeWizardPage::~PreviousPostponeWizardPage()
{
  delete ui;
}
