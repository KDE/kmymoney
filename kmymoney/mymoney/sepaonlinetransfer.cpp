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

#include "sepaonlinetransfer.h"

// Ktoblz check
#include <iban.h>

#include "mymoney/mymoneyfile.h"

#include "germanonlinetransfer.h"
#include "onlinejobadministration.h"

ONLINETASK_META_INIT(sepaOnlineTransfer);

sepaOnlineTransfer::sepaOnlineTransfer()
  : onlineTransfer(),
    _settings( QSharedPointer<const settings>() ),
    _value(0),
    _purpose(QString("")),
    _endToEndReference(QString("")),
    _originAccount( QString() ),
    _remoteAccount( sepaAccountIdentifier() ),
    _textKey(51),
    _subTextKey(0)
{

}

sepaOnlineTransfer::sepaOnlineTransfer(const sepaOnlineTransfer& other)
  : onlineTransfer( other ),
    _settings( other._settings ),
    _value( other._value ),
    _purpose( other._purpose ),
    _endToEndReference(other._endToEndReference),
    _originAccount( other._originAccount ),
    _remoteAccount( other._remoteAccount ),
    _textKey( other._textKey ),
    _subTextKey( other._subTextKey )
{

}

sepaOnlineTransfer *sepaOnlineTransfer::clone() const
{
  sepaOnlineTransfer *transfer = new sepaOnlineTransfer( *this );
  return transfer;
}

//! @todo add validation of local name
bool sepaOnlineTransfer::isValid() const
{
  QSharedPointer<const sepaOnlineTransfer::settings> settings = getSettings();
  if ( settings->checkPurposeLength( _purpose ) == creditTransferSettingsBase::ok
    && settings->checkPurposeMaxLines( _purpose )
    && settings->checkPurposeLineLength( _purpose )
    && settings->checkPurposeCharset( _purpose )
    && settings->checkEndToEndReferenceLength( _endToEndReference ) == creditTransferSettingsBase::ok
    && settings->checkRecipientCharset( _remoteAccount.ownerName() )
    && settings->checkRecipientLength( _remoteAccount.ownerName()) == creditTransferSettingsBase::ok
    && settings->isIbanValid( _remoteAccount.accountNumber() )
    && settings->checkRecipientBic( _remoteAccount.bankCode() )
    && value().isPositive()
  )
    return true;
  return false;
}

sepaAccountIdentifier* sepaOnlineTransfer::originAccountIdentifier() const
{
  sepaAccountIdentifier* ident = new sepaAccountIdentifier();
//  ident->setAccountNumber( originMyMoneyAccount().number() );
//  ident->setBankCode( MyMoneyFile::instance()->institution( originMyMoneyAccount().institutionId()).sortcode() );
//  ident->setOwnerName( MyMoneyFile::instance()->user().name() );
  return ident;
}

/** @todo Return EUR */
MyMoneySecurity sepaOnlineTransfer::currency() const
{
#if 0
  return MyMoneyFile::instance()->security(originMyMoneyAccount().currencyId());
#endif
  return MyMoneyFile::instance()->baseCurrency();
}

onlineTask::convertType sepaOnlineTransfer::canConvertInto( const QString& onlineTaskName ) const
{
  Q_UNUSED(onlineTaskName);
  return onlineTask::convertImpossible;
}

onlineTask::convertType sepaOnlineTransfer::canConvert( const QString& onlineTaskName ) const
{
  if (onlineTaskName == "org.kmymoney.creditTransfer.germany")
    return onlineTask::convertionLoseless;
  return onlineTask::convertionLossy;
}

onlineTask* sepaOnlineTransfer::convertInto( const QString& onlineTaskName, QString& messageString, bool& payeeChanged ) const
{
  Q_UNUSED(onlineTaskName);
  Q_UNUSED(messageString);
  Q_UNUSED(payeeChanged);
  throw new onlineTask::badConvert;
}

void sepaOnlineTransfer::convert(const onlineTask &task, QString& messageString, bool& payeeChanged )
{
  Q_UNUSED(messageString);
  if ( task.taskHash() == germanOnlineTransfer::hash ) {
    const germanOnlineTransfer& germanTask = static_cast<const germanOnlineTransfer&>(task);
    payeeChanged = true;
    setOriginAccount( germanTask.responsibleAccount() );
    setValue( germanTask.value() );
    setPurpose( germanTask.purpose() );
    setEndToEndReference( QString() );
    return;
  }
  throw new onlineTask::badConvert;
}

QSharedPointer<const sepaOnlineTransfer::settings> sepaOnlineTransfer::getSettings() const
{
  if (_settings.isNull()) {
    _settings = onlineJobAdministration::instance()->taskSettings<sepaOnlineTransfer::settings>( name(), _originAccount );
  }
  return _settings;
}

void sepaOnlineTransfer::setOriginAccount(const QString &accountId)
{
    if ( _originAccount != accountId ) {
      _originAccount = accountId;
      _settings = QSharedPointer<const sepaOnlineTransfer::settings>();
    }
}

sepaOnlineTransfer::settings::lengthStatus sepaOnlineTransfer::settings::checkEndToEndReferenceLength(const QString& reference) const
{
    if (reference.length() > m_endToEndReferenceLength)
        return tooLong;
    return ok;
}

bool sepaOnlineTransfer::settings::isIbanValid( const QString& iban )
{
    IbanCheck checker;
    switch (checker.check( Iban( iban.toLatin1().constData() ) )) {
    case IbanCheck::COUNTRY_NOT_FOUND:
    case IbanCheck::OK: return true;
    case IbanCheck::BAD_CHECKSUM:
    case IbanCheck::TOO_SHORT:
    case IbanCheck::WRONG_LENGTH:
    case IbanCheck::WRONG_COUNTRY:
    default: return false;
    }
}
