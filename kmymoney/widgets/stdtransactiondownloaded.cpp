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

#include <QStyleOptionViewItem>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"

using namespace KMyMoneyRegister;
using namespace KMyMoneyTransactionForm;

StdTransactionDownloaded::StdTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  StdTransaction(parent, transaction, split, uniqueId)
{
}

StdTransactionDownloaded::~StdTransactionDownloaded()
{
}

const char* StdTransactionDownloaded::className()
{
  return "StdTransactionDownloaded";
}

bool StdTransactionDownloaded::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)

{
  auto rc = Transaction::paintRegisterCellSetup(painter, option, index);
  // if not selected paint in selected background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionImported));
    option.palette.setColor(QPalette::AlternateBase, KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionImported));
  }
  return rc;
}

InvestTransactionDownloaded::InvestTransactionDownloaded(Register *parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId) :
  InvestTransaction(parent, transaction, split, uniqueId)
{
}

InvestTransactionDownloaded::~InvestTransactionDownloaded()
{
}

const char* InvestTransactionDownloaded::className()
{
  return "InvestTransactionDownloaded";
}

bool InvestTransactionDownloaded::paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index)
{
  auto rc = Transaction::paintRegisterCellSetup(painter, option, index);
  // if not selected paint in selected background color
  if (!isSelected()) {
    option.palette.setColor(QPalette::Base, KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionImported));
    option.palette.setColor(QPalette::AlternateBase, KMyMoneyGlobalSettings::schemeColor(SchemeColor::TransactionImported));
  }
  return rc;
}

