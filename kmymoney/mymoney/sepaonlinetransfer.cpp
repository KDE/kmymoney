#include "sepaonlinetransfer.h"

#include "mymoney/mymoneyfile.h"

#include "germanonlinetransfer.h"
#include "onlinejobadministration.h"

ONLINETASK_META_INIT(sepaOnlineTransfer);

sepaOnlineTransfer::sepaOnlineTransfer()
  : onlineTransfer(),
    _settings( QSharedPointer<const settings>() ),
    _value(0),
    _purpose(QString("")),
    _reference(QString("")),
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
    _reference(other._reference),
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

bool sepaOnlineTransfer::isValid() const
{
  return true;
}

sepaAccountIdentifier* sepaOnlineTransfer::originAccountIdentifier() const
{
  sepaAccountIdentifier* ident = new sepaAccountIdentifier();
//  ident->setAccountNumber( originMyMoneyAccount().number() );
//  ident->setBankCode( MyMoneyFile::instance()->institution( originMyMoneyAccount().institutionId()).sortcode() );
//  ident->setOwnerName( MyMoneyFile::instance()->user().name() );
  return ident;
}

/** @todo make alive */
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
    setValue( germanTask.value() );
    setPurpose( germanTask.purpose() );
    setReference( QString() );
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
