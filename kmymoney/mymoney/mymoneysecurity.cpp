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

#include <klocale.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

MyMoneySecurity::MyMoneySecurity() :
    m_securityType(SECURITY_NONE),
    m_smallestAccountFraction(100),
    m_smallestCashFraction(100),
    m_partsPerUnit(100)
{
}

MyMoneySecurity::MyMoneySecurity(const QString& id, const QString& name, const QString& symbol, const int partsPerUnit, const int smallestCashFraction, const int smallestAccountFraction) :
    MyMoneyObject(id),
    m_name(name),
    m_securityType(SECURITY_CURRENCY)
{
  if (symbol.isEmpty())
    m_tradingSymbol = id;
  else
    m_tradingSymbol = symbol;

  m_partsPerUnit = partsPerUnit;
  m_smallestCashFraction = smallestCashFraction;
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
    MyMoneyKeyValueContainer(node.elementsByTagName("KEYVALUEPAIRS").item(0).toElement())
{
  if (("SECURITY" != node.tagName())
      && ("EQUITY" != node.tagName())
      && ("CURRENCY" != node.tagName()))
    throw MYMONEYEXCEPTION("Node was not SECURITY or CURRENCY");

  setName(QStringEmpty(node.attribute("name")));
  setTradingSymbol(QStringEmpty(node.attribute("symbol")));
  setSecurityType(static_cast<eSECURITYTYPE>(node.attribute("type").toInt()));
  setSmallestAccountFraction(node.attribute("saf").toInt());

  if (isCurrency()) {
    setPartsPerUnit(node.attribute("ppu").toInt());
    setSmallestCashFraction(node.attribute("scf").toInt());
  } else {
    setTradingCurrency(QStringEmpty(node.attribute("trading-currency")));
    setTradingMarket(QStringEmpty(node.attribute("trading-market")));
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
         && (m_tradingSymbol == r.m_tradingSymbol)
         && (m_tradingCurrency == r.m_tradingCurrency)
         && (m_securityType == r.m_securityType)
         && (m_smallestAccountFraction == r.m_smallestAccountFraction)
         && (m_smallestCashFraction == r.m_smallestCashFraction)
         && (m_partsPerUnit == r.m_partsPerUnit)
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
    el = document.createElement("CURRENCY");
  else
    el = document.createElement("SECURITY");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("symbol", m_tradingSymbol);
  el.setAttribute("type", static_cast<int>(m_securityType));
  el.setAttribute("saf", m_smallestAccountFraction);
  if (isCurrency()) {
    el.setAttribute("ppu", m_partsPerUnit);
    el.setAttribute("scf", m_smallestCashFraction);
  } else {
    el.setAttribute("trading-currency", m_tradingCurrency);
    el.setAttribute("trading-market", m_tradingMarket);
  }

  //Add in Key-Value Pairs for securities.
  MyMoneyKeyValueContainer::writeXML(document, el);

  parent.appendChild(el);
}

QString MyMoneySecurity::securityTypeToString(const eSECURITYTYPE securityType)
{
  QString returnString;

  switch (securityType) {
    case MyMoneySecurity::SECURITY_STOCK:
      returnString = I18N_NOOP("Stock");
      break;
    case MyMoneySecurity::SECURITY_MUTUALFUND:
      returnString = I18N_NOOP("Mutual Fund");
      break;
    case MyMoneySecurity::SECURITY_BOND:
      returnString = I18N_NOOP("Bond");
      break;
    case MyMoneySecurity::SECURITY_CURRENCY:
      returnString = I18N_NOOP("Currency");
      break;
    case MyMoneySecurity::SECURITY_NONE:
      returnString = I18N_NOOP("None");
      break;
    default:
      returnString = I18N_NOOP("Unknown");
  }

  return returnString;
}

