/***************************************************************************
                          mymoneycostcenter.h
                             -------------------
    copyright            : (C) 2015 by Thomas Baumgart <tbaumgart@kde.org>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYMONEYCOSTCENTER_H
#define MYMONEYCOSTCENTER_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <qobjectdefs.h>
#include <QMetaType>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject.h"

/**
  * This class represents a tag within the MyMoney engine.
  */
class KMM_MYMONEY_EXPORT MyMoneyCostCenter : public MyMoneyObject
{
  Q_GADGET
  KMM_MYMONEY_UNIT_TESTABLE

public:
    enum attrNameE { anName };
    Q_ENUM(attrNameE)

private:
  // Simple fields
  QString m_name;

  static const QString getAttrName(const attrNameE _attr);

public:
  MyMoneyCostCenter();
  MyMoneyCostCenter(const QString& id, const MyMoneyCostCenter& tag);
  explicit MyMoneyCostCenter(const QString& name);

  /**
    * This is the constructor for a tag that is described by a
    * QDomElement (e.g. from a file).
    *
    * @param el const reference to the QDomElement from which to
    *           create the object
    */
  MyMoneyCostCenter(const QDomElement& el);

  ~MyMoneyCostCenter();

  // Simple get operations
  const QString& name() const            {
    return m_name;
  }

  // Simple set operations
  void setName(const QString& val)      {
    m_name = val;
  }

  /**
   * This member returns a possible number leading the name. If there
   * is no number infront of the name, then the full name will be returned
   * @sa name()
   */
  QString shortName() const;

  // Copy constructors
  MyMoneyCostCenter(const MyMoneyCostCenter&);

  // Equality operator
  bool operator == (const MyMoneyCostCenter &) const;
  bool operator <(const MyMoneyCostCenter& right) const;

  void writeXML(QDomDocument& document, QDomElement& parent) const;

  /**
    * This method checks if a reference to the given object exists. It returns,
    * a @p true if the object is referencing the one requested by the
    * parameter @p id. If it does not, this method returns @p false.
    *
    * @param id id of the object to be checked for references
    * @retval true This object references object with id @p id.
    * @retval false This object does not reference the object with id @p id.
    */
  virtual bool hasReferenceTo(const QString& id) const;

  static MyMoneyCostCenter null;
};

inline bool operator==(const MyMoneyCostCenter& lhs, const QString& rhs)
{
  return lhs.id() == rhs;
}

/**
  * Make it possible to hold @ref MyMoneyCostCenter objects inside @ref QVariant objects.
  */
Q_DECLARE_METATYPE(MyMoneyCostCenter)

#endif
