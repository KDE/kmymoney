/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "unavailableplugin.h"

#include <typeinfo>

namespace payeeIdentifiers
{

payeeIdentifierUnavailable::payeeIdentifierUnavailable()
    : payeeIdentifierData(),
    m_data(QDomElement())
{
}

payeeIdentifierUnavailable::payeeIdentifierUnavailable(QDomElement data)
    : payeeIdentifierData(),
    m_data(data)
{

}

payeeIdentifierUnavailable* payeeIdentifierUnavailable::clone() const
{
  return new payeeIdentifierUnavailable(m_data);
}

void payeeIdentifierUnavailable::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent = m_data;
}

payeeIdentifierUnavailable* payeeIdentifierUnavailable::createFromXml(const QDomElement& element) const
{
  return new payeeIdentifierUnavailable(element);
}

bool payeeIdentifierUnavailable::isValid() const
{
  return false;
}

bool payeeIdentifierUnavailable::operator==(const payeeIdentifierData& other) const
{
  if (payeeIdentifierId() == other.payeeIdentifierId()) {
    try {
      const payeeIdentifierUnavailable& otherCasted = dynamic_cast<const payeeIdentifierUnavailable&>(other);
      return operator==(otherCasted);
    } catch (const std::bad_cast&) {
    }
  }
  return false;
}

bool payeeIdentifierUnavailable::operator==(const payeeIdentifierUnavailable& other) const
{
  return (m_data == other.m_data);
}


} // namespace payeeIdentifiers
