/***************************************************************************
                          mymoneyobject.cpp
                          -------------------
    copyright            : (C) 2005 by Thomas Baumagrt
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QDomElement>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"
#include "mymoneyexception.h"

const QString MyMoneyObject::m_emptyId;

MyMoneyObject::MyMoneyObject(const QString& id)
{
  m_id = id;
}

MyMoneyObject::MyMoneyObject(const QDomElement& el, const bool forceId)
{
  m_id = el.attribute("id");
  if (m_id.length() == 0 && forceId)
    throw MYMONEYEXCEPTION("Node has no ID");
}

MyMoneyObject::MyMoneyObject()
{
}

MyMoneyObject::~MyMoneyObject()
{
}

void MyMoneyObject::setId(const QString& id)
{
  m_id = id;
}

bool MyMoneyObject::operator == (const MyMoneyObject& right) const
{
  return m_id == right.m_id;
}

void MyMoneyObject::clearId()
{
  m_id.clear();
}

const QString& MyMoneyObject::emptyId()
{
  return m_emptyId;
}

void MyMoneyObject::writeBaseXML(QDomDocument& document, QDomElement& el) const
{
  Q_UNUSED(document);

  el.setAttribute(QStringLiteral("id"), m_id);
}
