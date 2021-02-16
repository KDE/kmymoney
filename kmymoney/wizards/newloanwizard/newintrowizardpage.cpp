/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newintrowizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_newintrowizardpage.h"

NewIntroWizardPage::NewIntroWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::NewIntroWizardPage)
{
  ui->setupUi(this);
}

NewIntroWizardPage::~NewIntroWizardPage()
{
  delete ui;
}
