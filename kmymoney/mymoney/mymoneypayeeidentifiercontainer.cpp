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

#include "mymoneypayeeidentifiercontainer.h"

#include <QDebug>

#include "payeeidentifier/payeeidentifierloader.h"

MyMoneyPayeeIdentifierContainer::MyMoneyPayeeIdentifierContainer()
  : m_payeeIdentifiers( QList<payeeIdentifier>() )
{
}

QList<payeeIdentifier> MyMoneyPayeeIdentifierContainer::payeeIdentifiers() const
{
  return m_payeeIdentifiers;
}

void MyMoneyPayeeIdentifierContainer::addPayeeIdentifier(payeeIdentifier& ident)
{
  ident.m_id = nextId();
  m_payeeIdentifiers.append(ident);
}

void MyMoneyPayeeIdentifierContainer::removePayeeIdentifier(const payeeIdentifier& ident)
{
  m_payeeIdentifiers.removeOne(ident);
}

void MyMoneyPayeeIdentifierContainer::modifyPayeeIdentifier(const payeeIdentifier& ident)
{
  QList<payeeIdentifier>::Iterator end = m_payeeIdentifiers.end();
  for( QList<payeeIdentifier>::Iterator iter = m_payeeIdentifiers.begin(); iter != end; ++iter) {
    if ( iter->m_id == ident.m_id ) {
      *iter = ident;
      return;
    }
  }
}

/**
 * @brief Return a free id
 *
 * It is not unique in the file, just in this container.
 */
unsigned int MyMoneyPayeeIdentifierContainer::nextId()
{
  unsigned int id = 1;
  foreach( const payeeIdentifier ident, m_payeeIdentifiers ) {
    if ( ident.m_id >= id )
      id = ident.m_id+1;
  }
  return id;
}

void MyMoneyPayeeIdentifierContainer::loadXML(QDomElement node)
{
  // Load identifiers
  QDomNodeList identifierNodes = node.elementsByTagName("payeeIdentifier");
  const uint identifierNodesLength = identifierNodes.length();
  for (uint i = 0; i < identifierNodesLength; ++i) {
    const QDomElement element = identifierNodes.item(i).toElement();
    payeeIdentifier ident = payeeIdentifierLoader::instance()->createPayeeIdentifierFromXML( element );
    if ( ident.isNull() ) {
      qWarning() << "Could not load payee identifier" << element.attribute("type", "*no pidid set*");
      continue;
    }
    addPayeeIdentifier( ident );
  }
}

void MyMoneyPayeeIdentifierContainer::writeXML(QDomDocument document, QDomElement parent) const
{
  // Add payee identifiers
  foreach( payeeIdentifier ident, m_payeeIdentifiers ) {
    if ( !ident.isNull() ) {
      ident.writeXML(document, parent);
    }
  }
}
