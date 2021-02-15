/*
 * SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mymoneytag.h"
#include "mymoneytag_p.h"

// ----------------------------------------------------------------------------
// QT Includes

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
