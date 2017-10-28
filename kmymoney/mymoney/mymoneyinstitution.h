/***************************************************************************
                          mymoneyinstitution.h
                          -------------------
    copyright            : (C) 2002-2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
  Q_DECLARE_PRIVATE(MyMoneyInstitution)
  MyMoneyInstitutionPrivate* d_ptr;

  KMM_MYMONEY_UNIT_TESTABLE

public:

  /**
    * This is the constructor for a new empty institution description
    */
  MyMoneyInstitution();

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
                              const QString& sortCode);

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

  /**
    * This is the constructor for an institution that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  explicit MyMoneyInstitution(const QDomElement& el);

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

  QString sortcode() const;
  void setSortcode(const QString& code);

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

  void writeXML(QDomDocument& document, QDomElement& parent) const override;

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

  QPixmap pixmap(const int size = 64) const;

private:

  enum class Element { AccountID,
                       AccountIDS,
                       Address };

  enum class Attribute { ID = 0,
                         Name,
                         Manager,
                         SortCode,
                         Street,
                         City,
                         Zip,
                         Telephone,
                         // insert new entries above this line
                         LastAttribute
                       };

  static QString getElName(const Element el);
  static QString getAttrName(const Attribute attr);
  friend uint qHash(const Attribute, uint seed);
  friend uint qHash(const Element, uint seed);
};

inline uint qHash(const MyMoneyInstitution::Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); } // krazy:exclude=inline
inline uint qHash(const MyMoneyInstitution::Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); } // krazy:exclude=inline

inline void swap(MyMoneyInstitution& first, MyMoneyInstitution& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
  swap(first.m_id, second.m_id);
  swap(first.m_kvp, second.m_kvp);
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
