/*
 * SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mymoneyobject.h"
#include "mymoneyobject_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

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
