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

#ifndef STDTRANSACTIONDOWNLOADED_H
#define STDTRANSACTIONDOWNLOADED_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "stdtransaction.h"
#include "investtransaction.h"

namespace KMyMoneyRegister
{

  class StdTransactionDownloaded : public StdTransaction
  {
  public:
    explicit StdTransactionDownloaded(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~StdTransactionDownloaded() override;

    const char* className() override;

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
  };

  class InvestTransactionDownloaded : public InvestTransaction
  {
  public:
    explicit InvestTransactionDownloaded(Register* getParent, const MyMoneyTransaction& transaction, const MyMoneySplit& split, int uniqueId);
    ~InvestTransactionDownloaded() override;

    const char* className() override;

    bool paintRegisterCellSetup(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) override;
  };


} // namespace

#endif
