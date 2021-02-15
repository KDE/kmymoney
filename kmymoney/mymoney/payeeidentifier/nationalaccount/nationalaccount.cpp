/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "payeeidentifier/nationalaccount/nationalaccount.h"

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
