/*
    SPDX-FileCopyrightText: 2005-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYPRICE_H
#define MYMONEYPRICE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"

class QString;
class QDate;
class QDomElement;

class MyMoneyMoney;

template <class T1, class T2> class QMap;
template <class T1, class T2> struct QPair;

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents an exchange rate of a security, currency or commodity
  * based on another security, currency or commodity for a specific date.
  * The term security is used in this class as a placeholder for all
  * those previously mentioned items.
  * In general, the other security is a currency.
  *
  * The securities and the rate form the following equation:
  *
  * @code
  *
  *   toSecurity = rate * fromSecurity
  *
  * @endcode
  *
  * Using the @p rate() member function, one can retrieve the conversion rate based
  * upon the @p toSecurity or the @p fromSecurity.
  */
class MyMoneyPricePrivate;
class KMM_MYMONEY_EXPORT MyMoneyPrice
{
    Q_DECLARE_PRIVATE(MyMoneyPrice)
    MyMoneyPricePrivate * d_ptr;

public:
    MyMoneyPrice();
    explicit MyMoneyPrice(const QString& from,
                          const QString& to,
                          const QDomElement& node);
    explicit MyMoneyPrice(const QString& from,
                          const QString& to,
                          const QDate& date,
                          const MyMoneyMoney& rate,
                          const QString& source);
    MyMoneyPrice(const MyMoneyPrice & other);
    MyMoneyPrice(MyMoneyPrice && other);
    MyMoneyPrice & operator=(MyMoneyPrice other);
    friend void swap(MyMoneyPrice& first, MyMoneyPrice& second);
    virtual ~MyMoneyPrice();

    /**
      * This method returns the price information based on the
      * security referenced by @p id. If @p id is empty (default), the
      * price is returned based on the toSecurity. If this price
      * object is invalid (see isValid()) MyMoneyMoney(1,1) is returned.
      *
      * @param id return price to be the factor to be used to convert a value into
      *           the corresponding value in security @p id.
      *
      * @return returns the exchange rate (price) as MyMoneyMoney object.
      *
      * If @p id is not empty and does not match either security ids of this price
      * an exception will be thrown.
      *
      * Example:
      * Assume the following code, where you have a price object and
      * and you wish to convert from an amount in GBP (@p valGBP) to ADF (@p valADF).
      * Then your code will look like this:
      *
      * @code
      *
      * MyMoneyPrice price("ADF", "GBP", QDate(2005,9,20), MyMoneyMoney(1,3), "User");
      * MyMoneyMoney valADF, valGBP(100,1);
      *
      * valADF = valGBP * price.rate("ADF");
      *
      * @endcode
      *
      * valADF will contain the value 300 after the assignment operation, because @p price.rate("ADF") returned
      * @p 3/1 even though the price information kept with the object was @p 1/3, but based on the other
      * conversion direction (from ADF to GBP).
      */
    MyMoneyMoney rate(const QString& id) const;

    QDate date() const;
    QString source() const;
    QString from() const;
    QString to() const;

    /**
      * Check whether the object is valid or not. A MyMoneyPrice object
      * is valid if the date is valid and both security ids are set. In case
      * of an invalid object, price() always returns 1.
      *
      * @retval true if price object is valid
      * @retval false if price object is not valid
      */
    bool isValid() const;

    // Equality operator
    bool operator == (const MyMoneyPrice &) const;

    // Inequality operator
    bool operator != (const MyMoneyPrice &right) const;

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
     * @copydoc MyMoneyObject::referencedObjects
     */
    QSet<QString> referencedObjects() const;
};

inline void swap(MyMoneyPrice& first, MyMoneyPrice& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyPrice::MyMoneyPrice(MyMoneyPrice && other) : MyMoneyPrice() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyPrice & MyMoneyPrice::operator=(MyMoneyPrice other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

typedef QPair<QString, QString> MyMoneySecurityPair;
typedef QMap<QDate, MyMoneyPrice> MyMoneyPriceEntries;
typedef QMap<MyMoneySecurityPair, MyMoneyPriceEntries> MyMoneyPriceList;

/**
  * Make it possible to hold @ref MyMoneyPrice objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyPrice)

#endif
