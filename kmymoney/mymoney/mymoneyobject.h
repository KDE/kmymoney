/*
 * Copyright 2005-2019  Thomas Baumgart <tbaumgart@kde.org>
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

#ifndef MYMONEYOBJECT_H
#define MYMONEYOBJECT_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qglobal.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
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
    * This method must be provided by all derived objects. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const = 0;

  /**
   * This method must be provided by all derived object. It returns
   * a QSet of object ids that this object references.
   *
   * @returns QSet<QString> of referenced objects
   */
  virtual QSet<QString> referencedObjects() const = 0;

  bool operator == (const MyMoneyObject& right) const;

protected:
  MyMoneyObjectPrivate * d_ptr;
  explicit MyMoneyObject(MyMoneyObjectPrivate &dd);
  MyMoneyObject(MyMoneyObjectPrivate &dd,
                const QString& id);
};

#endif

