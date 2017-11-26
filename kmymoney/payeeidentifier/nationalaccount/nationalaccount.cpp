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
#include <QSqlQuery>
#include <QSqlError>

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

nationalAccount* nationalAccount::createFromSqlDatabase(QSqlDatabase db, const QString& identId) const
{
  QSqlQuery query(db);
  query.prepare("SELECT countryCode, accountNumber, bankCode, name FROM kmmNationalAccountNumber WHERE id = ?;");
  query.bindValue(0, identId);
  if (!query.exec() || !query.next()) {
    qWarning("Could load national account number from database");
    return 0;
  }

  nationalAccount *const ident = new nationalAccount;
  ident->setCountry(query.value(0).toString());
  ident->setAccountNumber(query.value(1).toString());
  ident->setBankCode(query.value(2).toString());
  ident->setOwnerName(query.value(3).toString());
  return ident;
}

QString nationalAccount::storagePluginIid() const
{
  return QLatin1String("org.kmymoney.payeeIdentifier.nationalAccount.sqlStoragePlugin");
}

bool nationalAccount::sqlSave(QSqlDatabase databaseConnection, const QString& objectId) const
{
  QSqlQuery query(databaseConnection);
  query.prepare("INSERT INTO kmmNationalAccountNumber "
                " ( id, countryCode, accountNumber, bankCode, name )"
                " VALUES( :id, :countryCode, :accountNumber, :bankCode, :name ) "
               );
  return writeQuery(query, objectId);
}

bool nationalAccount::sqlModify(QSqlDatabase databaseConnection, const QString& objectId) const
{
  QSqlQuery query(databaseConnection);
  query.prepare("UPDATE kmmNationalAccountNumber SET countryCode = :countryCode, accountNumber = :accountNumber, bankCode = :bankCode, name = :name WHERE id = :id;");
  return writeQuery(query, objectId);
}

bool nationalAccount::sqlRemove(QSqlDatabase databaseConnection, const QString& objectId) const
{
  QSqlQuery query(databaseConnection);
  query.prepare("DELETE FROM kmmNationalAccountNumber WHERE id = ?;");
  query.bindValue(0, objectId);
  if (!query.exec()) {
    qWarning("Error while deleting national account number '%s': %s", qPrintable(objectId), qPrintable(query.lastError().text()));
    return false;
  }
  return true;
}

bool nationalAccount::writeQuery(QSqlQuery& query, const QString& id) const
{
  query.bindValue(":id", id);
  query.bindValue(":countryCode", country());
  query.bindValue(":accountNumber", accountNumber());
  query.bindValue(":bankCode", (bankCode().isEmpty()) ? QVariant(QVariant::String) : bankCode());
  query.bindValue(":name", ownerName());
  if (!query.exec()) { // krazy:exclude=crashy
    qWarning("Error while saving national account number for '%s': %s", qPrintable(id), qPrintable(query.lastError().text()));
    return false;
  }
  return true;
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
