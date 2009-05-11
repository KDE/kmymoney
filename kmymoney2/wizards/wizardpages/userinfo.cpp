/***************************************************************************
                             userinfo.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "userinfo.h"
#include <kmymoney/mymoneypayee.h>

UserInfo::UserInfo(QWidget* parent, const char* name) :
  UserInfoDecl(parent, name)
{
  m_userNameEdit->setFocus();
}

MyMoneyPayee UserInfo::user(void) const
{
  MyMoneyPayee user;
  user.setName(m_userNameEdit->text());
  user.setAddress(m_streetEdit->text());
  user.setCity(m_townEdit->text());
  user.setState(m_countyEdit->text());
  user.setPostcode(m_postcodeEdit->text());
  user.setTelephone(m_telephoneEdit->text());
  user.setEmail(m_emailEdit->text());
  return user;
}

#include "userinfo.moc"
