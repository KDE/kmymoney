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

QString payeeIdentifierUnavailable::storagePluginIid() const
{
  return QString();
}

bool payeeIdentifierUnavailable::sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  Q_UNUSED(databaseConnection)
  Q_UNUSED(onlineJobId)
  return false;
}

bool payeeIdentifierUnavailable::sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  Q_UNUSED(databaseConnection)
  Q_UNUSED(onlineJobId)
  return false;
}

bool payeeIdentifierUnavailable::sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  Q_UNUSED(databaseConnection)
  Q_UNUSED(onlineJobId)
  return false;
}

payeeIdentifierData* payeeIdentifierUnavailable::createFromSqlDatabase(QSqlDatabase db, const QString& identId) const
{
  Q_UNUSED(db);
  Q_UNUSED(identId);
  return 0;
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
