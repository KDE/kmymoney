/***************************************************************************
                          mymoneymymoney.cpp  -  description
                             -------------------
    begin      : Thu Feb 21 2002
    copyright  : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                 (C) 2001-2011 by Thomas Baumgart <tbaumgart@kde.org>
                 (C) 2011 by Carlos Eduardo da Silva <kaduardo@gmail.com>
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

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>
#include <QtDebug>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneysecurity.h"


const MyMoneyMoney MyMoneyMoney::ONE = MyMoneyMoney(1, 1);
const MyMoneyMoney MyMoneyMoney::MINUS_ONE = MyMoneyMoney(-1, 1);

QChar MyMoneyMoney::_thousandSeparator = ',';
QChar MyMoneyMoney::_decimalSeparator = '.';
MyMoneyMoney::signPosition MyMoneyMoney::_negativeMonetarySignPosition = BeforeQuantityMoney;
MyMoneyMoney::signPosition MyMoneyMoney::_positiveMonetarySignPosition = BeforeQuantityMoney;
bool MyMoneyMoney::_negativePrefixCurrencySymbol = false;
bool MyMoneyMoney::_positivePrefixCurrencySymbol = false;

MyMoneyMoney::fileVersionE MyMoneyMoney::_fileVersion = MyMoneyMoney::FILE_4_BYTE_VALUE;

MyMoneyMoney MyMoneyMoney::maxValue = MyMoneyMoney(INT64_MAX, 100);
MyMoneyMoney MyMoneyMoney::minValue = MyMoneyMoney(INT64_MIN, 100);
MyMoneyMoney MyMoneyMoney::autoCalc = MyMoneyMoney(INT64_MIN + 1, 100);

void MyMoneyMoney::setNegativePrefixCurrencySymbol(const bool flag)
{
  _negativePrefixCurrencySymbol = flag;
}

void MyMoneyMoney::setPositivePrefixCurrencySymbol(const bool flag)
{
  _positivePrefixCurrencySymbol = flag;
}

void MyMoneyMoney::setNegativeMonetarySignPosition(const signPosition pos)
{
  _negativeMonetarySignPosition = pos;
}

MyMoneyMoney::signPosition MyMoneyMoney::negativeMonetarySignPosition()
{
  return _negativeMonetarySignPosition;
}

void MyMoneyMoney::setPositiveMonetarySignPosition(const signPosition pos)
{
  _positiveMonetarySignPosition = pos;
}

MyMoneyMoney::signPosition MyMoneyMoney::positiveMonetarySignPosition()
{
  return _positiveMonetarySignPosition;
}

void MyMoneyMoney::setThousandSeparator(const QChar &separator)
{
  if (separator != ' ')
    _thousandSeparator = separator;
  else
    _thousandSeparator = 0;
}

const QChar MyMoneyMoney::thousandSeparator()
{
  return _thousandSeparator;
}

void MyMoneyMoney::setDecimalSeparator(const QChar &separator)
{
  if (separator != ' ')
    _decimalSeparator = separator;
  else
    _decimalSeparator = 0;
}

const QChar MyMoneyMoney::decimalSeparator()
{
  return _decimalSeparator;
}

void MyMoneyMoney::setFileVersion(fileVersionE version)
{
  _fileVersion = version;
}


MyMoneyMoney::MyMoneyMoney(const QString& pszAmount)
    : AlkValue(pszAmount, _decimalSeparator)
{
}

MyMoneyMoney::~MyMoneyMoney()
{
}

QString MyMoneyMoney::formatMoney(int denom, bool showThousandSeparator) const
{
  return formatMoney("", denomToPrec(denom), showThousandSeparator);
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
  signed int d;
  if (mpz_fits_sint_p(denom.get_mpz_t())) {
    d = mpz_get_si(denom.get_mpz_t());
  } else {
    d = 1000000000;
  }
  value = static_cast<const MyMoneyMoney>(convertDenominator(d)).valueRef().get_num();
#else
  value = static_cast<const MyMoneyMoney>(convertDenominator(denom)).valueRef().get_num();
#endif

  // Once we really support multiple currencies then this method will
  // be much better than using KGlobal::locale()->formatMoney.
  bool bNegative = false;
  mpz_class left = value / static_cast<MyMoneyMoney>(convertDenominator(d)).valueRef().get_den();
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

    QString rs  = QString("%1").arg(right.get_str().c_str());
    if (prec != -1)
      rs = rs.rightJustified(prec, '0', true);
    else {
      rs = rs.rightJustified(9, '0', true);
      // no trailing zeroes or decimal separators
      while (rs.endsWith('0'))
        rs.truncate(rs.length() - 1);
      while (rs.endsWith(QChar(decimalSeparator())))
        rs.truncate(rs.length() - 1);
    }
    res += rs;
  }

  signPosition signpos = bNegative ? _negativeMonetarySignPosition : _positiveMonetarySignPosition;
  QString sign = bNegative ? "-" : "";

  switch (signpos) {
    case ParensAround:
      res.prepend('(');
      res.append(')');
      break;
    case BeforeQuantityMoney:
      res.prepend(sign);
      break;
    case AfterQuantityMoney:
      res.append(sign);
      break;
    case BeforeMoney:
      tmpCurrency.prepend(sign);
      break;
    case AfterMoney:
      tmpCurrency.append(sign);
      break;
  }
  if (!tmpCurrency.isEmpty()) {
    if (bNegative ? _negativePrefixCurrencySymbol : _positivePrefixCurrencySymbol) {
      res.prepend(' ');
      res.prepend(tmpCurrency);
    } else {
      res.append(' ');
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
  return AlkValue::operator+(_b);
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
  return AlkValue::operator-(_b);
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
  return AlkValue::operator*(_b);
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
  return AlkValue::operator/(_b);
}


MyMoneyMoney MyMoneyMoney::convert(const signed64 _denom, const roundingMethod how) const
{
  return convertDenominator(_denom, static_cast<RoundingMethod>(how));
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

QDebug operator<<(QDebug dbg, const MyMoneyMoney &a)
{
  dbg << "MyMoneyMoney("
      << "isAutoCalc" << a.isAutoCalc()
      << "isNegative" << a.isNegative()
      << "isPositive" << a.isPositive()
      << "isZero" << a.isZero()
      << "value" << a.toString()
      << ")";
  return dbg;
}
