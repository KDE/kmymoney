/*
 * Copyright 2008-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "scheduledtransaction.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QStyleOptionViewItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionScheduled::StdTransactionScheduled(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  StdTransaction(parent, transaction, split, uniqueId)
{
  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneySettings::showRegisterDetailed()));
}

StdTransactionScheduled::~StdTransactionScheduled()
{
}

const char* StdTransactionScheduled::className()
{
  return "StdTransactionScheduled";
}

bool StdTransactionScheduled::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  auto rc = Transaction::paintRegisterCellSetup(painter, option, index);
  option.palette.setCurrentColorGroup(QPalette::Disabled);
  return rc;
}

bool StdTransactionScheduled::isSelectable() const
{
  return true;
}

bool StdTransactionScheduled::canHaveFocus() const
{
  return true;
}

bool StdTransactionScheduled::isScheduled() const
{
  return true;
}

int StdTransactionScheduled::sortSamePostDate() const
{
  return 4;
}


