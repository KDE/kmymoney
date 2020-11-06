/*
 * Copyright 2000-2001  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2003       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2004-2006  Ace Jones <acejones@users.sourceforge.net>
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

#ifndef MYMONEYINSTITUTION_H
#define MYMONEYINSTITUTION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"
#include "mymoneykeyvaluecontainer.h"
#include "kmm_mymoney_export.h"

class QString;
class QStringList;
class QPixmap;

/**
  * This class represents a Bank contained within a MyMoneyFile object
  *
  * @author Thomas Baumgart
  * @author Łukasz Wojniłowicz
  */
class MyMoneyInstitutionPrivate;
class KMM_MYMONEY_EXPORT MyMoneyInstitution : public MyMoneyObject, public MyMoneyKeyValueContainer
{
  Q_DECLARE_PRIVATE_D(MyMoneyObject::d_ptr, MyMoneyInstitution)

  KMM_MYMONEY_UNIT_TESTABLE

public:

  /**
    * This is the constructor for a new empty institution description
    */
  MyMoneyInstitution();
  explicit MyMoneyInstitution(const QString &id);

  /**
    * This is the constructor used by an application to fill the
    * values required for a new institution. This object should then be
    * passed to @see MyMoneyFile::addInstitution
    */
  explicit MyMoneyInstitution(const QString& name,
                              const QString& city,
                              const QString& street,
                              const QString& postcode,
                              const QString& telephone,
                              const QString& manager,
                              const QString& bankcode);

  MyMoneyInstitution(const MyMoneyInstitution & other);
  MyMoneyInstitution(MyMoneyInstitution && other);
  MyMoneyInstitution & operator=(MyMoneyInstitution other);
  friend void swap(MyMoneyInstitution& first, MyMoneyInstitution& second);

  /**
    * This is the destructor for any MyMoneyInstitution object
    */
  ~MyMoneyInstitution();

  /**
    * This is the constructor for a new institution known to the current file
    *
    * @param id id assigned to the new institution object
    * @param right institution definition
    */
  MyMoneyInstitution(const QString& id, const MyMoneyInstitution& other);

  QString manager() const;
  void setManager(const QString& manager);

  QString name() const;
  void setName(const QString& name);

  QString postcode() const;
  void setPostcode(const QString& code);

  QString street() const;
  void setStreet(const QString& street);

  QString telephone() const;
  void setTelephone(const QString& tel);

  QString town() const;
  void setTown(const QString& town);

  QString city() const;
  void setCity(const QString& town);

  QString bankcode() const;
  void setBankCode(const QString& code);

  /**
    * This method adds the id of an account to the account list of
    * this institution It is verified, that the account is only
    * mentioned once.
    *
    * @param account id of the account to be added
    */
  void addAccountId(const QString& account);

  /**
    * This method deletes the id of an account from the account list
    * of this institution
    *
    * @param account id of the account to be deleted
    * @return id of account deleted, otherwise empty string
    */
  QString removeAccountId(const QString& account);

  /**
    * This method is used to return the set of accounts known to
    * this institution
    * return QStringList of account ids
    */
  QStringList accountList() const;

  /**
    * This method returns the number of accounts known to
    * this institution
    * @return number of accounts
    */
  unsigned int accountCount() const;

  bool operator == (const MyMoneyInstitution&) const;
  bool operator < (const MyMoneyInstitution& right) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  bool hasReferenceTo(const QString& id) const override;

  /**
   * @copydoc MyMoneyObject::referencedObjects
   */
  QSet<QString> referencedObjects() const override;

  static QPixmap pixmap(const int size = 64);
};

inline void swap(MyMoneyInstitution& first, MyMoneyInstitution& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.MyMoneyObject::d_ptr, second.MyMoneyObject::d_ptr);
  swap(first.MyMoneyKeyValueContainer::d_ptr, second.MyMoneyKeyValueContainer::d_ptr);
}

inline MyMoneyInstitution::MyMoneyInstitution(MyMoneyInstitution && other) : MyMoneyInstitution() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyInstitution & MyMoneyInstitution::operator=(MyMoneyInstitution other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

/**
  * Make it possible to hold @ref MyMoneyInstitution objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyInstitution)

#endif
