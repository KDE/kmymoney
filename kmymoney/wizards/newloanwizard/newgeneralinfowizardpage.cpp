/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newgeneralinfowizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newgeneralinfowizardpage.h"

NewGeneralInfoWizardPage::NewGeneralInfoWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::NewGeneralInfoWizardPage)
{
  ui->setupUi(this);
}

NewGeneralInfoWizardPage::~NewGeneralInfoWizardPage()
{
  delete ui;
}
