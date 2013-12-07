#ifndef ONLINEPLUGINEXTENDED_H
#define ONLINEPLUGINEXTENDED_H

#include "kmymoneyplugin.h"
#include <QtCore/QObject>
#include <QtCore/QList>

#include "mymoney/onlinejob.h"
#include "mymoney/onlinetask.h"

namespace KMyMoneyPlugin
{

/**
 * @brief Interface between KMyMoney and Online Banking plugins for executing transactions
 *
 * This interface is under active development and will change often! Do not use it at the moment!
 *
 * @author Christian David (christian-david@web.de)
 */
class KMM_PLUGINS_EXPORT OnlinePluginExtended : public Plugin, public OnlinePlugin
{
  Q_OBJECT

public:
  OnlinePluginExtended(QObject* parent, const char* name);

  /**
   * @brief List onlineJobs supported by an account
   *
   * KMyMoney will use this function to ask the online plugin which online jobs it supports.
   * Later changes can be made public using the jobAvailable signals.
   *
   * @return A QStringList with supported onlineTask::name()s as values.
   */
  virtual QStringList availableJobs( QString accountId ) = 0;

  /**
   * @brief Get settings for onlineTask
   *
   * @see onlineTask::settings
   */
  virtual QSharedPointer<const onlineTask::settings> settings( QString accountId, QString taskName ) = 0;

public slots:
  /**
   * @brief Send onlineJobs to bank
   *
   * @param jobs Do not delete the onlineJob objects. You can edit them but expect them to be deleted after
   * you returned from this function.
   */
  virtual QList<onlineJob> sendOnlineJob(QList<onlineJob> jobs) = 0;


signals:
/**
 * @brief Emit to make onlineJob available
 *
 * In case a onlineJob got available during runtime, emit one of these signals.
 */
  void jobAvailable( QString accountId, size_t );
  void jobAvailable( QString accountId, QList<size_t> );
  void jobUnavailable( QString accountId, size_t );
  //void jobUnavailable( QString accountId );
};

} // namespace KMyMoneyPlugin

#endif // ONLINEPLUGINEXTENDED_H
