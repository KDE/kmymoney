/*
    SPDX-FileCopyrightText: 2005-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MYMONEYOBJECT_H
#define MYMONEYOBJECT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "kmmset.h"
#include "mymoneyunittestable.h"

class QString;

/**
  * @author Thomas Baumgart
  */

/**
  * This class represents the base class of all MyMoney objects.
  */
class MyMoneyObjectPrivate;
class KMM_MYMONEY_EXPORT MyMoneyObject
{
    Q_DECLARE_PRIVATE(MyMoneyObject)

    KMM_MYMONEY_UNIT_TESTABLE

public:
    /**
      * This is the destructor for the MyMoneyObject object
      */
    virtual ~MyMoneyObject();

    /**
      * This method retrieves the id of the object
      *
      * @return ID of object
      */
    QString id() const;

    /**
      * This method clears the id of the object
      */
    void clearId();

    /**
     * This method returns @p true if the object is referencing the
     * one requested by the parameter @p id. If it does not,
     * this method returns @p false.
     *
     * @param id id of the object to be checked for references
     * @retval true This object references object with id @p id.
     * @retval false This object does not reference the object with id @p id.
     *
     * @sa referencedObjects()
     */
    bool hasReferenceTo(const QString& id) const;

    /**
     * This method returns a std::unordered_set of object ids that this object references.
     *
     * @returns KMMStringSet of referenced objects
     *
     * @sa hasReferenceTo()
     */
    KMMStringSet referencedObjects() const;

    bool operator == (const MyMoneyObject& right) const;

protected:
    MyMoneyObjectPrivate * d_ptr;
    explicit MyMoneyObject(MyMoneyObjectPrivate &dd);
    MyMoneyObject(MyMoneyObjectPrivate &dd,
                  const QString& id);
};

#endif

