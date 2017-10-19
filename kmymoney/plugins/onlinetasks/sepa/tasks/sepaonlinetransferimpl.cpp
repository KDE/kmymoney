/*
  This file is part of KMyMoney, A Personal Finance Manager by KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sepaonlinetransferimpl.h"

#include <QSqlError>
#include <QSqlQuery>

#include "mymoneyutils.h"
#include "mymoney/mymoneyfile.h"
#include "mymoney/onlinejobadministration.h"
#include "misc/validators.h"
#include "payeeidentifiertyped.h"
#include "tasks/sepaonlinetransfer.h"

static const unsigned short defaultTextKey = 51;
static const unsigned short defaultSubTextKey = 0;

/**
 * @brief Fallback if plugin fails to create settings correctly
 */
class sepaOnlineTransferSettingsFallback : public sepaOnlineTransfer::settings
{
public:
  // Limits getter
  virtual int purposeMaxLines() const {
    return 1;
  }
  virtual int purposeLineLength() const {
    return 27;
  }
  virtual int purposeMinLength() const {
    return 0;
  }

  virtual int recipientNameLineLength() const {
    return 1;
  }
  virtual int recipientNameMinLength() const {
    return 0;
  }

  virtual int payeeNameLineLength() const {
    return 0;
  }
  virtual int payeeNameMinLength() const {
    return 0;
  }

  virtual QString allowedChars() const {
    return QString();
  }

  // Checker
  virtual bool checkPurposeCharset(const QString&) const {
    return false;
  }
  virtual bool checkPurposeLineLength(const QString&) const {
    return false;
  }
  virtual validators::lengthStatus checkPurposeLength(const QString&) const {
    return validators::tooLong;
  }
  virtual bool checkPurposeMaxLines(const QString&) const {
    return false;
  }

  virtual validators::lengthStatus checkNameLength(const QString&) const {
    return validators::tooLong;
  }
  virtual bool checkNameCharset(const QString&) const {
    return false;
  }

  virtual validators::lengthStatus checkRecipientLength(const QString&) const {
    return validators::tooLong;
  }
  virtual bool checkRecipientCharset(const QString&) const {
    return false;
  }

  virtual int endToEndReferenceLength() const {
    return 0;
  }
  virtual validators::lengthStatus checkEndToEndReferenceLength(const QString&) const {
    return validators::tooLong;
  }
  virtual bool isIbanValid(const QString&) const {
    return false;
  }

  virtual bool checkRecipientBic(const QString&) const {
    return false;
  }

  virtual bool isBicMandatory(const QString&, const QString&) const {
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
  } catch (payeeIdentifier::exception&) {
  }

  QSharedPointer<const sepaOnlineTransfer::settings> settings = getSettings();
  if (settings->checkPurposeLength(_purpose) == validators::ok
      && settings->checkPurposeMaxLines(_purpose)
      && settings->checkPurposeLineLength(_purpose)
      && settings->checkPurposeCharset(_purpose)
      && settings->checkEndToEndReferenceLength(_endToEndReference) == validators::ok
      //&& settings->checkRecipientCharset( _beneficiaryAccount.ownerName() )
      //&& settings->checkRecipientLength( _beneficiaryAccount.ownerName()) == validators::ok
      && _beneficiaryAccount.isIbanValid() // do not check the BIC, maybe it is not needed
      && (!settings->isBicMandatory(iban, _beneficiaryAccount.electronicIban()) || (settings->checkRecipientBic(_beneficiaryAccount.bic()) && _beneficiaryAccount.isValid() /** @todo double check of BIC here, fix that */))
      && value().isPositive()
     )
    return true;
  return false;
}

payeeIdentifier sepaOnlineTransferImpl::originAccountIdentifier() const
{
  QList< payeeIdentifierTyped<payeeIdentifiers::ibanBic> > idents = MyMoneyFile::instance()->account(_originAccount).payeeIdentifiersByType<payeeIdentifiers::ibanBic>();
  if (!idents.isEmpty()) {
    payeeIdentifierTyped<payeeIdentifiers::ibanBic> ident = idents[0];
    ident->setOwnerName(MyMoneyFile::instance()->user().name());
    return ident;
  }
  return payeeIdentifier(new payeeIdentifiers::ibanBic);
}

/** @todo Return EUR */
MyMoneySecurity sepaOnlineTransferImpl::currency() const
{
#if 0
  return MyMoneyFile::instance()->security(originMyMoneyAccount().currencyId());
#endif
  return MyMoneyFile::instance()->baseCurrency();
}

/**
 * @internal To ensure that we never return a null_ptr, @a sepaOnlineTransferSettingsFallback is used if the online plugin fails
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

void sepaOnlineTransferImpl::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent.setAttribute("originAccount", _originAccount);
  parent.setAttribute("value", _value.toString());
  parent.setAttribute("textKey", _textKey);
  parent.setAttribute("subTextKey", _subTextKey);

  if (!_purpose.isEmpty()) {
    parent.setAttribute("purpose", _purpose);
  }

  if (!_endToEndReference.isEmpty()) {
    parent.setAttribute("endToEndReference", _endToEndReference);
  }

  QDomElement beneficiaryEl = document.createElement("beneficiary");
  _beneficiaryAccount.writeXML(document, beneficiaryEl);
  parent.appendChild(beneficiaryEl);
}

sepaOnlineTransfer* sepaOnlineTransferImpl::createFromXml(const QDomElement& element) const
{
  sepaOnlineTransferImpl* task = new sepaOnlineTransferImpl();
  task->setOriginAccount(element.attribute("originAccount", QString()));
  task->setValue(MyMoneyMoney(QStringEmpty(element.attribute("value", QString()))));
  task->_textKey = element.attribute("textKey", QString().setNum(defaultTextKey)).toUShort();
  task->_subTextKey = element.attribute("subTextKey", QString().setNum(defaultSubTextKey)).toUShort();
  task->setPurpose(element.attribute("purpose", QString()));
  task->setEndToEndReference(element.attribute("endToEndReference", QString()));

  payeeIdentifiers::ibanBic beneficiary;
  payeeIdentifiers::ibanBic* beneficiaryPtr = 0;
  QDomElement beneficiaryEl = element.firstChildElement("beneficiary");
  if (!beneficiaryEl.isNull()) {
    beneficiaryPtr = beneficiary.createFromXml(beneficiaryEl);
  }

  if (beneficiaryPtr == 0) {
    task->_beneficiaryAccount = beneficiary;
  } else {
    task->_beneficiaryAccount = *beneficiaryPtr;
  }

  delete beneficiaryPtr;
  return task;
}

onlineTask* sepaOnlineTransferImpl::createFromSqlDatabase(QSqlDatabase connection, const QString& onlineJobId) const
{
  Q_ASSERT(!onlineJobId.isEmpty());
  Q_ASSERT(connection.isOpen());

  QSqlQuery query = QSqlQuery(
                      "SELECT originAccount, value, purpose, endToEndReference, beneficiaryName, beneficiaryIban, "
                      " beneficiaryBic, textKey, subTextKey FROM kmmSepaOrders WHERE id = ?",
                      connection
                    );
  query.bindValue(0, onlineJobId);
  if (query.exec() && query.next()) {
    sepaOnlineTransferImpl* task = new sepaOnlineTransferImpl();
    task->setOriginAccount(query.value(0).toString());
    task->setValue(MyMoneyMoney(query.value(1).toString()));
    task->setPurpose(query.value(2).toString());
    task->setEndToEndReference(query.value(3).toString());
    task->_textKey = query.value(7).toUInt();
    task->_subTextKey = query.value(8).toUInt();

    payeeIdentifiers::ibanBic beneficiary;
    beneficiary.setOwnerName(query.value(4).toString());
    beneficiary.setIban(query.value(5).toString());
    beneficiary.setBic(query.value(6).toString());
    task->_beneficiaryAccount = beneficiary;
    return task;
  }

  return 0;
}

void sepaOnlineTransferImpl::bindValuesToQuery(QSqlQuery& query, const QString& id) const
{
  query.bindValue(":id", id);
  query.bindValue(":originAccount", _originAccount);
  query.bindValue(":value", _value.toString());
  query.bindValue(":purpose", _purpose);
  query.bindValue(":endToEndReference", (_endToEndReference.isEmpty()) ? QVariant() : QVariant::fromValue(_endToEndReference));
  query.bindValue(":beneficiaryName", _beneficiaryAccount.ownerName());
  query.bindValue(":beneficiaryIban", _beneficiaryAccount.electronicIban());
  query.bindValue(":beneficiaryBic", (_beneficiaryAccount.storedBic().isEmpty()) ? QVariant() : QVariant::fromValue(_beneficiaryAccount.storedBic()));
  query.bindValue(":textKey", _textKey);
  query.bindValue(":subTextKey", _subTextKey);
}

bool sepaOnlineTransferImpl::sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  QSqlQuery query = QSqlQuery(databaseConnection);
  query.prepare("INSERT INTO kmmSepaOrders ("
                " id, originAccount, value, purpose, endToEndReference, beneficiaryName, beneficiaryIban, "
                " beneficiaryBic, textKey, subTextKey) "
                " VALUES( :id, :originAccount, :value, :purpose, :endToEndReference, :beneficiaryName, :beneficiaryIban, "
                "         :beneficiaryBic, :textKey, :subTextKey ) "
               );
  bindValuesToQuery(query, onlineJobId);
  if (!query.exec()) {
    qWarning("Error while saving sepa order '%s': %s", qPrintable(onlineJobId), qPrintable(query.lastError().text()));
    return false;
  }
  return true;
}

bool sepaOnlineTransferImpl::sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  QSqlQuery query = QSqlQuery(databaseConnection);
  query.prepare(
    "UPDATE kmmSepaOrders SET"
    " originAccount = :originAccount,"
    " value = :value,"
    " purpose = :purpose,"
    " endToEndReference = :endToEndReference,"
    " beneficiaryName = :beneficiaryName,"
    " beneficiaryIban = :beneficiaryIban,"
    " beneficiaryBic = :beneficiaryBic,"
    " textKey = :textKey,"
    " subTextKey = :subTextKey "
    " WHERE id = :id");
  bindValuesToQuery(query, onlineJobId);
  if (!query.exec()) {
    qWarning("Could not modify sepaOnlineTransfer '%s': %s", qPrintable(onlineJobId), qPrintable(query.lastError().text()));
    return false;
  }
  return true;
}

bool sepaOnlineTransferImpl::sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  QSqlQuery query = QSqlQuery(databaseConnection);
  query.prepare("DELETE FROM kmmSepaOrders WHERE id = ?");
  query.bindValue(0, onlineJobId);
  return query.exec();
}

bool sepaOnlineTransferImpl::hasReferenceTo(const QString& id) const
{
  return (id == _originAccount);
}

QString sepaOnlineTransferImpl::jobTypeName() const
{
   return QLatin1String("SEPA Credit Transfer");
}
