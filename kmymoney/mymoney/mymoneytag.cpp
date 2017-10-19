/***************************************************************************
                          mymoneytag.cpp
                             -------------------
    copyright            : (C) 2012 by Alessandro Russo <alessandro@russo.it>

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

#include <QDomDocument>
#include <QDomElement>
#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

MyMoneyTag MyMoneyTag::null;

MyMoneyTag::MyMoneyTag() :
    m_closed(false)
{
}

MyMoneyTag::MyMoneyTag(const QString& id, const MyMoneyTag& tag) :
    m_closed(false)
{
  *this = tag;
  m_id = id;
  m_tag_color = QColor("black");
}

MyMoneyTag::MyMoneyTag(const QString& name, const QColor& tabColor) :
    m_closed(false)
{
  m_name      = name;
  m_tag_color = tabColor;
}

MyMoneyTag::MyMoneyTag(const QDomElement& node) :
    MyMoneyObject(node)
{
  if (nodeNames[nnTag] != node.tagName()) {
    throw MYMONEYEXCEPTION("Node was not TAG");
  }
  m_name = node.attribute(getAttrName(anName));
  if (node.hasAttribute(getAttrName(anTagColor))) {
    m_tag_color.setNamedColor(node.attribute(getAttrName(anTagColor)));
  }
  if (node.hasAttribute(getAttrName(anNotes))) {
    m_notes = node.attribute(getAttrName(anNotes));
  }
  m_closed = node.attribute(getAttrName(anClosed), "0").toUInt();
}

MyMoneyTag::~MyMoneyTag()
{
}

MyMoneyTag::MyMoneyTag(const MyMoneyTag& right) :
    MyMoneyObject(right)
{
  *this = right;
}

bool MyMoneyTag::operator == (const MyMoneyTag& right) const
{
  return (MyMoneyObject::operator==(right) &&
          ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
          ((m_tag_color.isValid() == false && right.m_tag_color.isValid() == false) || (m_tag_color.name() == right.m_tag_color.name())) &&
          (m_closed == right.m_closed));
}

bool MyMoneyTag::operator < (const MyMoneyTag& right) const
{
  return m_name < right.name();
}

void MyMoneyTag::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement(nodeNames[nnTag]);

  writeBaseXML(document, el);

  el.setAttribute(getAttrName(anName), m_name);
  el.setAttribute(getAttrName(anClosed), m_closed);
  if (m_tag_color.isValid())
    el.setAttribute(getAttrName(anTagColor), m_tag_color.name());
  if (!m_notes.isEmpty())
    el.setAttribute(getAttrName(anNotes), m_notes);
  parent.appendChild(el);
}

bool MyMoneyTag::hasReferenceTo(const QString& /*id*/) const
{
  return false;
}

const QString MyMoneyTag::getAttrName(const attrNameE _attr)
{
  static const QHash<attrNameE, QString> attrNames = {
    {anName, QStringLiteral("name")},
    {anType, QStringLiteral("type")},
    {anTagColor, QStringLiteral("tagcolor")},
    {anClosed, QStringLiteral("closed")},
    {anNotes, QStringLiteral("notes")},
  };
  return attrNames[_attr];
}
