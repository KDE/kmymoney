/***************************************************************************
                          mymoneycostcenter.cpp
                             -------------------
    copyright            : (C) 2015 Thomas Baumgart <tbaumgart@kde.org>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mymoneycostcenter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomElement>
#include <QCollator>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

MyMoneyCostCenter MyMoneyCostCenter::null;

class MyMoneyCostCenterPrivate {

public:
  QString m_name;
};

MyMoneyCostCenter::MyMoneyCostCenter() :
  d_ptr(new MyMoneyCostCenterPrivate)
{
}

MyMoneyCostCenter::MyMoneyCostCenter(const QString& name) :
  d_ptr(new MyMoneyCostCenterPrivate)
{
  Q_D(MyMoneyCostCenter);
  d->m_name = name;
}

MyMoneyCostCenter::MyMoneyCostCenter(const QDomElement& node) :
    MyMoneyObject(node),
    d_ptr(new MyMoneyCostCenterPrivate)
{
  if (nodeNames[nnCostCenter] != node.tagName())
    throw MYMONEYEXCEPTION("Node was not COSTCENTER");

  Q_D(MyMoneyCostCenter);
  d->m_name = node.attribute(getAttrName(Attribute::Name));
}

MyMoneyCostCenter::MyMoneyCostCenter(const MyMoneyCostCenter& other) :
  MyMoneyObject(other.id()),
  d_ptr(new MyMoneyCostCenterPrivate(*other.d_func()))
{
}

MyMoneyCostCenter::MyMoneyCostCenter(const QString& id, const MyMoneyCostCenter& other) :
  MyMoneyObject(id),
  d_ptr(new MyMoneyCostCenterPrivate(*other.d_func()))
{
}

MyMoneyCostCenter::~MyMoneyCostCenter()
{
  Q_D(MyMoneyCostCenter);
  delete d;
}

bool MyMoneyCostCenter::operator == (const MyMoneyCostCenter& right) const
{
  Q_D(const MyMoneyCostCenter);
  auto d2 = static_cast<const MyMoneyCostCenterPrivate *>(right.d_func());
  return (MyMoneyObject::operator==(right) &&
          ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)));
}

bool MyMoneyCostCenter::operator < (const MyMoneyCostCenter& right) const
{
  Q_D(const MyMoneyCostCenter);
  auto d2 = static_cast<const MyMoneyCostCenterPrivate *>(right.d_func());
  QCollator col;
  return col.compare(d->m_name, d2->m_name);
}

void MyMoneyCostCenter::writeXML(QDomDocument& document, QDomElement& parent) const
{
  auto el = document.createElement(nodeNames[nnCostCenter]);

  writeBaseXML(document, el);

  Q_D(const MyMoneyCostCenter);
  el.setAttribute(getAttrName(Attribute::Name), d->m_name);
  parent.appendChild(el);
}

bool MyMoneyCostCenter::hasReferenceTo(const QString& /*id*/) const
{
  return false;
}

QString MyMoneyCostCenter::name() const
{
  Q_D(const MyMoneyCostCenter);
  return d->m_name;
}

void MyMoneyCostCenter::setName(const QString& val)
{
  Q_D(MyMoneyCostCenter);
  d->m_name = val;
}


QString MyMoneyCostCenter::shortName() const
{
  Q_D(const MyMoneyCostCenter);
  QRegExp shortNumberExp("^(\\d+)\\s.+");
  if(shortNumberExp.exactMatch(d->m_name)) {
    return shortNumberExp.cap(1);
  }
  return d->m_name;
}

QString MyMoneyCostCenter::getAttrName(const Attribute attr)
{
  static const QMap<Attribute, QString> attrNames = {
    {Attribute::Name, QStringLiteral("name")},
  };
  return attrNames[attr];
}
