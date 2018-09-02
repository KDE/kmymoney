/*
 * Copyright 2006-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef INVESTTRANSACTION_P_H
#define INVESTTRANSACTION_P_H

#include "transaction_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyenums.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"

using namespace KMyMoneyRegister;

namespace KMyMoneyRegister
{
  class InvestTransactionPrivate : public TransactionPrivate
  {
  public:
    QList<MyMoneySplit>       m_feeSplits;
    QList<MyMoneySplit>       m_interestSplits;
    MyMoneySplit              m_assetAccountSplit;
    MyMoneySecurity           m_security;
    MyMoneySecurity           m_currency;
    eMyMoney::Split::InvestmentTransactionType    m_transactionType;
    QString                   m_feeCategory;
    QString                   m_interestCategory;
    MyMoneyMoney              m_feeAmount;
    MyMoneyMoney              m_interestAmount;
    MyMoneyMoney              m_totalAmount;
  };
}

#endif
