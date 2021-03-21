/*
    SPDX-FileCopyrightText: 2012-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneycostcenter.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCollator>
#include <QSet>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"
#include "mymoneyexception.h"

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

bool MyMoneyCostCenter::hasReferenceTo(const QString& /*id*/) const
{
    return false;
}

QSet<QString> MyMoneyCostCenter::referencedObjects() const
{
    return {};
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
