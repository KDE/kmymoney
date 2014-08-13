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

#include "payeeidentifier.h"

payeeIdentifier::payeeIdentifier()
  : m_payeeIdentifier(0)
{
}

payeeIdentifier::payeeIdentifier(const payeeIdentifier& other)
  : m_payeeIdentifier(0)
{
  if ( other.m_payeeIdentifier != 0 )
    m_payeeIdentifier = other.m_payeeIdentifier->clone();
}

payeeIdentifier::payeeIdentifier(payeeIdentifierData*const identifier)
  : m_payeeIdentifier( identifier )
{

}

payeeIdentifier::~payeeIdentifier()
{
  delete m_payeeIdentifier;
}

payeeIdentifierData* payeeIdentifier::operator->()
{
  if ( m_payeeIdentifier == 0 )
    throw nullPayeeIdentifier(__FILE__, __LINE__);
  return m_payeeIdentifier;
}

const payeeIdentifierData* payeeIdentifier::operator->() const
{
  if ( m_payeeIdentifier == 0 )
    throw nullPayeeIdentifier(__FILE__, __LINE__);
  return m_payeeIdentifier;
}

payeeIdentifierData* payeeIdentifier::data()
{
  return operator->();
}

const payeeIdentifierData* payeeIdentifier::data() const
{
  return operator->();
}

bool payeeIdentifier::isValid() const
{
  if( m_payeeIdentifier != 0 )
    return m_payeeIdentifier->isValid();
  return false;
}

QString payeeIdentifier::iid() const
{
  if ( m_payeeIdentifier != 0 )
    return m_payeeIdentifier->payeeIdentifierId();
  return QString();
}

payeeIdentifier& payeeIdentifier::operator=(const payeeIdentifier& other)
{
  if ( this == &other )
    return *this;

  m_payeeIdentifier = other.m_payeeIdentifier->clone();

  return *this;
}

bool payeeIdentifier::operator==(const payeeIdentifier& other)
{
  if ( isNull() || other.isNull() ) {
    if ( !isNull() ||  !other.isNull() )
      return false;
    return true;
  }
  return (*data() == *(other.data()));
}

void payeeIdentifier::writeXML(QDomDocument& document, QDomElement& parent, const QString& elemenName) const
{
  // Important: type must be set before calling m_payeeIdentifier->writeXML()
  // the plugin for unavailable plugins must be able to set type itself
  QDomElement elem = document.createElement(elemenName);
  if ( !isNull() ) {
    elem.setAttribute("type", m_payeeIdentifier->payeeIdentifierId());
    m_payeeIdentifier->writeXML(document, elem);
  }
  parent.appendChild(elem);
}

