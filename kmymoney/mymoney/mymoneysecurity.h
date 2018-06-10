/*
 * Copyright 2005-2017  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef MYMONEYSECURITY_H
#define MYMONEYSECURITY_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <alkimia/alkvalue.h>
#include "mymoneyobject.h"
#include "mymoneykeyvaluecontainer.h"

class QString;

namespace eMyMoney { namespace Security { enum class Type; } }

/**
  * Class that holds all the required information about a security that the user
  * has entered information about. A security can be a stock, a mutual fund, a bond
  * or a currency.
  *
  * @author Kevin Tambascio
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */

class MyMoneySecurityPrivate;
class KMM_MYMONEY_EXPORT MyMoneySecurity : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  Q_DECLARE_PRIVATE_D(MyMoneyObject::d_ptr, MyMoneySecurity)

  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneySecurity();
  explicit MyMoneySecurity(const QString &id);

  explicit MyMoneySecurity(const QString& id,
                           const QString& name,
                           const QString& symbol = QString(),
                           const int smallestCashFraction = 100,
                           const int smallestAccountFraction = 100,
                           const int pricePrecision = 4);

  explicit MyMoneySecurity(const QDomElement& node);

  MyMoneySecurity(const QString& id,
                  const MyMoneySecurity& other);

  MyMoneySecurity(const MyMoneySecurity & other);
  MyMoneySecurity(MyMoneySecurity && other);
  MyMoneySecurity & operator=(MyMoneySecurity other);
  friend void swap(MyMoneySecurity& first, MyMoneySecurity& second);

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

  QString name() const;
  void setName(const QString& str);

  QString tradingSymbol() const;
  void setTradingSymbol(const QString& str);

  eMyMoney::Security::Type securityType() const;
  void setSecurityType(const eMyMoney::Security::Type s);

  bool isCurrency() const;

  AlkValue::RoundingMethod roundingMethod() const;
  void setRoundingMethod(const AlkValue::RoundingMethod rnd);

  QString tradingMarket() const;
  void setTradingMarket(const QString& str);

  QString tradingCurrency() const;
  void setTradingCurrency(const QString& str);

  int smallestAccountFraction() const;
  void setSmallestAccountFraction(const int sf);

  int smallestCashFraction() const;
  void setSmallestCashFraction(const int cf);

  int pricePrecision() const;
  void setPricePrecision(const int pp);

  void writeXML(QDomDocument& document, QDomElement& parent) const override;

  /**
   * This method checks if a reference to the given object exists. It returns,
   * a @p true if the object is referencing the one requested by the
   * parameter @p id. If it does not, this method returns @p false.
   *
   * @param id id of the object to be checked for references
   * @retval true This object references object with id @p id.
   * @retval false This object does not reference the object with id @p id.
   */
  bool hasReferenceTo(const QString& id) const override;

  /**
   * This method is used to convert the internal representation of
   * an security type into a human readable format
   *
   * @param securityType enumerated representation of the security type.
   *                     For possible values, see MyMoneySecurity::eSECURITYTYPE
   *
   * @return QString representing the human readable form
   */
  static QString securityTypeToString(const eMyMoney::Security::Type securityType);

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
};

inline void swap(MyMoneySecurity& first, MyMoneySecurity& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.MyMoneyObject::d_ptr, second.MyMoneyObject::d_ptr);
  swap(first.MyMoneyKeyValueContainer::d_ptr, second.MyMoneyKeyValueContainer::d_ptr);
}

inline MyMoneySecurity::MyMoneySecurity(MyMoneySecurity && other) : MyMoneySecurity() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneySecurity & MyMoneySecurity::operator=(MyMoneySecurity other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}


/**
  * Make it possible to hold @ref MyMoneySecurity objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneySecurity)

#endif
