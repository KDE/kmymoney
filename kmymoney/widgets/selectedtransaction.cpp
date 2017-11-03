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

#include "selectedtransaction.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "register.h"
#include "mymoneysplit.h"
#include "mymoneyfile.h"

namespace KMyMoneyRegister
{

int SelectedTransaction::warnLevel() const
{
  auto warnLevel = 0;
  foreach (const auto split, transaction().splits()) {
    try {
      auto acc = MyMoneyFile::instance()->account(split.accountId());
      if (acc.isClosed())
        warnLevel = 3;
      else if (split.reconcileFlag() == eMyMoney::Split::State::Frozen)
        warnLevel = 2;
      else if (split.reconcileFlag() == eMyMoney::Split::State::Reconciled && warnLevel < 1)
        warnLevel = 1;
    } catch (const MyMoneyException &) {
      //qDebug("Exception in SelectedTransaction::warnLevel(): %s", qPrintable(e.what()));
      warnLevel = 0;
    }
  }
  return warnLevel;
}

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

} // namespace
