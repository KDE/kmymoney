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

#ifndef SCHEDULEDTRANSACTION_H
#define SCHEDULEDTRANSACTION_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "stdtransaction.h"

namespace KMyMoneyRegister
{

  class KMM_OLDREGISTER_EXPORT StdTransactionScheduled : public StdTransaction
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
