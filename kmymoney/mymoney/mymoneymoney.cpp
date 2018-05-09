/***************************************************************************
                          mymoneymymoney.cpp  -  description
                             -------------------
    begin      : Thu Feb 21 2002
    copyright  : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                 (C) 2011 by Carlos Eduardo da Silva <kaduardo@gmail.com>
                 (C) 2001-2017 by Thomas Baumgart <tbaumgart@kde.org>
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

// make sure, that this is defined before we even include any other header file
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS         // force definition of min and max values
#endif

#include "mymoneymoney.h"

#include <stdint.h>
#include <gmpxx.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyenums.h"

const MyMoneyMoney MyMoneyMoney::ONE = MyMoneyMoney(1, 1);
const MyMoneyMoney MyMoneyMoney::MINUS_ONE = MyMoneyMoney(-1, 1);
namespace eMyMoney
{
  namespace Money {

    enum fileVersionE : int {
      FILE_4_BYTE_VALUE = 0,
      FILE_8_BYTE_VALUE
    };

    QChar _thousandSeparator = QLatin1Char(',');
    QChar _decimalSeparator = QLatin1Char('.');
    eMyMoney::Money::signPosition _negativeMonetarySignPosition = BeforeQuantityMoney;
    eMyMoney::Money::signPosition _positiveMonetarySignPosition = BeforeQuantityMoney;
    bool _negativePrefixCurrencySymbol = false;
    bool _positivePrefixCurrencySymbol = false;
    eMyMoney::Money::fileVersionE _fileVersion = fileVersionE::FILE_4_BYTE_VALUE;
  }
}

//eMyMoney::Money::_thousandSeparator = QLatin1Char(',');
//eMyMoney::Money::_decimalSeparator = QLatin1Char('.');
//eMyMoney::Money::signPosition eMyMoney::Money::_negativeMonetarySignPosition = BeforeQuantityMoney;
//eMyMoney::Money::signPosition eMyMoney::Money::_positiveMonetarySignPosition = BeforeQuantityMoney;
//bool eMyMoney::Money::_negativePrefixCurrencySymbol = false;
//bool eMyMoney::Money::_positivePrefixCurrencySymbol = false;

//MyMoneyMoney::fileVersionE eMyMoney::Money::_fileVersion = MyMoneyMoney::FILE_4_BYTE_VALUE;

MyMoneyMoney MyMoneyMoney::maxValue = MyMoneyMoney(INT64_MAX, 100);
MyMoneyMoney MyMoneyMoney::minValue = MyMoneyMoney(INT64_MIN, 100);
MyMoneyMoney MyMoneyMoney::autoCalc = MyMoneyMoney(INT64_MIN + 1, 100);

void MyMoneyMoney::setNegativePrefixCurrencySymbol(const bool flag)
{
  eMyMoney::Money::_negativePrefixCurrencySymbol = flag;
}

void MyMoneyMoney::setPositivePrefixCurrencySymbol(const bool flag)
{
  eMyMoney::Money::_positivePrefixCurrencySymbol = flag;
}

void MyMoneyMoney::setNegativeMonetarySignPosition(const eMyMoney::Money::signPosition pos)
{
  eMyMoney::Money::_negativeMonetarySignPosition = pos;
}

eMyMoney::Money::signPosition MyMoneyMoney::negativeMonetarySignPosition()
{
  return eMyMoney::Money::_negativeMonetarySignPosition;
}

void MyMoneyMoney::setPositiveMonetarySignPosition(const eMyMoney::Money::signPosition pos)
{
  eMyMoney::Money::_positiveMonetarySignPosition = pos;
}

eMyMoney::Money::signPosition MyMoneyMoney::positiveMonetarySignPosition()
{
  return eMyMoney::Money::_positiveMonetarySignPosition;
}

void MyMoneyMoney::setThousandSeparator(const QChar &separator)
{
  if (separator != QLatin1Char(' '))
    eMyMoney::Money::_thousandSeparator = separator;
  else
    eMyMoney::Money::_thousandSeparator = 0;
}

const QChar MyMoneyMoney::thousandSeparator()
{
  return eMyMoney::Money::_thousandSeparator;
}

void MyMoneyMoney::setDecimalSeparator(const QChar &separator)
{
  if (separator != QLatin1Char(' '))
    eMyMoney::Money::_decimalSeparator = separator;
  else
    eMyMoney::Money::_decimalSeparator = 0;
}

const QChar MyMoneyMoney::decimalSeparator()
{
  return eMyMoney::Money::_decimalSeparator;
}

MyMoneyMoney::MyMoneyMoney(const QString& pszAmount)
    : AlkValue(pszAmount, eMyMoney::Money::_decimalSeparator)
{
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a signed64 value
//   Returns: None
//    Throws: Nothing.
// Arguments: Amount - signed 64 object containing amount
//            denom  - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
MyMoneyMoney::MyMoneyMoney(signed64 Amount, const signed64 denom)
{
  if (!denom)
    throw MYMONEYEXCEPTION("Denominator 0 not allowed!");

  *this = AlkValue(QString::fromLatin1("%1/%2").arg(Amount).arg(denom), eMyMoney::Money::_decimalSeparator);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a integer value
//   Returns: None
//    Throws: Nothing.
// Arguments: iAmount - integer object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
MyMoneyMoney::MyMoneyMoney(const int iAmount, const signed64 denom)
{
  if (!denom)
    throw MYMONEYEXCEPTION("Denominator 0 not allowed!");
  *this = AlkValue(iAmount, denom);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a long integer value
//   Returns: None
//    Throws: Nothing.
// Arguments: iAmount - integer object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
MyMoneyMoney::MyMoneyMoney(const long int iAmount, const signed64 denom)
{
  if (!denom)
    throw MYMONEYEXCEPTION("Denominator 0 not allowed!");
  *this = AlkValue(QString::fromLatin1("%1/%2").arg(iAmount).arg(denom), eMyMoney::Money::_decimalSeparator);
}


MyMoneyMoney::~MyMoneyMoney()
{
}

MyMoneyMoney MyMoneyMoney::abs() const
{
  return static_cast<const MyMoneyMoney>(AlkValue::abs());
}

QString MyMoneyMoney::formatMoney(int denom, bool showThousandSeparator) const
{
  return formatMoney(QString(), denomToPrec(denom), showThousandSeparator);
}

QString MyMoneyMoney::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
  QString res;
  QString tmpCurrency = currency;
  int tmpPrec = prec;
  mpz_class denom = 1;
  mpz_class value;

  // if prec == -1 we want the maximum possible but w/o trailing zeroes
  if (tmpPrec > -1) {
    while (tmpPrec--) {
      denom *= 10;
    }
  } else {
    // fix it to a max of 9 digits on the right side for now
    denom = 1000000000;
  }

  // as long as AlkValue::convertDenominator() does not take an
  // mpz_class as the denominator, we need to use a signed int
  // and limit the precision to 9 digits (the max we can
  // present with 31 bits
#if 1
  signed int denominator;
  if (mpz_fits_sint_p(denom.get_mpz_t())) {
    denominator = mpz_get_si(denom.get_mpz_t());
  } else {
    denominator = 1000000000;
  }
  value = static_cast<const MyMoneyMoney>(convertDenominator(denominator)).valueRef().get_num();
#else
  value = static_cast<const MyMoneyMoney>(convertDenominator(denom)).valueRef().get_num();
#endif

  // Once we really support multiple currencies then this method will
  // be much better than using KLocale::global()->formatMoney.
  bool bNegative = false;
  mpz_class left = value / static_cast<MyMoneyMoney>(convertDenominator(denominator)).valueRef().get_den();
  mpz_class right = mpz_class((valueRef() - mpq_class(left)) * denom);

  if (right < 0) {
    right = -right;
    bNegative = true;
  }
  if (left < 0) {
    left = -left;
    bNegative = true;
  }

  // convert the integer (left) part to a string
  res.append(left.get_str().c_str());

  // if requested, insert thousand separators every three digits
  if (showThousandSeparator) {
    int pos = res.length();
    while ((0 < (pos -= 3)) && thousandSeparator() != 0)
      res.insert(pos, thousandSeparator());
  }

  // take care of the fractional part
  if (prec > 0 || (prec == -1 && right != 0)) {
    if (decimalSeparator() != 0)
      res += decimalSeparator();

    auto rs  = QString::fromLatin1("%1").arg(right.get_str().c_str());
    if (prec != -1)
      rs = rs.rightJustified(prec, QLatin1Char('0'), true);
    else {
      rs = rs.rightJustified(9, QLatin1Char('0'), true);
      // no trailing zeroes or decimal separators
      while (rs.endsWith(QLatin1Char('0')))
        rs.truncate(rs.length() - 1);
      while (rs.endsWith(decimalSeparator()))
        rs.truncate(rs.length() - 1);
    }
    res += rs;
  }

  eMyMoney::Money::signPosition signpos = bNegative ? eMyMoney::Money::_negativeMonetarySignPosition : eMyMoney::Money::_positiveMonetarySignPosition;
  auto sign = bNegative ? QString::fromLatin1("-") : QString();

  switch (signpos) {
    case eMyMoney::Money::ParensAround:
      res.prepend(QLatin1Char('('));
      res.append(QLatin1Char(')'));
      break;
    case eMyMoney::Money::BeforeQuantityMoney:
      res.prepend(sign);
      break;
    case eMyMoney::Money::AfterQuantityMoney:
      res.append(sign);
      break;
    case eMyMoney::Money::BeforeMoney:
      tmpCurrency.prepend(sign);
      break;
    case eMyMoney::Money::AfterMoney:
      tmpCurrency.append(sign);
      break;
  }
  if (!tmpCurrency.isEmpty()) {
    if (bNegative ? eMyMoney::Money::_negativePrefixCurrencySymbol : eMyMoney::Money::_positivePrefixCurrencySymbol) {
      res.prepend(QLatin1Char(' '));
      res.prepend(tmpCurrency);
    } else {
      res.append(QLatin1Char(' '));
      res.append(tmpCurrency);
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be added
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator+(const MyMoneyMoney& _b) const
{
  return static_cast<const MyMoneyMoney>(AlkValue::operator+(_b));
}


////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator-(const MyMoneyMoney& _b) const
{
  return static_cast<const MyMoneyMoney>(AlkValue::operator-(_b));
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator*(const MyMoneyMoney& _b) const
{
  return static_cast<const MyMoneyMoney>(AlkValue::operator*(_b));
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator/
//   Purpose: Division operator - divides the object by the input amount
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be used as dividend
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator/(const MyMoneyMoney& _b) const
{
  return static_cast<const MyMoneyMoney>(AlkValue::operator/(_b));
}

bool MyMoneyMoney::isNegative() const
{
  return (valueRef() < 0) ? true : false;
}

bool MyMoneyMoney::isPositive() const
{
  return (valueRef() > 0) ? true : false;
}

bool MyMoneyMoney::isZero() const
{
  return valueRef() == 0;
}

bool MyMoneyMoney::isAutoCalc() const
{
  return (*this == autoCalc);
}

MyMoneyMoney MyMoneyMoney::convert(const signed64 _denom, const AlkValue::RoundingMethod how) const
{
  return static_cast<const MyMoneyMoney>(convertDenominator(_denom, how));
}

MyMoneyMoney MyMoneyMoney::reduce() const
{
  MyMoneyMoney out(*this);
  out.canonicalize();
  return out;
}

signed64 MyMoneyMoney::precToDenom(int prec)
{
  signed64 denom = 1;

  while (prec--)
    denom *= 10;

  return denom;
}

double MyMoneyMoney::toDouble() const
{
  return valueRef().get_d();
}

int MyMoneyMoney::denomToPrec(signed64 fract)
{
  int rc = 0;
  while (fract > 1) {
    rc++;
    fract /= 10;
  }
  return rc;
}

