/***************************************************************************
                          mymoneykeyvaluecontainer.cpp
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneykeyvaluecontainer.h"

#include <QtDebug>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

K_GLOBAL_STATIC(QString, nullString)

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer()
{
}

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer(const QDomElement& node)
{
  if (!node.isNull()) {
    if ("KEYVALUEPAIRS" != node.tagName())
      throw MYMONEYEXCEPTION("Node was not KEYVALUEPAIRS");

    m_kvp.clear();

    QDomNodeList nodeList = node.elementsByTagName("PAIR");
    for (int i = 0; i < nodeList.count(); ++i) {
      const QDomElement& el(nodeList.item(i).toElement());
      m_kvp[el.attribute("key")] = el.attribute("value");
    }
  }
}

MyMoneyKeyValueContainer::~MyMoneyKeyValueContainer()
{
}

const QString& MyMoneyKeyValueContainer::value(const QString& key) const
{
  QMap<QString, QString>::ConstIterator it;

  it = m_kvp.find(key);
  if (it != m_kvp.end())
    return (*it);
  return *nullString;
}

void MyMoneyKeyValueContainer::setValue(const QString& key, const QString& value)
{
  m_kvp[key] = value;
}


void MyMoneyKeyValueContainer::setPairs(const QMap<QString, QString>& list)
{
  m_kvp = list;
}

void MyMoneyKeyValueContainer::deletePair(const QString& key)
{
  QMap<QString, QString>::Iterator it;

  it = m_kvp.find(key);
  if (it != m_kvp.end())
    m_kvp.erase(it);
}

void MyMoneyKeyValueContainer::clear()
{
  m_kvp.clear();
}

bool MyMoneyKeyValueContainer::operator == (const MyMoneyKeyValueContainer& right) const
{
  QMap<QString, QString>::ConstIterator it_a, it_b;

  it_a = m_kvp.begin();
  it_b = right.m_kvp.begin();

  while (it_a != m_kvp.end() && it_b != right.m_kvp.end()) {
    if (it_a.key() != it_b.key()
        || (((*it_a).length() != 0 || (*it_b).length() != 0) && *it_a != *it_b))
      return false;
    ++it_a;
    ++it_b;
  }

  return (it_a == m_kvp.end() && it_b == right.m_kvp.end());
}

void MyMoneyKeyValueContainer::writeXML(QDomDocument& document, QDomElement& parent) const
{
  if (m_kvp.count() != 0) {
    QDomElement el = document.createElement("KEYVALUEPAIRS");

    QMap<QString, QString>::ConstIterator it;
    for (it = m_kvp.begin(); it != m_kvp.end(); ++it) {
      QDomElement pair = document.createElement("PAIR");
      pair.setAttribute("key", it.key());
      pair.setAttribute("value", it.value());
      el.appendChild(pair);
    }

    parent.appendChild(el);
  }
}

QDebug operator<<(QDebug dbg, const MyMoneyKeyValueContainer &a)
{
  dbg << "MyMoneyKeyValueContainer(";
  foreach(const QString &key, a.pairs().keys()) {
    dbg << key << a[key];
  }
  dbg << ")";
  return dbg;
}
