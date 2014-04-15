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

#include <QtCore/QtPlugin>
#include <QtCore/QSharedPointer>
#include <QtCore/QHash>
#include <QtXml/QDomElement>

/**
 * @brief Define a unique identifier for an payeeIdentifier subclass
 * 
 * @param PIDID the payeeIdentifier id, e.g. "org.kmymoney.payeeIdentifier.swift". Must be
 * unique among all payeeIdentifiers as it is used internaly to store data, to compare
 * end for type casting (there must not be more than one class which uses that pidid).
 */
#define PAYEEIDENTIFIER_ID(className, PIDID) \
/** @brief Returns the payeeIdentifier Id */ \
static const QString& staticPayeeIdentifierId() { \
  static const QString _pidid = PIDID; \
  return _pidid; \
  } \
  /** @brief Returns the payeeIdentifier Id */ \
  virtual QString payeeIdentifierId() const { \
    return className::staticPayeeIdentifierId(); \
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
 * Any payee (@see MyMoneyPayee) can have several payeeIdentifiers.
 * 
 * The online banking system uses payeeIdentifiers to dertermine if it is able so create a credit-transfer
 * to a given payee. During import the payeeIdentifiers are used to find a payee.
 * 
 * You should use the shared pointer payeeIdentifier::ptr to handle payeeIdentifiers. To copy them used
 * cloneSharedPtr().
 * 
 * @intenal First this is more complex than creating a superset of all possible identifiers. But there
 * are many of them. And using this method it is a lot easier to create the comparison operatiors and
 * things like isValid().
 * 
 * @section Inheriting
 * 
 * To identify the type of an payeeIdentifier you must use the macro @see PAYEEIDENTIFIER_ID(className, PIDID)
 * in the public section of your subclass.
 */
class payeeIdentifier
{
public:
  /** @brief Shared pointer to payeeIdentifier */
  typedef QSharedPointer<payeeIdentifier> ptr;
  
  /** @brief constant variant of payeeIdentifier::ptr */
  typedef QSharedPointer<const payeeIdentifier> constPtr;
  
  /**
   * @brief Indexed list for payeeIdentifier::ptr
   * 
   * Usually the key is used as index within a payee ( the payees id + this index is a unique key for an element ).
   */
  typedef QHash< unsigned int, payeeIdentifier::ptr> list;
  
  /** @brief Constant variant of payeeIdentifier::list */
  typedef QHash< unsigned int, payeeIdentifier::constPtr> constList;
  
  virtual ~payeeIdentifier() {}
  
  /**
   * Use PAYEEIDENTIFIER_ID(className, PIDID) to reimplement this method.
   */
  virtual QString payeeIdentifierId() const = 0;
  
  /**
   * @brief Comparison operator
   */
  virtual bool operator==(const payeeIdentifier& other) const = 0;
  virtual bool operator!=(const payeeIdentifier& other) { return (!operator==(other)); }
  
  /**
   * @brief Check if this payeeIdentifier contains correct data
   * 
   * You should be able to handle invalid data. It is the task of the ui to prevent
   * invalid data. But during several proceedures invalid data could be used (e.g.
   * during import).
   */
  virtual bool isValid() const = 0;
  
  /**
   * @brief Create a new payeeIdentifier form XML data
   * 
   * @param element Note: there could be more data in that elemenet than you created in writeXML()
   */
  virtual payeeIdentifier* createFromXml(const QDomElement &element) const = 0;

  /**
   * @see MyMoneyObject::writeXML()
   * @warning Do not set an attribute "type" to parent, it is used to store the payeeIdentifierId and set
   * automatically.
   */
  virtual void writeXML(QDomDocument &document, QDomElement &parent) const = 0;
  
  /**
   * @brief Helper to copy a list of pointers
   * 
   * Copies list but uses the clone() method of it's elements to create new objects.
   * 
   * @return a new list with cloned elements
   */
  static QHash< unsigned int, payeeIdentifier::ptr > cloneList( QHash< unsigned int, payeeIdentifier::ptr > list );
  
  /**
   * @brief Create a deep copy
   * 
   * Internaly it calls clone(). No need to overwrite it.
   */
  payeeIdentifier::ptr cloneSharedPtr() const;
  
protected:
  /**
   * @brief Create deep copy
   */
  virtual payeeIdentifier* clone() const = 0;
};

Q_DECLARE_INTERFACE(payeeIdentifier, "org.kmymoney.payeeIdentifier")

#endif // PAYEEIDENTIFIER_H
