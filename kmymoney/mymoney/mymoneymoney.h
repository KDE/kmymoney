/*
 * Copyright 2000-2004  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2001-2002  Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2002-2004  Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2005       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2007-2010  Alvaro Soliverez <asoliverez@gmail.com>
 * Copyright 2011       Carlos Eduardo da Silva <kaduardo@gmail.com>
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

// krazy:excludeall=dpointer

#ifndef MYMONEYMONEY_H
#define MYMONEYMONEY_H

// So we can save this object
#include <QMetaType>

#include "kmm_mymoney_export.h"
#include "mymoneyunittestable.h"

#include <alkimia/alkvalue.h>

typedef qint64 signed64;
typedef quint64 unsigned64;

namespace eMyMoney { namespace Money { enum signPosition : int; } }

/**
  * This class represents a value within the MyMoney Engine
  *
  * @author Michael Edwardes
  * @author Thomas Baumgart
  */
class KMM_MYMONEY_EXPORT MyMoneyMoney : public AlkValue
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  // construction
  MyMoneyMoney();
  explicit MyMoneyMoney(const int iAmount, const unsigned int denom);
  explicit MyMoneyMoney(const long int iAmount, const unsigned int denom);
  explicit MyMoneyMoney(const QString& pszAmount);
  explicit MyMoneyMoney(const qint64 Amount, const unsigned int denom);
  explicit MyMoneyMoney(const double dAmount, const unsigned int denom = 100);

  // copy constructor
  MyMoneyMoney(const MyMoneyMoney& Amount);
  explicit MyMoneyMoney(const AlkValue& Amount);

  virtual ~MyMoneyMoney();

  MyMoneyMoney abs() const;

  /**
    * This method returns a formatted string according to the settings
    * of _thousandSeparator, _decimalSeparator, _negativeMonetarySignPosition,
    * _positiveMonetaryPosition, _negativePrefixCurrencySymbol,
    * _positivePrefixCurrencySymbol, _negativeSpaceSeparatesSymbol and
    * _positiveSpaceSeparatesSymbol. Those values can be modified using
    * the appropriate set-methods.
    *
    * @param currency The currency symbol
    * @param prec The number of fractional digits
    * @param showThousandSeparator should the thousandSeparator symbol
    *                               be inserted (@a true)
    *                               or not (@a false) (default true)
    */
  QString formatMoney(const QString& currency, const int prec, bool showThousandSeparator = true) const;

  /**
   * This is a convenience method. It behaves exactly as the above one,
   * but takes the information about precision out of the denomination
   * @a denom. No currency symbol is shown. If you want to add a currency
   * symbol, please use MyMoneyUtils::formatMoney(const MyMoneyAccount& acc, const MyMoneySecurity& sec, bool showThousandSeparator)
   * instead.
   *
   * @note denom is often set to account.fraction(security).
   */
  QString formatMoney(int denom, bool showThousandSeparator = true) const;

  /**
    * This method is used to convert the smallest fraction information into
    * the corresponding number of digits used for precision.
    *
    * @param fract smallest fractional part (e.g. 100 for cents)
    * @return number of precision digits (e.g. 2 for cents)
    */
  static int denomToPrec(signed64 fract);

  MyMoneyMoney convert(const signed64 denom = 100, const AlkValue::RoundingMethod how = AlkValue::RoundRound) const;
  static signed64 precToDenom(int prec);
  double toDouble() const;

  static void setThousandSeparator(const QChar &);
  static void setDecimalSeparator(const QChar &);
  static void setNegativeMonetarySignPosition(const eMyMoney::Money::signPosition pos);
  static void setPositiveMonetarySignPosition(const eMyMoney::Money::signPosition pos);
  static void setNegativePrefixCurrencySymbol(const bool flag);
  static void setPositivePrefixCurrencySymbol(const bool flag);
  static void setNegativeSpaceSeparatesSymbol(const bool flag);
  static void setPositiveSpaceSeparatesSymbol(const bool flag);

  static const QChar thousandSeparator();
  static const QChar decimalSeparator();
  static eMyMoney::Money::signPosition negativeMonetarySignPosition();
  static eMyMoney::Money::signPosition positiveMonetarySignPosition();

  const MyMoneyMoney& operator=(const QString& pszAmount);
  const MyMoneyMoney& operator=(const AlkValue& val);
  const MyMoneyMoney& operator=(const MyMoneyMoney& val);

  // comparison
  bool operator==(const MyMoneyMoney& Amount) const;
  bool operator!=(const MyMoneyMoney& Amount) const;
  bool operator<(const MyMoneyMoney& Amount) const;
  bool operator>(const MyMoneyMoney& Amount) const;
  bool operator<=(const MyMoneyMoney& Amount) const;
  bool operator>=(const MyMoneyMoney& Amount) const;

  bool operator==(const QString& pszAmount) const;
  bool operator!=(const QString& pszAmount) const;
  bool operator<(const QString& pszAmount) const;
  bool operator>(const QString& pszAmount) const;
  bool operator<=(const QString& pszAmount) const;
  bool operator>=(const QString& pszAmount) const;

  // calculation
  const MyMoneyMoney operator+(const MyMoneyMoney& Amount) const;
  const MyMoneyMoney operator-(const MyMoneyMoney& Amount) const;
  const MyMoneyMoney operator*(const MyMoneyMoney& factor) const;
  const MyMoneyMoney operator/(const MyMoneyMoney& Amount) const;
  const MyMoneyMoney operator-() const;
  const MyMoneyMoney operator*(int factor) const;

  static MyMoneyMoney maxValue;
  static MyMoneyMoney minValue;
  static MyMoneyMoney autoCalc;

  bool isNegative() const;
  bool isPositive() const;
  bool isZero() const;
  bool isAutoCalc() const;

  MyMoneyMoney reduce() const;

  static const MyMoneyMoney ONE;
  static const MyMoneyMoney MINUS_ONE;
};

//=============================================================================
//
//  Inline functions
//
//=============================================================================

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object set to 0.
//   Returns: None
//    Throws: Nothing.
// Arguments: None
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney() :
    AlkValue()
{
}


////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a double value
//   Returns: None
//    Throws: Nothing.
// Arguments: dAmount - double object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const double dAmount, const unsigned int denom) :
    AlkValue(dAmount, denom)
{
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Copy Constructor - constructs object from another
//            MyMoneyMoney object
//   Returns: None
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be copied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const MyMoneyMoney& Amount) :
    AlkValue(Amount)
{
}

inline MyMoneyMoney::MyMoneyMoney(const AlkValue& Amount) :
    AlkValue(Amount)
{
}

inline const MyMoneyMoney& MyMoneyMoney::operator=(const AlkValue & val)
{
  AlkValue::operator=(val);
  return *this;
}

inline const MyMoneyMoney& MyMoneyMoney::operator=(const MyMoneyMoney & val)
{
  AlkValue::operator=(val);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input NULL terminated
//            string
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: pszAmount - NULL terminated string that contains amount
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(const QString & pszAmount)
{
  AlkValue::operator=(pszAmount);
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input MyMoneyMoney object
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(const MyMoneyMoney& Amount) const
{
  return AlkValue::operator==(Amount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input MyMoneyMoney object
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(const MyMoneyMoney& Amount) const
{
  return AlkValue::operator!=(Amount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<
//   Purpose: Compare less than operator - compares object with input MyMoneyMoney object
//   Returns: true if object less than input amount
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<(const MyMoneyMoney& Amount) const
{
  return AlkValue::operator<(Amount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>
//   Purpose: Compare greater than operator - compares object with input MyMoneyMoney
//            object
//   Returns: true if object greater than input amount
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>(const MyMoneyMoney& Amount) const
{
  return AlkValue::operator>(Amount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator<=
//   Purpose: Compare less than equal to operator - compares object with input
//            MyMoneyMoney object
//   Returns: true if object less than or equal to input amount
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator<=(const MyMoneyMoney& Amount) const
{
  return AlkValue::operator<=(Amount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator>=
//   Purpose: Compare greater than equal to operator - compares object with input
//            MyMoneyMoney object
//   Returns: true if object greater than or equal to input amount
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be compared with
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator>=(const MyMoneyMoney& Amount) const
{
  return AlkValue::operator>=(Amount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator==
//   Purpose: Compare equal operator - compares object with input amount in a
//            NULL terminated string
//   Returns: true if equal, otherwise false
//    Throws: Nothing.
// Arguments: pszAmount - NULL terminated string that contains amount
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator==(const QString& pszAmount) const
{
  return *this == MyMoneyMoney(pszAmount);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator!=
//   Purpose: Compare not equal operator - compares object with input amount in
//            a NULL terminated string
//   Returns: true if not equal, otherwise false
//    Throws: Nothing.
// Arguments: pszAmount - NULL terminated string that contains amount
//
////////////////////////////////////////////////////////////////////////////////
inline bool MyMoneyMoney::operator!=(const QString& pszAmount) const
{
  return ! operator==(pszAmount) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Unary operator - returns the negative value from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: None
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney MyMoneyMoney::operator-() const
{
  return static_cast<const MyMoneyMoney>(AlkValue::operator-());
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the object with factor
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney MyMoneyMoney::operator*(int factor) const
{
  return static_cast<const MyMoneyMoney>(AlkValue::operator*(factor));
}

/**
  * Make it possible to hold @ref MyMoneyMoney objects
  * inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyMoney)

#endif

