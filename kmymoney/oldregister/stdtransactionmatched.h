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

#ifndef STDTRANSACTIONMATCHED_H
#define STDTRANSACTIONMATCHED_H

#include "kmm_oldregister_export.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "stdtransaction.h"

class MyMoneySplit;
class MyMoneyTransaction;

namespace KMyMoneyRegister
{

  class Register;
  class KMM_OLDREGISTER_EXPORT StdTransactionMatched : public StdTransaction
  {
    static const int m_additionalRows = 3;

  public:
    explicit StdTransactionMatched(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~StdTransactionMatched() override;

    const char* className() override;

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;

    void registerCellText(QString& txt, Qt::Alignment& align, int row, int col, QPainter* painter = 0) override;

    /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister(bool)
    */
    int numRowsRegister(bool expanded) const override;

    /**
    * Provided for internal reasons. No API change. See RegisterItem::numRowsRegister()
    */
    int numRowsRegister() const override;
  };

} // namespace

#endif
