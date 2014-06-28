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

// ----------------------------------------------------------------------------
// QT Includes

#include <QStringList>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include <mymoneyexception.h>

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
  if ("TAG" != node.tagName()) {
    throw MYMONEYEXCEPTION("Node was not TAG");
  }
  m_name = node.attribute("name");
  if (node.hasAttribute("tagcolor")) {
    m_tag_color.setNamedColor(node.attribute("tagcolor"));
  }
  if (node.hasAttribute("notes")) {
    m_notes = node.attribute("notes");
  }
  m_closed = node.attribute("closed", "0").toUInt();
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
  QDomElement el = document.createElement("TAG");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("closed", m_closed);
  if (m_tag_color.isValid())
    el.setAttribute("tagcolor", m_tag_color.name());
  if (!m_notes.isEmpty())
    el.setAttribute("notes", m_notes);
  parent.appendChild(el);
}

bool MyMoneyTag::hasReferenceTo(const QString& /*id*/) const
{
  return false;
}
