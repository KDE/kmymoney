#ifndef ONLINEJOBADMINISTRATION_H
#define ONLINEJOBADMINISTRATION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QPair>

// ----------------------------------------------------------------------------
// Project Includes

#include "onlinejob.h"
#include "onlinejobknowntask.h"
#include "onlinetask.h"

namespace KMyMoneyPlugin {
  class OnlinePluginExtended;
}

/**
 * @brief Connection between KMyMoney and the plugins
 *
 * It's main task is the communication with plugins
 * and caching thier information during run-time. During
 * sending this class selects the correct plugin for each
 * onlineJob.
 *
 * This class keeps an overview which account can handle which jobs and
 * offers methods to access these information.
 *
 * onlineJobAdministration is created with singleton pattern. Get the
 * instance with @ref onlineJobAdministration::instance() .
 */
class KMM_MYMONEY_EXPORT onlineJobAdministration : public QObject
{
  Q_OBJECT

public:
  explicit onlineJobAdministration(QObject *parent = 0);
  ~onlineJobAdministration();
  
  //onlineJobList availableJobs( QString accountId );

  static onlineJobAdministration* instance() { return &m_instance; }

  bool isJobSupported(const QString& accountId, const QString& name) const;
  bool isJobSupported(const QString& accountId, const size_t& hash) const;
  onlineTask::convertType canConvert( const QString& originalName, const QString& destinationName ) const;
  onlineTask::convertType canConvert(const onlineJob& original, const QString& destinationName ) const;

  template<class T>
  onlineJobKnownTask<T> convert( const onlineJob& original, const QString& destinationName, const QString& id = MyMoneyObject::emptyId() ) const;

  onlineJob convert( const onlineJob& original, const QString& destinationName, const QString& id = MyMoneyObject::emptyId() ) const;

  /**
   * @brief Request onlineTask::settings from plugin
   *
   * @return QSharedPointer to settings from plugin or settings with default values if error occours
   * (so you never get QSharedPointer::isNull() == true)
   */
  template<class T>
  QSharedPointer<const T> taskSettings( const QString& taskName, const QString& accountId ) const;

  /**
   * @brief Request onlineTask::settings from plugin
   *
   * @see onlineTask::settings
   *
   * @param taskName onlineTask::name()
   * @param accountId MyMoneyAccount.id()
   * @return QSharedPointer to settings. QSharedPointer::isNull() is true if an error occours
   * (e.g. plugin does not support the task).
   */
  QSharedPointer<const onlineTask::settings> taskSettings( const QString& taskName, const QString& accountId ) const;

signals:
    
public slots:
  void addPlugin( const QString& pluginName, KMyMoneyPlugin::OnlinePluginExtended* );

  /**
   * @brief Slot for plugins to make an onlineJob available.
   * @param accountId Account which supports this job
   * @param jobType onlineTask::name() of the job
   */
  void makeOnlineJobAvailable(const QString& accountId, const QString& jobType );
    
private:
  /**
   * @brief Find onlinePlugin which is resposible for accountId
   * @param accountId
   * @return Pointer to onlinePluginExtended, do not delete.
   */
  KMyMoneyPlugin::OnlinePluginExtended* getOnlinePlugin( const QString& accountId ) const;

  /**
   * The key is the onlinePlugin's name
   */
  QMap<QString, KMyMoneyPlugin::OnlinePluginExtended *> m_onlinePlugins;
  
  static onlineJobAdministration m_instance;

  /** @brief Checks destinationName's canConvert() */
  onlineTask::convertType canConvertFrom( const QString& originalName, const QString& destinationName ) const;

  /** @brief Checks originalName's canConvertInto() */
  onlineTask::convertType canConvertInto( const QString& originalName, const QString& destinationName ) const;

  /** @brief Use onlineTask::name() to create a corresponding onlineJob */
  onlineJob createOnlineJobByName( const QString& name ) const;
  
  /**
   * @brief Converts onlineTask::hash into onlineTask::name()
   * @return A onlineTask::name() or QString() if not found.
   */
  QString getTaskNameByHash( const size_t& hash ) const;
};

template<class T>
QSharedPointer<const T> onlineJobAdministration::taskSettings( const QString& taskName, const QString& accountId ) const
{
  QSharedPointer<const onlineTask::settings> settings = taskSettings( taskName, accountId );
  if ( !settings.isNull() ) {
    QSharedPointer<const T> settingsFinal = settings.dynamicCast<const T>();
    if ( Q_LIKELY( !settingsFinal.isNull() ) ) // This can only happen if the onlinePlugin has a bug.
      return settingsFinal;
  }
  return QSharedPointer<const T>( new T() );
}

#endif // ONLINEJOBADMINISTRATION_H
