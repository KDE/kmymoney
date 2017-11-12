/***************************************************************************
                          scheduledtransaction.h
                             -------------------
    begin                : Tue Aug 19 2008
    copyright            : (C) 2008 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "stdtransaction.h"

namespace KMyMoneyRegister
{

  class StdTransactionScheduled : public StdTransaction
  {
  public:
    explicit StdTransactionScheduled(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~StdTransactionScheduled() override;

    const char* className();

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;

    bool isSelectable() const override;
    bool canHaveFocus() const override;
    bool isScheduled() const override;
    int sortSamePostDate() const override;

    //   virtual void paintRegisterGrid(QPainter* painter, int row, int col, const QRect& r, const QColorGroup& cg) const;

    //   void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0);
  };

} // namespace

#endif
