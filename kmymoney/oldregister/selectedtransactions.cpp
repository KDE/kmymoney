/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
