/***************************************************************************
                          scheduledtransaction.cpp
                             -------------------
    begin                : Tue Aug 19 2008
    copyright            : (C) 2008 by Thomas Baumgart
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

#include "scheduledtransaction.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"
#include "transaction.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionScheduled::StdTransactionScheduled(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
    StdTransaction(parent, transaction, split, uniqueId)
{
  // setup initial size
  setNumRowsRegister(numRowsRegister(KMyMoneyGlobalSettings::showRegisterDetailed()));
}

bool StdTransactionScheduled::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  bool rc = Transaction::paintRegisterCellSetup(painter, option, index);
  option.palette.setCurrentColorGroup(QPalette::Disabled);
  return rc;
}


