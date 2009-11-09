/***************************************************************************
                             accounts.h
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

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_accountsdecl.h"

/**
  * @author Thomas Baumgart
  */

class AccountsDecl : public QWidget, public Ui::AccountsDecl
{
public:
  AccountsDecl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};

class Accounts : public AccountsDecl
{
  Q_OBJECT
public:
  Accounts(QWidget* parent = 0);

private:
};


#endif
