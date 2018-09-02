/*
 * Copyright 2005-2017  Thomas Baumgart <tbaumgart@kde.org>
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
