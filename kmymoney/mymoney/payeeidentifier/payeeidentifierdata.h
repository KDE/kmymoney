/*
 * Copyright 2014       Christian Dávid <christian-david@web.de>
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

#ifndef PAYEEIDENTIFIERDATA_H
#define PAYEEIDENTIFIERDATA_H

#include "payeeidentifier/kmm_payeeidentifier_export.h"

#include <QtPlugin>
#include <QSharedPointer>
#include <QHash>
#include <QMetaType>
#include <QDomElement>

class payeeIdentifier;
class payeeIdentifierLoader;

/**
 * @brief Define a unique identifier for an payeeIdentifier subclass
 *
 * Use this macro in your class's public section.
 *
 * This also defines the helper ::ptr, ::constPtr and className::ptr cloneSharedPtr()
 *
 * @param PIDID the payeeIdentifier id, e.g. "org.kmymoney.payeeIdentifier.swift". Must be
 * unique among all payeeIdentifiers as it is used internaly to store data, to compare
 * types and for type casting (there must not be more than one class which uses that pidid).
 */
#define PAYEEIDENTIFIER_IID(className, iid) \
  /** @brief Returns the payeeIdentifier Iid */ \
  static const QString& staticPayeeIdentifierIid() { \
    static const QString _pidid = QLatin1String( iid ); \
    return _pidid; \
  } \
  /** @brief Returns the payeeIdentifier Id */ \
  QString payeeIdentifierId() const final override { \
    return className::staticPayeeIdentifierIid(); \
  }

/**
 * @brief "Something" that identifies a payee (or an account of a payee)
 *
 * The simplest form of this class is an identifier for an bank account (consisting of an account number
 * and a bank code). But also an e-mail address which is used by an online money-transfer service could be
 * such an identifier (that is the reason for the abstract name "payeeIdentifier").
 *
 * But also the creditor identifier for debit-notes in sepa-countries can be a subclass. It does not
 * address an account but a company.
 *
 * Any payee (@ref MyMoneyPayee) can have several payeeIdentifiers.
 *
 * The online banking system uses payeeIdentifiers to dertermine if it is able so create a credit-transfer
 * to a given payee. During import the payeeIdentifiers are used to find a payee.
 *
 * You should use the shared pointer payeeIdentifier::ptr to handle payeeIdentifiers. To copy them used
 * cloneSharedPtr().
 *
 * @intenal First this is more complex than creating a superset of all possible identifiers. But there
 * are many of them. And using this method it is a lot easier to create the comparison operators and
 * things like isValid().
 *
 * @section Inheriting
 *
 * To identify the type of an payeeIdentifier you must use the macro @ref PAYEEIDENTIFIER_IID()
 * in the public section of your subclass.
 */
class KMM_PAYEEIDENTIFIER_EXPORT payeeIdentifierData
{
public:
  virtual ~payeeIdentifierData() {}

  /**
   * Use PAYEEIDENTIFIER_ID(className, PIDID) to reimplement this method.
   */
  virtual QString payeeIdentifierId() const = 0;

  /**
   * @brief Comparison operator
   */
  virtual bool operator==(const payeeIdentifierData& other) const = 0;
  virtual bool operator!=(const payeeIdentifierData& other) const {
    return (!operator==(other));
  }

  /**
   * @brief Check if this payeeIdentifier contains correct data
   *
   * You should be able to handle invalid data. It is the task of the ui to prevent
   * invalid data. But during several procedures invalid data could be used (e.g.
   * during import).
   */
  virtual bool isValid() const = 0;

  /**
   * @brief Create a new payeeIdentifier form XML data
   *
   * @param element Note: there could be more data in that elemenet than you created in writeXML()
   */
  virtual payeeIdentifierData* createFromXml(const QDomElement &element) const = 0;

  /**
   * @see MyMoneyObject::writeXML()
   *
   * @warning Do not set an attribute "type" or "id" to parent, it is used to store internal data and is
   * set automatically.
   */
  virtual void writeXML(QDomDocument &document, QDomElement &parent) const = 0;

protected:
  /**
   * @brief Create deep copy
   */
  virtual payeeIdentifierData* clone() const = 0;
  friend class payeeIdentifierLoader;
  friend class payeeIdentifier;
};

Q_DECLARE_INTERFACE(payeeIdentifierData, "org.kmymoney.payeeIdentifier")

#endif // PAYEEIDENTIFIERDATA_H
