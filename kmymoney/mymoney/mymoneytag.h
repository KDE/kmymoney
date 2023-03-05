/*
    SPDX-FileCopyrightText: 2012 Alessandro Russo <axela74@yahoo.it>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYTAG_H
#define MYMONEYTAG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneyobject.h"

class QString;
class QColor;

/**
  * This class represents a tag within the MyMoney engine.
  */
class MyMoneyTagPrivate;
class KMM_MYMONEY_EXPORT MyMoneyTag : public MyMoneyObject
{
    Q_DECLARE_PRIVATE(MyMoneyTag)

    KMM_MYMONEY_UNIT_TESTABLE

public:
    MyMoneyTag();
    explicit MyMoneyTag(const QString &id);

    explicit MyMoneyTag(const QString& name,
                        const QColor& tagColor
                       );

    MyMoneyTag(const QString& id,
               const MyMoneyTag& tag);

    MyMoneyTag(const MyMoneyTag & other);
    MyMoneyTag(MyMoneyTag && other);
    MyMoneyTag & operator=(MyMoneyTag other);
    friend void swap(MyMoneyTag& first, MyMoneyTag& second);

    ~MyMoneyTag();

    QString name() const;
    void setName(const QString& val);

    bool isClosed() const;
    void setClosed(bool val);

    QColor tagColor() const;
    void setTagColor(const QColor& val);
    void setNamedTagColor(const QString &val);

    QString notes() const;
    void setNotes(const QString& val);

    // Equality operator
    bool operator == (const MyMoneyTag &) const;
    bool operator <(const MyMoneyTag& right) const;

    static MyMoneyTag null;
};

inline void swap(MyMoneyTag& first, MyMoneyTag& second) // krazy:exclude=inline
{
    using std::swap;
    swap(first.d_ptr, second.d_ptr);
}

inline MyMoneyTag::MyMoneyTag(MyMoneyTag && other) : MyMoneyTag() // krazy:exclude=inline
{
    swap(*this, other);
}

inline MyMoneyTag & MyMoneyTag::operator=(MyMoneyTag other) // krazy:exclude=inline
{
    swap(*this, other);
    return *this;
}

//inline bool operator==(const MyMoneyTag& lhs, const QString& rhs)
//{
//  return lhs.id() == rhs;
//}

/**
  * Make it possible to hold @ref MyMoneyTag objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyTag)

#endif
