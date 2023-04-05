/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYOBJECT_P_H
#define MYMONEYOBJECT_P_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QSet>
#include <QString>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyObjectPrivate
{
public:
    MyMoneyObjectPrivate()
    {
    }

    MyMoneyObjectPrivate(const MyMoneyObjectPrivate& right)
    {
        *this = right;
    }

    MyMoneyObjectPrivate& operator=(const MyMoneyObjectPrivate& right)
    {
        m_id = right.m_id;
        m_referencedObjects = right.m_referencedObjects;
        return *this;
    }

    virtual ~MyMoneyObjectPrivate()
    {
    }

    void setId(const QString& id)
    {
        m_id = id;
    }

    /**
     * This method must be provided by all derived object. It fills
     * the @c m_referencedObjects set with the ids that are
     * referenced by this object.
     */
    virtual void collectReferencedObjects() = 0;

    void clearReferences()
    {
        m_referencedObjects.clear();
    }

    QString m_id;
    QSet<QString> m_referencedObjects;
};

#endif
