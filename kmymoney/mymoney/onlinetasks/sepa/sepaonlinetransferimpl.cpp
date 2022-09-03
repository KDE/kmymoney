/*
    SPDX-FileCopyrightText: 2013-2018 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sepaonlinetransferimpl.h"

#include <QSet>
#include <QVariant>
#include <QXmlStreamReader>

#include "misc/validators.h"
#include "mymoney/mymoneyfile.h"
#include "mymoney/onlinejobadministration.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneypayee.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "payeeidentifier/ibanbic/ibanbic.h"
#include "payeeidentifiertyped.h"
#include "sepaonlinetransfer.h"
#include "xmlhelper/xmlstoragehelper.h"

static const unsigned short defaultTextKey = 51;
static const unsigned short defaultSubTextKey = 0;

/**
 * @brief Fallback if plugin fails to create settings correctly
 */
class sepaOnlineTransferSettingsFallback : public sepaOnlineTransfer::settings
{
public:
    // Limits getter
    int purposeMaxLines() const final override {
        return 1;
    }
    int purposeLineLength() const final override {
        return 27;
    }
    int purposeMinLength() const final override {
        return 0;
    }

    int recipientNameLineLength() const final override {
        return 1;
    }
    int recipientNameMinLength() const final override {
        return 0;
    }

    int payeeNameLineLength() const final override {
        return 0;
    }
    int payeeNameMinLength() const final override {
        return 0;
    }

    QString allowedChars() const final override {
        return QString();
    }

    // Checker
    bool checkPurposeCharset(const QString&) const final override {
        return false;
    }
    bool checkPurposeLineLength(const QString&) const final override {
        return false;
    }
    validators::lengthStatus checkPurposeLength(const QString&) const final override {
        return validators::tooLong;
    }
    bool checkPurposeMaxLines(const QString&) const final override {
        return false;
    }

    validators::lengthStatus checkNameLength(const QString&) const final override {
        return validators::tooLong;
    }
    bool checkNameCharset(const QString&) const final override {
        return false;
    }

    validators::lengthStatus checkRecipientLength(const QString&) const final override {
        return validators::tooLong;
    }
    bool checkRecipientCharset(const QString&) const final override {
        return false;
    }

    int endToEndReferenceLength() const final override {
        return 0;
    }
    validators::lengthStatus checkEndToEndReferenceLength(const QString&) const final override {
        return validators::tooLong;
    }
    virtual bool isIbanValid(const QString&) const {
        return false;
    }

    bool checkRecipientBic(const QString&) const final override {
        return false;
    }

    bool isBicMandatory(const QString&, const QString&) const final override {
        return true;
    }
};

sepaOnlineTransferImpl::sepaOnlineTransferImpl()
    : sepaOnlineTransfer(),
      _settings(QSharedPointer<const settings>()),
      _originAccount(QString()),
      _value(0),
      _purpose(QString("")),
      _endToEndReference(QString("")),
      _beneficiaryAccount(payeeIdentifiers::ibanBic()),
      _textKey(defaultTextKey),
      _subTextKey(defaultSubTextKey)
{

}

sepaOnlineTransferImpl::sepaOnlineTransferImpl(const sepaOnlineTransferImpl& other)
    : sepaOnlineTransfer(other),
      _settings(other._settings),
      _originAccount(other._originAccount),
      _value(other._value),
      _purpose(other._purpose),
      _endToEndReference(other._endToEndReference),
      _beneficiaryAccount(other._beneficiaryAccount),
      _textKey(other._textKey),
      _subTextKey(other._subTextKey)
{

}

sepaOnlineTransfer *sepaOnlineTransferImpl::clone() const
{
    sepaOnlineTransfer *transfer = new sepaOnlineTransferImpl(*this);
    return transfer;
}

//! @todo add validation of local name
bool sepaOnlineTransferImpl::isValid() const
{
    QString iban;
    try {
        payeeIdentifier ident = originAccountIdentifier();
        iban = ident.data<payeeIdentifiers::ibanBic>()->electronicIban();
    } catch (const payeeIdentifier::empty &) {
    } catch (const payeeIdentifier::badCast &) {
    }

    QSharedPointer<const sepaOnlineTransfer::settings> localSettings = getSettings();
    if (localSettings->checkPurposeLength(_purpose) == validators::ok
            && localSettings->checkPurposeMaxLines(_purpose)
            && localSettings->checkPurposeLineLength(_purpose)
            && localSettings->checkPurposeCharset(_purpose)
            && localSettings->checkEndToEndReferenceLength(_endToEndReference) == validators::ok
            //&& settings->checkRecipientCharset( _beneficiaryAccount.ownerName() )
            //&& settings->checkRecipientLength( _beneficiaryAccount.ownerName()) == validators::ok
            && _beneficiaryAccount.isIbanValid() // do not check the BIC, maybe it is not needed
            && (!localSettings->isBicMandatory(iban, _beneficiaryAccount.electronicIban()) || (localSettings->checkRecipientBic(_beneficiaryAccount.bic()) && _beneficiaryAccount.isValid() /** @todo double check of BIC here, fix that */))
            && value().isPositive()
       )
        return true;
    return false;
}

payeeIdentifier sepaOnlineTransferImpl::originAccountIdentifier() const
{
    if (!_originAccount.isEmpty()) {
        payeeIdentifierTyped<payeeIdentifiers::ibanBic> ident = payeeIdentifierTyped<payeeIdentifiers::ibanBic>(new payeeIdentifiers::ibanBic);
        auto acc = MyMoneyFile::instance()->account(_originAccount);
        ident->setIban(acc.value(QStringLiteral("iban")));

        if (!acc.institutionId().isEmpty()) {
            const auto institution = MyMoneyFile::instance()->institution(acc.institutionId());
            ident->setBic(institution.value(QStringLiteral("bic")));
        }

        ident->setOwnerName(MyMoneyFile::instance()->user().name());
        return ident;
    }
    return payeeIdentifier(new payeeIdentifiers::ibanBic);
}

MyMoneySecurity sepaOnlineTransferImpl::currency() const
{
    if (!_originAccount.isEmpty()) {
        const QString currencyId = MyMoneyFile::instance()->account(_originAccount).currencyId();
        return MyMoneyFile::instance()->security(currencyId);
    }
    return MyMoneyFile::instance()->baseCurrency();
}

/**
 * @internal To ensure that we never return a nullptr, @a sepaOnlineTransferSettingsFallback is used if the online plugin fails
 * to give us an correct value
 */
QSharedPointer<const sepaOnlineTransfer::settings> sepaOnlineTransferImpl::getSettings() const
{
    if (_settings.isNull()) {
        _settings = onlineJobAdministration::instance()->taskSettings<sepaOnlineTransferImpl::settings>(name(), _originAccount);

        if (_settings.isNull())
            _settings = QSharedPointer< const sepaOnlineTransfer::settings >(new sepaOnlineTransferSettingsFallback);
    }
    Q_CHECK_PTR(_settings);
    return _settings;
}

void sepaOnlineTransferImpl::setOriginAccount(const QString &accountId)
{
    if (_originAccount != accountId) {
        _originAccount = accountId;
        _settings = QSharedPointer<const sepaOnlineTransferImpl::settings>();
    }
}

void sepaOnlineTransferImpl::writeXML(QXmlStreamWriter* writer) const
{
    writer->writeAttribute("originAccount", _originAccount);
    writer->writeAttribute("value", _value.toString());
    writer->writeAttribute("textKey", QString::number(_textKey));
    writer->writeAttribute("subTextKey", QString::number(_subTextKey));

    if (!_purpose.isEmpty()) {
        writer->writeAttribute("purpose", _purpose);
    }

    if (!_endToEndReference.isEmpty()) {
        writer->writeAttribute("endToEndReference", _endToEndReference);
    }

    writer->writeStartElement("beneficiary");
    _beneficiaryAccount.writeXML(writer);
    writer->writeEndElement();
}

sepaOnlineTransfer* sepaOnlineTransferImpl::createFromXml(QXmlStreamReader* reader) const
{
    sepaOnlineTransferImpl* task = new sepaOnlineTransferImpl();
    task->setOriginAccount(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("originAccount")));
    task->setValue(MyMoneyXmlHelper::readValueAttribute(reader, QLatin1String("value")));
    task->_textKey = MyMoneyXmlHelper::readUintAttribute(reader, QLatin1String("textKey"), defaultTextKey);
    task->_subTextKey = MyMoneyXmlHelper::readUintAttribute(reader, QLatin1String("subTextKey"), defaultSubTextKey);
    task->setPurpose(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("purpose")));
    task->setEndToEndReference(MyMoneyXmlHelper::readStringAttribute(reader, QLatin1String("endToEndReference")));

    payeeIdentifiers::ibanBic beneficiary;
    payeeIdentifiers::ibanBic* beneficiaryPtr = nullptr;

    while (reader->readNextStartElement()) {
        if (reader->name() == QLatin1String("beneficiary")) {
            delete beneficiaryPtr;
            beneficiaryPtr = beneficiary.createFromXml(reader);
        } else {
            reader->skipCurrentElement();
        }
    }

    if (beneficiaryPtr == nullptr) {
        task->_beneficiaryAccount = beneficiary;
    } else {
        task->_beneficiaryAccount = *beneficiaryPtr;
        delete beneficiaryPtr;
    }

    return task;
}

bool sepaOnlineTransferImpl::hasReferenceTo(const QString& id) const
{
    return (id == _originAccount);
}

QSet<QString> sepaOnlineTransferImpl::referencedObjects() const
{
    return { _originAccount };
}

QString sepaOnlineTransferImpl::jobTypeName() const
{
    return QLatin1String("SEPA Credit Transfer");
}
