/***************************************************************************
                          scheduledtransaction.h
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

#ifndef SCHEDULEDTRANSACTION_H
#define SCHEDULEDTRANSACTION_H

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

class StdTransactionScheduled : public StdTransaction
{
public:
  StdTransactionScheduled(Register* parent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
  virtual ~StdTransactionScheduled() {}

  virtual const char* className() {
    return "StdTransactionScheduled";
  }

  virtual bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index);

  bool isSelectable() const {
    return true;
  }
  bool canHaveFocus() const {
    return true;
  }
  virtual bool isScheduled() const {
    return true;
  }

  virtual int sortSamePostDate() const {
    return 4;
  }

//   virtual void paintRegisterGrid(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg) const;

//   void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0);
};

} // namespace

#endif
