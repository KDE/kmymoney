/***************************************************************************
                             accounts.cpp
                             -------------------
    begin                : Fri Jun  1 2007
    copyright            : (C) 2007 Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "accounts.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <q3header.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <k3listview.h>
#include <kmymoneyaccounttree.h>

// ----------------------------------------------------------------------------
// Project Includes

Accounts::Accounts(QWidget* parent) :
  AccountsDecl(parent)
{
}

#include "accounts.moc"
