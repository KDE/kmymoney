/*
 * Copyright 2012       Alessandro Russo <axela74@yahoo.it>
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

#include "mymoneytag.h"
#include "mymoneytag_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomDocument>
#include <QDomElement>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

MyMoneyTag MyMoneyTag::null;

MyMoneyTag::MyMoneyTag() :
  MyMoneyObject(*new MyMoneyTagPrivate)
{
}

MyMoneyTag::MyMoneyTag(const QString &id) :
  MyMoneyObject(*new MyMoneyTagPrivate, id)
{
}

MyMoneyTag::MyMoneyTag(const QString& name, const QColor& tabColor) :
  MyMoneyObject(*new MyMoneyTagPrivate)
{
  Q_D(MyMoneyTag);
  d->m_name      = name;
  d->m_tag_color = tabColor;
}

MyMoneyTag::MyMoneyTag(const QDomElement& node) :
  MyMoneyObject(*new MyMoneyTagPrivate, node)
{
  if (nodeNames[nnTag] != node.tagName()) {
    throw MYMONEYEXCEPTION_CSTRING("Node was not TAG");
  }
  Q_D(MyMoneyTag);
  d->m_name = node.attribute(d->getAttrName(Tag::Attribute::Name));
  if (node.hasAttribute(d->getAttrName(Tag::Attribute::TagColor))) {
    d->m_tag_color.setNamedColor(node.attribute(d->getAttrName(Tag::Attribute::TagColor)));
  }
  if (node.hasAttribute(d->getAttrName(Tag::Attribute::Notes))) {
    d->m_notes = node.attribute(d->getAttrName(Tag::Attribute::Notes));
  }
  d->m_closed = node.attribute(d->getAttrName(Tag::Attribute::Closed), "0").toUInt();
}

MyMoneyTag::MyMoneyTag(const MyMoneyTag& other) :
  MyMoneyObject(*new MyMoneyTagPrivate(*other.d_func()), other.id())
{
}

MyMoneyTag::MyMoneyTag(const QString& id, const MyMoneyTag& other) :
  MyMoneyObject(*new MyMoneyTagPrivate(*other.d_func()), id)
{
  Q_D(MyMoneyTag);
  d->m_tag_color = QColor("black");
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

void MyMoneyTag::writeXML(QDomDocument& document, QDomElement& parent) const
{
  auto el = document.createElement(nodeNames[nnTag]);

  Q_D(const MyMoneyTag);
  d->writeBaseXML(document, el);

  el.setAttribute(d->getAttrName(Tag::Attribute::Name), d->m_name);
  el.setAttribute(d->getAttrName(Tag::Attribute::Closed), d->m_closed);
  if (d->m_tag_color.isValid())
    el.setAttribute(d->getAttrName(Tag::Attribute::TagColor), d->m_tag_color.name());
  if (!d->m_notes.isEmpty())
    el.setAttribute(d->getAttrName(Tag::Attribute::Notes), d->m_notes);
  parent.appendChild(el);
}

bool MyMoneyTag::hasReferenceTo(const QString& /*id*/) const
{
  return false;
}
