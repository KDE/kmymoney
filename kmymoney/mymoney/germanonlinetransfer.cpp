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


#include "germanonlinetransfer.h"

#include <QtGui/QRegExpValidator>

#include "mymoney/mymoneyfile.h"
#include "mymoney/sepaonlinetransfer.h"
#include "onlinejobadministration.h"

ONLINETASK_META_INIT(germanOnlineTransfer);

static const unsigned short defaultTextKey = 51;
static const unsigned short defaultSubTextKey = 0;

germanOnlineTransfer::germanOnlineTransfer()
  : onlineTransfer(),
    _settings( QSharedPointer<const settings>() ),
    _value(0),
    _purpose(QString()),
    _originAccount( QString() ),
    _remoteAccount( germanAccountIdentifier() ),
    _textKey(defaultTextKey),
    _subTextKey(defaultSubTextKey)
{

}

germanOnlineTransfer::germanOnlineTransfer(const germanOnlineTransfer& other)
  : onlineTransfer( other ),
    _settings( other._settings ),
    _value( other._value ),
    _purpose( other._purpose ),
    _originAccount( other._originAccount ),
    _remoteAccount( other._remoteAccount ),
    _textKey( other._textKey ),
    _subTextKey( other._subTextKey )
{

}

germanOnlineTransfer *germanOnlineTransfer::clone() const
{
  germanOnlineTransfer *transfer = new germanOnlineTransfer( *this );
  return transfer;
}

bool germanOnlineTransfer::isValid() const
{
  QSharedPointer<const germanOnlineTransfer::settings> settings = getSettings();

  if ( settings->checkPurposeLength( _purpose ) == creditTransferSettingsBase::ok
    && settings->checkPurposeMaxLines( _purpose )
    && settings->checkPurposeLineLength( _purpose )
    && settings->checkPurposeCharset( _purpose )
    && settings->checkRecipientCharset( _remoteAccount.ownerName() )
    && settings->checkRecipientLength( _remoteAccount.ownerName() ) == creditTransferSettingsBase::ok
    && settings->checkRecipientAccountNumber( _remoteAccount.accountNumber() ) == creditTransferSettingsBase::ok
    && settings->checkRecipientBankCode( _remoteAccount.bankCode() ) == creditTransferSettingsBase::ok
    && value().isPositive()
  )
    return true;
  return false;
}

/** @todo make alive */
germanAccountIdentifier* germanOnlineTransfer::originAccountIdentifier() const
{
  germanAccountIdentifier* ident = new germanAccountIdentifier();
#if 0
  ident->setAccountNumber( originMyMoneyAccount().number() );
  ident->setBankCode( MyMoneyFile::instance()->institution( originMyMoneyAccount().institutionId()).sortcode() );
  ident->setOwnerName( MyMoneyFile::instance()->user().name() );
#endif
  return ident;
}

/** @todo make alive */
MyMoneySecurity germanOnlineTransfer::currency() const
{
#if 0
  return MyMoneyFile::instance()->security(originMyMoneyAccount().currencyId());
#endif
  return MyMoneyFile::instance()->baseCurrency();
}

onlineTask::convertType germanOnlineTransfer::canConvertInto( const QString& onlineTaskName ) const
{
  Q_UNUSED(onlineTaskName);
  return onlineTask::convertImpossible;
}

onlineTask::convertType germanOnlineTransfer::canConvert( const QString& onlineTaskName ) const
{
  if(onlineTaskName == "org.kmymoney.creditTransfer.sepa")
    return onlineTask::convertionLossy;

  return onlineTask::convertImpossible;
}

onlineTask* germanOnlineTransfer::convertInto( const QString& onlineTaskName , QString& messageString, bool& payeeChanged ) const
{
  Q_UNUSED(onlineTaskName);
  Q_UNUSED(messageString);
  Q_UNUSED(payeeChanged);
  throw new onlineTask::badConvert;
}

void germanOnlineTransfer::convert(const onlineTask &task, QString& messageString, bool& payeeChanged )
{
  if( task.taskHash() == sepaOnlineTransfer::hash ) {
    const sepaOnlineTransfer& sepaTask = static_cast<const sepaOnlineTransfer&>(task);
    payeeChanged = true;
    setOriginAccount( sepaTask.responsibleAccount() );
    setValue( sepaTask.value() );
    _purpose = QString("");
    if ( !sepaTask.endToEndReference().isEmpty() ) {
      messageString = i18n("National credit transfer has no field for SEPA reference. It was added to the purpose instead.");
      _purpose = sepaTask.endToEndReference() + QChar('\n');
    }
    _purpose.append( sepaTask.purpose() );
    return;
  }
  throw new onlineTask::badConvert;
}

QSharedPointer<const germanOnlineTransfer::settings> germanOnlineTransfer::getSettings() const
{
  if (_settings.isNull()) {
    _settings = onlineJobAdministration::instance()->taskSettings<germanOnlineTransfer::settings>( name(), _originAccount );
  }
  return _settings;
}

void germanOnlineTransfer::setOriginAccount( const QString& accountId )
{
  if (accountId != _originAccount) {
    _originAccount = accountId;
    _settings = QSharedPointer<const settings>();
  }
}

/** @todo save remote account */
void germanOnlineTransfer::writeXML(QDomDocument& document, QDomElement& parent) const
{
  Q_UNUSED(document);
  parent.setAttribute("originAccount", _originAccount);
  parent.setAttribute("value", _value.toString());
  parent.setAttribute("textKey", _textKey);
  parent.setAttribute("subTextKey", _subTextKey);
  
  if (!_purpose.isEmpty()) {
    parent.setAttribute("purpose", _purpose);
  }
}

/** @todo load remote account */
germanOnlineTransfer* germanOnlineTransfer::createFromXml(const QDomElement& element) const
{
  germanOnlineTransfer* task = new germanOnlineTransfer();
  task->setOriginAccount( element.attribute("originAccount", QString()) );
  task->setValue( MyMoneyMoney( QStringEmpty(element.attribute("value", QString())) ) );
  task->_textKey = element.attribute("textKey", QString().setNum(defaultTextKey)).toUShort();
  task->_subTextKey = element.attribute("subTextKey", QString().setNum(defaultSubTextKey)).toUShort(); 
  task->setPurpose( element.attribute("purpose", QString()) );
  return task;
}

bool germanOnlineTransfer::hasReferenceTo(const QString& id) const
{
  return (id == _originAccount);
}
