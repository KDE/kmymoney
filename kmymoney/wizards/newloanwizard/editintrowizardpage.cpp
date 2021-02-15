/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "editintrowizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_editintrowizardpage.h"

EditIntroWizardPage::EditIntroWizardPage(QWidget *parent)
  : QWizardPage(parent),
    ui(new Ui::EditIntroWizardPage)
{
  ui->setupUi(this);
}

EditIntroWizardPage::~EditIntroWizardPage()
{
  delete ui;
}
