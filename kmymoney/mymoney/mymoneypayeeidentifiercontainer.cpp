/*
    SPDX-FileCopyrightText: 2014-2016 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mymoneypayeeidentifiercontainer.h"

#include <QDebug>

#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "payeeidentifier/unavailableplugin/unavailableplugin.h"
#include "payeeidentifier.h"

MyMoneyPayeeIdentifierContainer::MyMoneyPayeeIdentifierContainer()
    : m_payeeIdentifiers(QList< ::payeeIdentifier >())
{
}

unsigned int MyMoneyPayeeIdentifierContainer::payeeIdentifierCount() const
{
  return m_payeeIdentifiers.count();
}

payeeIdentifier MyMoneyPayeeIdentifierContainer::payeeIdentifier(unsigned int index) const
{
  return m_payeeIdentifiers.at(index);
}

QList<payeeIdentifier> MyMoneyPayeeIdentifierContainer::payeeIdentifiers() const
{
  return m_payeeIdentifiers;
}

void MyMoneyPayeeIdentifierContainer::addPayeeIdentifier(const ::payeeIdentifier& ident)
{
  m_payeeIdentifiers.append(ident);
}

void MyMoneyPayeeIdentifierContainer::addPayeeIdentifier(const unsigned int position, const ::payeeIdentifier& ident)
{
  m_payeeIdentifiers.insert(position, ident);
}

void MyMoneyPayeeIdentifierContainer::removePayeeIdentifier(const ::payeeIdentifier& ident)
{
  m_payeeIdentifiers.removeOne(ident);
}

void MyMoneyPayeeIdentifierContainer::removePayeeIdentifier(const int index)
{
  Q_ASSERT(m_payeeIdentifiers.count() > index && index >= 0);
  m_payeeIdentifiers.removeAt(index);
}

void MyMoneyPayeeIdentifierContainer::modifyPayeeIdentifier(const ::payeeIdentifier& ident)
{
  QList< ::payeeIdentifier >::Iterator end = m_payeeIdentifiers.end();
  for (QList< ::payeeIdentifier >::Iterator iter = m_payeeIdentifiers.begin(); iter != end; ++iter) {
    if (iter->id() == ident.id()) {
      *iter = ident;
      return;
    }
  }
}

void MyMoneyPayeeIdentifierContainer::modifyPayeeIdentifier(const int index, const ::payeeIdentifier& ident)
{
  Q_ASSERT(m_payeeIdentifiers.count() > index && index >= 0);
  m_payeeIdentifiers[index] = ident;
}

void MyMoneyPayeeIdentifierContainer::resetPayeeIdentifiers(const QList< ::payeeIdentifier >& list)
{
  m_payeeIdentifiers = list;
}

void MyMoneyPayeeIdentifierContainer::loadXML(QDomElement node)
{
  // Load identifiers
  QDomNodeList identifierNodes = node.elementsByTagName("payeeIdentifier");
  const uint identifierNodesLength = identifierNodes.length();
  for (uint i = 0; i < identifierNodesLength; ++i) {
    const QDomElement element = identifierNodes.item(i).toElement();

    const auto payeeIdentifierId = element.attribute("type");
    payeeIdentifierData* identData;
    if (payeeIdentifierId == payeeIdentifiers::ibanBic::staticPayeeIdentifierIid()) {
      auto creator = new payeeIdentifiers::ibanBic();
      identData = creator->createFromXml(element);
      delete creator;
    } else if (payeeIdentifierId == payeeIdentifiers::nationalAccount::staticPayeeIdentifierIid()) {
      auto creator = new payeeIdentifiers::nationalAccount();
      identData = creator->createFromXml(element);
      delete creator;
    } else {
      identData = new payeeIdentifiers::payeeIdentifierUnavailable(element);
    }
    ::payeeIdentifier ident(identData);
    ident.m_id = element.attribute("id", 0).toUInt();

    if (ident.isNull()) {
      qWarning() << "Could not load payee identifier" << element.attribute("type", "*no pidid set*");
      continue;
    }
    addPayeeIdentifier(ident);
  }
}

void MyMoneyPayeeIdentifierContainer::writeXML(QDomDocument document, QDomElement parent) const
{
  // Add payee identifiers
  foreach (const ::payeeIdentifier& ident, m_payeeIdentifiers) {
    if (!ident.isNull()) {
      ident.writeXML(document, parent);
    }
  }
}
