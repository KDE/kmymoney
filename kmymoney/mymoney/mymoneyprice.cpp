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

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

MyMoneyPrice::MyMoneyPrice()
{
}

MyMoneyPrice::MyMoneyPrice(const QString& from, const QString& to, const QDomElement& node)
{
  if ("PRICE" != node.tagName())
    throw MYMONEYEXCEPTION("Node was not PRICE");

  m_fromSecurity = from;
  m_toSecurity = to;

  m_date = QDate::fromString(node.attribute("date"), Qt::ISODate);
  m_rate = MyMoneyMoney(node.attribute("price"));
  m_source = node.attribute("source");

  if (!m_rate.isZero())
    m_invRate = MyMoneyMoney(1, 1) / m_rate;
  else
    qDebug("Price with zero value loaded");
}

MyMoneyPrice::MyMoneyPrice(const QString& from, const QString& to, const QDate& date, const MyMoneyMoney& rate, const QString& source) :
    m_fromSecurity(from),
    m_toSecurity(to),
    m_date(date),
    m_rate(rate),
    m_source(source)
{
  if (!m_rate.isZero())
    m_invRate = MyMoneyMoney(1, 1) / m_rate;
  else
    qDebug("Price with zero value created for '%s' to '%s'",
           qPrintable(from), qPrintable(to));
}

MyMoneyPrice::~MyMoneyPrice()
{
}

const MyMoneyMoney& MyMoneyPrice::rate(const QString& id) const
{
  static MyMoneyMoney dummyPrice(1, 1);

  if (!isValid())
    return dummyPrice;

  if (id.isEmpty() || id == m_toSecurity)
    return m_rate;
  if (id == m_fromSecurity)
    return m_invRate;

  QString msg = QString("Unknown security id %1 for price info %2/%3.").arg(id).arg(m_fromSecurity).arg(m_toSecurity);
  throw MYMONEYEXCEPTION(msg);
}

bool MyMoneyPrice::isValid(void) const
{
  return (m_date.isValid() && !m_fromSecurity.isEmpty() && !m_toSecurity.isEmpty());
}

// Equality operator
bool MyMoneyPrice::operator == (const MyMoneyPrice &right) const
{
  return ((m_date == right.m_date) &&
          (m_rate == right.m_rate) &&
          ((m_fromSecurity.length() == 0 && right.m_fromSecurity.length() == 0) || (m_fromSecurity == right.m_fromSecurity)) &&
          ((m_toSecurity.length() == 0 && right.m_toSecurity.length() == 0) || (m_toSecurity == right.m_toSecurity)) &&
          ((m_source.length() == 0 && right.m_source.length() == 0) || (m_source == right.m_source)));
}

bool MyMoneyPrice::hasReferenceTo(const QString& id) const
{
  return (id == m_fromSecurity) || (id == m_toSecurity);
}
