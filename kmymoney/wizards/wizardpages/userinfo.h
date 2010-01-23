/***************************************************************************
                             userinfo.h
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

#ifndef USERINFO_H
#define USERINFO_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_userinfodecl.h"
class MyMoneyPayee;

/**
  * @author Thomas Baumgart
  */

class UserInfoDecl : public QWidget, public Ui::UserInfoDecl
{
public:
  UserInfoDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};
class UserInfo : public UserInfoDecl
{
  Q_OBJECT
public:
  UserInfo(QWidget* parent = 0);
  MyMoneyPayee user(void) const;

private:
};


#endif
