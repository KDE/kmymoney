/*
 * SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2002-2019 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2003 Kevin Tambascio <ktambascio@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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

namespace eMyMoney { namespace Payee { enum class MatchType; } }

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

  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneyPayee();
  explicit MyMoneyPayee(const QString &id);

  MyMoneyPayee(const QString& id,
               const MyMoneyPayee& other);

  MyMoneyPayee(const MyMoneyPayee & other);
  MyMoneyPayee(MyMoneyPayee && other);
  MyMoneyPayee & operator=(MyMoneyPayee other);
  friend void swap(MyMoneyPayee& first, MyMoneyPayee& second);

  ~MyMoneyPayee() override;

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

  bool isMatchingEnabled() const;
  bool isUsingMatchKey() const;
  bool isMatchKeyIgnoreCase() const;
  QString matchKey() const;

  /**
   * Get all match data in one call
   *
   * @param ignorecase Bool which will be replaced to indicate whether the match is
   * case-sensitive (false) or case-insensitive (true)
   * @param keys List of strings which will be replaced by the match key to use for this payee
   *
   * @return the matching type (see payeeMatchType for details)
   */
  eMyMoney::Payee::MatchType matchData(bool& ignorecase, QStringList& keys) const;

  /**
   * Set all match data in one call
   *
   * @param type matching type (see payeeMatchType for details)
   * @param ignorecase Whether case should be ignored for the key/name match
   * @param keys A list of keys themselves, if applicable
   */
  void setMatchData(eMyMoney::Payee::MatchType type, bool ignorecase, const QStringList& keys);

  /**
   * Get all match data in one call (overloaded version for database module)
   *
   * @param ignorecase Bool which will be replaced to indicate whether the match is
   * case-sensitive (false) or case-insensitive (true)
   * @param keyString A list of keys in single-string format, if applicable
   *
   * @return the matching type (see payeeMatchType for details)
   */
  eMyMoney::Payee::MatchType matchData(bool& ignorecase, QString& keyString) const;

  /**
   * Set all match data in one call (overloaded version for database module)
   *
   * @param type matching type (see payeeMatchType for details)
   * @param ignorecase Whether case should be ignored for the key/name match
   * @param keys A list of keys in single-string format, if applicable
   */
  void setMatchData(eMyMoney::Payee::MatchType type, bool ignorecase, const QString& keys);

  QString defaultAccountId() const;

  void setDefaultAccountId(const QString& id = QString());

  // Equality operator
  bool operator == (const MyMoneyPayee &) const;
//  bool operator == (const MyMoneyPayee& lhs, const QString& rhs) const;
  bool operator <(const MyMoneyPayee& right) const;

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

  static MyMoneyPayee null;
};

inline void swap(MyMoneyPayee& first, MyMoneyPayee& second) // krazy:exclude=inline
{
  using std::swap;
  swap(first.d_ptr, second.d_ptr);
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
