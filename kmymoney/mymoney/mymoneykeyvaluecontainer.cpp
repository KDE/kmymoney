/*
 * Copyright 2002-2011  Thomas Baumgart <tbaumgart@kde.org>
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

#include "mymoneykeyvaluecontainer.h"
#include "mymoneykeyvaluecontainer_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

Q_GLOBAL_STATIC(QString, nullString)

MyMoneyKeyValueContainer::MyMoneyKeyValueContainer() :
  d_ptr(new MyMoneyKeyValueContainerPrivate)
{
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
