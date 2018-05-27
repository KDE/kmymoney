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

namespace eMyMoney
{
  namespace Security
  {
    enum class Attribute { Name = 0,
                           Symbol,
                           Type,
                           RoundingMethod,
                           SAF,
                           PP,
                           SCF,
                           TradingCurrency,
                           TradingMarket,
                           // insert new entries above this line
                           LastAttribute
                         };
    uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
  }
}

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

  static QString getAttrName(const Security::Attribute attr)
  {
    static const QHash<Security::Attribute, QString> attrNames {
      {Security::Attribute::Name,             QStringLiteral("name")},
      {Security::Attribute::Symbol,           QStringLiteral("symbol")},
      {Security::Attribute::Type,             QStringLiteral("type")},
      {Security::Attribute::RoundingMethod,   QStringLiteral("rounding-method")},
      {Security::Attribute::SAF,              QStringLiteral("saf")},
      {Security::Attribute::PP,               QStringLiteral("pp")},
      {Security::Attribute::SCF,              QStringLiteral("scf")},
      {Security::Attribute::TradingCurrency,  QStringLiteral("trading-currency")},
      {Security::Attribute::TradingMarket,    QStringLiteral("trading-market")}
    };
    return attrNames[attr];
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
