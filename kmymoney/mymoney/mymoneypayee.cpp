/*
 * Copyright 2000-2001  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2002-2017  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2003       Kevin Tambascio <ktambascio@users.sourceforge.net>
 * Copyright 2006       Ace Jones <acejones@users.sourceforge.net>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "mymoneypayee.h"
#include "mymoneypayee_p.h"

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

MyMoneyPayee::MyMoneyPayee() :
  MyMoneyObject(*new MyMoneyPayeePrivate)
{
}

MyMoneyPayee::MyMoneyPayee(const QString& name,
                           const QString& address,
                           const QString& city,
                           const QString& state,
                           const QString& postcode,
                           const QString& telephone,
                           const QString& email) :
  MyMoneyObject(*new MyMoneyPayeePrivate)
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
    MyMoneyObject(*new MyMoneyPayeePrivate, node)
{
  if (nodeNames[nnPayee] != node.tagName()) {
    throw MYMONEYEXCEPTION_CSTRING("Node was not PAYEE");
  }

  Q_D(MyMoneyPayee);
  d->m_name = node.attribute(d->getAttrName(Payee::Attribute::Name));
  d->m_reference = node.attribute(d->getAttrName(Payee::Attribute::Reference));
  d->m_email = node.attribute(d->getAttrName(Payee::Attribute::Email));

  d->m_matchingEnabled = node.attribute(d->getAttrName(Payee::Attribute::MatchingEnabled), "0").toUInt();
  if (d->m_matchingEnabled) {
    setMatchData((node.attribute(d->getAttrName(Payee::Attribute::UsingMatchKey), "0").toUInt() != 0) ? matchKey : matchName,
                 node.attribute(d->getAttrName(Payee::Attribute::MatchIgnoreCase), "0").toUInt(),
                 node.attribute(d->getAttrName(Payee::Attribute::MatchKey)));
  }

  if (node.hasAttribute(d->getAttrName(Payee::Attribute::Notes))) {
    d->m_notes = node.attribute(d->getAttrName(Payee::Attribute::Notes));
  }

  if (node.hasAttribute(d->getAttrName(Payee::Attribute::DefaultAccountID))) {
    d->m_defaultAccountId = node.attribute(d->getAttrName(Payee::Attribute::DefaultAccountID));
  }

  // Load Address
  QDomNodeList nodeList = node.elementsByTagName(d->getElName(Payee::Element::Address));
  if (nodeList.isEmpty())
    throw MYMONEYEXCEPTION(QString::fromLatin1("No ADDRESS in payee %1").arg(d->m_name));

  QDomElement addrNode = nodeList.item(0).toElement();
  d->m_address = addrNode.attribute(d->getAttrName(Payee::Attribute::Street));
  d->m_city = addrNode.attribute(d->getAttrName(Payee::Attribute::City));
  d->m_postcode = addrNode.attribute(d->getAttrName(Payee::Attribute::PostCode));
  d->m_state = addrNode.attribute(d->getAttrName(Payee::Attribute::State));
  d->m_telephone = addrNode.attribute(d->getAttrName(Payee::Attribute::Telephone));

  MyMoneyPayeeIdentifierContainer::loadXML(node);
}

MyMoneyPayee::MyMoneyPayee(const MyMoneyPayee& other) :
  MyMoneyObject(*new MyMoneyPayeePrivate(*other.d_func()), other.id()),
  MyMoneyPayeeIdentifierContainer(other)
{
}

MyMoneyPayee::MyMoneyPayee(const QString& id, const MyMoneyPayee& other) :
  MyMoneyObject(*new MyMoneyPayeePrivate(*other.d_func()), id),
  MyMoneyPayeeIdentifierContainer(other)
{
}

MyMoneyPayee::~MyMoneyPayee()
{
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

  Q_D(const MyMoneyPayee);
  d->writeBaseXML(document, el);

  el.setAttribute(d->getAttrName(Payee::Attribute::Name), d->m_name);
  el.setAttribute(d->getAttrName(Payee::Attribute::Reference), d->m_reference);
  el.setAttribute(d->getAttrName(Payee::Attribute::Email), d->m_email);
  if (!d->m_notes.isEmpty())
    el.setAttribute(d->getAttrName(Payee::Attribute::Notes), d->m_notes);

  el.setAttribute(d->getAttrName(Payee::Attribute::MatchingEnabled), d->m_matchingEnabled);
  if (d->m_matchingEnabled) {
    el.setAttribute(d->getAttrName(Payee::Attribute::UsingMatchKey), d->m_usingMatchKey);
    el.setAttribute(d->getAttrName(Payee::Attribute::MatchIgnoreCase), d->m_matchKeyIgnoreCase);
    el.setAttribute(d->getAttrName(Payee::Attribute::MatchKey), d->m_matchKey);
  }

  if (!d->m_defaultAccountId.isEmpty()) {
    el.setAttribute(d->getAttrName(Payee::Attribute::DefaultAccountID), d->m_defaultAccountId);
  }

  // Save address
  QDomElement address = document.createElement(d->getElName(Payee::Element::Address));
  address.setAttribute(d->getAttrName(Payee::Attribute::Street), d->m_address);
  address.setAttribute(d->getAttrName(Payee::Attribute::City), d->m_city);
  address.setAttribute(d->getAttrName(Payee::Attribute::PostCode), d->m_postcode);
  address.setAttribute(d->getAttrName(Payee::Attribute::State), d->m_state);
  address.setAttribute(d->getAttrName(Payee::Attribute::Telephone), d->m_telephone);

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
  setDefaultAccountId(QString());
}

// vim:cin:si:ai:et:ts=2:sw=2:
