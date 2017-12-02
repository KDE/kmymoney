/***************************************************************************
                          mymoneysecurity.cpp  -  description
                             -------------------
    begin                : Tue Jan 29 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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
