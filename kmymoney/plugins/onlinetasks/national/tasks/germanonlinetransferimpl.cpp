/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
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


#include "germanonlinetransferimpl.h"

#include <QSqlQuery>
#include <QSqlError>

#include "mymoney/mymoneyfile.h"
#include "mymoney/onlinejobadministration.h"
#include "misc/validators.h"

class germanOnlineTransferSettingsFallback : public germanOnlineTransfer::settings
{
public:
  // Limits getter
  virtual int purposeMaxLines() const { return 0; }
  virtual int purposeLineLength() const { return 0; }
  virtual int purposeMinLength() const { return 0; }

  virtual int recipientNameLineLength() const { return 0; }
  virtual int recipientNameMinLength() const { return 0; }

  virtual int payeeNameLineLength() const { return 0; }
  virtual int payeeNameMinLength() const { return 0; }

  virtual QString allowedChars() const { return QString(); }

  // Limits validators
  virtual bool checkPurposeCharset( const QString& ) const { return false; }
  virtual bool checkPurposeLineLength(const QString& ) const { return false; }
  virtual validators::lengthStatus checkPurposeLength(const QString&) const { return validators::tooLong; }
  virtual bool checkPurposeMaxLines(const QString&) const { return false; }

  virtual validators::lengthStatus checkNameLength(const QString&) const { return validators::tooLong; }
  virtual bool checkNameCharset( const QString& ) const { return false; }

  virtual validators::lengthStatus checkRecipientLength(const QString&) const { return validators::tooLong; }
  virtual bool checkRecipientCharset( const QString& ) const { return false; }

  virtual validators::lengthStatus checkRecipientAccountNumber( const QString& ) const { return validators::tooLong; }
  virtual validators::lengthStatus checkRecipientBankCode( const QString& ) const { return validators::tooLong; }
};

static const unsigned short defaultTextKey = 51;
static const unsigned short defaultSubTextKey = 0;

germanOnlineTransferImpl::germanOnlineTransferImpl()
  : germanOnlineTransfer(),
    _settings( QSharedPointer<const settings>() ),
    _value(0),
    _purpose(QString()),
    _originAccount( QString() ),
    _beneficiaryAccount( payeeIdentifiers::nationalAccount() ),
    _textKey(defaultTextKey),
    _subTextKey(defaultSubTextKey)
{

}

germanOnlineTransferImpl::germanOnlineTransferImpl(const germanOnlineTransferImpl& other)
  : germanOnlineTransfer( other ),
    _settings( other._settings ),
    _value( other._value ),
    _purpose( other._purpose ),
    _originAccount( other._originAccount ),
    _beneficiaryAccount( other._beneficiaryAccount ),
    _textKey( other._textKey ),
    _subTextKey( other._subTextKey )
{

}

germanOnlineTransfer *germanOnlineTransferImpl::clone() const
{
  germanOnlineTransfer *transfer = new germanOnlineTransferImpl( *this );
  return transfer;
}

bool germanOnlineTransferImpl::isValid() const
{
  QSharedPointer<const germanOnlineTransfer::settings> settings = getSettings();

  if ( settings->checkPurposeLength( _purpose ) == validators::ok
    && settings->checkPurposeMaxLines( _purpose )
    && settings->checkPurposeLineLength( _purpose )
    && settings->checkPurposeCharset( _purpose )
    //&& settings->checkRecipientCharset( _beneficiaryAccount.ownerName() )
    //&& settings->checkRecipientLength( _beneficiaryAccount.ownerName() ) == validators::ok
    && settings->checkRecipientAccountNumber( _beneficiaryAccount.accountNumber() ) == validators::ok
    && settings->checkRecipientBankCode( _beneficiaryAccount.bankCode() ) == validators::ok
    && value().isPositive()
  )
    return true;
  return false;
}

payeeIdentifier germanOnlineTransferImpl::originAccountIdentifier() const
{
  QList< payeeIdentifierTyped<payeeIdentifiers::nationalAccount> > idents = MyMoneyFile::instance()->account(_originAccount).payeeIdentifiersByType<payeeIdentifiers::nationalAccount>();
  if ( !idents.isEmpty() ) {
    payeeIdentifierTyped<payeeIdentifiers::nationalAccount> ident = idents[0];
    ident->setOwnerName(MyMoneyFile::instance()->user().name());
    return ident;
  }
  return payeeIdentifier( new payeeIdentifiers::nationalAccount );
}

/** @todo make alive */
MyMoneySecurity germanOnlineTransferImpl::currency() const
{
#if 0
  return MyMoneyFile::instance()->security(originMyMoneyAccount().currencyId());
#endif
  return MyMoneyFile::instance()->baseCurrency();
}

QSharedPointer<const germanOnlineTransfer::settings> germanOnlineTransferImpl::getSettings() const
{
  if (_settings.isNull()) {
    _settings = onlineJobAdministration::instance()->taskSettings<germanOnlineTransfer::settings>( name(), _originAccount );
    if ( _settings.isNull() )
      _settings = QSharedPointer<const germanOnlineTransfer::settings>( new germanOnlineTransferSettingsFallback );
  }
  Q_ASSERT( !_settings.isNull() );
  return _settings;
}

void germanOnlineTransferImpl::setOriginAccount( const QString& accountId )
{
  if (accountId != _originAccount) {
    _originAccount = accountId;
    _settings = QSharedPointer<const settings>();
  }
}

/** @todo save remote account */
void germanOnlineTransferImpl::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent.setAttribute("originAccount", _originAccount);
  parent.setAttribute("value", _value.toString());
  parent.setAttribute("textKey", _textKey);
  parent.setAttribute("subTextKey", _subTextKey);

  if (!_purpose.isEmpty()) {
    parent.setAttribute("purpose", _purpose);
  }

  QDomElement beneficiaryEl = document.createElement("beneficiary");
  _beneficiaryAccount.writeXML(document, beneficiaryEl);
  parent.appendChild(beneficiaryEl);
}

/** @todo load remote account */
germanOnlineTransferImpl* germanOnlineTransferImpl::createFromXml(const QDomElement& element) const
{
  germanOnlineTransferImpl* task = new germanOnlineTransferImpl();
  task->setOriginAccount( element.attribute("originAccount", QString()) );
  task->setValue( MyMoneyMoney( QStringEmpty(element.attribute("value", QString())) ) );
  task->_textKey = element.attribute("textKey", QString().setNum(defaultTextKey)).toUShort();
  task->_subTextKey = element.attribute("subTextKey", QString().setNum(defaultSubTextKey)).toUShort();
  task->setPurpose( element.attribute("purpose", QString()) );

  payeeIdentifiers::nationalAccount beneficiary;
  payeeIdentifiers::nationalAccount* beneficiaryPtr = 0;
  QDomElement beneficiaryEl = element.firstChildElement("beneficiary");
  if ( !beneficiaryEl.isNull() ) {
    beneficiaryPtr = beneficiary.createFromXml(beneficiaryEl);
  }

  if ( beneficiaryPtr == 0 ) {
    task->_beneficiaryAccount = beneficiary;
  } else {
    task->_beneficiaryAccount = *beneficiaryPtr;
  }

  return task;
}

onlineTask* germanOnlineTransferImpl::createFromSqlDatabase(QSqlDatabase connection, const QString& onlineJobId) const
{
  Q_ASSERT( !onlineJobId.isEmpty() );
  Q_ASSERT( connection.isOpen() );

  QSqlQuery query = QSqlQuery(
    "SELECT originAccount, value, purpose, beneficiaryName, beneficiaryAccountNumber, "
    " beneficiaryBankCode, textKey, subTextKey FROM kmmNationalOrders WHERE id = ?",
    connection
  );
  query.bindValue(0, onlineJobId);

  if ( query.exec() && query.next() ) {
    germanOnlineTransferImpl* task = new germanOnlineTransferImpl();
    task->setOriginAccount( query.value(0).toString() );
    task->setValue( MyMoneyMoney( query.value(1).toString() ) );
    task->setPurpose( query.value(2).toString() );
    task->_textKey = query.value(6).toUInt();
    task->_subTextKey = query.value(7).toUInt();

    payeeIdentifiers::nationalAccount beneficiary;
    beneficiary.setOwnerName( query.value(3).toString() );
    beneficiary.setAccountNumber( query.value(4).toString() );
    beneficiary.setBankCode( query.value(5).toString() );
    task->_beneficiaryAccount = beneficiary;
    return task;
  }

  return 0;
}

bool germanOnlineTransferImpl::sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  QSqlQuery query = QSqlQuery(databaseConnection);
  query.prepare("INSERT INTO kmmNationalOrders ("
  " id, originAccount, value, purpose, beneficiaryName, beneficiaryAccountNumber, "
  " beneficiaryBankCode, textKey, subTextKey) "
  " VALUES( :id, :originAccount, :value, :purpose, :beneficiaryName, :beneficiaryAccountNumber, "
  "         :beneficiaryBankCode, :textKey, :subTextKey ) "
  );
  bindValuesToQuery( query, onlineJobId );
  if ( !query.exec() ) {
    qWarning("Error while inserting national order '%s': %s", qPrintable(onlineJobId), qPrintable(query.lastError().text()));
    return false;
  }
  return true;
}

bool germanOnlineTransferImpl::sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  QSqlQuery query = QSqlQuery(databaseConnection);
  query.prepare(
    "UPDATE kmmNationalOrders SET"
    " originAccount = :originAccount,"
    " value = :value,"
    " purpose = :purpose,"
    " beneficiaryName = :beneficiaryName,"
    " beneficiaryAccountNumber = :beneficiaryAccountNumber,"
    " beneficiaryBankCode = :beneficiaryBankCode,"
    " textKey = :textKey,"
    " subTextKey = :subTextKey "
    " WHERE id = :id");
  bindValuesToQuery( query, onlineJobId );
  if ( !query.exec() ) {
    qWarning("Could not modify national order: %s", qPrintable(query.lastError().text()));
    return false;
  }
  return true;
}

bool germanOnlineTransferImpl::sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const
{
  QSqlQuery query = QSqlQuery(databaseConnection);
  query.prepare("DELETE FROM kmmNationalOrders WHERE id = ?");
  query.bindValue(0, onlineJobId);
  return query.exec();
}

void germanOnlineTransferImpl::bindValuesToQuery(QSqlQuery& query, const QString& id) const
{
  query.bindValue(":id", id);
  query.bindValue(":originAccount", _originAccount);
  query.bindValue(":value", _value.toString());
  query.bindValue(":purpose", _purpose);
  query.bindValue(":beneficiaryName", _beneficiaryAccount.ownerName());
  query.bindValue(":beneficiaryAccountNumber", _beneficiaryAccount.accountNumber());
  query.bindValue(":beneficiaryBankCode", _beneficiaryAccount.bankCode());
  query.bindValue(":textKey", _textKey);
  query.bindValue(":subTextKey", _subTextKey);
}

bool germanOnlineTransferImpl::hasReferenceTo(const QString& id) const
{
  return (id == _originAccount);
}
