/***************************************************************************
                          mymoneypayee.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2008 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef MYMONEYPAYEE_P_H
#define MYMONEYPAYEE_P_H


// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QMap>
#include <QHash>

// ----------------------------------------------------------------------------
// Project Includes

namespace Payee
{
  enum class Element { Address };
  uint qHash(const Element key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

  enum class Attribute { Name = 0,
                         Type,
                         Reference,
                         Notes,
                         MatchingEnabled,
                         UsingMatchKey,
                         MatchIgnoreCase,
                         MatchKey,
                         DefaultAccountID,
                         Street,
                         City,
                         PostCode,
                         Email,
                         State,
                         Telephone,
                         // insert new entries above this line
                         LastAttribute
                       };
  uint qHash(const Attribute key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

class MyMoneyPayeePrivate {

public:

  MyMoneyPayeePrivate() :
    m_matchingEnabled(false),
    m_usingMatchKey(false),
    m_matchKeyIgnoreCase(true)
  {
  }

  static QString getElName(const Payee::Element el)
  {
    static const QMap<Payee::Element, QString> elNames {
      {Payee::Element::Address, QStringLiteral("ADDRESS")}
    };
    return elNames[el];
  }

  static QString getAttrName(const Payee::Attribute attr)
  {
    static const QHash<Payee::Attribute, QString> attrNames {
      {Payee::Attribute::Name,             QStringLiteral("name")},
      {Payee::Attribute::Type,             QStringLiteral("type")},
      {Payee::Attribute::Reference,        QStringLiteral("reference")},
      {Payee::Attribute::Notes,            QStringLiteral("notes")},
      {Payee::Attribute::MatchingEnabled,  QStringLiteral("matchingenabled")},
      {Payee::Attribute::UsingMatchKey,    QStringLiteral("usingmatchkey")},
      {Payee::Attribute::MatchIgnoreCase,  QStringLiteral("matchignorecase")},
      {Payee::Attribute::MatchKey,         QStringLiteral("matchkey")},
      {Payee::Attribute::DefaultAccountID, QStringLiteral("defaultaccountid")},
      {Payee::Attribute::Street,           QStringLiteral("street")},
      {Payee::Attribute::City,             QStringLiteral("city")},
      {Payee::Attribute::PostCode,         QStringLiteral("postcode")},
      {Payee::Attribute::Email,            QStringLiteral("email")},
      {Payee::Attribute::State,            QStringLiteral("state")},
      {Payee::Attribute::Telephone,        QStringLiteral("telephone")},
    };
    return attrNames[attr];
  }

  // Simple fields
  QString m_name;
  QString m_address;
  QString m_city;
  QString m_state;
  QString m_postcode;
  QString m_telephone;
  QString m_email;
  QString m_notes;

  // Transaction matching fields
  bool m_matchingEnabled;      //< Whether this payee should be matched at all
  bool m_usingMatchKey;        //< If so, whether a m_matchKey list is used (true), or just m_name is used (false)
  bool m_matchKeyIgnoreCase;   //< Whether to ignore the case of the match key or name

  /**
   * Semicolon separated list of matching keys used when trying to find a suitable
   * payee for imported transactions.
   */
  QString m_matchKey;

  // Category (account) matching fields
  QString m_defaultAccountId;

  /**
    * This member keeps a reference to an external database
    * (e.g. kaddressbook). It is the responsibility of the
    * application to format the reference string
    * (e.g. encoding the name of the external database into the
    * reference string).
    * If no external database is available it should be kept
    * empty by the application.
    */
  QString m_reference;

};

#endif

// vim:cin:si:ai:et:ts=2:sw=2:
