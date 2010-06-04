/***************************************************************************
                          mymoneymymoney.cpp  -  description
                             -------------------
    begin                : Thu Feb 21 2002
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

// make sure, that this is defined before we even include any other header file
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS         // force definition of min and max values
#endif

#include "mymoneymoney.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QRegExp>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneysecurity.h"

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

const MyMoneyMoney::signPosition MyMoneyMoney::negativeMonetarySignPosition(void)
{
  return _negativeMonetarySignPosition;
}

void MyMoneyMoney::setPositiveMonetarySignPosition(const signPosition pos)
{
  _positiveMonetarySignPosition = pos;
}

const MyMoneyMoney::signPosition MyMoneyMoney::positiveMonetarySignPosition(void)
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

const QChar MyMoneyMoney::thousandSeparator(void)
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

const QChar MyMoneyMoney::decimalSeparator(void)
{
  return _decimalSeparator;
}

void MyMoneyMoney::setFileVersion(fileVersionE version)
{
  _fileVersion = version;
}

MyMoneyMoney::MyMoneyMoney(const QString& pszAmount)
{
  m_num = 0;
  m_denom = 1;


  // an empty string is zero
  if (pszAmount.isEmpty())
    return;

  // take care of prices given in the form "8 5/16"
  // and our own internal represenation
  QRegExp regExp("^((\\d+)\\s+|-)?(\\d+)/(\\d+)");
  //                +-#2-+        +-#3-+ +-#4-+
  //               +-----#1-----+
  if (regExp.indexIn(pszAmount) > -1) {
    m_num = regExp.cap(3).toLongLong();
    m_denom = regExp.cap(4).toLongLong();
    const QString& part1 = regExp.cap(1);
    if (!part1.isEmpty()) {
      if (part1 == QLatin1String("-")) {
        m_num = -m_num;

      } else {
        *this += MyMoneyMoney(regExp.cap(2));
      }
    }
    return;
  }

  QString res = pszAmount;
  // get rid of anything that is not
  // a) numeric
  // b) _decimalSeparator
  // c) negative indicator
  QString validChars = QString("\\d%1").arg(QChar(decimalSeparator()));
  QString negChars("-");
  if (_negativeMonetarySignPosition == ParensAround) {
    // FIXME: If we want to allow '-' as well as '()' for negative entry
    //        we would have to replase '=' with '+=' in the next line
    //        Also, the logic in kMyMoneyEdit::theTextChanged has to be
    //        adjusted to allow both methods at the same time.
    negChars = "()";
  }
  validChars += negChars;
  // qDebug("0: '%s'", qPrintable(validChars));

  QRegExp invChars(QString("[^%1]").arg(validChars));
  // qDebug("1: '%s'", qPrintable(res));
  res.remove(invChars);

  QRegExp negCharSet(QString("[%1]").arg(negChars));
  bool isNegative = false;
  if (res.indexOf(negCharSet) != -1) {
    isNegative = true;
    res.remove(negCharSet);
  }
  // qDebug("2: '%s'", qPrintable(res));
  int pos;

  // qDebug("3: '%s'", qPrintable(res));
  if ((pos = res.indexOf(_decimalSeparator)) != -1) {
    // make sure, we get the denominator right
    m_denom = precToDenom(res.length() - pos - 1);

    // now remove the decimal symbol
    res.remove(pos, 1);
  }
  // qDebug("4: '%s'", qPrintable(res));
  if (res.length() > 0)
    m_num =  res.toLongLong();

  if (isNegative)
    m_num = -m_num;

  // qDebug("5: %Ld", m_num);
  // qDebug("6: %Ld", m_denom);
}

QString MyMoneyMoney::formatMoney(int denom, bool showThousandSeparator) const
{
  return formatMoney("", denomToPrec(denom), showThousandSeparator);
}

QString MyMoneyMoney::formatMoney(const MyMoneyAccount& acc, const MyMoneySecurity& sec, bool showThousandSeparator) const
{
  return formatMoney(sec.tradingSymbol(), denomToPrec(acc.fraction()), showThousandSeparator);
}

QString MyMoneyMoney::formatMoney(const MyMoneySecurity& sec, bool showThousandSeparator) const
{
  return formatMoney(sec.tradingSymbol(), denomToPrec(sec.smallestAccountFraction()), showThousandSeparator);
}

QString MyMoneyMoney::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
  QString res;
  QString tmpCurrency = currency;
  int tmpPrec = prec;
  signed64 denom = 1;
  signed64 m_64Value;

  // if prec == -1 we want the maximum possible but w/o trailing zeroes
  if (tmpPrec > -1) {
    while (tmpPrec--) {
      denom *= 10;
    }
  } else {
    // fix it to a max of 8 digits on the right side for now
    denom = 100000000;
  }

  m_64Value = convert(denom).m_num;

  // Once we really support multiple currencies then this method will
  // be much better than using KGlobal::locale()->formatMoney.
  bool bNegative = false;
  signed64 left = m_64Value / denom;
  signed64 right = m_64Value % denom;

  if (right < 0) {
    right = -right;
    bNegative = true;
  }
  if (left < 0) {
    left = -left;
    bNegative = true;
  }

  if (left & 0xFFFFFFFF00000000LL) {
    signed64 tmp = left;

    // QString.sprintf("%Ld") did not work :-(,  so I had to
    // do it the old ugly way.
    while (tmp) {
      res.insert(0, QString("%1").arg(static_cast<int>(tmp % 10)));
      tmp /= 10;
    }

  } else
    res = QString("%1").arg((long)left);

  if (showThousandSeparator) {
    int pos = res.length();
    while ((0 < (pos -= 3)) && thousandSeparator() != 0)
      res.insert(pos, thousandSeparator());
  }

  if (prec > 0 || (prec == -1 && right != 0)) {
    if (decimalSeparator() != 0)
      res += decimalSeparator();

    // using
    //
    //   res += QString("%1").arg(right).rightJustified(prec, '0', true);
    //
    // caused some weird results if right was rather large. Eg: right being
    // 666600000 should have appended a 0, but instead it prepended a 0. With
    // res being "2," the result wasn't "2,6666000000" as expected, but rather
    // "2,0666600000" which was not usable. The code below works for me.
    QString rs  = QString("%1").arg(right);
    if (prec != -1)
      rs = rs.rightJustified(prec, '0', true);
    else {
      rs = rs.rightJustified(8, '0', true);
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

const QString MyMoneyMoney::toString(void) const
{
  signed64 tmp = m_num < 0 ? - m_num : m_num;
  QString  res;
  QString  resf;

  // QString.sprintf("%Ld") did not work :-(,  so I had to
  // do it the old ugly way.
  while (tmp) {
    res.prepend(QString("%1").arg(static_cast<int>(tmp % 10)));
    tmp /= 10;
  }
  if (res.isEmpty())
    res = QString("0");

  if (m_num < 0)
    res.prepend('-');

  tmp = m_denom;
  while (tmp) {
    resf.prepend(QString("%1").arg(static_cast<int>(tmp % 10)));
    tmp /= 10;
  }
  return res + '/' + resf;
}

QDataStream &operator<<(QDataStream &s, const MyMoneyMoney &_money)
{
  // We WILL lose data here if the user has more than 2 billion pounds :-(
  // QT defined it here as long:
  // qglobal.h:typedef long          qint64;

  MyMoneyMoney money = _money.convert(100);

  switch (MyMoneyMoney::_fileVersion) {
    case MyMoneyMoney::FILE_4_BYTE_VALUE:
      if (money.m_num & 0xffffffff00000000LL)
        qWarning("Lost data while writing out MyMoneyMoney object using deprecated 4 byte writer");

      s << static_cast<qint32>(money.m_num & 0xffffffff);
      break;

    default:
      qDebug("Unknown file version while writing MyMoneyMoney object! Use FILE_8_BYTE_VALUE");
      // tricky fall through here

    case MyMoneyMoney::FILE_8_BYTE_VALUE:
      s << static_cast<qint32>(money.m_num >> 32);
      s << static_cast<qint32>(money.m_num & 0xffffffff);
      break;
  }
  return s;
}

QDataStream &operator>>(QDataStream &s, MyMoneyMoney &money)
{
  qint32 tmp;
  switch (MyMoneyMoney::_fileVersion) {
    case MyMoneyMoney::FILE_4_BYTE_VALUE:
      s >> tmp;
      money.m_num = static_cast<signed64>(tmp);
      money.m_denom = 100;
      break;

    default:
      qDebug("Unknown file version while writing MyMoneyMoney object! FILE_8_BYTE_VALUE assumed");
      // tricky fall through here

    case MyMoneyMoney::FILE_8_BYTE_VALUE:
      s >> tmp;
      money.m_num = static_cast<signed64>(tmp);
      money.m_num <<= 32;
      s >> tmp;
      money.m_num |= static_cast<signed64>(tmp);
      money.m_denom = 100;
      break;
  }
  return s;
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
  MyMoneyMoney a(*this);
  MyMoneyMoney b(_b);
  MyMoneyMoney sum;
  signed64 lcd;

  if (a.m_denom < 0) {
    a.m_num *= a.m_denom;
    a.m_denom = 1;
  }
  if (b.m_denom < 0) {
    b.m_num *= b.m_denom;
    b.m_denom = 1;
  }

  if (a.m_denom == b.m_denom) {
    sum.m_num = a.m_num + b.m_num;
    sum.m_denom = a.m_denom;
  } else {
    lcd = a.getLcd(b);
    sum.m_num   = a.m_num * (lcd / a.m_denom) + b.m_num * (lcd / b.m_denom);
    sum.m_denom = lcd;
  }
  return sum;
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
  MyMoneyMoney a(*this);
  MyMoneyMoney b(_b);
  MyMoneyMoney diff;
  signed64 lcd;

  if (a.m_denom < 0) {
    a.m_num *= a.m_denom;
    a.m_denom = 1;
  }
  if (b.m_denom < 0) {
    b.m_num *= b.m_denom;
    b.m_denom = 1;
  }

  if (a.m_denom == b.m_denom) {
    diff.m_num = a.m_num - b.m_num;
    diff.m_denom = a.m_denom;
  } else {
    lcd = a.getLcd(b);
    diff.m_num   = a.m_num * (lcd / a.m_denom) - b.m_num * (lcd / b.m_denom);
    diff.m_denom = lcd;
  }
  return diff;
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
  MyMoneyMoney a(*this);
  MyMoneyMoney b(_b);
  MyMoneyMoney product;

  if (a.m_denom < 0) {
    a.m_num *= a.m_denom;
    a.m_denom = 1;
  }
  if (b.m_denom < 0) {
    b.m_num *= b.m_denom;
    b.m_denom = 1;
  }

  product.m_num = a.m_num * b.m_num;
  product.m_denom = a.m_denom * b.m_denom;

  if (product.m_denom < 0) {
    product.m_num = -product.m_num;
    product.m_denom = -product.m_denom;
  }
  return product;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator/
//   Purpose: Division operator - divides the object by the input amount
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be used as dividend
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator / (const MyMoneyMoney& _b) const
{
  MyMoneyMoney a(*this);
  MyMoneyMoney b(_b);
  MyMoneyMoney quotient;
  signed64 lcd;

  if (a.m_denom < 0) {
    a.m_num *= a.m_denom;
    a.m_denom = 1;
  }
  if (b.m_denom < 0) {
    b.m_num *= b.m_denom;
    b.m_denom = 1;
  }

  if (a.m_denom == b.m_denom) {
    quotient.m_num = a.m_num;
    quotient.m_denom = b.m_num;
  } else {
    /* ok, convert to the lcd and compute from there... */
    lcd = a.getLcd(b);
    quotient.m_num   = a.m_num * (lcd / a.m_denom);
    quotient.m_denom = b.m_num * (lcd / b.m_denom);
  }

  if (quotient.m_denom < 0) {
    quotient.m_num   = -quotient.m_num;
    quotient.m_denom = -quotient.m_denom;
  }

  Q_ASSERT(quotient.m_denom != 0);

  return quotient;
}

signed64 MyMoneyMoney::getLcd(const MyMoneyMoney& b) const
{
  signed64 current_divisor = 2;
  signed64 max_square;
  signed64 three_count = 0;
  signed64 small_denom;
  signed64 big_denom;

  if (b.m_denom < m_denom) {
    small_denom = b.m_denom;
    big_denom = m_denom;
  } else {
    small_denom = m_denom;
    big_denom = b.m_denom;
  }

  /* special case: smaller divides smoothly into larger */
  if ((big_denom % small_denom) == 0) {
    return big_denom;
  }

  max_square = small_denom;

  /* the LCM algorithm : factor out the union of the prime factors of the
   * two args and then multiply the remainders together.
   *
   * To do this, we find the successive prime factors of the smaller
   * denominator and eliminate them from both the smaller and larger
   * denominator (so we only count factors on a one-on-one basis),
   * then multiply the original smaller by the remains of the larger.
   *
   * I.e. LCM 100,96875 == 2*2*5*5,31*5*5*5*5 = 2*2,31*5*5
   *      answer: multiply 100 by 31*5*5 == 387500
   */
  while ((current_divisor * current_divisor) <= max_square) {
    if (((small_denom % current_divisor) == 0) &&
        ((big_denom % current_divisor) == 0)) {
      big_denom = big_denom / current_divisor;
      small_denom = small_denom / current_divisor;
    } else {
      if (current_divisor == 2) {
        current_divisor++;
      } else if (three_count == 3) {
        current_divisor += 4;
        three_count = 1;
      } else {
        current_divisor += 2;
        three_count++;
      }
    }

    if ((current_divisor > small_denom) ||
        (current_divisor > big_denom)) {
      break;
    }
  }

  /* max_sqaure is the original small_denom */
  return max_square * big_denom;
}

const MyMoneyMoney MyMoneyMoney::convert(const signed64 _denom, const roundingMethod how) const
{
  MyMoneyMoney out(*this);
  MyMoneyMoney in(*this);
  MyMoneyMoney temp;

  signed64 denom = _denom;
  signed64 temp_bc;
  signed64 temp_a;
  signed64 remainder;
  signed64 sign;
  int denom_neg = 0;

  if (m_denom != denom) {
    /* if the denominator of the input value is negative, get rid of that. */
    if (m_denom < 0) {
      in.m_num = in.m_num * (- in.m_denom);
      in.m_denom = 1;
    }

    sign = (in.m_num < 0) ? -1 : 1;

    /* if the denominator is less than zero, we are to interpret it as
     * the reciprocal of its magnitude. */
    if (denom < 0) {
      denom       = - denom;
      denom_neg   = 1;
      temp_a      = (in.m_num < 0) ? -in.m_num : in.m_num;
      temp_bc     = in.m_denom * denom;
      remainder   = in.m_num % temp_bc;
      out.m_num   = in.m_num / temp_bc;
      out.m_denom = -denom;
    } else {
      /* do all the modulo and int division on positive values to make
       * things a little clearer. Reduce the fraction denom/in.denom to
       * help with range errors (FIXME : need bigger intermediate rep) */
      temp.m_num   = denom;
      temp.m_denom = in.m_denom;
      temp = temp.reduce();

      out.m_num    = in.m_num * temp.m_num;
      out.m_num    = (out.m_num < 0) ? -out.m_num : out.m_num;
      remainder    = out.m_num % temp.m_denom;
      out.m_num    = out.m_num / temp.m_denom;
      out.m_denom  = denom;
    }

    if (remainder > 0) {
      switch (how) {
        case RndFloor:
          if (sign < 0) {
            out.m_num = out.m_num + 1;
          }
          break;

        case RndCeil:
          if (sign > 0) {
            out.m_num = out.m_num + 1;
          }
          break;

        case RndTrunc:
          break;

        case RndPromote:
          out.m_num = out.m_num + 1;
          break;

        case RndHalfDown:
          if (denom_neg) {
            if ((2 * remainder) > in.m_denom*denom) {
              out.m_num = out.m_num + 1;
            }
          } else if ((2 * remainder) > temp.m_denom) {
            out.m_num = out.m_num + 1;
          }
          break;

        case RndHalfUp:
          if (denom_neg) {
            if ((2 * remainder) >= in.m_denom*denom) {
              out.m_num = out.m_num + 1;
            }
          } else if ((2 * remainder) >= temp.m_denom) {
            out.m_num = out.m_num + 1;
          }
          break;

        case RndRound:
          if (denom_neg) {
            if ((2 * remainder) > in.m_denom*denom) {
              out.m_num = out.m_num + 1;
            } else if ((2 * remainder) == in.m_denom*denom) {
              if (out.m_num % 2) {
                out.m_num = out.m_num + 1;
              }
            }
          } else {
            if ((2 * remainder) > temp.m_denom) {
              out.m_num = out.m_num + 1;
            } else if ((2 * remainder) == temp.m_denom) {
              if (out.m_num % 2) {
                out.m_num = out.m_num + 1;
              }
            }
          }
          break;

        case RndNever:
#ifdef SIZEOF_LONG_DOUBLE
          qWarning("MyMoneyMoney: have remainder \"%Ld/%Ld\"->convert(%Ld, %d)",
                   m_num, m_denom, _denom, how);
#else
          qWarning("MyMoneyMoney: have remainder \"%Ld/%Ld\"->convert(%Ld, %d)",
                   m_num, m_denom, _denom, how);
#endif
          break;
      }
    }
    out.m_num = (sign > 0) ? out.m_num : (-out.m_num);
  }

  return out;
}

/********************************************************************
 *  gnc_numeric_reduce
 *  reduce a fraction by GCF elimination.  This is NOT done as a
 *  part of the arithmetic API unless GNC_DENOM_REDUCE is specified
 *  as the output denominator.
 ********************************************************************/
const MyMoneyMoney MyMoneyMoney::reduce(void) const
{
  MyMoneyMoney out;
  signed64 t;
  signed64 num = (m_num < 0) ? (- m_num) : m_num ;
  signed64 denom = m_denom;

  /* the strategy is to use euclid's algorithm */
  while (denom > 0) {
    t = num % denom;
    num = denom;
    denom = t;
  }
  /* num = gcd */

  /* all calculations are done on positive num, since it's not
   * well defined what % does for negative values */
  out.m_num   = m_num / num;
  out.m_denom = m_denom / num;
  return out;
}

signed64 MyMoneyMoney::precToDenom(int prec)
{
  signed64 denom = 1;

  while (prec--)
    denom *= 10;

  return denom;
}

double MyMoneyMoney::toDouble(void) const
{
  return static_cast<double>(m_num) / static_cast<double>(m_denom);
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

MyMoneyMoney::operator int() const
{
  return static_cast<int>(m_num / m_denom);
}
