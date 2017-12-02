/***************************************************************************
                          mymoneykeyvaluecontainer.cpp
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2002-2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "mymoneykeyvaluecontainer.h"
#include "mymoneykeyvaluecontainer_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomDocument>
#include <QDomElement>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

Q_GLOBAL_STATIC(QString, nullString)

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer() :
  d_ptr(new MyMoneyKeyValueContainerPrivate)
{
}

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer(const QDomElement& node) :
  d_ptr(new MyMoneyKeyValueContainerPrivate)
{
  Q_D(MyMoneyKeyValueContainer);
  if (!node.isNull()) {
    if (nodeNames[nnKeyValuePairs] != node.tagName())
      throw MYMONEYEXCEPTION("Node was not KEYVALUEPAIRS");

    d->m_kvp.clear();

    QDomNodeList nodeList = node.elementsByTagName(d->getElName(Element::Pair));
    for (int i = 0; i < nodeList.count(); ++i) {
      const QDomElement& el(nodeList.item(i).toElement());
      d->m_kvp[el.attribute(d->getAttrName(Attribute::Key))] = el.attribute(d->getAttrName(Attribute::Value));
    }
  }
}

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer(const MyMoneyKeyValueContainer& other) :
  d_ptr(new MyMoneyKeyValueContainerPrivate(*other.d_func()))
{
}

MyMoneyKeyValueContainer::~MyMoneyKeyValueContainer()
{
  Q_D(MyMoneyKeyValueContainer);
  delete d;
}

QString MyMoneyKeyValueContainer::value(const QString& key) const
{
  Q_D(const MyMoneyKeyValueContainer);
  QMap<QString, QString>::ConstIterator it;

  it = d->m_kvp.find(key);
  if (it != d->m_kvp.end())
    return (*it);
  return *nullString;
}

void MyMoneyKeyValueContainer::setValue(const QString& key, const QString& value)
{
  Q_D(MyMoneyKeyValueContainer);
  d->m_kvp[key] = value;
}

QMap<QString, QString> MyMoneyKeyValueContainer::pairs() const
{
  Q_D(const MyMoneyKeyValueContainer);
  return d->m_kvp;
}

void MyMoneyKeyValueContainer::setPairs(const QMap<QString, QString>& list)
{
  Q_D(MyMoneyKeyValueContainer);
  d->m_kvp = list;
}

void MyMoneyKeyValueContainer::deletePair(const QString& key)
{
  Q_D(MyMoneyKeyValueContainer);
  QMap<QString, QString>::Iterator it;

  it = d->m_kvp.find(key);
  if (it != d->m_kvp.end())
    d->m_kvp.erase(it);
}

void MyMoneyKeyValueContainer::clear()
{
  Q_D(MyMoneyKeyValueContainer);
  d->m_kvp.clear();
}

bool MyMoneyKeyValueContainer::operator == (const MyMoneyKeyValueContainer& right) const
{
  Q_D(const MyMoneyKeyValueContainer);
  QMap<QString, QString>::ConstIterator it_a, it_b;

  auto d2 = static_cast<const MyMoneyKeyValueContainerPrivate *>(right.d_func());
  it_a = d->m_kvp.begin();
  it_b = d2->m_kvp.begin();

  while (it_a != d->m_kvp.end() && it_b != d2->m_kvp.end()) {
    if (it_a.key() != it_b.key()
        || (((*it_a).length() != 0 || (*it_b).length() != 0) && *it_a != *it_b))
      return false;
    ++it_a;
    ++it_b;
  }

  return (it_a == d->m_kvp.end() && it_b == d2->m_kvp.end());
}


QString MyMoneyKeyValueContainer::operator[](const QString& k) const
{
  return value(k);
}

QString& MyMoneyKeyValueContainer::operator[](const QString& k)
{
  Q_D(MyMoneyKeyValueContainer);
  return d->m_kvp[k];
}

void MyMoneyKeyValueContainer::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_D(const MyMoneyKeyValueContainer);
  if (d->m_kvp.count() != 0) {
    QDomElement el = document.createElement(nodeNames[nnKeyValuePairs]);

    QMap<QString, QString>::ConstIterator it;
    for (it = d->m_kvp.begin(); it != d->m_kvp.end(); ++it) {
      QDomElement pair = document.createElement(d->getElName(Element::Pair));
      pair.setAttribute(d->getAttrName(Attribute::Key), it.key());
      pair.setAttribute(d->getAttrName(Attribute::Value), it.value());
      el.appendChild(pair);
    }

    parent.appendChild(el);
  }
}
