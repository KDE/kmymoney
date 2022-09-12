/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYCATEGORY_H
#define MYMONEYCATEGORY_H

// ----------------------------------------------------------------------------
// QT Includes

#include "qcontainerfwd.h"
#include <qglobal.h>

class QString;

/**
  * @deprecated This class represents an Income or Expense category. Please don't
  *             use it anymore, as it will be removed sooner or later.
  */
class MyMoneyCategoryPrivate;
class MyMoneyCategory
{
    Q_DECLARE_PRIVATE(MyMoneyCategory)
    MyMoneyCategoryPrivate * d_ptr;

    friend QDataStream &operator<<(QDataStream &, MyMoneyCategory &);
    friend QDataStream &operator>>(QDataStream &, MyMoneyCategory &);

public:
    MyMoneyCategory();
    explicit MyMoneyCategory(const bool income, const QString& name);
    explicit MyMoneyCategory(const bool income, const QString& name, QStringList minors);
    MyMoneyCategory(const MyMoneyCategory & other);
    MyMoneyCategory(MyMoneyCategory && other);
    MyMoneyCategory & operator=(MyMoneyCategory other);
    friend void swap(MyMoneyCategory& first, MyMoneyCategory& second);
    ~MyMoneyCategory();

    // Simple get operations
    QString name() const;
    QStringList& minorCategories();

    // Simple set operations
    bool isIncome() const;
    void setIncome(const bool val);
    void setName(const QString& val);

    bool setMinorCategories(QStringList values);
    bool addMinorCategory(const QString& val);
    bool removeMinorCategory(const QString& val);
    bool renameMinorCategory(const QString& oldVal, const QString& newVal);
    bool addMinorCategory(QStringList values);
    bool removeAllMinors();
    QString firstMinor();

    void clear();
};

inline void swap(MyMoneyCategory& first, MyMoneyCategory& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyCategory::MyMoneyCategory(MyMoneyCategory && other) : MyMoneyCategory() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyCategory & MyMoneyCategory::operator=(MyMoneyCategory other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

#endif
