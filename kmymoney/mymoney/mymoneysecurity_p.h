/*
    SPDX-FileCopyrightText: 2005-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYSECURITY_P_H
#define MYMONEYSECURITY_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include <alkimia/alkvalue.h>
#include "mymoneyenums.h"

using namespace eMyMoney;

class MyMoneySecurityPrivate : public MyMoneyObjectPrivate
{
public:

    MyMoneySecurityPrivate() :
        m_securityType(eMyMoney::Security::Type::None),
        m_smallestCashFraction(100),
        m_smallestAccountFraction(100),
        m_pricePrecision(4),
        m_roundingMethod(AlkValue::RoundRound)
    {
    }

    void collectReferencedObjects() override
    {
        m_referencedObjects = {m_tradingCurrency};
    }

    QString                   m_name;
    QString                   m_tradingSymbol;
    QString                   m_tradingMarket;
    QString                   m_tradingCurrency;
    eMyMoney::Security::Type        m_securityType;
    int                       m_smallestCashFraction;
    int                       m_smallestAccountFraction;
    int                       m_pricePrecision;
    AlkValue::RoundingMethod  m_roundingMethod;
};

#endif
