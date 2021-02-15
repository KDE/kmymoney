/*
 * SPDX-FileCopyrightText: 2006-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
