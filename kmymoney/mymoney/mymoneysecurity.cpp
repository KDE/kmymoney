/*
    SPDX-FileCopyrightText: 2005-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneysecurity.h"
#include "mymoneysecurity_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QSet>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

using namespace eMyMoney;

MyMoneySecurity::MyMoneySecurity() :
  MyMoneyObject(*new MyMoneySecurityPrivate)
{
}

MyMoneySecurity::MyMoneySecurity(const QString &id) :
  MyMoneyObject(*new MyMoneySecurityPrivate, id)
{
}

MyMoneySecurity::MyMoneySecurity(const QString& id,
                                 const QString& name,
                                 const QString& symbol,
                                 const int smallestCashFraction,
                                 const int smallestAccountFraction,
                                 const int pricePrecision) :
  MyMoneyObject(*new MyMoneySecurityPrivate, id),
  MyMoneyKeyValueContainer()
{
  Q_D(MyMoneySecurity);
  d->m_name = name;
  d->m_smallestCashFraction = smallestCashFraction;
  d->m_pricePrecision = pricePrecision;
  d->m_securityType = eMyMoney::Security::Type::Currency;

  if (symbol.isEmpty())
    d->m_tradingSymbol = id;
  else
    d->m_tradingSymbol = symbol;

  if (smallestAccountFraction)
    d->m_smallestAccountFraction = smallestAccountFraction;
  else
    d->m_smallestAccountFraction = smallestCashFraction;
}

MyMoneySecurity::MyMoneySecurity(const MyMoneySecurity& other) :
  MyMoneyObject(*new MyMoneySecurityPrivate(*other.d_func()), other.id()),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneySecurity::MyMoneySecurity(const QString& id, const MyMoneySecurity& other) :
  MyMoneyObject(*new MyMoneySecurityPrivate(*other.d_func()), id),
  MyMoneyKeyValueContainer(other)
{
}

MyMoneySecurity::~MyMoneySecurity()
{
}

bool MyMoneySecurity::operator == (const MyMoneySecurity& right) const
{
  Q_D(const MyMoneySecurity);
  auto d2 = static_cast<const MyMoneySecurityPrivate *>(right.d_func());
  return (d->m_id == d2->m_id)
         && (d->m_name == d2->m_name)
         && (d->m_tradingSymbol == d2->m_tradingSymbol)
         && (d->m_tradingMarket == d2->m_tradingMarket)
         && (d->m_roundingMethod == d2->m_roundingMethod)
         && (d->m_tradingSymbol == d2->m_tradingSymbol)
         && (d->m_tradingCurrency == d2->m_tradingCurrency)
         && (d->m_securityType == d2->m_securityType)
         && (d->m_smallestAccountFraction == d2->m_smallestAccountFraction)
         && (d->m_smallestCashFraction == d2->m_smallestCashFraction)
         && (d->m_pricePrecision == d2->m_pricePrecision)
         && this->MyMoneyKeyValueContainer::operator == (right);
}

bool MyMoneySecurity::operator < (const MyMoneySecurity& right) const
{
  Q_D(const MyMoneySecurity);
  auto d2 = static_cast<const MyMoneySecurityPrivate *>(right.d_func());
  if (d->m_securityType == d2->m_securityType)
    return d->m_name < d2->m_name;
  return d->m_securityType < d2->m_securityType;
}

QString MyMoneySecurity::name() const
{
  Q_D(const MyMoneySecurity);
  return d->m_name;
}

void MyMoneySecurity::setName(const QString& str)
{
  Q_D(MyMoneySecurity);
  d->m_name = str;
}

QString MyMoneySecurity::tradingSymbol() const
{
  Q_D(const MyMoneySecurity);
  return d->m_tradingSymbol;
}

void MyMoneySecurity::setTradingSymbol(const QString& str)
{
  Q_D(MyMoneySecurity);
  d->m_tradingSymbol = str;
}

QString MyMoneySecurity::tradingMarket() const
{
  Q_D(const MyMoneySecurity);
  return d->m_tradingMarket;
}

void MyMoneySecurity::setTradingMarket(const QString& str)
{
  Q_D(MyMoneySecurity);
  d->m_tradingMarket = str;
}

QString MyMoneySecurity::tradingCurrency() const
{
  Q_D(const MyMoneySecurity);
  return d->m_tradingCurrency;
}

void MyMoneySecurity::setTradingCurrency(const QString& str)
{
  Q_D(MyMoneySecurity);
  d->m_tradingCurrency = str;
}

bool MyMoneySecurity::operator != (const MyMoneySecurity& r) const
{
  return !(*this == r);
}

eMyMoney::Security::Type MyMoneySecurity::securityType() const
{
  Q_D(const MyMoneySecurity);
  return d->m_securityType;
}

void MyMoneySecurity::setSecurityType(const eMyMoney::Security::Type s)
{
  Q_D(MyMoneySecurity);
  d->m_securityType = s;
}

bool MyMoneySecurity::isCurrency() const
{
  Q_D(const MyMoneySecurity);
  return d->m_securityType == eMyMoney::Security::Type::Currency;
}

AlkValue::RoundingMethod MyMoneySecurity::roundingMethod() const
{
  Q_D(const MyMoneySecurity);
  return d->m_roundingMethod;
}

void MyMoneySecurity::setRoundingMethod(const AlkValue::RoundingMethod rnd)
{
  Q_D(MyMoneySecurity);
  d->m_roundingMethod = rnd;
}

int MyMoneySecurity::smallestAccountFraction() const
{
  Q_D(const MyMoneySecurity);
  return d->m_smallestAccountFraction;
}

void MyMoneySecurity::setSmallestAccountFraction(const int sf)
{
  Q_D(MyMoneySecurity);
  d->m_smallestAccountFraction = sf;
}

int MyMoneySecurity::smallestCashFraction() const
{
  Q_D(const MyMoneySecurity);
  return d->m_smallestCashFraction;
}

void MyMoneySecurity::setSmallestCashFraction(const int cf)
{
  Q_D(MyMoneySecurity);
  d->m_smallestCashFraction = cf;
}

int MyMoneySecurity::pricePrecision() const
{
  Q_D(const MyMoneySecurity);
  return d->m_pricePrecision;
}

void MyMoneySecurity::setPricePrecision(const int pp)
{
  Q_D(MyMoneySecurity);
  d->m_pricePrecision = pp;
}

bool MyMoneySecurity::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneySecurity);
  return (id == d->m_tradingCurrency);
}

QSet<QString> MyMoneySecurity::referencedObjects() const
{
  Q_D(const MyMoneySecurity);
  return { d->m_tradingCurrency };
}

QString MyMoneySecurity::securityTypeToString(const eMyMoney::Security::Type securityType)
{
  switch (securityType) {
    case eMyMoney::Security::Type::Stock:
      return i18nc("Security type", "Stock");
    case eMyMoney::Security::Type::MutualFund:
      return i18nc("Security type", "Mutual Fund");
    case eMyMoney::Security::Type::Bond:
      return i18nc("Security type", "Bond");
    case eMyMoney::Security::Type::Currency:
      return i18nc("Security type", "Currency");
    case eMyMoney::Security::Type::None:
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
