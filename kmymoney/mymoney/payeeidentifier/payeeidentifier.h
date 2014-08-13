/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include <QtXml/QDomElement>

/** @todo fix include path after upgrade to cmake 3 */
#include "payeeidentifier/kmm_payeeidentifier_export.h"
#include "mymoneyexception.h"

// Q_DECLARE_METATYPE requries this include
#include "payeeidentifierdata.h"

class KMM_PAYEEIDENTIFIER_EXPORT payeeIdentifier
{
public:
  explicit payeeIdentifier();
  explicit payeeIdentifier( payeeIdentifierData *const identifier );

  payeeIdentifier(const payeeIdentifier& other);
  ~payeeIdentifier();
  payeeIdentifier& operator=(const payeeIdentifier& other);
  bool operator==(const payeeIdentifier& other);

  /** @brief Check if any data is associated */
  bool isNull() const { return (m_payeeIdentifier == 0); }

  /**
   * @brief create xml to save this payeeIdentifier
   *
   * It creates a new element below parent which is used to store all data.
   *
   * The counter part to load a payee identifier again is payeeIdentifierLoader::createPayeeIdentifierFromXML().
   */
  void writeXML(QDomDocument &document, QDomElement &parent, const QString& elementName = QLatin1String("payeeIdentifier") ) const;

  /**
   * @throws nullPayeeIdentifier
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

  bool isValid() const;

  /**
   * @brief Get payeeIdentifier id
   *
   * @return An payeeIdentifier id or QString() if no data is associated
   */
  QString iid() const;

  /**
   * @brief Thrown if a cast of a task fails
   *
   * This is inspired by std::bad_cast
   * @todo inherit from MyMoneyException
   */
  class badPayeeIdenitifierCast
  {
  public:
    badPayeeIdenitifierCast(const QString& file = "", const long unsigned int& line = 0)
    //: MyMoneyException("Casted payeeIdentifier with wrong type", file, line)
    { Q_UNUSED(file); Q_UNUSED(line); }
  };

  /**
   * @brief Thrown if a task of an invalid onlineJob is requested
   * @todo inherit from MyMoneyException
   */
  class nullPayeeIdentifier
  {
  public:
    nullPayeeIdentifier(const QString& file = "", const long unsigned int& line = 0)
    //: MyMoneyException("Requested payeeIdentifierData of empty payeeIdentifier", file, line)
    { Q_UNUSED(file); Q_UNUSED(line); }
  };

private:
  payeeIdentifierData* m_payeeIdentifier;
};

template<class T>
T* payeeIdentifier::data()
{
  T *const ident = dynamic_cast<T*>(m_payeeIdentifier);
  if ( ident == 0 )
    throw badPayeeIdenitifierCast(__FILE__, __LINE__);
  return ident;
}

Q_DECLARE_METATYPE( payeeIdentifier )

#endif // PAYEEIDENTIFIER_H
