/***************************************************************************
                          mymoneypayee.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef MYMONEYPAYEE_H
#define MYMONEYPAYEE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmm_mymoney_export.h"
#include "mymoneyobject.h"
#include "mymoneypayeeidentifiercontainer.h"

class QString;
class QStringList;

/**
  * This class represents a payee or receiver within the MyMoney engine.
  * Since it is not payee-specific, it is also used as a generic address
  * book entry.
  *
  * @author Thomas Baumgart
  */
class MyMoneyPayeePrivate;
class KMM_MYMONEY_EXPORT MyMoneyPayee : public MyMoneyObject, public MyMoneyPayeeIdentifierContainer
{
  Q_DECLARE_PRIVATE(MyMoneyPayee)
  MyMoneyPayeePrivate* d_ptr;

  KMM_MYMONEY_UNIT_TESTABLE

public:
  typedef enum {
    matchDisabled = 0,
    matchName,
    matchKey,
    matchNameExact
  } payeeMatchType;

  MyMoneyPayee();

  explicit MyMoneyPayee(const QString& name,
                        const QString& address = QString(),
                        const QString& city = QString(),
                        const QString& state = QString(),
                        const QString& postcode = QString(),
                        const QString& telephone = QString(),
                        const QString& email = QString());
  /**
    * This is the constructor for a payee that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  explicit MyMoneyPayee(const QDomElement& node);

  MyMoneyPayee(const QString& id,
               const MyMoneyPayee& other);

  MyMoneyPayee(const MyMoneyPayee & other);
  MyMoneyPayee(MyMoneyPayee && other);
  MyMoneyPayee & operator=(MyMoneyPayee other);
  friend void swap(MyMoneyPayee& first, MyMoneyPayee& second);

  ~MyMoneyPayee();

  QString name() const;
  void setName(const QString& val);

  QString address() const;
  void setAddress(const QString& val);

  QString city() const;
  void setCity(const QString& val);

  QString state() const;
  void setState(const QString& val);

  QString postcode() const;
  void setPostcode(const QString& val);

  QString telephone() const;
  void setTelephone(const QString& val);

  QString email() const;
  void setEmail(const QString& val);

  QString notes() const;
  void setNotes(const QString& val);

  QString reference() const;
  void setReference(const QString& ref);

  /**
   * Get all match data in one call
   *
   * @param ignorecase Bool which will be replaced to indicate whether the match is
   * case-sensitive (false) or case-insensitive (true)
   * @param keys List of strings which will be replaced by the match key to use for this payee
   *
   * @return the matching type (see payeeMatchType for details)
   */
  payeeMatchType matchData(bool& ignorecase, QStringList& keys) const;

  /**
   * Set all match data in one call
   *
   * @param type matching type (see payeeMatchType for details)
   * @param ignorecase Whether case should be ignored for the key/name match
   * @param keys A list of keys themselves, if applicable
   */
  void setMatchData(payeeMatchType type, bool ignorecase, const QStringList& keys);

  /**
   * Get all match data in one call (overloaded version for database module)
   *
   * @param ignorecase Bool which will be replaced to indicate whether the match is
   * case-sensitive (false) or case-insensitive (true)
   * @param keyString A list of keys in single-string format, if applicable
   *
   * @return the matching type (see payeeMatchType for details)
   */
  payeeMatchType matchData(bool& ignorecase, QString& keyString) const;

  /**
   * Set all match data in one call (overloaded version for database module)
   *
   * @param type matching type (see payeeMatchType for details)
   * @param ignorecase Whether case should be ignored for the key/name match
   * @param keys A list of keys in single-string format, if applicable
   */
  void setMatchData(payeeMatchType type, bool ignorecase, const QString& keys);


  bool defaultAccountEnabled() const;

  QString defaultAccountId() const;

  void setDefaultAccountId(const QString& id);
  void setDefaultAccountId();

  // Equality operator
  bool operator == (const MyMoneyPayee &) const;
//  bool operator == (const MyMoneyPayee& lhs, const QString& rhs) const;
  bool operator <(const MyMoneyPayee& right) const;

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

  static MyMoneyPayee null;
};

inline void swap(MyMoneyPayee& first, MyMoneyPayee& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
  swap(first.m_id, second.m_id);
  swap(first.m_payeeIdentifiers, second.m_payeeIdentifiers);
}

inline MyMoneyPayee::MyMoneyPayee(MyMoneyPayee && other) : MyMoneyPayee() // krazy:exclude=inline
{
  swap(*this, other);
}

inline MyMoneyPayee & MyMoneyPayee::operator=(MyMoneyPayee other) // krazy:exclude=inline
{
  swap(*this, other);
  return *this;
}

/**
  * Make it possible to hold @ref MyMoneyPayee objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyPayee)

#endif
