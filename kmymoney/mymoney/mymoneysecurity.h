/***************************************************************************
                          mymoneysecurity.h  -  description
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
// krazy:excludeall=dpointer

#ifndef MYMONEYSECURITY_H
#define MYMONEYSECURITY_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <qobjectdefs.h>
#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkvalue.h>
#include "mymoneyobject.h"
#include "mymoneykeyvaluecontainer.h"
#include "mymoneyenums.h"

/**
  * Class that holds all the required information about a security that the user
  * has entered information about. A security can be a stock, a mutual fund, a bond
  * or a currency.
  *
  * @author Kevin Tambascio
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

class QString;
class KMM_MYMONEY_EXPORT MyMoneySecurity : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneySecurity();
  explicit MyMoneySecurity(const QString& id,
                           const MyMoneySecurity& equity);
  explicit MyMoneySecurity(const QString& id,
                           const QString& name,
                           const QString& symbol = QString(),
                           const int smallestCashFraction = DEFAULT_CASH_FRACTION,
                           const int smallestAccountFraction = DEFAULT_ACCOUNT_FRACTION,
                           const int pricePrecision = DEFAULT_PRICE_PRECISION);
  explicit MyMoneySecurity(const QDomElement& node);
  ~MyMoneySecurity();

  bool operator < (const MyMoneySecurity&) const;

  /**
    * This operator tests for equality of two MyMoneySecurity objects
    */
  bool operator == (const MyMoneySecurity&) const;

  /**
    * This operator tests for inequality of this MyMoneySecurity object
    * and the one passed by @p r
    *
    * @param r the right side of the comparison
    */
  bool operator != (const MyMoneySecurity& r) const;

  const QString name() const;
  void setName(const QString& str);

  const QString tradingSymbol() const;
  void setTradingSymbol(const QString& str);

  eMyMoney::Security securityType() const;
  void setSecurityType(const eMyMoney::Security s);

  bool isCurrency() const;

  AlkValue::RoundingMethod roundingMethod() const;
  void setRoundingMethod(const AlkValue::RoundingMethod rnd);

  const QString tradingMarket() const;
  void setTradingMarket(const QString& str);

  const QString tradingCurrency() const;
  void setTradingCurrency(const QString& str);

  int smallestAccountFraction() const;
  void setSmallestAccountFraction(const int sf);

  int smallestCashFraction() const;
  void setSmallestCashFraction(const int cf);

  int pricePrecision() const;
  void setPricePrecision(const int pp);

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
   * This method checks if a reference to the given object exists. It returns,
   * a @p true if the object is referencing the one requested by the
   * parameter @p id. If it does not, this method returns @p false.
   *
   * @param id id of the object to be checked for references
   * @retval true This object references object with id @p id.
   * @retval false This object does not reference the object with id @p id.
   */
  bool hasReferenceTo(const QString& id) const;

  /**
   * This method is used to convert the internal representation of
   * an security type into a human readable format
   *
   * @param securityType enumerated representation of the security type.
   *                     For possible values, see MyMoneySecurity::eSECURITYTYPE
   *
   * @return QString representing the human readable form
   */
  static QString securityTypeToString(const eMyMoney::Security securityType);

  /**
   * This method is used to convert the internal representation of
   * an rounding method into a human readable format
   *
   * @param roundingMethod enumerated representation of the rouding method.
   *                     For possible values, see AlkValue::RoundingMethod
   *
   * @return QString representing the human readable form
   */
  static QString roundingMethodToString(const AlkValue::RoundingMethod roundingMethod);

private:
  QString                   m_name;
  QString                   m_tradingSymbol;
  QString                   m_tradingMarket;
  QString                   m_tradingCurrency;
  eMyMoney::Security        m_securityType;
  int                       m_smallestCashFraction;
  int                       m_smallestAccountFraction;
  int                       m_pricePrecision;
  AlkValue::RoundingMethod  m_roundingMethod;

  enum DefaultValues {
    DEFAULT_CASH_FRACTION = 100,
    DEFAULT_ACCOUNT_FRACTION = 100,
    DEFAULT_PRICE_PRECISION = 4,
  };

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

  static QString getAttrName(const Attribute attr);

  friend uint qHash(const Attribute, uint seed);
};


inline bool MyMoneySecurity::operator != (const MyMoneySecurity& r) const // krazy:exclude=inline
{
  return !(*this == r);
}

inline eMyMoney::Security MyMoneySecurity::securityType() const // krazy:exclude=inline
{
  return m_securityType;
}

inline void MyMoneySecurity::setSecurityType(const eMyMoney::Security s) // krazy:exclude=inline
{
  m_securityType = s;
}

inline bool MyMoneySecurity::isCurrency() const // krazy:exclude=inline
{
  return m_securityType == eMyMoney::Security::Currency;
}

inline AlkValue::RoundingMethod MyMoneySecurity::roundingMethod() const // krazy:exclude=inline
{
  return m_roundingMethod;
}

inline void MyMoneySecurity::setRoundingMethod(const AlkValue::RoundingMethod rnd) // krazy:exclude=inline
{
  m_roundingMethod = rnd;
}

inline int MyMoneySecurity::smallestAccountFraction() const // krazy:exclude=inline
{
  return m_smallestAccountFraction;
}

inline void MyMoneySecurity::setSmallestAccountFraction(const int sf) // krazy:exclude=inline
{
  m_smallestAccountFraction = sf;
}

inline int MyMoneySecurity::smallestCashFraction() const // krazy:exclude=inline
{
  return m_smallestCashFraction;
}

inline void MyMoneySecurity::setSmallestCashFraction(const int cf) // krazy:exclude=inline
{
  m_smallestCashFraction = cf;
}

inline int MyMoneySecurity::pricePrecision() const // krazy:exclude=inline
{
  return m_pricePrecision;
}

inline void MyMoneySecurity::setPricePrecision(const int pp) // krazy:exclude=inline
{
  m_pricePrecision = pp;
}

inline uint qHash(const MyMoneySecurity::Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); } // krazy:exclude=inline
/**
  * Make it possible to hold @ref MyMoneySecurity objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneySecurity)

#endif
