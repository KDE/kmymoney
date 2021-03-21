/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NATIONALACCOUNTID_H
#define NATIONALACCOUNTID_H

#include "kmm_mymoney_export.h"

#include "mymoney/payeeidentifier/payeeidentifierdata.h"

namespace payeeIdentifiers
{

class KMM_MYMONEY_EXPORT nationalAccount : public payeeIdentifierData
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
