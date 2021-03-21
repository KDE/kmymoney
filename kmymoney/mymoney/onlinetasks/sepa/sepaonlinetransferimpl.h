/*
    SPDX-FileCopyrightText: 2013-2018 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEPAONLINETRANSFERIMPL_H
#define SEPAONLINETRANSFERIMPL_H

#include "kmm_mymoney_export.h"

#include "sepaonlinetransfer.h"
#include "mymoneymoney.h"

/**
 * @brief SEPA Credit Transfer
 */
class KMM_MYMONEY_EXPORT sepaOnlineTransferImpl : public sepaOnlineTransfer
{

public:
    ONLINETASK_META(sepaOnlineTransfer, "org.kmymoney.creditTransfer.sepa");
    sepaOnlineTransferImpl();
    sepaOnlineTransferImpl(const sepaOnlineTransferImpl &other);

    QString responsibleAccount() const final override {
        return _originAccount;
    }
    void setOriginAccount(const QString& accountId) final override;

    MyMoneyMoney value() const final override {
        return _value;
    }
    void setValue(MyMoneyMoney value) final override {
        _value = value;
    }

    void setBeneficiary(const payeeIdentifiers::ibanBic& accountIdentifier) final override {
        _beneficiaryAccount = accountIdentifier;
    }
    payeeIdentifier beneficiary() const final override {
        return payeeIdentifier(_beneficiaryAccount.clone());
    }
    payeeIdentifiers::ibanBic beneficiaryTyped() const final override {
        return _beneficiaryAccount;
    }

    void setPurpose(const QString purpose) final override {
        _purpose = purpose;
    }
    QString purpose() const final override {
        return _purpose;
    }

    void setEndToEndReference(const QString& reference) final override {
        _endToEndReference = reference;
    }
    QString endToEndReference() const final override {
        return _endToEndReference;
    }

    payeeIdentifier originAccountIdentifier() const final override;

    MyMoneySecurity currency() const final override;

    bool isValid() const final override;

    QString jobTypeName() const final override;

    unsigned short int textKey() const final override {
        return _textKey;
    }

    void setTextKey(unsigned short int textKey) final override {
        _textKey = textKey;
    }

    unsigned short int subTextKey() const final override {
        return _subTextKey;
    }

    void setSubTextKey(unsigned short int subTextKey) final override {
        _subTextKey = subTextKey;
    }

    bool hasReferenceTo(const QString& id) const final override;

    /**
     * @copydoc MyMoneyObject::referencedObjects
     */
    QSet<QString> referencedObjects() const override;

    QSharedPointer<const sepaOnlineTransfer::settings> getSettings() const final override;

    void writeXML(QDomDocument& document, QDomElement& parent) const final override;

protected:
    sepaOnlineTransfer* clone() const final override;

    sepaOnlineTransfer* createFromXml(const QDomElement &element) const final override;

private:
    mutable QSharedPointer<const settings> _settings;

    QString _originAccount;
    MyMoneyMoney _value;
    QString _purpose;
    QString _endToEndReference;

    payeeIdentifiers::ibanBic _beneficiaryAccount;

    unsigned short int _textKey;
    unsigned short int _subTextKey;
};

#endif // SEPAONLINETRANSFERIMPL_H
