#include "onlinejobadministration.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QScopedPointer>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneykeyvaluecontainer.h"
#include "plugins/onlinepluginextended.h"

#include "germanonlinetransfer.h"
#include "sepaonlinetransfer.h"

onlineJobAdministration onlineJobAdministration::m_instance;

onlineJobAdministration::onlineJobAdministration(QObject *parent) :
    QObject(parent)
{
}

onlineJobAdministration::~onlineJobAdministration()
{

}

KMyMoneyPlugin::OnlinePluginExtended* onlineJobAdministration::getOnlinePlugin( const QString& accountId ) const
{
    MyMoneyAccount acc = MyMoneyFile::instance()->account( accountId );

    QMap<QString, KMyMoneyPlugin::OnlinePluginExtended*>::const_iterator it_p;
    it_p = m_onlinePlugins.constFind(acc.onlineBankingSettings().value("provider"));

    if (it_p != m_onlinePlugins.constEnd() ) {
      // plugin found, use it
      return *it_p;
    }
    return 0;
}

void onlineJobAdministration::addPlugin(const QString& pluginName, KMyMoneyPlugin::OnlinePluginExtended *plugin )
{
  m_onlinePlugins.insert(pluginName, plugin);
}

bool onlineJobAdministration::isJobSupported(const QString& accountId, const QString& name) const
{
  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, m_onlinePlugins) {
    if (plugin->availableJobs(accountId).contains(name))
      return true;
  }
  return false;
}

bool onlineJobAdministration::isJobSupported(const QString& accountId, const size_t& hash) const
{
  const QString name = getTaskNameByHash( hash );
  if ( name.isNull() )
    return false;
  return isJobSupported(accountId, name);
}

onlineJob onlineJobAdministration::createOnlineJobByName( const QString& name ) const
{
  onlineTask* task = 0;
  if ( germanOnlineTransfer::name() == name )
    task = new germanOnlineTransfer;
  else if ( sepaOnlineTransfer::name() == name )
    task = new sepaOnlineTransfer;

  return onlineJob( task );
}

onlineTask::convertType onlineJobAdministration::canConvertFrom(const QString& originalName, const QString& destinationName) const
{
  Q_UNUSED(originalName); // it is used in the macro
  Q_UNUSED(destinationName); // it is used in the macro

#define CAN_CONVERT_REQUEST( TASKCLASS ) ( TASKCLASS::name() == originalName ) { \
  QScopedPointer< TASKCLASS > task( new TASKCLASS() ); \
  convertType = task->canConvert( destinationName ); \
  if ( convertType != onlineTask::convertImpossible ) { return convertType; } \
  }

  onlineTask::convertType convertType;
  Q_UNUSED(convertType); // it is used in the macro

  if CAN_CONVERT_REQUEST(sepaOnlineTransfer)
  else if CAN_CONVERT_REQUEST(germanOnlineTransfer)

  return onlineTask::convertImpossible;

#undef CAN_CONVERT_REQUEST
}


onlineTask::convertType onlineJobAdministration::canConvertInto(const QString& originalName, const QString& destinationName) const
{
  Q_UNUSED(originalName); // it is used in the macro
  Q_UNUSED(destinationName); // it is used in the macro
#define CAN_CONVERT_INTO_REQUEST( TASKCLASS ) ( TASKCLASS::name() == originalName ) { \
  QScopedPointer< TASKCLASS > task( new TASKCLASS() ); \
  convertType = task->canConvert( destinationName ); \
  if ( convertType != onlineTask::convertImpossible ) { return convertType; } \
  }

  onlineTask::convertType convertType;
  Q_UNUSED(convertType); // it is used in the macro

  if CAN_CONVERT_INTO_REQUEST(sepaOnlineTransfer)
  else if CAN_CONVERT_INTO_REQUEST(germanOnlineTransfer)

  return onlineTask::convertImpossible;

#undef CAN_CONVERT_INTO_REQUEST
}

onlineTask::convertType onlineJobAdministration::canConvert(const QString& originalName, const QString& destinationName ) const
{
  if ( originalName == destinationName )
    return onlineTask::convertionLoseless;

  // test onlineTask::canConvert
  onlineTask::convertType convertType = canConvertFrom(originalName, destinationName);
  if (convertType != onlineTask::convertImpossible)
    return convertType;

  // test onlineTask::canConvertInto
  return canConvertInto(originalName, destinationName);
}

onlineTask::convertType onlineJobAdministration::canConvert( const onlineJob& original, const QString& destinationName ) const
{
  return canConvert(original.task()->taskName(), destinationName);
}

template<class T>
onlineJobTyped<T> onlineJobAdministration::convert(const onlineJob& original, const QString& destinationName, const QString& id ) const
{
  onlineJob job = convert(original, destinationName, id);
  return onlineJobTyped<T>(job);
}

onlineJob onlineJobAdministration::convert( const onlineJob& original, const QString& destinationName, const QString& id ) const
{
  const QString originalName = original.task()->taskName();
  if ( originalName == destinationName )
    return onlineJob(id, original);

  QString messageString = QString();
  bool payeeChanged = false;

  if ( canConvertFrom(originalName, destinationName) != onlineTask::convertImpossible ) {
    // Use onlineTask::convert()
    onlineJob job = createOnlineJobByName(destinationName);
    if ( job.isNull() )
      throw new onlineTask::badConvert(__FILE__, __LINE__);
    job.task()->convert( *original.constTask(), messageString, payeeChanged );
    return job;
  } else if ( canConvertInto(originalName, destinationName) !=  onlineTask::convertImpossible ) {
    // Use onlineTask::convertInto()
    onlineTask* const task = original.constTask()->convertInto( destinationName, messageString, payeeChanged );
    onlineJob newJob = onlineJob(task, id);
    return newJob;
  }

  throw new onlineTask::badConvert(__FILE__, __LINE__);
}

void onlineJobAdministration::makeOnlineJobAvailable(const QString& accountId, const QString& jobType)
{
#if 0
  // Remove job of same type first
  if ( m_availableJobList.contains(accountId) ) {
    onlineJob_t newJobIndex = jobType->getTypeIndex();
    QList<onlineJob> jobList = m_availableJobList.values( accountId );
    QList<onlineJob>::Iterator end = jobList.end();
    for( QList<onlineJob>::Iterator iter = jobList.begin(); iter != end ; iter++) {
      if ( (*iter)->getTypeIndex() == newJobIndex ) {
        delete (*iter);
        *iter = jobType;
        break;
      }
    }
  } else {
    m_availableJobList.insert(accountId, jobType);
  }
#endif
}

QSharedPointer<const onlineTask::settings> onlineJobAdministration::taskSettings( const QString& taskName, const QString& accountId ) const
{
  KMyMoneyPlugin::OnlinePluginExtended* plugin = getOnlinePlugin(accountId);
  if (plugin != 0)
    return ( plugin->settings(accountId, taskName) );
  return QSharedPointer<const onlineTask::settings>();
}

QString onlineJobAdministration::getTaskNameByHash(const size_t& hash) const
{
  if ( germanOnlineTransfer::hash == hash )
    return germanOnlineTransfer::name();
  else if ( sepaOnlineTransfer::hash == hash )
    return sepaOnlineTransfer::name();
  
  return QString();
}
