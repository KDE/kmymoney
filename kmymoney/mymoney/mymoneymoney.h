/***************************************************************************
                          mymoneymoney.h
                             -------------------
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _MYMONEYMONEY_H
#define _MYMONEYMONEY_H

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif

#ifndef HAVE_ATOLL
#  ifdef HAVE_STRTOLL
#    define atoll(a) strtoll(a, 0, 10)
#  endif
#endif

#include <cmath>

#ifdef _GLIBCPP_HAVE_MODFL
#define HAVE_LONG_DOUBLE  1
#endif

#ifndef HAVE_LONG_DOUBLE
#define HAVE_LONG_DOUBLE  0
#endif

// So we can save this object
#include <QChar>
#include <QString>
#include <qdatastream.h>
#include <kmm_mymoney_export.h>
#include <mymoneyexception.h>

// Check for standard definitions
#ifdef HAVE_STDINT_H
  #ifndef __STDC_LIMIT_MACROS
    #define __STDC_LIMIT_MACROS         // force definition of min and max values
  #endif
  #include <stdint.h>
#else
  #include <limits.h>
  #define INT64_MAX LLONG_MAX
  #define INT64_MIN LLONG_MIN
#endif

typedef int64_t signed64;
typedef uint64_t unsigned64;

class MyMoneyAccount;
class MyMoneySecurity;

/**
  * This class represents a value within the MyMoney Engine
  *
  * @author Michael Edwardes
  */
class KMM_MYMONEY_EXPORT MyMoneyMoney
{
public:
  enum fileVersionE {
    FILE_4_BYTE_VALUE = 0,
    FILE_8_BYTE_VALUE
  };

  enum signPosition {
    // keep those in sync with the ones defined in klocale.h
    ParensAround = 0,
    BeforeQuantityMoney = 1,
    AfterQuantityMoney = 2,
    BeforeMoney = 3,
    AfterMoney = 4
  };

  enum roundingMethod {
    RndNever = 0,
    RndFloor,
    RndCeil,
    RndTrunc,
    RndPromote,
    RndHalfDown,
    RndHalfUp,
    RndRound
  };

  // construction
  MyMoneyMoney();
  explicit MyMoneyMoney( const int iAmount, const signed64 denom = 100 );
  MyMoneyMoney( const QString& pszAmount );
  explicit MyMoneyMoney( const signed64 Amount, const signed64 denom = 100  );
  explicit MyMoneyMoney( const double dAmount, const signed64 denom = 100  );
#if HAVE_LONG_DOUBLE
  explicit MyMoneyMoney( const long double dAmount, const signed64 denom = 100  );
#endif

  // copy constructor
  MyMoneyMoney( const MyMoneyMoney& AmountInPence );

  // signed64 value(const int prec = 2) const;
  const MyMoneyMoney abs(void) const { return m_num < 0 ? -(*this) : *this; };

  /**
    * This method returns a formatted string according to the settings
    * of _thousandSeparator, _decimalSeparator, _negativeMonetarySignPosition,
    * _positiveMonetaryPosition, _negativePrefixCurrencySymbol and
    * _positivePrefixCurrencySymbol. Those values can be modified using
    * the appropriate set-methods.
    *
    * @param currency The currency symbol
    * @param prec The number of fractional digits
    * @param showThousandSeparator should the thousandSeparator symbol be inserted
    *                              (@a true) or not (@a false) (default true)
    */
  QString formatMoney(const QString& currency, const int prec, bool showThousandSeparator = true) const;

  /**
   * This is a convenience method. It behaves exactly as the above one, but takes the information
   * about currency symbol and precision out of the MyMoneySecurity and MyMoneyAccount objects
   * @a acc and @a sec.
   */
  QString formatMoney(const MyMoneyAccount& acc, const MyMoneySecurity& sec, bool showThousandSeparator = true) const;

  /**
   * This is a convenience method. It behaves exactly as the above one, but takes the information
   * about currency symbol and precision out of the MyMoneySecurity object @a sec.
   */
  QString formatMoney(const MyMoneySecurity& sec, bool showThousandSeparator = true) const;

  /**
   * This is a convenience method. It behaves exactly as the above one, but takes the information
   * about precision out of the denomination @a denom. No currency symbol is shown. If you want
   * to see a currency symbol, please use formatMoney(const MyMoneyAccount& acc, const MyMoneySecurity& sec, bool showThousandSeparator)
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

  const QString toString(void) const;
  void fromString(const QString& str);
  const MyMoneyMoney convert(const signed64 denom = 100, const roundingMethod how = RndRound) const;
  static signed64 precToDenom(int prec);
  double toDouble(void) const;

  static void setThousandSeparator(const QChar &);
  static void setDecimalSeparator(const QChar &);
  static void setNegativeMonetarySignPosition(const signPosition pos);
  static void setPositiveMonetarySignPosition(const signPosition pos);
  static void setNegativePrefixCurrencySymbol(const bool flags);
  static void setPositivePrefixCurrencySymbol(const bool flags);

  static QChar thousandSeparator(void);
  static QChar decimalSeparator(void);
  static signPosition negativeMonetarySignPosition(void);
  static signPosition positiveMonetarySignPosition(void);
  static void setFileVersion(const fileVersionE version);

  // assignment
  const MyMoneyMoney& operator=( const MyMoneyMoney& Amount );
  const MyMoneyMoney& operator=( const QString& pszAmount );

  // comparison
  bool operator==( const MyMoneyMoney& Amount ) const;
  bool operator!=( const MyMoneyMoney& Amount ) const;
  bool operator<( const MyMoneyMoney& Amount ) const;
  bool operator>( const MyMoneyMoney& Amount ) const;
  bool operator<=( const MyMoneyMoney& Amount ) const;
  bool operator>=( const MyMoneyMoney& Amount ) const;

  bool operator==( const QString& pszAmount ) const;
  bool operator!=( const QString& pszAmount ) const;
  bool operator<( const QString& pszAmount ) const;
  bool operator>( const QString& pszAmount ) const;
  bool operator<=( const QString& pszAmount ) const;
  bool operator>=( const QString& pszAmount ) const;

  // calculation
  MyMoneyMoney operator+( const MyMoneyMoney& Amount ) const;

  MyMoneyMoney operator-( const MyMoneyMoney& Amount ) const;
  MyMoneyMoney operator-( ) const;

  MyMoneyMoney operator*( const MyMoneyMoney& factor ) const;
  MyMoneyMoney operator*( int factor ) const;
  MyMoneyMoney operator*( signed64 factor ) const;
  MyMoneyMoney operator/( const MyMoneyMoney& Amount ) const;

  // unary operators
  MyMoneyMoney& operator+= ( const MyMoneyMoney&  Amount );
  MyMoneyMoney& operator-= ( const MyMoneyMoney&  Amount );
  MyMoneyMoney& operator*= ( const MyMoneyMoney&  Amount );
  MyMoneyMoney& operator/= ( const MyMoneyMoney&  Amount );

  // conversion
  operator int() const;

  static MyMoneyMoney maxValue;
  static MyMoneyMoney minValue;
  static MyMoneyMoney autoCalc;

  bool isNegative() const { return (m_num < 0) ? true : false; }
  bool isPositive() const { return (m_num > 0) ? true : false; }
  bool isZero() const { return m_num == 0; }
  bool isAutoCalc(void) const { return (*this == autoCalc); }

  const MyMoneyMoney reduce(void) const;

private:
  signed64 m_num;
  signed64 m_denom;

  signed64 getLcd(const MyMoneyMoney& b) const;

  KMM_MYMONEY_EXPORT friend QDataStream &operator<<(QDataStream &, const MyMoneyMoney &);
  KMM_MYMONEY_EXPORT friend QDataStream &operator>>(QDataStream &, MyMoneyMoney &);

  static QChar _thousandSeparator;
  static QChar _decimalSeparator;
  static signPosition _negativeMonetarySignPosition;
  static signPosition _positiveMonetarySignPosition;
  static bool _negativePrefixCurrencySymbol;
  static bool _positivePrefixCurrencySymbol;
  static MyMoneyMoney::fileVersionE _fileVersion;

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
inline MyMoneyMoney::MyMoneyMoney()
{
  m_num = 0;
  m_denom = 1;
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
inline MyMoneyMoney::MyMoneyMoney(signed64 Amount, const signed64 denom)
{
  if(!denom)
    throw new MYMONEYEXCEPTION("Denominator 0 not allowed!");

  m_num = Amount;
  m_denom = denom;
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
inline MyMoneyMoney::MyMoneyMoney(const double dAmount, const signed64 denom)
{
  double adj = dAmount < 0 ? -0.5 : 0.5;
  m_denom = denom;
  m_num = (signed64) (dAmount * (double)m_denom + adj);
}

#if HAVE_LONG_DOUBLE
////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a long double value
//   Returns: None
//    Throws: Nothing.
// Arguments: dAmount - long double object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const long double dAmount, const signed64 denom)
{
  long double adj = dAmount < 0 ? -0.5 : 0.5;
  m_denom = denom;
  m_num = static_cast<signed64> (dAmount * m_denom + adj);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a integer value
//   Returns: None
//    Throws: Nothing.
// Arguments: iAmount - integer object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const int iAmount, const signed64 denom)
{
  if(!denom)
    throw new MYMONEYEXCEPTION("Denominator 0 not allowed!");

  m_num = static_cast<signed64>(iAmount);
  m_denom = denom;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Copy Constructor - constructs object from another MyMoneyMoney object
//   Returns: None
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be copied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney::MyMoneyMoney(const MyMoneyMoney& Amount)
 : m_num (Amount.m_num), m_denom(Amount.m_denom)
{ }

////////////////////////////////////////////////////////////////////////////////
//      Name: operator=
//   Purpose: Assignment operator - modifies object from input MyMoneyMoney object
//   Returns: Const reference to the object
//    Throws: Nothing.
// Arguments: Amount - MyMoneyMoney object to be modified from
//
////////////////////////////////////////////////////////////////////////////////
inline const MyMoneyMoney& MyMoneyMoney::operator=(const MyMoneyMoney& Amount)
{
  m_num = Amount.m_num;
  m_denom = Amount.m_denom;
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
inline const MyMoneyMoney& MyMoneyMoney::operator=(const QString& pszAmount)
{
  *this = MyMoneyMoney( pszAmount );
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
  if(m_denom == Amount.m_denom)
    return m_num == Amount.m_num;

  if(m_num == 0 && Amount.m_num == 0)
    return true;

  return (*this - Amount).m_num == 0;
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
  if(m_num == Amount.m_num && m_denom == Amount.m_denom)
    return false;

  return (*this - Amount).m_num != 0;
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
  if(m_denom == Amount.m_denom)
    return (m_num < Amount.m_num);

  signed64 ab, ba;

  ab = m_num * Amount.m_denom;
  ba = m_denom * Amount.m_num;

  return ( ab < ba ) ;
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
  if(m_denom == Amount.m_denom)
    return (m_num > Amount.m_num);

  signed64 ab, ba;

  ab = m_num * Amount.m_denom;
  ba = m_denom * Amount.m_num;

  return ( ab > ba ) ;
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
  if(m_denom == Amount.m_denom)
    return (m_num <= Amount.m_num);

  signed64 ab, ba;

  ab = m_num * Amount.m_denom;
  ba = m_denom * Amount.m_num;

  return ( ab <= ba ) ;
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
  if(m_denom == Amount.m_denom)
    return (m_num >= Amount.m_num);

  signed64 ab, ba;

  ab = m_num * Amount.m_denom;
  ba = m_denom * Amount.m_num;

  return ( ab >= ba ) ;
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
  MyMoneyMoney Amount( pszAmount );
  return ( *this == Amount ) ;
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
  MyMoneyMoney Amount( pszAmount );
  return ( *this != Amount ) ;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Unary operator - returns the negative value from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: None
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator-() const
{
  MyMoneyMoney result(*this);
  result.m_num = -result.m_num;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the object with factor
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - signed64 object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator*(signed64 factor) const
{
  MyMoneyMoney result(*this);
  result.m_num *= factor;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the object with factor
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - long object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney MyMoneyMoney::operator*(int factor) const
{
  MyMoneyMoney result(*this);
  result.m_num *= factor;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+=
//   Purpose: Addition operator - adds the input amount to the object together
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be added
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator+=(const MyMoneyMoney& AmountInPence)
{
  *this = *this + AmountInPence;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator-=
//   Purpose: Subtraction operator - subtracts the input amount from the object
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator-=(const MyMoneyMoney& AmountInPence)
{
  *this = *this - AmountInPence;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*=
//   Purpose: Multiplication operator - multiplies the input amount by the object
//   Returns: Reference to the current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
inline MyMoneyMoney& MyMoneyMoney::operator*=(const MyMoneyMoney& AmountInPence)
{
  *this = *this * AmountInPence;
  return *this;
}

inline MyMoneyMoney& MyMoneyMoney::operator/=(const MyMoneyMoney& AmountInPence)
{
  *this = *this / AmountInPence;
  return *this;
}

#endif

