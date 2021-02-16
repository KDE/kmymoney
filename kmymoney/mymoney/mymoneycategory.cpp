/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneycategory.h"

#include <QString>
#include <QStringList>
#include <QDataStream>

class MyMoneyCategoryPrivate
{
public:
  bool m_income; // if false, m_income == expense
  QString m_name;
  QStringList m_minorCategories;
};

MyMoneyCategory::MyMoneyCategory() :
  d_ptr(new MyMoneyCategoryPrivate)
{
  Q_D(MyMoneyCategory);
  d->m_income = true;
}

MyMoneyCategory::MyMoneyCategory(const bool income, const QString& name) :
  d_ptr(new MyMoneyCategoryPrivate)
{
  Q_D(MyMoneyCategory);
  d->m_income = income;
  d->m_name = name;
}

MyMoneyCategory::MyMoneyCategory(const bool income, const QString& name, QStringList minors) :
  d_ptr(new MyMoneyCategoryPrivate)
{
  Q_D(MyMoneyCategory);
  d->m_income = income;
  d->m_name = name;
  d->m_minorCategories = minors;
}

MyMoneyCategory::MyMoneyCategory(const MyMoneyCategory& other) :
  d_ptr(new MyMoneyCategoryPrivate(*other.d_func()))
{
}

MyMoneyCategory::~MyMoneyCategory()
{
  Q_D(MyMoneyCategory);
  delete d;
}

QString MyMoneyCategory::name() const
{
  Q_D(const MyMoneyCategory);
  return d->m_name;
}

QStringList& MyMoneyCategory::minorCategories()
{
  Q_D(MyMoneyCategory);
  return d->m_minorCategories;
}

bool MyMoneyCategory::isIncome() const
{
  Q_D(const MyMoneyCategory);
  return d->m_income;
}

void MyMoneyCategory::setIncome(const bool val)
{
  Q_D(MyMoneyCategory);
  d->m_income = val;
}

void MyMoneyCategory::setName(const QString& val)
{
  Q_D(MyMoneyCategory);
  d->m_name = val;
}

// Functions use the find method to search the list
bool MyMoneyCategory::addMinorCategory(const QString& val)
{
  Q_D(MyMoneyCategory);
  if (val.isEmpty() || val.isNull())
    return false;

  if (d->m_minorCategories.indexOf(val) == -1) {
    d->m_minorCategories.append(val);
    return true;
  }

  return false;
}

bool MyMoneyCategory::removeMinorCategory(const QString& val)
{
  Q_D(MyMoneyCategory);
  if (val.isEmpty() || val.isNull())
    return false;

  if (d->m_minorCategories.indexOf(val) != -1) {
    d->m_minorCategories.removeOne(val);
    return true;
  }

  return false;
}

bool MyMoneyCategory::renameMinorCategory(const QString& oldVal, const QString& newVal)
{
  Q_D(MyMoneyCategory);
  if (oldVal.isEmpty() || oldVal.isNull() || newVal.isEmpty() || newVal.isNull())
    return false;

  if ((d->m_minorCategories.indexOf(oldVal) != -1) &&
      (d->m_minorCategories.indexOf(newVal) == -1)) {

    d->m_minorCategories.removeOne(oldVal);
    return addMinorCategory(newVal);
  }

  return false;
}

bool MyMoneyCategory::addMinorCategory(QStringList values)
{
  for (QStringList::Iterator it = values.begin(); it != values.end(); ++it) {
    addMinorCategory(*it);
  }

  return true;
}

bool MyMoneyCategory::setMinorCategories(QStringList values)
{
  Q_D(MyMoneyCategory);
  d->m_minorCategories.clear();
  return addMinorCategory(values);
}

bool MyMoneyCategory::removeAllMinors()
{
  Q_D(MyMoneyCategory);
  d->m_minorCategories.clear();
  return true;
}

QString MyMoneyCategory::firstMinor()
{
  Q_D(MyMoneyCategory);
  return d->m_minorCategories.first();
}

QDataStream &operator<<(QDataStream &s, MyMoneyCategory &category)
{
  if (category.d_func()->m_income)
    s << (qint32)1;
  else
    s << (qint32)0;

  s << category.d_func()->m_name;

  s << (quint32)category.d_func()->m_minorCategories.count();
  for (QStringList::Iterator it = category.d_func()->m_minorCategories.begin(); it != category.d_func()->m_minorCategories.end(); ++it) {
    s << (*it);
  }

  return s;
}

QDataStream &operator>>(QDataStream &s, MyMoneyCategory &category)
{
  qint32 inc;
  s >> inc;
  if (inc == 0)
    category.d_func()->m_income = false;
  else
    category.d_func()->m_income = true;

  s >> category.d_func()->m_name;

  quint32 minorCount;
  QString buffer;

  s >> minorCount;
  category.d_func()->m_minorCategories.clear();
  for (unsigned int i = 0; i < minorCount; i++) {
    s >> buffer;
    category.d_func()->m_minorCategories.append(buffer);
  }

  return s;
}

void MyMoneyCategory::clear()
{
  Q_D(MyMoneyCategory);
  d->m_minorCategories.clear();
}
