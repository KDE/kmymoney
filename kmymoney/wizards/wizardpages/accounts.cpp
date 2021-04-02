/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "accounts.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_accounts.h"

Accounts::Accounts(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Accounts)
{
    ui->setupUi(this);
}

Accounts::~Accounts()
{
    delete ui;
}
