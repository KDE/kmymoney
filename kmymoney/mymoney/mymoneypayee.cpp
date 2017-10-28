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

#include "mymoneypayee.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QString>
#include <QStringList>
#include <QDomElement>
#include <QMap>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyexception.h>
#include "mymoneystoragenames.h"

using namespace MyMoneyStorageNodes;

MyMoneyPayee MyMoneyPayee::null;

class MyMoneyPayeePrivate {

public:

  MyMoneyPayeePrivate() :
    m_matchingEnabled(false),
    m_usingMatchKey(false),
    m_matchKeyIgnoreCase(true)
  {
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

MyMoneyPayee::MyMoneyPayee() :
  d_ptr(new MyMoneyPayeePrivate)
{
}

MyMoneyPayee::MyMoneyPayee(const QString& name,
                           const QString& address,
                           const QString& city,
                           const QString& state,
                           const QString& postcode,
                           const QString& telephone,
                           const QString& email) :
  d_ptr(new MyMoneyPayeePrivate)
{
  Q_D(MyMoneyPayee);
  d->m_name      = name;
  d->m_address   = address;
  d->m_city      = city;
  d->m_state     = state;
  d->m_postcode  = postcode;
  d->m_telephone = telephone;
  d->m_email     = email;
  d->m_matchingEnabled = false;
  d->m_usingMatchKey = false;
  d->m_matchKeyIgnoreCase = true;
}

MyMoneyPayee::MyMoneyPayee(const QDomElement& node) :
    MyMoneyObject(node),
    d_ptr(new MyMoneyPayeePrivate)
{
  if (nodeNames[nnPayee] != node.tagName()) {
    throw MYMONEYEXCEPTION("Node was not PAYEE");
  }

  Q_D(MyMoneyPayee);
  d->m_name = node.attribute(getAttrName(Attribute::Name));
  d->m_reference = node.attribute(getAttrName(Attribute::Reference));
  d->m_email = node.attribute(getAttrName(Attribute::Email));

  d->m_matchingEnabled = node.attribute(getAttrName(Attribute::MatchingEnabled), "0").toUInt();
  if (d->m_matchingEnabled) {
    setMatchData((node.attribute(getAttrName(Attribute::UsingMatchKey), "0").toUInt() != 0) ? matchKey : matchName,
                 node.attribute(getAttrName(Attribute::MatchIgnoreCase), "0").toUInt(),
                 node.attribute(getAttrName(Attribute::MatchKey)));
  }

  if (node.hasAttribute(getAttrName(Attribute::Notes))) {
    d->m_notes = node.attribute(getAttrName(Attribute::Notes));
  }

  if (node.hasAttribute(getAttrName(Attribute::DefaultAccountID))) {
    d->m_defaultAccountId = node.attribute(getAttrName(Attribute::DefaultAccountID));
  }

  // Load Address
  QDomNodeList nodeList = node.elementsByTagName(getElName(Element::Address));
  if (nodeList.count() == 0) {
    QString msg = QString("No ADDRESS in payee %1").arg(d->m_name);
    throw MYMONEYEXCEPTION(msg);
  }

  QDomElement addrNode = nodeList.item(0).toElement();
  d->m_address = addrNode.attribute(getAttrName(Attribute::Street));
  d->m_city = addrNode.attribute(getAttrName(Attribute::City));
  d->m_postcode = addrNode.attribute(getAttrName(Attribute::PostCode));
  d->m_state = addrNode.attribute(getAttrName(Attribute::State));
  d->m_telephone = addrNode.attribute(getAttrName(Attribute::Telephone));

  MyMoneyPayeeIdentifierContainer::loadXML(node);
}

MyMoneyPayee::MyMoneyPayee(const MyMoneyPayee& other) :
  MyMoneyObject(other.id()),
  MyMoneyPayeeIdentifierContainer(other),
  d_ptr(new MyMoneyPayeePrivate(*other.d_func()))
{
}

MyMoneyPayee::MyMoneyPayee(const QString& id, const MyMoneyPayee& other) :
  MyMoneyObject(id),
  MyMoneyPayeeIdentifierContainer(other),
  d_ptr(new MyMoneyPayeePrivate(*other.d_func()))
{
}

MyMoneyPayee::~MyMoneyPayee()
{
  Q_D(MyMoneyPayee);
  delete d;
}

bool MyMoneyPayee::operator == (const MyMoneyPayee& right) const
{
  Q_D(const MyMoneyPayee);
  auto d2 = static_cast<const MyMoneyPayeePrivate *>(right.d_func());
  return (MyMoneyObject::operator==(right) &&
          ((d->m_name.length() == 0 && d2->m_name.length() == 0) || (d->m_name == d2->m_name)) &&
          ((d->m_address.length() == 0 && d2->m_address.length() == 0) || (d->m_address == d2->m_address)) &&
          ((d->m_city.length() == 0 && d2->m_city.length() == 0) || (d->m_city == d2->m_city)) &&
          ((d->m_state.length() == 0 && d2->m_state.length() == 0) || (d->m_state == d2->m_state)) &&
          ((d->m_postcode.length() == 0 && d2->m_postcode.length() == 0) || (d->m_postcode == d2->m_postcode)) &&
          ((d->m_telephone.length() == 0 && d2->m_telephone.length() == 0) || (d->m_telephone == d2->m_telephone)) &&
          ((d->m_email.length() == 0 && d2->m_email.length() == 0) || (d->m_email == d2->m_email)) &&
          (d->m_matchingEnabled == d2->m_matchingEnabled) &&
          (d->m_usingMatchKey == d2->m_usingMatchKey) &&
          (d->m_matchKeyIgnoreCase == d2->m_matchKeyIgnoreCase) &&
          ((d->m_matchKey.length() == 0 && d2->m_matchKey.length() == 0) || d->m_matchKey == d2->m_matchKey) &&
          ((d->m_reference.length() == 0 && d2->m_reference.length() == 0) || (d->m_reference == d2->m_reference)) &&
          ((d->m_defaultAccountId.length() == 0 && d2->m_defaultAccountId.length() == 0) || d->m_defaultAccountId == d2->m_defaultAccountId));
}

//bool MyMoneyPayee::operator == (const MyMoneyPayee& lhs, const QString& rhs) const
//{
//  Q_D(const MyMoneyPayee);
//  return lhs.id() == rhs;
//}

bool MyMoneyPayee::operator < (const MyMoneyPayee& right) const
{
  Q_D(const MyMoneyPayee);
  auto d2 = static_cast<const MyMoneyPayeePrivate *>(right.d_func());
  return d->m_name < d2->m_name;
}

void MyMoneyPayee::writeXML(QDomDocument& document, QDomElement& parent) const
{
  auto el = document.createElement(nodeNames[nnPayee]);

  writeBaseXML(document, el);

  Q_D(const MyMoneyPayee);
  el.setAttribute(getAttrName(Attribute::Name), d->m_name);
  el.setAttribute(getAttrName(Attribute::Reference), d->m_reference);
  el.setAttribute(getAttrName(Attribute::Email), d->m_email);
  if (!d->m_notes.isEmpty())
    el.setAttribute(getAttrName(Attribute::Notes), d->m_notes);

  el.setAttribute(getAttrName(Attribute::MatchingEnabled), d->m_matchingEnabled);
  if (d->m_matchingEnabled) {
    el.setAttribute(getAttrName(Attribute::UsingMatchKey), d->m_usingMatchKey);
    el.setAttribute(getAttrName(Attribute::MatchIgnoreCase), d->m_matchKeyIgnoreCase);
    el.setAttribute(getAttrName(Attribute::MatchKey), d->m_matchKey);
  }

  if (!d->m_defaultAccountId.isEmpty()) {
    el.setAttribute(getAttrName(Attribute::DefaultAccountID), d->m_defaultAccountId);
  }

  // Save address
  QDomElement address = document.createElement(getElName(Element::Address));
  address.setAttribute(getAttrName(Attribute::Street), d->m_address);
  address.setAttribute(getAttrName(Attribute::City), d->m_city);
  address.setAttribute(getAttrName(Attribute::PostCode), d->m_postcode);
  address.setAttribute(getAttrName(Attribute::State), d->m_state);
  address.setAttribute(getAttrName(Attribute::Telephone), d->m_telephone);

  el.appendChild(address);

  // Save payeeIdentifiers (account numbers)
  MyMoneyPayeeIdentifierContainer::writeXML(document, el);

  parent.appendChild(el);
}

bool MyMoneyPayee::hasReferenceTo(const QString& id) const
{
  Q_D(const MyMoneyPayee);
  return id == d->m_defaultAccountId;
}

QString MyMoneyPayee::name() const
{
  Q_D(const MyMoneyPayee);
  return d->m_name;
}

void MyMoneyPayee::setName(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_name = val;
}

QString MyMoneyPayee::address() const
{
  Q_D(const MyMoneyPayee);
  return d->m_address;
}

void MyMoneyPayee::setAddress(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_address = val;
}

QString MyMoneyPayee::city() const
{
  Q_D(const MyMoneyPayee);
  return d->m_city;
}

void MyMoneyPayee::setCity(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_city = val;
}

QString MyMoneyPayee::state() const
{
  Q_D(const MyMoneyPayee);
  return d->m_state;
}

void MyMoneyPayee::setState(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_state = val;
}

QString MyMoneyPayee::postcode() const
{
  Q_D(const MyMoneyPayee);
  return d->m_postcode;
}

void MyMoneyPayee::setPostcode(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_postcode = val;
}

QString MyMoneyPayee::telephone() const
{
  Q_D(const MyMoneyPayee);
  return d->m_telephone;
}

void MyMoneyPayee::setTelephone(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_telephone = val;
}

QString MyMoneyPayee::email() const
{
  Q_D(const MyMoneyPayee);
  return d->m_email;
}

void MyMoneyPayee::setEmail(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_email = val;
}

QString MyMoneyPayee::notes() const
{
  Q_D(const MyMoneyPayee);
  return d->m_notes;
}

void MyMoneyPayee::setNotes(const QString& val)
{
  Q_D(MyMoneyPayee);
  d->m_notes = val;
}

QString MyMoneyPayee::reference() const
{
  Q_D(const MyMoneyPayee);
  return d->m_reference;
}

void MyMoneyPayee::setReference(const QString& ref)
{
  Q_D(MyMoneyPayee);
  d->m_reference = ref;
}

MyMoneyPayee::payeeMatchType MyMoneyPayee::matchData(bool& ignorecase, QStringList& keys) const
{
  payeeMatchType type = matchDisabled;
  keys.clear();

  Q_D(const MyMoneyPayee);
  ignorecase = d->m_matchKeyIgnoreCase;

  if (d->m_matchingEnabled) {
    type = d->m_usingMatchKey ? matchKey : matchName;
    if (type == matchKey) {
      if (d->m_matchKey.contains(QLatin1Char('\n')))
        keys = d->m_matchKey.split(QLatin1Char('\n'));
      else
        keys = d->m_matchKey.split(QLatin1Char(';'));  // for compatibility with 4.8.0
    } else if (d->m_matchKey.compare(QLatin1String("^$")) == 0) {
      type = matchNameExact;
    }
  }

  return type;
}

MyMoneyPayee::payeeMatchType MyMoneyPayee::matchData(bool& ignorecase, QString& keyString) const
{
  QStringList keys;
  payeeMatchType type = matchData(ignorecase, keys);
  keyString = keys.join(QLatin1Char('\n'));
  return type;
}

void MyMoneyPayee::setMatchData(payeeMatchType type, bool ignorecase, const QStringList& keys)
{
  Q_D(MyMoneyPayee);
  d->m_matchingEnabled = (type != matchDisabled);
  d->m_matchKeyIgnoreCase = ignorecase;
  d->m_matchKey.clear();

  if (d->m_matchingEnabled) {
    d->m_usingMatchKey = (type == matchKey);
    if (d->m_usingMatchKey) {
      QRegExp validKeyRegExp("[^ ]");
      QStringList filteredKeys = keys.filter(validKeyRegExp);
      d->m_matchKey = filteredKeys.join(QLatin1Char('\n'));
    } else if(type == matchNameExact) {
      d->m_matchKey = QLatin1String("^$");
    }
  }
}

void MyMoneyPayee::setMatchData(payeeMatchType type, bool ignorecase, const QString& keys)
{
  if (keys.contains(QLatin1Char('\n')))
    setMatchData(type, ignorecase, keys.split(QLatin1Char('\n')));
  else
    setMatchData(type, ignorecase, keys.split(QLatin1Char(';'))); // for compatibility with 4.8.0
}

bool MyMoneyPayee::defaultAccountEnabled() const
{
  Q_D(const MyMoneyPayee);
  return !d->m_defaultAccountId.isEmpty();
}

QString MyMoneyPayee::defaultAccountId() const
{
  Q_D(const MyMoneyPayee);
  return d->m_defaultAccountId;
}

void MyMoneyPayee::setDefaultAccountId(const QString& id)
{
  Q_D(MyMoneyPayee);
  d->m_defaultAccountId = id;
}

void MyMoneyPayee::setDefaultAccountId()
{
  Q_D(MyMoneyPayee);
  setDefaultAccountId(QString());
}


QString MyMoneyPayee::getElName(const Element el)
{
  static const QMap<Element, QString> elNames = {
    {Element::Address, QStringLiteral("ADDRESS")}
  };
  return elNames[el];
}

QString MyMoneyPayee::getAttrName(const Attribute attr)
{
  static const QHash<Attribute, QString> attrNames = {
    {Attribute::Name,             QStringLiteral("name")},
    {Attribute::Type,             QStringLiteral("type")},
    {Attribute::Reference,        QStringLiteral("reference")},
    {Attribute::Notes,            QStringLiteral("notes")},
    {Attribute::MatchingEnabled,  QStringLiteral("matchingenabled")},
    {Attribute::UsingMatchKey,    QStringLiteral("usingmatchkey")},
    {Attribute::MatchIgnoreCase,  QStringLiteral("matchignorecase")},
    {Attribute::MatchKey,         QStringLiteral("matchkey")},
    {Attribute::DefaultAccountID, QStringLiteral("defaultaccountid")},
    {Attribute::Street,           QStringLiteral("street")},
    {Attribute::City,             QStringLiteral("city")},
    {Attribute::PostCode,         QStringLiteral("postcode")},
    {Attribute::Email,            QStringLiteral("email")},
    {Attribute::State,            QStringLiteral("state")},
    {Attribute::Telephone,        QStringLiteral("telephone")},
  };
  return attrNames[attr];
}

// vim:cin:si:ai:et:ts=2:sw=2:
