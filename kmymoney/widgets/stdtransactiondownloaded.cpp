/***************************************************************************
                          stdtransactiondownloaded.cpp
                             -------------------
    begin                : Sun May 11 2008
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

#include "stdtransactiondownloaded.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocale>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoneyglobalsettings.h>
#include <register.h>

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionDownloaded::StdTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
    StdTransaction(parent, transaction, split, uniqueId)
{
}

bool StdTransactionDownloaded::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItemV4 &option, const QModelIndex &index)

{
  bool rc = Transaction::paintRegisterCellSetup(painter, option, index);
  // if not selected paint in selected background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::importedTransactionColor());
    option.palette.setColor(QPalette::AlternateBase, KMyMoneyGlobalSettings::importedTransactionColor());
  }
  return rc;
}

InvestTransactionDownloaded::InvestTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
    InvestTransaction(parent, transaction, split, uniqueId)
{
}

bool InvestTransactionDownloaded::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItemV4 &option, const QModelIndex &index)

{
  bool rc = Transaction::paintRegisterCellSetup(painter, option, index);
  // if not selected paint in selected background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::importedTransactionColor());
    option.palette.setColor(QPalette::AlternateBase, KMyMoneyGlobalSettings::importedTransactionColor());
  }
  return rc;
}

