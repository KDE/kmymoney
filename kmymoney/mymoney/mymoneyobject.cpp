/***************************************************************************
                          mymoneyobject.cpp
                          -------------------
    copyright            : (C) 2005 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
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

#include "mymoneyobject.h"
#include "mymoneyobject_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

MyMoneyObject::MyMoneyObject(const QString& id) :
  d_ptr(new MyMoneyObjectPrivate)
{
  Q_D(MyMoneyObject);
  d->m_id = id;
}

MyMoneyObject::MyMoneyObject() :
  d_ptr(new MyMoneyObjectPrivate)
{
}

MyMoneyObject::MyMoneyObject(MyMoneyObjectPrivate &dd) :
  d_ptr(&dd)
{
}

MyMoneyObject::MyMoneyObject(MyMoneyObjectPrivate &dd,
                             const QString& id) :
  d_ptr(&dd)
{
  Q_D(MyMoneyObject);
  d->m_id = id;
}

MyMoneyObject::MyMoneyObject(MyMoneyObjectPrivate &dd,
                             const QDomElement& node,
                             bool forceId) :
  d_ptr(&dd)
{
  Q_D(MyMoneyObject);
  d->m_id = node.attribute(QStringLiteral("id"));
  if (d->m_id.length() == 0 && forceId)
    throw MYMONEYEXCEPTION("Node has no ID");
}

MyMoneyObject::~MyMoneyObject()
{
  Q_D(MyMoneyObject);
  delete d;
}

QString MyMoneyObject::id() const
{
  Q_D(const MyMoneyObject);
  return d->m_id;
}

bool MyMoneyObject::operator == (const MyMoneyObject& right) const
{
  Q_D(const MyMoneyObject);
  return d->m_id == right.d_func()->m_id;
}

void MyMoneyObject::clearId()
{
  Q_D(MyMoneyObject);
  d->m_id.clear();
}
