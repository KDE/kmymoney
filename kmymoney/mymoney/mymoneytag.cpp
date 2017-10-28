/***************************************************************************
                          mymoneytag.cpp
                             -------------------
    copyright            : (C) 2012 by Alessandro Russo <alessandro@russo.it>
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

#include "mymoneytag.h"
#include <mymoneyexception.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDomDocument>
#include <QDomElement>
#include <QHash>
#include <QColor>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

MyMoneyTag MyMoneyTag::null;

class MyMoneyTagPrivate {

public:

  MyMoneyTagPrivate() :
    m_closed(false)
  {
  }

  // Simple fields
  QString m_name;
  // Closed tags will not be shown in the selector inside a transaction, only in the Tag tab
  bool m_closed;
  // Set the color showed in the ledger
  QColor m_tag_color;
  QString m_notes;
};

MyMoneyTag::MyMoneyTag() :
  d_ptr(new MyMoneyTagPrivate)
{
}

MyMoneyTag::MyMoneyTag(const QString& name) :
  d_ptr(new MyMoneyTagPrivate)
{
  Q_D(MyMoneyTag);
  d->m_name      = name;
  d->m_tag_color = QColor();
}

MyMoneyTag::MyMoneyTag(const QString& name, const QColor& tabColor) :
  d_ptr(new MyMoneyTagPrivate)
{
  Q_D(MyMoneyTag);
  d->m_name      = name;
  d->m_tag_color = tabColor;
}

MyMoneyTag::MyMoneyTag(const QDomElement& node) :
  MyMoneyObject(node),
  d_ptr(new MyMoneyTagPrivate)
{
  if (nodeNames[nnTag] != node.tagName()) {
    throw MYMONEYEXCEPTION("Node was not TAG");
  }
  Q_D(MyMoneyTag);
  d->m_name = node.attribute(getAttrName(Attribute::Name));
  if (node.hasAttribute(getAttrName(Attribute::TagColor))) {
    d->m_tag_color.setNamedColor(node.attribute(getAttrName(Attribute::TagColor)));
  }
  if (node.hasAttribute(getAttrName(Attribute::Notes))) {
    d->m_notes = node.attribute(getAttrName(Attribute::Notes));
  }
  d->m_closed = node.attribute(getAttrName(Attribute::Closed), "0").toUInt();
}

MyMoneyTag::MyMoneyTag(const MyMoneyTag& other) :
  MyMoneyObject(other.id()),
  d_ptr(new MyMoneyTagPrivate(*other.d_func()))
{
}

MyMoneyTag::MyMoneyTag(const QString& id, const MyMoneyTag& other) :
  MyMoneyObject(id),
  d_ptr(new MyMoneyTagPrivate(*other.d_func()))
{
  Q_D(MyMoneyTag);
  d->m_tag_color = QColor("black");
}

MyMoneyTag::~MyMoneyTag()
{
  Q_D(MyMoneyTag);
  delete d;
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

  writeBaseXML(document, el);

  Q_D(const MyMoneyTag);
  el.setAttribute(getAttrName(Attribute::Name), d->m_name);
  el.setAttribute(getAttrName(Attribute::Closed), d->m_closed);
  if (d->m_tag_color.isValid())
    el.setAttribute(getAttrName(Attribute::TagColor), d->m_tag_color.name());
  if (!d->m_notes.isEmpty())
    el.setAttribute(getAttrName(Attribute::Notes), d->m_notes);
  parent.appendChild(el);
}

bool MyMoneyTag::hasReferenceTo(const QString& /*id*/) const
{
  return false;
}

QString MyMoneyTag::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::Name,     QStringLiteral("name")},
    {Attribute::Type,     QStringLiteral("type")},
    {Attribute::TagColor, QStringLiteral("tagcolor")},
    {Attribute::Closed,   QStringLiteral("closed")},
    {Attribute::Notes,    QStringLiteral("notes")},
  };
  return attrNames[attr];
}
