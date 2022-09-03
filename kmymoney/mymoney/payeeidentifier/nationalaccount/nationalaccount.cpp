/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QVariant>

#include "payeeidentifier/nationalaccount/nationalaccount.h"
#include "xmlhelper/xmlstoragehelper.h"

#include <typeinfo>

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

nationalAccount* nationalAccount::createFromXml(QXmlStreamReader* reader) const
{
    nationalAccount* ident = new nationalAccount;

    ident->setBankCode(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("bankcode")));
    ident->setAccountNumber(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("accountnumber")));
    ident->setOwnerName(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("ownername")));
    ident->setCountry(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("country")));
    return ident;
}

void nationalAccount::writeXML(QXmlStreamWriter* writer) const
{
    writer->writeAttribute("accountnumber", m_accountNumber);
    if (!m_bankCode.isEmpty()) {
        writer->writeAttribute("bankcode", m_bankCode);
    }
    writer->writeAttribute("ownername", m_ownerName);
    writer->writeAttribute("country", m_country);
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
