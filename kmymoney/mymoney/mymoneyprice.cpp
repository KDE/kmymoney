/***************************************************************************
                          mymoneyprice  -  description
                             -------------------
    begin                : Sun Nov 21 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
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

/**
  * @author Thomas Baumgart
  */

#include "mymoneyprice.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDate>
#include <QString>
#include <QDomElement>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneymoney.h"
#include "mymoneyexception.h"

class MyMoneyPricePrivate
{
public:
  QString       m_fromSecurity;
  QString       m_toSecurity;
  QDate         m_date;
  MyMoneyMoney  m_rate;
  MyMoneyMoney  m_invRate;
  QString       m_source;
};

MyMoneyPrice::MyMoneyPrice() :
  d_ptr(new MyMoneyPricePrivate)
{
}

MyMoneyPrice::MyMoneyPrice(const QString& from,
                           const QString& to,
                           const QDomElement& node) :
  d_ptr(new MyMoneyPricePrivate)
{
  if ("PRICE" != node.tagName())
    throw MYMONEYEXCEPTION("Node was not PRICE");

  Q_D(MyMoneyPrice);
  d->m_fromSecurity = from;
  d->m_toSecurity = to;

  d->m_date = QDate::fromString(node.attribute("date"), Qt::ISODate);
  d->m_rate = MyMoneyMoney(node.attribute("price"));
  d->m_source = node.attribute("source");

  if (!d->m_rate.isZero())
    d->m_invRate = MyMoneyMoney::ONE / d->m_rate;
  else
    qDebug("Price with zero value loaded");
}

MyMoneyPrice::MyMoneyPrice(const QString& from, const QString& to, const QDate& date, const MyMoneyMoney& rate, const QString& source) :
  d_ptr(new MyMoneyPricePrivate)
{
  Q_D(MyMoneyPrice);
  d->m_fromSecurity =from;
  d->m_toSecurity = to;
  d->m_date = date;
  d->m_rate = rate;
  d->m_source = source;

  if (!d->m_rate.isZero())
    d->m_invRate = MyMoneyMoney::ONE / d->m_rate;
  else
    qDebug("Price with zero value created for '%s' to '%s'",
           qPrintable(from), qPrintable(to));
}

MyMoneyPrice::MyMoneyPrice(const MyMoneyPrice& other) :
  d_ptr(new MyMoneyPricePrivate(*other.d_func()))
{
}

MyMoneyPrice::~MyMoneyPrice()
{
  Q_D(MyMoneyPrice);
  delete d;
}

MyMoneyMoney MyMoneyPrice::rate(const QString& id) const
{
  Q_D(const MyMoneyPrice);
  static MyMoneyMoney dummyPrice(1, 1);

  if (!isValid())
    return dummyPrice;

  if (id.isEmpty() || id == d->m_toSecurity)
    return d->m_rate;
  if (id == d->m_fromSecurity)
    return d->m_invRate;

  QString msg = QString("Unknown security id %1 for price info %2/%3.").arg(id).arg(d->m_fromSecurity).arg(d->m_toSecurity);
  throw MYMONEYEXCEPTION(msg);
}

QDate MyMoneyPrice::date() const
{
  Q_D(const MyMoneyPrice);
  return d->m_date;
}

QString MyMoneyPrice::source() const
{
  Q_D(const MyMoneyPrice);
  return d->m_source;
}

QString MyMoneyPrice::from() const
{
  Q_D(const MyMoneyPrice);
  return d->m_fromSecurity;
}

QString MyMoneyPrice::to() const
{
  Q_D(const MyMoneyPrice);
  return d->m_toSecurity;
}

bool MyMoneyPrice::isValid() const
{
  Q_D(const MyMoneyPrice);
  return (d->m_date.isValid() && !d->m_fromSecurity.isEmpty() && !d->m_toSecurity.isEmpty());
}

// Equality operator
bool MyMoneyPrice::operator == (const MyMoneyPrice &right) const
{
  Q_D(const MyMoneyPrice);
  auto d2 = static_cast<const MyMoneyPricePrivate *>(right.d_func());
  return ((d->m_date == d2->m_date) &&
          (d->m_rate == d2->m_rate) &&
          ((d->m_fromSecurity.length() == 0 && d2->m_fromSecurity.length() == 0) || (d->m_fromSecurity == d2->m_fromSecurity)) &&
          ((d->m_toSecurity.length() == 0 && d2->m_toSecurity.length() == 0) || (d->m_toSecurity == d2->m_toSecurity)) &&
          ((d->m_source.length() == 0 && d2->m_source.length() == 0) || (d->m_source == d2->m_source)));
}

bool MyMoneyPrice::operator != (const MyMoneyPrice &right) const
{
  return !(operator == (right));
}

bool MyMoneyPrice::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyPrice);
  return (id == d->m_fromSecurity) || (id == d->m_toSecurity);
}
