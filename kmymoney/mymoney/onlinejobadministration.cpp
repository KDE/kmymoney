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
  registerOnlineTask( new germanOnlineTransfer );
  registerOnlineTask( new sepaOnlineTransfer );
}

onlineJobAdministration::~onlineJobAdministration()
{
//  qDeleteAll(m_onlinePlugins);
  qDeleteAll(m_onlineTasks);
  // no m_onlinePlugins.clear(), as this is done anyway
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

/**
 * @internal The real work is done here.
 */
bool onlineJobAdministration::isJobSupported(const QString& accountId, const QString& name) const
{
  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, m_onlinePlugins) {
    if (plugin->availableJobs(accountId).contains(name))
      return true;
  }
  return false;
}

bool onlineJobAdministration::isJobSupported(const QString& accountId, const QStringList& names) const
{
  foreach ( QString name, names ) {
    if ( isJobSupported(accountId, name) )
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

bool onlineJobAdministration::isAnyJobSupported(const QString& accountId) const
{
  if (accountId.isEmpty())
    return false;

  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, m_onlinePlugins) {
    if (!(plugin->availableJobs(accountId).isEmpty()))
      return true;
  }
  return false;
}

onlineJob onlineJobAdministration::createOnlineJob( const QString& name, const QString& id ) const
{
  return ( onlineJob( createOnlineTask(name), id ) );
}

onlineTask* onlineJobAdministration::createOnlineTask(const QString& name) const
{
  const onlineTask* task = rootOnlineTask(name);
  if (task != 0)
    return task->clone();
  return 0;
}

onlineTask* onlineJobAdministration::createOnlineTaskByXml(const QString& iid, const QDomElement& element) const
{
  onlineTask* task = rootOnlineTask(iid);
  if (task != 0) {
    return task->createFromXml(element);
  }
  return 0;
}

onlineTask* onlineJobAdministration::rootOnlineTask(const QString& name) const
{
  return m_onlineTasks.value(name);
}


onlineTask::convertType onlineJobAdministration::canConvertFrom(const QString& originalName, const QString& destinationName) const
{
  const onlineTask* task = m_onlineTasks.value(destinationName);
  if (task != 0) {
    return task->canConvert(originalName);
  }

  return onlineTask::convertImpossible;
}


onlineTask::convertType onlineJobAdministration::canConvertInto(const QString& originalName, const QString& destinationName) const
{
  const onlineTask* task = m_onlineTasks.value(originalName);
  if (task != 0) {
    return task->canConvertInto(destinationName);
  }

  return onlineTask::convertImpossible;
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
  try {
    return canConvert(original.task()->taskName(), destinationName);
  } catch ( onlineJob::emptyTask* e ) {
    delete e;
  }
  return onlineTask::convertImpossible;
}

onlineTask::convertType onlineJobAdministration::canConvert( const onlineJob& original, const QStringList& destinationNames) const
{
  onlineTask::convertType bestConvertType = onlineTask::convertImpossible;
  foreach (QString destinationName, destinationNames) {
    onlineTask::convertType type = canConvert( original, destinationName );
    if ( type == onlineTask::convertionLossy )
      bestConvertType = onlineTask::convertionLossy;
    else if ( type == onlineTask::convertionLoseless )
      return onlineTask::convertionLoseless;
  }
  return bestConvertType;
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
    onlineJob job = createOnlineJob(destinationName, id);
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

onlineJob onlineJobAdministration::convertBest( const onlineJob& original, const QStringList& destinationNames, const QString& id ) const
{
  onlineTask::convertType bestConvertType = onlineTask::convertImpossible;
  QString bestDestination = QString();
  
  foreach (QString destinationName, destinationNames) {
    onlineTask::convertType type = canConvert( original, destinationName );
    if ( type == onlineTask::convertionLossy ) {
      bestConvertType = onlineTask::convertionLossy;
      bestDestination = destinationName;
    } else if ( type == onlineTask::convertionLoseless ) {
      bestConvertType = onlineTask::convertionLossy;
      bestDestination = destinationName;
      break;
    }
  }

  if ( !bestDestination.isNull() )
    return convert( original, bestDestination, id );

  throw new onlineTask::badConvert(__FILE__, __LINE__);
}

void onlineJobAdministration::registerOnlineTask(onlineTask *const task)
{
  if (Q_UNLIKELY( task == 0 ))
    return;

  m_onlineTasks.insert(task->taskName(), task);
  qDebug() << "onlineTask available" << task->taskName();
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
