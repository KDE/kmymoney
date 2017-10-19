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
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

/**
  * Class that holds all the required information about a security that the user
  * has entered information about. A security can be a stock, a mutual fund, a bond
  * or a currency.
  *
  * @author Kevin Tambascio
  * @author Thomas Baumgart
  */
class KMM_MYMONEY_EXPORT MyMoneySecurity : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  Q_GADGET
  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneySecurity();
  MyMoneySecurity(const QString& id, const MyMoneySecurity& equity);
  MyMoneySecurity(const QString& id, const QString& name, const QString& symbol = QString(),
                  const int smallestCashFraction = DEFAULT_CASH_FRACTION,
                  const int smallestAccountFraction = DEFAULT_ACCOUNT_FRACTION,
                  const int pricePrecision = DEFAULT_PRICE_PRECISION);
  MyMoneySecurity(const QDomElement& node);
  virtual ~MyMoneySecurity();

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
  bool operator != (const MyMoneySecurity& r) const {
    return !(*this == r);
  }

public:
  typedef enum {
    SECURITY_STOCK,
    SECURITY_MUTUALFUND,
    SECURITY_BOND,
    SECURITY_CURRENCY,
    SECURITY_NONE
  } eSECURITYTYPE;

  enum attrNameE { anName, anSymbol, anType, anRoundingMethod,
                   anSAF, anPP, anSCF,
                   anTradingCurrency, anTradingMarket
                 };
  Q_ENUM(attrNameE)

  const QString& name() const                 {
    return m_name;
  }

  void           setName(const QString& str)   {
    m_name = str;
  }

  const QString&  tradingSymbol() const               {
    return m_tradingSymbol;
  }

  void            setTradingSymbol(const QString& str) {
    m_tradingSymbol = str;
  }

  eSECURITYTYPE securityType() const                {
    return m_securityType;
  }

  void          setSecurityType(const eSECURITYTYPE& s)   {
    m_securityType = s;
  }

  bool    isCurrency() const {
    return m_securityType == SECURITY_CURRENCY;
  }

  AlkValue::RoundingMethod roundingMethod() const  {
    return m_roundingMethod;
  }

  void           setRoundingMethod(const AlkValue::RoundingMethod& rnd) {
    m_roundingMethod = rnd;
  }

  const QString& tradingMarket() const  {
    return m_tradingMarket;
  }

  void           setTradingMarket(const QString& str) {
    m_tradingMarket = str;
  }

  const QString& tradingCurrency() const {
    return m_tradingCurrency;
  }

  void           setTradingCurrency(const QString& str) {
    m_tradingCurrency = str;
  }

  int smallestAccountFraction() const {
    return m_smallestAccountFraction;
  }

  void setSmallestAccountFraction(const int sf) {
    m_smallestAccountFraction = sf;
  }

  int smallestCashFraction() const {
    return m_smallestCashFraction;
  }

  void setSmallestCashFraction(const int sf) {
    m_smallestCashFraction = sf;
  }

  int pricePrecision() const {
    return m_pricePrecision;
  }

  void setPricePrecision(const int pp) {
    m_pricePrecision = pp;
  }

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
  static QString securityTypeToString(const MyMoneySecurity::eSECURITYTYPE securityType);

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

protected:
  QString                   m_name;
  QString                   m_tradingSymbol;
  QString                   m_tradingMarket;
  QString                   m_tradingCurrency;
  eSECURITYTYPE             m_securityType;
  int                       m_smallestCashFraction;
  int                       m_smallestAccountFraction;
  int                       m_pricePrecision;
  AlkValue::RoundingMethod  m_roundingMethod;

private:
  enum DefaultValues {
      DEFAULT_CASH_FRACTION = 100,
      DEFAULT_ACCOUNT_FRACTION = 100,
      DEFAULT_PRICE_PRECISION = 4,
  };

  static const QString getAttrName(const attrNameE _attr);
};

/**
  * Make it possible to hold @ref MyMoneySecurity objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneySecurity)


#endif
