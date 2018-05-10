/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PAYEEIDENTIFIER_H
#define PAYEEIDENTIFIER_H

#define KMM_STRINGIFY(x) #x
#define KMM_TOSTRING(x) KMM_STRINGIFY(x)

#include <QMetaType>
#include <QString>
#include <qglobal.h>
#include "mymoneyexception.h"

/** @todo fix include path after upgrade to cmake 3 */
#include "payeeidentifier/kmm_payeeidentifier_export.h"

#define PAYEEIDENTIFIERBADCASTEXCEPTION(exceptionMessage) payeeIdentifier::badCast("Casted payeeIdentifier with wrong type " __FILE__ ":" KMM_TOSTRING(__LINE__))
#define PAYEEIDENTIFIEREMPTYEXCEPTION(exceptionMessage) payeeIdentifier::empty("Requested payeeIdentifierData of empty payeeIdentifier " __FILE__ ":" KMM_TOSTRING(__LINE__))

// Q_DECLARE_METATYPE requries this include

class QDomDocument;
class QDomElement;
class payeeIdentifierData;
class KMM_PAYEEIDENTIFIER_EXPORT payeeIdentifier
{
public:
  typedef unsigned int id_t;

  explicit payeeIdentifier();
  explicit payeeIdentifier(payeeIdentifierData *const data);
  explicit payeeIdentifier(const id_t& id, payeeIdentifierData *const data);
  explicit payeeIdentifier(const QString& id, payeeIdentifierData *const data);
  explicit payeeIdentifier(const id_t& id, const payeeIdentifier& other);

  payeeIdentifier(const payeeIdentifier& other);
  ~payeeIdentifier();
  payeeIdentifier& operator=(const payeeIdentifier& other);
  bool operator==(const payeeIdentifier& other);

  /** @brief Check if any data is associated */
  bool isNull() const {
    return (m_payeeIdentifier == 0);
  }

  /**
   * @brief create xml to save this payeeIdentifier
   *
   * It creates a new element below parent which is used to store all data.
   *
   * The counter part to load a payee identifier again is payeeIdentifierLoader::createPayeeIdentifierFromXML().
   */
  void writeXML(QDomDocument &document, QDomElement &parent, const QString& elementName = QLatin1String("payeeIdentifier")) const;

  /**
   * @throws payeeIdentifier::empty
   */
  payeeIdentifierData* operator->();

  /** @copydoc operator->() */
  const payeeIdentifierData* operator->() const;

  /** @copydoc operator->() */
  payeeIdentifierData* data();

  /** @copydoc operator->() */
  const payeeIdentifierData* data() const;

  template< class T >
  T* data();

  template< class T >
  const T* data() const;

  bool isValid() const;


  id_t id() const {
    return m_id;
  }

  QString idString() const;

  void clearId() {
    m_id = 0;
  }

  /**
   * @brief Get payeeIdentifier Iid which identifiers the type
   *
   * @return An payeeIdentifier id or QString() if no data is associated
   */
  QString iid() const;

  /**
   * @brief Thrown if a cast of a payeeIdentifier fails
   *
   * This is inspired by std::bad_cast
   */
  class badCast : public MyMoneyException
  {
  public:
    explicit badCast(const char *exceptionMessage) : MyMoneyException(exceptionMessage) {}
  };

  /**
   * @brief Thrown if one tried to access the data of a null payeeIdentifier
   */
  class empty : public MyMoneyException
  {
  public:
    explicit empty(const char *exceptionMessage) : MyMoneyException(exceptionMessage) {}
  };

private:
  /**
   * The id is only used in MyMoneyPayeeIdentifierContainer at the moment.
   */
  id_t m_id;

  // Must access the id, but the id should not be used outside of that class at the moment
  friend class MyMoneyPayeeIdentifierContainer;
  friend class payeeIdentifierLoader;

  payeeIdentifierData* m_payeeIdentifier;
};

template<class T>
T* payeeIdentifier::data()
{
  T *const ident = dynamic_cast<T*>(operator->());
  if (ident == 0)
    throw PAYEEIDENTIFIERBADCASTEXCEPTION();
  return ident;
}

template<class T>
const T* payeeIdentifier::data() const
{
  const T *const ident = dynamic_cast<const T*>(operator->());
  if (ident == 0)
    throw PAYEEIDENTIFIERBADCASTEXCEPTION();
  return ident;
}

Q_DECLARE_METATYPE(payeeIdentifier)

#endif // PAYEEIDENTIFIER_H
