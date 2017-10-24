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

#include "mymoneysecurity.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneystoragenames.h"

using namespace eMyMoney;
using namespace MyMoneyStorageNodes;

MyMoneySecurity::MyMoneySecurity() :
    m_securityType(eMyMoney::Security::None),
    m_smallestCashFraction(DEFAULT_CASH_FRACTION),
    m_smallestAccountFraction(DEFAULT_ACCOUNT_FRACTION),
    m_pricePrecision(DEFAULT_PRICE_PRECISION),
    m_roundingMethod(AlkValue::RoundRound)
{
}

MyMoneySecurity::MyMoneySecurity(const QString& id,
                                 const QString& name,
                                 const QString& symbol,
                                 const int smallestCashFraction,
                                 const int smallestAccountFraction,
                                 const int pricePrecision) :
    MyMoneyObject(id),
    m_name(name),
    m_securityType(eMyMoney::Security::Currency),
    m_smallestCashFraction(smallestCashFraction),
    m_smallestAccountFraction(smallestAccountFraction),
    m_pricePrecision(pricePrecision),
    m_roundingMethod(AlkValue::RoundRound)
{
  if (symbol.isEmpty())
    m_tradingSymbol = id;
  else
    m_tradingSymbol = symbol;

  if (smallestAccountFraction)
    m_smallestAccountFraction = smallestAccountFraction;
  else
    m_smallestAccountFraction = smallestCashFraction;
}

MyMoneySecurity::MyMoneySecurity(const QString& id, const MyMoneySecurity& equity) :
    MyMoneyObject(id)
{
  *this = equity;
  m_id = id;
}

MyMoneySecurity::MyMoneySecurity(const QDomElement& node) :
    MyMoneyObject(node),
    MyMoneyKeyValueContainer(node.elementsByTagName(nodeNames[nnKeyValuePairs]).item(0).toElement())
{
  {
    const auto tag = node.tagName();
    if ((nodeNames[nnSecurity] != tag)
        && (nodeNames[nnEquity] != tag)
        && (nodeNames[nnCurrency] != tag))
      throw MYMONEYEXCEPTION("Node was not SECURITY or CURRENCY");
  }

  m_name = node.attribute(getAttrName(Attribute::Name));
  m_tradingSymbol = node.attribute(getAttrName(Attribute::Symbol));
  m_securityType = static_cast<eMyMoney::Security>(node.attribute(getAttrName(Attribute::Type)).toInt());
  m_roundingMethod = static_cast<AlkValue::RoundingMethod>(node.attribute(getAttrName(Attribute::RoundingMethod)).toInt());
  m_smallestAccountFraction = node.attribute(getAttrName(Attribute::SAF)).toUInt();
  m_pricePrecision = node.attribute(getAttrName(Attribute::PP)).toUInt();

  if (m_smallestAccountFraction == 0)
    m_smallestAccountFraction = 100;
  if (m_pricePrecision == 0 || m_pricePrecision > 10)
    m_pricePrecision = 4;

  if (isCurrency()) {
    m_smallestCashFraction = node.attribute(getAttrName(Attribute::SCF)).toUInt();
    if (m_smallestCashFraction == 0)
      m_smallestCashFraction = 100;
  } else {
    m_tradingCurrency = node.attribute(getAttrName(Attribute::TradingCurrency));
    m_tradingMarket = node.attribute(getAttrName(Attribute::TradingMarket));
  }
}

MyMoneySecurity::~MyMoneySecurity()
{
}

bool MyMoneySecurity::operator == (const MyMoneySecurity& r) const
{
  return (m_id == r.m_id)
         && (m_name == r.m_name)
         && (m_tradingSymbol == r.m_tradingSymbol)
         && (m_tradingMarket == r.m_tradingMarket)
         && (m_roundingMethod == r.m_roundingMethod)
         && (m_tradingSymbol == r.m_tradingSymbol)
         && (m_tradingCurrency == r.m_tradingCurrency)
         && (m_securityType == r.m_securityType)
         && (m_smallestAccountFraction == r.m_smallestAccountFraction)
         && (m_smallestCashFraction == r.m_smallestCashFraction)
         && (m_pricePrecision == r.m_pricePrecision)
         && this->MyMoneyKeyValueContainer::operator == (r);
}

bool MyMoneySecurity::operator < (const MyMoneySecurity& right) const
{
  if (m_securityType == right.m_securityType)
    return m_name < right.m_name;
  return m_securityType < right.m_securityType;
}

const QString MyMoneySecurity::name() const
{
  return m_name;
}

void MyMoneySecurity::setName(const QString& str)
{
  m_name = str;
}

const QString MyMoneySecurity::tradingSymbol() const
{
  return m_tradingSymbol;
}

void MyMoneySecurity::setTradingSymbol(const QString& str)
{
  m_tradingSymbol = str;
}

const QString MyMoneySecurity::tradingMarket() const
{
  return m_tradingMarket;
}

void MyMoneySecurity::setTradingMarket(const QString& str)
{
  m_tradingMarket = str;
}

const QString MyMoneySecurity::tradingCurrency() const
{
  return m_tradingCurrency;
}

void MyMoneySecurity::setTradingCurrency(const QString& str)
{
  m_tradingCurrency = str;
}

bool MyMoneySecurity::hasReferenceTo(const QString& id) const
{
  return (id == m_tradingCurrency);
}

void MyMoneySecurity::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el;
  if (isCurrency())
    el = document.createElement(nodeNames[nnCurrency]);
  else
    el = document.createElement(nodeNames[nnSecurity]);

  writeBaseXML(document, el);

  el.setAttribute(getAttrName(Attribute::Name), m_name);
  el.setAttribute(getAttrName(Attribute::Symbol),m_tradingSymbol);
  el.setAttribute(getAttrName(Attribute::Type), static_cast<int>(m_securityType));
  el.setAttribute(getAttrName(Attribute::RoundingMethod), static_cast<int>(m_roundingMethod));
  el.setAttribute(getAttrName(Attribute::SAF), m_smallestAccountFraction);
  el.setAttribute(getAttrName(Attribute::PP), m_pricePrecision);
  if (isCurrency())
    el.setAttribute(getAttrName(Attribute::SCF), m_smallestCashFraction);
  else {
    el.setAttribute(getAttrName(Attribute::TradingCurrency), m_tradingCurrency);
    el.setAttribute(getAttrName(Attribute::TradingMarket), m_tradingMarket);
  }

  //Add in Key-Value Pairs for securities.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

QString MyMoneySecurity::securityTypeToString(const eMyMoney::Security securityType)
{
  switch (securityType) {
    case eMyMoney::Security::Stock:
      return i18nc("Security type", "Stock");
    case eMyMoney::Security::MutualFund:
      return i18nc("Security type", "Mutual Fund");
    case eMyMoney::Security::Bond:
      return i18nc("Security type", "Bond");
    case eMyMoney::Security::Currency:
      return i18nc("Security type", "Currency");
    case eMyMoney::Security::None:
      return i18nc("Security type", "None");
    default:
      return i18nc("Security type", "Unknown");
  }
}

QString MyMoneySecurity::roundingMethodToString(const AlkValue::RoundingMethod roundingMethod)
{
  switch (roundingMethod) {
    case AlkValue::RoundNever:
      return I18N_NOOP("Never");
    case AlkValue::RoundFloor:
      return I18N_NOOP("Floor");
    case AlkValue::RoundCeil:
      return I18N_NOOP("Ceil");
    case AlkValue::RoundTruncate:
      return I18N_NOOP("Truncate");
    case AlkValue::RoundPromote:
      return I18N_NOOP("Promote");
    case AlkValue::RoundHalfDown:
      return I18N_NOOP("HalfDown");
    case AlkValue::RoundHalfUp:
      return I18N_NOOP("HalfUp");
    case AlkValue::RoundRound:
      return I18N_NOOP("Round");
    default:
      return I18N_NOOP("Unknown");
  }
}

QString MyMoneySecurity::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::Name,             QStringLiteral("name")},
    {Attribute::Symbol,           QStringLiteral("symbol")},
    {Attribute::Type,             QStringLiteral("type")},
    {Attribute::RoundingMethod,   QStringLiteral("rounding-method")},
    {Attribute::SAF,              QStringLiteral("saf")},
    {Attribute::PP,               QStringLiteral("pp")},
    {Attribute::SCF,              QStringLiteral("scf")},
    {Attribute::TradingCurrency,  QStringLiteral("trading-currency")},
    {Attribute::TradingMarket,    QStringLiteral("trading-market")}
  };
  return attrNames[attr];
}
