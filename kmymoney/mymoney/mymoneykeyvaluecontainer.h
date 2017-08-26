/***************************************************************************
                          mymoneykeyvaluecontainer.h
                             -------------------
    begin                : Sun Nov 10 2002
    copyright            : (C) 2000-2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYKEYVALUECONTAINER_H
#define MYMONEYKEYVALUECONTAINER_H


/**
  * @author Thomas Baumgart
  */

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QMap>
#include <QDomDocument>
#include <QDomElement>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmm_mymoney_export.h>
#include <mymoneyunittestable.h>

/**
  * This class implements a container for key/value pairs. This is used
  * to store an arbitrary number of attributes with any of the engine
  * object. The container can also be used to have attributes that are
  * attached to this object only for a limited time (e.g. between
  * start of reconciliation end it's end).
  *
  * To give any class the ability to have a key/value pair container,
  * just derive the class from this one. See MyMoneyAccount as an example.
  */
class KMM_MYMONEY_EXPORT MyMoneyKeyValueContainer
{
  KMM_MYMONEY_UNIT_TESTABLE

public:
  MyMoneyKeyValueContainer();
  MyMoneyKeyValueContainer(const QDomElement& node);

  ~MyMoneyKeyValueContainer();

  /**
    * This method can be used to retrieve the value for a specific @p key.
    * If the key is unknown in this container, an empty string will be returned.
    *
    * @param key const reference to QString with the key to search for
    * @return reference to value of this key. If the key does not exist,
    *         an emtpy string is returned.
    */
  const QString& value(const QString& key) const;

  /**
    * This method is used to add a key/value pair to the container or
    * modify an existing pair.
    *
    * @param key const reference to QString with the key to search for
    * @param value const reference to QString with the value for this key
    */
  void setValue(const QString& key, const QString& value);

  /**
    * This method is used to remove an existing key/value pair from the
    * container. If the key does not exist, the container is not changed.
    *
    * @param key const reference to QString with the key to remove
    */
  void deletePair(const QString& key);

  /**
    * This method clears all pairs currently in the container.
    */
  void clear();

  /**
    * This method is used to retrieve the whole set of key/value pairs
    * from the container. It is meant to be used for permanent storage
    * functionality.
    *
    * @return QMap<QString, QString> containing all key/value pairs of
    *         this container.
    */
  const QMap<QString, QString>& pairs() const {
    return m_kvp;
  };

  /**
    * This method is used to initially store a set of key/value pairs
    * in the container. It is meant to be used for loading functionality
    * from permanent storage.
    *
    * @param list const QMap<QString, QString> containing the set of
    *             key/value pairs to be loaded into the container.
    *
    * @note All existing key/value pairs in the container will be deleted.
    */
  void setPairs(const QMap<QString, QString>& list);

  /**
    * This operator tests for equality of two MyMoneyKeyValueContainer objects
    */
  bool operator == (const MyMoneyKeyValueContainer &) const;

  const QString& operator[](const QString& k) const {
    return value(k);
  }

  QString& operator[](const QString& k) {
    return m_kvp[k];
  }

  /**
    * This method creates a QDomElement for the @p document
    * under the parent node @p parent.
    *
    * @param document reference to QDomDocument
    * @param parent reference to QDomElement parent node
    */
  void writeXML(QDomDocument& document, QDomElement& parent) const;

private:
  /**
    * This member variable represents the container of key/value pairs.
    */
  QMap<QString, QString>  m_kvp;
};

QDebug operator<<(QDebug dbg, const MyMoneyKeyValueContainer &a);
#endif
