/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#include "nationalaccount.h"

#include <typeinfo>

#include <QVariant>

namespace payeeIdentifiers
{

nationalAccount::nationalAccount()
    : m_ownerName(),
    m_country(),
    m_bankCode(),
    m_accountNumber()
{
}

nationalAccount::nationalAccount(const nationalAccount& other)
    : m_ownerName(other.m_ownerName),
    m_country(other.m_country),
    m_bankCode(other.m_bankCode),
    m_accountNumber(other.m_accountNumber)
{

}

nationalAccount* nationalAccount::createFromXml(const QDomElement& element) const
{
  nationalAccount* ident = new nationalAccount;

  ident->setBankCode(element.attribute("bankcode", QString()));
  ident->setAccountNumber(element.attribute("accountnumber", QString()));
  ident->setOwnerName(element.attribute("ownername", QString()));
  ident->setCountry(element.attribute("country", QString()));
  return ident;
}

void nationalAccount::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent.setAttribute("accountnumber", m_accountNumber);
  if (!m_bankCode.isEmpty())
    parent.setAttribute("bankcode", m_bankCode);
  parent.setAttribute("ownername", m_ownerName);
  parent.setAttribute("country", m_country);
}

/** @todo implement */
bool nationalAccount::isValid() const
{
  return true;
}

bool nationalAccount::operator==(const payeeIdentifierData& other) const
{
  try {
    const nationalAccount& otherCasted = dynamic_cast<const nationalAccount&>(other);
    return operator==(otherCasted);
  } catch (const std::bad_cast&) {
  }
  return false;
}

bool nationalAccount::operator==(const nationalAccount& other) const
{
  return (m_accountNumber == other.m_accountNumber && m_bankCode == other.m_bankCode && m_ownerName == other.m_ownerName && m_country == other.m_country);
}

} // namespace payeeIdentifiers
