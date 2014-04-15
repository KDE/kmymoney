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

#include "nationalaccountid.h"

#include <typeinfo>

nationalAccountId* nationalAccountId::createFromXml(const QDomElement& element) const
{
  nationalAccountId* ident = new nationalAccountId;
  
  ident->setBankCode( element.attribute("bankcode", QString()) );
  ident->setAccountNumber( element.attribute("accountnumber", QString()) );
  return ident;
}

void nationalAccountId::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED( document );
  parent.setAttribute("accountNumber", m_accountNumber);
  parent.setAttribute("bankCode", m_bankCode );  
}

bool nationalAccountId::isValid() const
{
  return true;
}

bool nationalAccountId::operator==(const payeeIdentifier& other) const
{
  try {
    const nationalAccountId otherCasted = dynamic_cast<const nationalAccountId&>(other);
    return operator==(otherCasted);
  } catch ( const std::bad_cast& ) {
  }
  return false;
}

bool nationalAccountId::operator==(const nationalAccountId& other) const
{
  return ( m_accountNumber == other.m_accountNumber && m_bankCode == other.m_bankCode );
}
