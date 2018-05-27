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

#ifndef NATIONALACCOUNTID_H
#define NATIONALACCOUNTID_H

#include "mymoney/payeeidentifier/payeeidentifierdata.h"
#include "nationalaccount_identifier_export.h"

namespace payeeIdentifiers
{

class NATIONALACCOUNT_IDENTIFIER_EXPORT nationalAccount : public payeeIdentifierData
{
public:
  PAYEEIDENTIFIER_IID(nationalAccount, "org.kmymoney.payeeIdentifier.national");

  nationalAccount();
  nationalAccount(const nationalAccount& other);

  bool isValid() const final override;
  bool operator==(const payeeIdentifierData& other) const final override;
  bool operator==(const nationalAccount& other) const;

  nationalAccount* clone() const final override {
    return new nationalAccount(*this);
  }
  nationalAccount* createFromXml(const QDomElement& element) const final override;
  void writeXML(QDomDocument& document, QDomElement& parent) const final override;

  void setBankCode(const QString& bankCode) {
    m_bankCode = bankCode;
  }
  QString bankCode() const {
    return m_bankCode;
  }

  /** @todo implement */
  QString bankName() const {
    return QString();
  }

  void setAccountNumber(const QString& accountNumber) {
    m_accountNumber = accountNumber;
  }
  QString accountNumber() const {
    return m_accountNumber;
  }

  QString country() const {
    return m_country;
  }
  void setCountry(const QString& countryCode) {
    m_country = countryCode.toUpper();
  }

  QString ownerName() const {
    return m_ownerName;
  }
  void setOwnerName(const QString& ownerName) {
    m_ownerName = ownerName;
  }

private:
  QString m_ownerName;
  QString m_country;
  QString m_bankCode;
  QString m_accountNumber;
};

} // namespace payeeIdentifiers

#endif // NATIONALACCOUNTID_H
