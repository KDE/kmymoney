/*
 * Copyright 2012-2016  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneycostcenter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDomElement>
#include <QCollator>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneyexception.h"
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

MyMoneyCostCenter MyMoneyCostCenter::null;

class MyMoneyCostCenterPrivate : public MyMoneyObjectPrivate
{
public:
  QString m_name;
};

MyMoneyCostCenter::MyMoneyCostCenter() :
  MyMoneyObject(*new MyMoneyCostCenterPrivate)
{
}

MyMoneyCostCenter::MyMoneyCostCenter(const QString &id) :
  MyMoneyObject(*new MyMoneyCostCenterPrivate, id)
{
}

MyMoneyCostCenter::MyMoneyCostCenter(const MyMoneyCostCenter& other) :
  MyMoneyObject(*new MyMoneyCostCenterPrivate(*other.d_func()), other.id())
{
}

MyMoneyCostCenter::MyMoneyCostCenter(const QString& id, const MyMoneyCostCenter& other) :
  MyMoneyObject(*new MyMoneyCostCenterPrivate(*other.d_func()), id)
{
}

MyMoneyCostCenter::~MyMoneyCostCenter()
{
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

  Q_D(const MyMoneyCostCenter);
  d->writeBaseXML(document, el);

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
