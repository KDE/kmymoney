/***************************************************************************
                          mymoneypayee.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                           (C) 2008 by Thomas Baumgart <ipwizard@users.sourceforge.net>

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

#include <QStringList>
#include <QDebug> //! @todo remove QDebugs

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyutils.h"
#include <mymoneyexception.h>
#include "payeeidentifier/payeeidentifierloader.h"
#include <pluginloader.h>

MyMoneyPayee MyMoneyPayee::null;

MyMoneyPayee::MyMoneyPayee() :
    m_matchingEnabled(false),
    m_usingMatchKey(false),
    m_matchKeyIgnoreCase(true)
{
}

MyMoneyPayee::MyMoneyPayee(const QString& id, const MyMoneyPayee& payee) :
    m_matchingEnabled(false),
    m_usingMatchKey(false),
    m_matchKeyIgnoreCase(true)
{
  *this = payee;
  m_id = id;
}

MyMoneyPayee::MyMoneyPayee(const QString& name, const QString& address,
                           const QString& city, const QString& state, const QString& postcode,
                           const QString& telephone, const QString& email) :
    m_matchingEnabled(false),
    m_usingMatchKey(false),
    m_matchKeyIgnoreCase(true)
{
  m_name      = name;
  m_address   = address;
  m_city      = city;
  m_state     = state;
  m_postcode  = postcode;
  m_telephone = telephone;
  m_email     = email;
}

MyMoneyPayee::MyMoneyPayee(const QDomElement& node) :
    MyMoneyObject(node)
{
  if ("PAYEE" != node.tagName()) {
    throw MYMONEYEXCEPTION("Node was not PAYEE");
  }

  m_name = node.attribute("name");
  m_reference = node.attribute("reference");
  m_email = node.attribute("email");

  m_matchingEnabled = node.attribute("matchingenabled", "0").toUInt();
  if (m_matchingEnabled) {
    setMatchData((node.attribute("usingmatchkey", "0").toUInt() != 0) ? matchKey : matchName,
                 node.attribute("matchignorecase", "0").toUInt(),
                 node.attribute("matchkey"));
  }

  if (node.hasAttribute("notes")) {
    m_notes = node.attribute("notes");
  }

  if (node.hasAttribute("defaultaccountid")) {
    m_defaultAccountId = node.attribute("defaultaccountid");
  }

  // Load Address
  QDomNodeList nodeList = node.elementsByTagName("ADDRESS");
  if (nodeList.count() == 0) {
    QString msg = QString("No ADDRESS in payee %1").arg(m_name);
    throw MYMONEYEXCEPTION(msg);
  }

  QDomElement addrNode = nodeList.item(0).toElement();
  m_address = addrNode.attribute("street");
  m_city = addrNode.attribute("city");
  m_postcode = addrNode.attribute("postcode");
  m_state = addrNode.attribute("state");
  m_telephone = addrNode.attribute("telephone");

  // Load identifiers
  QDomNodeList identifierNodes = node.elementsByTagName("payeeIdentifier");
  uint identifierNodesLength = identifierNodes.length();
  for (uint i = 0; i < identifierNodesLength; ++i) {
    const QDomElement element = identifierNodes.item(i).toElement();
    const QString payeeIdentifierId = element.attribute("type", QString());
    if ( !payeeIdentifierId.isEmpty() ) {
      payeeIdentifier::ptr ident = payeeIdentifierLoader::instance()->createPayeeIdentifierFromXML( payeeIdentifierId, element );
      if ( ident.isNull() ) {
        /** @internal improve error handling of payee identifier loading if they are not linked staticaly anymore. */
        qWarning() << "Could not load payee identifier" << payeeIdentifierId;
        continue;
      }
      addPayeeIdentifier( ident );
      qDebug() << "Got something " << payeeIdentifierId;

    }
  }
}

MyMoneyPayee::~MyMoneyPayee()
{
}

MyMoneyPayee::MyMoneyPayee(const MyMoneyPayee& right) :
    MyMoneyObject(right)
{
  *this = right;
}

bool MyMoneyPayee::operator == (const MyMoneyPayee& right) const
{
  return (MyMoneyObject::operator==(right) &&
          ((m_name.length() == 0 && right.m_name.length() == 0) || (m_name == right.m_name)) &&
          ((m_address.length() == 0 && right.m_address.length() == 0) || (m_address == right.m_address)) &&
          ((m_city.length() == 0 && right.m_city.length() == 0) || (m_city == right.m_city)) &&
          ((m_state.length() == 0 && right.m_state.length() == 0) || (m_state == right.m_state)) &&
          ((m_postcode.length() == 0 && right.m_postcode.length() == 0) || (m_postcode == right.m_postcode)) &&
          ((m_telephone.length() == 0 && right.m_telephone.length() == 0) || (m_telephone == right.m_telephone)) &&
          ((m_email.length() == 0 && right.m_email.length() == 0) || (m_email == right.m_email)) &&
          (m_matchingEnabled == right.m_matchingEnabled) &&
          (m_usingMatchKey == right.m_usingMatchKey) &&
          (m_matchKeyIgnoreCase == right.m_matchKeyIgnoreCase) &&
          ((m_matchKey.length() == 0 && right.m_matchKey.length() == 0) || m_matchKey == right.m_matchKey) &&
          ((m_reference.length() == 0 && right.m_reference.length() == 0) || (m_reference == right.m_reference)) &&
          ((m_defaultAccountId.length() == 0 && right.m_defaultAccountId.length() == 0) || m_defaultAccountId == right.m_defaultAccountId));
}

bool MyMoneyPayee::operator < (const MyMoneyPayee& right) const
{
  return m_name < right.name();
}

int MyMoneyPayee::addPayeeIdentifier(const payeeIdentifier::ptr identifier)
{
  if (identifier.isNull())
    throw MYMONEYEXCEPTION("Added invalid payeeIdentifier to an payee");
  const unsigned int newId = m_payeeIdentifiers.count();
  m_payeeIdentifiers.insert( newId , identifier->cloneSharedPtr() );
  return newId;
}

void MyMoneyPayee::modifyPayeeIdentifier(const unsigned int& index, payeeIdentifier::ptr identifier)
{
  Q_ASSERT( m_payeeIdentifiers.constFind( index ) != m_payeeIdentifiers.constEnd() );
  Q_CHECK_PTR( identifier );

  m_payeeIdentifiers[index] = identifier;
}

void MyMoneyPayee::removePayeeIdentifier(const unsigned int& index)
{
  m_payeeIdentifiers.remove( index );
}

void MyMoneyPayee::writeXML(QDomDocument& document, QDomElement& parent) const
{
  QDomElement el = document.createElement("PAYEE");

  writeBaseXML(document, el);

  el.setAttribute("name", m_name);
  el.setAttribute("reference", m_reference);
  el.setAttribute("email", m_email);
  if (!m_notes.isEmpty())
    el.setAttribute("notes", m_notes);

  el.setAttribute("matchingenabled", m_matchingEnabled);
  if (m_matchingEnabled) {
    el.setAttribute("usingmatchkey", m_usingMatchKey);
    el.setAttribute("matchignorecase", m_matchKeyIgnoreCase);
    el.setAttribute("matchkey", m_matchKey);
  }

  if (!m_defaultAccountId.isEmpty()) {
    el.setAttribute("defaultaccountid", m_defaultAccountId);
  }

  // Save address
  QDomElement address = document.createElement("ADDRESS");
  address.setAttribute("street", m_address);
  address.setAttribute("city", m_city);
  address.setAttribute("postcode", m_postcode);
  address.setAttribute("state", m_state);
  address.setAttribute("telephone", m_telephone);

  el.appendChild(address);

  // Add payee identifiers
  foreach( payeeIdentifier::ptr ident, m_payeeIdentifiers ) {
    if ( !ident.isNull() ) {
      QDomElement identElement = document.createElement("payeeIdentifier");
      identElement.setAttribute("type", ident->payeeIdentifierId());
      ident->writeXML(document, identElement);
      el.appendChild(identElement);
    }
  }

  parent.appendChild(el);
}

bool MyMoneyPayee::hasReferenceTo(const QString& id) const
{
  return id == m_defaultAccountId;

}

MyMoneyPayee::payeeMatchType MyMoneyPayee::matchData(bool& ignorecase, QStringList& keys) const
{
  payeeMatchType type = matchDisabled;
  keys.clear();
  ignorecase = m_matchKeyIgnoreCase;

  if (m_matchingEnabled) {
    type = m_usingMatchKey ? matchKey : matchName;
    if (type == matchKey)
      keys = m_matchKey.split(';');
  }

  return type;
}

MyMoneyPayee::payeeMatchType MyMoneyPayee::matchData(bool& ignorecase, QString& keyString) const
{
  QStringList keys;
  payeeMatchType type = matchData(ignorecase, keys);
  keyString = keys.join(";");
  return type;
}

void MyMoneyPayee::setMatchData(payeeMatchType type, bool ignorecase, const QStringList& keys)
{
  m_matchingEnabled = (type != matchDisabled);
  m_matchKeyIgnoreCase = ignorecase;
  m_matchKey.clear();

  if (m_matchingEnabled) {
    m_usingMatchKey = (type == matchKey);
    if (m_usingMatchKey) {
      QRegExp validKeyRegExp("[^ ]");
      QStringList filteredKeys = keys.filter(validKeyRegExp);
      m_matchKey = filteredKeys.join(";");
    }
  }
}

void MyMoneyPayee::setMatchData(payeeMatchType type, bool ignorecase, const QString& keys)
{
  setMatchData(type, ignorecase, keys.split(';'));
}

payeeIdentifier::constList MyMoneyPayee::payeeIdentifiers() const
{
  payeeIdentifier::constList list;
  list.reserve(m_payeeIdentifiers.count());

  const payeeIdentifier::list::const_iterator end = m_payeeIdentifiers.constEnd();
  for (payeeIdentifier::list::const_iterator iter = m_payeeIdentifiers.constBegin(); iter != end; ++iter) {
    list.insert(iter.key(), iter.value().constCast<const payeeIdentifier>());
  }
  return list;
}

// vim:cin:si:ai:et:ts=2:sw=2:
