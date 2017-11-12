/***************************************************************************
                          selectedtransaction.cpp  -  description
                             -------------------
    begin                : Fri Jun 2008
    copyright            : (C) 2000-2008 by Thomas Baumgart
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

#include "selectedtransactions.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"

using namespace KMyMoneyRegister;

SelectedTransactions::SelectedTransactions(const Register* r)
{
  r->selectedTransactions(*this);
}

int SelectedTransactions::warnLevel() const
{
  int warnLevel = 0;
  SelectedTransactions::const_iterator it_t;
  for (it_t = begin(); warnLevel < 3 && it_t != end(); ++it_t) {
    int thisLevel = (*it_t).warnLevel();
    if (thisLevel > warnLevel)
      warnLevel = thisLevel;
  }
  return warnLevel;
}

bool SelectedTransactions::canModify() const
{
  return warnLevel() < 2;
}

bool SelectedTransactions::canDuplicate() const
{
  return warnLevel() < 3;
}
