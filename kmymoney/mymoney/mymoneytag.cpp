/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneytag.h"
#include "mymoneytag_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

MyMoneyTag MyMoneyTag::null;

MyMoneyTag::MyMoneyTag() :
  MyMoneyObject(*new MyMoneyTagPrivate)
{
}

MyMoneyTag::MyMoneyTag(const QString &id) :
  MyMoneyObject(*new MyMoneyTagPrivate, id)
{
}

MyMoneyTag::MyMoneyTag(const QString& name, const QColor& tagColor) :
  MyMoneyObject(*new MyMoneyTagPrivate)
{
  Q_D(MyMoneyTag);
  d->m_name      = name;
  d->m_tag_color = tagColor;
}

MyMoneyTag::MyMoneyTag(const MyMoneyTag& other) :
  MyMoneyObject(*new MyMoneyTagPrivate(*other.d_func()), other.id())
{
}

MyMoneyTag::MyMoneyTag(const QString& id, const MyMoneyTag& other) :
  MyMoneyObject(*new MyMoneyTagPrivate(*other.d_func()), id)
{
}

MyMoneyTag::~MyMoneyTag()
{
}

QString MyMoneyTag::name() const
{
  Q_D(const MyMoneyTag);
  return d->m_name;
}

void MyMoneyTag::setName(const QString& val)
{
  Q_D(MyMoneyTag);
  d->m_name = val;
}

bool MyMoneyTag::isClosed() const
{
  Q_D(const MyMoneyTag);
  return d->m_closed;
}

void MyMoneyTag::setClosed(bool val)
{
  Q_D(MyMoneyTag);
  d->m_closed = val;
}

QColor MyMoneyTag::tagColor() const
{
  Q_D(const MyMoneyTag);
  return d->m_tag_color;
}

void MyMoneyTag::setTagColor(const QColor& val)
{
  Q_D(MyMoneyTag);
  d->m_tag_color = val;
}

void MyMoneyTag::setNamedTagColor(const QString &val)
{
  Q_D(MyMoneyTag);
  d->m_tag_color.setNamedColor(val);
}

QString MyMoneyTag::notes() const
{
  Q_D(const MyMoneyTag);
  return d->m_notes;
}

void MyMoneyTag::setNotes(const QString& val)
{
  Q_D(MyMoneyTag);
  d->m_notes = val;
}

bool MyMoneyTag::operator == (const MyMoneyTag& right) const
{
  Q_D(const MyMoneyTag);
  auto d2 = static_cast<const MyMoneyTagPrivate *>(right.d_func());
  return (MyMoneyObject::operator==(right) &&
          ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)) &&
          ((d->m_tag_color.isValid() == false && d2->m_tag_color.isValid() == false) || (d->m_tag_color.name() == d2->m_tag_color.name())) &&
          (d->m_closed == d2->m_closed));
}

bool MyMoneyTag::operator < (const MyMoneyTag& right) const
{
  Q_D(const MyMoneyTag);
  auto d2 = static_cast<const MyMoneyTagPrivate *>(right.d_func());
  return d->m_name < d2->m_name;
}

bool MyMoneyTag::hasReferenceTo(const QString& /*id*/) const
{
  return false;
}

QSet<QString> MyMoneyTag::referencedObjects() const
{
  return {};
}
