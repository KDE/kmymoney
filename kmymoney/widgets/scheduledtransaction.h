/*
 * SPDX-FileCopyrightText: 2008-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

    const char* className() override;

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
