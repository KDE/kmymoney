/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

SelectedTransaction::warnLevel_t SelectedTransactions::warnLevel() const
{
  SelectedTransaction::warnLevel_t warnLevel = SelectedTransaction::NoWarning;
  SelectedTransactions::const_iterator it_t;
  for (it_t = begin(); warnLevel < SelectedTransaction::OneAccountClosed && it_t != end(); ++it_t) {
    SelectedTransaction::warnLevel_t thisLevel = (*it_t).warnLevel();
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
