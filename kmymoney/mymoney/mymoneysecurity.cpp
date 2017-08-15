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

using namespace MyMoneyStorageNodes;

MyMoneySecurity::MyMoneySecurity() :
    m_securityType(SECURITY_NONE),
    m_smallestCashFraction(DEFAULT_CASH_FRACTION),
    m_smallestAccountFraction(DEFAULT_ACCOUNT_FRACTION),
    m_pricePrecision(DEFAULT_PRICE_PRECISION),
    m_roundingMethod(AlkValue::RoundRound)
{
}

MyMoneySecurity::MyMoneySecurity(const QString& id, const QString& name, const QString& symbol,
                                 const int smallestCashFraction, const int smallestAccountFraction, const int pricePrecision) :
    MyMoneyObject(id),
    m_name(name),
    m_securityType(SECURITY_CURRENCY),
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
  if ((nodeNames[nnSecurity] != node.tagName())
      && (nodeNames[nnEquity] != node.tagName())
      && (nodeNames[nnCurrency] != node.tagName()))
    throw MYMONEYEXCEPTION("Node was not SECURITY or CURRENCY");

  setName(node.attribute(getAttrName(anName)));
  setTradingSymbol(node.attribute(getAttrName(anSymbol)));
  setSecurityType(static_cast<eSECURITYTYPE>(node.attribute(getAttrName(anType)).toInt()));
  setRoundingMethod(static_cast<AlkValue::RoundingMethod>(node.attribute(getAttrName(anRoundingMethod)).toInt()));
  int saf = node.attribute(getAttrName(anSAF)).toUInt();
  int pp = node.attribute(getAttrName(anPP)).toUInt();
  if (saf == 0)
    saf = 100;
  if (pp == 0 || pp > 10)
    pp = 4;
  setSmallestAccountFraction(saf);
  setPricePrecision(pp);

  if (isCurrency()) {
    int scf = node.attribute(getAttrName(anSCF)).toUInt();
    if (scf == 0)
      scf = 100;
    setSmallestCashFraction(scf);
  } else {
    setTradingCurrency(node.attribute(getAttrName(anTradingCurrency)));
    setTradingMarket(node.attribute(getAttrName(anTradingMarket)));
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

  el.setAttribute(getAttrName(anName), m_name);
  el.setAttribute(getAttrName(anSymbol), m_tradingSymbol);
  el.setAttribute(getAttrName(anType), static_cast<int>(m_securityType));
  el.setAttribute(getAttrName(anRoundingMethod), static_cast<int>(m_roundingMethod));
  el.setAttribute(getAttrName(anSAF), m_smallestAccountFraction);
  el.setAttribute(getAttrName(anPP), m_pricePrecision);
  if (isCurrency())
    el.setAttribute(getAttrName(anSCF), m_smallestCashFraction);
  else {
    el.setAttribute(getAttrName(anTradingCurrency), m_tradingCurrency);
    el.setAttribute(getAttrName(anTradingMarket), m_tradingMarket);
  }

  //Add in Key-Value Pairs for securities.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

QString MyMoneySecurity::securityTypeToString(const eSECURITYTYPE securityType)
{
  switch (securityType) {
    case MyMoneySecurity::SECURITY_STOCK:
      return i18nc("Security type", "Stock");
    case MyMoneySecurity::SECURITY_MUTUALFUND:
      return i18nc("Security type", "Mutual Fund");
    case MyMoneySecurity::SECURITY_BOND:
      return i18nc("Security type", "Bond");
    case MyMoneySecurity::SECURITY_CURRENCY:
      return i18nc("Security type", "Currency");
    case MyMoneySecurity::SECURITY_NONE:
      return i18nc("Security type", "None");
    default:
      return i18nc("Security type", "Unknown");
  }
}

QString MyMoneySecurity::roundingMethodToString(const AlkValue::RoundingMethod roundingMethod)
{
  QString returnString;

  switch (roundingMethod) {
    case AlkValue::RoundNever:
      returnString = I18N_NOOP("Never");
      break;
    case AlkValue::RoundFloor:
      returnString = I18N_NOOP("Floor");
      break;
    case AlkValue::RoundCeil:
      returnString = I18N_NOOP("Ceil");
      break;
    case AlkValue::RoundTruncate:
      returnString = I18N_NOOP("Truncate");
      break;
    case AlkValue::RoundPromote:
      returnString = I18N_NOOP("Promote");
      break;
    case AlkValue::RoundHalfDown:
      returnString = I18N_NOOP("HalfDown");
      break;
    case AlkValue::RoundHalfUp:
      returnString = I18N_NOOP("HalfUp");
      break;
    case AlkValue::RoundRound:
      returnString = I18N_NOOP("Round");
      break;
    default:
      returnString = I18N_NOOP("Unknown");
  }

  return returnString;
}

const QString MyMoneySecurity::getAttrName(const attrNameE _attr)
{
  static const QHash<attrNameE, QString> attrNames = {
    {anName, QStringLiteral("name")},
    {anSymbol, QStringLiteral("symbol")},
    {anType, QStringLiteral("type")},
    {anRoundingMethod, QStringLiteral("rounding-method")},
    {anSAF, QStringLiteral("saf")},
    {anPP, QStringLiteral("pp")},
    {anSCF, QStringLiteral("scf")},
    {anTradingCurrency, QStringLiteral("trading-currency")},
    {anTradingMarket, QStringLiteral("trading-market")}
  };
  return attrNames[_attr];
}
