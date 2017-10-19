/***************************************************************************
                          stdtransactiondownloaded.h
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

#ifndef STDTRANSACTIONDOWNLOADED_H
#define STDTRANSACTIONDOWNLOADED_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "transaction.h"

namespace KMyMoneyTransactionForm
{
class TransactionForm;
} // namespace

namespace KMyMoneyRegister
{

class StdTransactionDownloaded : public StdTransaction
{
public:
  StdTransactionDownloaded(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransactionDownloaded() {}

  virtual const char* className() {
    return "StdTransactionDownloaded";
  }

  virtual bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index);
};

class InvestTransactionDownloaded : public InvestTransaction
{
public:
  InvestTransactionDownloaded(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~InvestTransactionDownloaded() {}

  virtual const char* className() {
    return "InvestTransactionDownloaded";
  }

  virtual bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index);
};


} // namespace

#endif
