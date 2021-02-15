/***************************************************************************
                             accounts.cpp
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

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
