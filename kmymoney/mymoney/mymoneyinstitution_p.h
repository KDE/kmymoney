/***************************************************************************
                          mymoneyinstitution.cpp
                          -------------------
    copyright            : (C) 2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2002-2005 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *******************************************************************
********/

#ifndef MYMONEYINSTITUTION_P_H
#define MYMONEYINSTITUTION_P_H

#include "mymoneyinstitution.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QHash>
#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyobject_p.h"

namespace Institution
{
  enum class Element { AccountID,
                       AccountIDS,
                       Address };
  uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

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
  uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

class MyMoneyInstitutionPrivate : public MyMoneyObjectPrivate
{
public:
  static QString getElName(const Institution::Element el)
  {
    static const QMap<Institution::Element, QString> elNames {
      {Institution::Element::AccountID,  QStringLiteral("ACCOUNTID")},
      {Institution::Element::AccountIDS, QStringLiteral("ACCOUNTIDS")},
      {Institution::Element::Address,    QStringLiteral("ADDRESS")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Institution::Attribute attr)
  {
    static const QHash<Institution::Attribute, QString> attrNames {
      {Institution::Attribute::ID,         QStringLiteral("id")},
      {Institution::Attribute::Name,       QStringLiteral("name")},
      {Institution::Attribute::Manager,    QStringLiteral("manager")},
      {Institution::Attribute::SortCode,   QStringLiteral("sortcode")},
      {Institution::Attribute::Street,     QStringLiteral("street")},
      {Institution::Attribute::City,       QStringLiteral("city")},
      {Institution::Attribute::Zip,        QStringLiteral("zip")},
      {Institution::Attribute::Telephone,  QStringLiteral("telephone")}
    };
    return attrNames[attr];
  }

  /**
    * This member variable keeps the name of the institution
    */
  QString m_name;

  /**
    * This member variable keeps the city of the institution
    */
  QString m_town;

  /**
    * This member variable keeps the street of the institution
    */
  QString m_street;

  /**
    * This member variable keeps the zip-code of the institution
    */
  QString m_postcode;

  /**
    * This member variable keeps the telephone number of the institution
    */
  QString m_telephone;

  /**
    * This member variable keeps the name of the representative of
    * the institution
    */
  QString m_manager;

  /**
    * This member variable keeps the sort code of the institution.
    * FIXME: I have no idea
    * what it is good for. I keep it because it was in the old engine.
    */
  QString m_sortcode;

  /**
    * This member variable keeps the sorted list of the account ids
    * available at this institution
    */
  QStringList m_accountList;
};

#endif
