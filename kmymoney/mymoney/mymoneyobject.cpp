/*
 * Copyright 2005-2018  Thomas Baumgart <tbaumgart@kde.org>
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
