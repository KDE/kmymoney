/*
    SPDX-FileCopyrightText: 2012-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYCOSTCENTER_H
#define MYMONEYCOSTCENTER_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>
#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"

/**
  * This class represents a tag within the MyMoney engine.
  */
class MyMoneyCostCenterPrivate;
class KMM_MYMONEY_EXPORT MyMoneyCostCenter : public MyMoneyObject
{
    Q_DECLARE_PRIVATE(MyMoneyCostCenter)

    KMM_MYMONEY_UNIT_TESTABLE

public:
    MyMoneyCostCenter();
    explicit MyMoneyCostCenter(const QString &id);

    MyMoneyCostCenter(const QString& id,
                      const MyMoneyCostCenter& other);
    MyMoneyCostCenter(const MyMoneyCostCenter & other);
    MyMoneyCostCenter(MyMoneyCostCenter && other);
    MyMoneyCostCenter & operator=(MyMoneyCostCenter other);
    friend void swap(MyMoneyCostCenter& first, MyMoneyCostCenter& second);

    ~MyMoneyCostCenter();

    QString name() const;
    void setName(const QString& val);

    /**
     * This member returns a possible number leading the name. If there
     * is no number infront of the name, then the full name will be returned
     * @sa name()
     */
    QString shortName() const;


    // Equality operator
    bool operator == (const MyMoneyCostCenter &) const;
    bool operator <(const MyMoneyCostCenter& right) const;

    static MyMoneyCostCenter null;
};

inline void swap(MyMoneyCostCenter& first, MyMoneyCostCenter& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyCostCenter::MyMoneyCostCenter(MyMoneyCostCenter && other) : MyMoneyCostCenter() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyCostCenter & MyMoneyCostCenter::operator=(MyMoneyCostCenter other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

//inline bool operator==(const MyMoneyCostCenter& lhs, const QString& rhs)
//{
//  return lhs.id() == rhs;
//}

/**
  * Make it possible to hold @ref MyMoneyCostCenter objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyCostCenter)

#endif
