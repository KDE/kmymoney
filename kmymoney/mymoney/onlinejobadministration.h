/*
 * Copyright 2013-2018  Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ONLINEJOBADMINISTRATION_H
#define ONLINEJOBADMINISTRATION_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QMap>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "onlinejob.h"
#include "onlinetasks/interfaces/tasks/onlinetask.h"
#include "onlinetasks/interfaces/tasks/ionlinetasksettings.h"
#include "onlinetasks/interfaces/tasks/credittransfer.h"
#include "onlinetasks/interfaces/converter/onlinetaskconverter.h"

class onlineTask;
class IonlineJobEdit;

namespace KMyMoneyPlugin
{
class OnlinePluginExtended;
}

/**
 * @brief Connection between KMyMoney and the plugins
 *
 * It's main task is the communication with plugins
 * and caching their information during run-time. During
 * sending this class selects the correct plugin for each
 * onlineJob.
 *
 * This class keeps an overview which account can handle which job and
 * offers methods to access these information.
 *
 * onlineJobAdministration is created with singleton pattern. Get the
 * instance with @ref onlineJobAdministration::instance() .
 */
class KMM_MYMONEY_EXPORT onlineJobAdministration : public QObject
{
  Q_OBJECT
  KMM_MYMONEY_UNIT_TESTABLE

  Q_PROPERTY(bool canSendAnyTask READ canSendAnyTask NOTIFY canSendAnyTaskChanged STORED false);
  Q_PROPERTY(bool canSendCreditTransfer READ canSendCreditTransfer NOTIFY canSendCreditTransferChanged STORED false);

protected:
  explicit onlineJobAdministration(QObject *parent = 0);

public:
  ~onlineJobAdministration();

  struct onlineJobEditOffer {
    QString fileName;
    QString pluginKeyword;
    QString name;
  };
  using onlineJobEditOffers = QVector<onlineJobEditOffer>;

  /**
   * @brief List all available onlineTasks
   */
  QStringList availableOnlineTasks();

  static onlineJobAdministration* instance();

  /** @brief clear the internal caches for shutdown */
  void clearCaches();

  /** @brief Use onlineTask::name() to create a corresponding onlineJob */
  onlineJob createOnlineJob(const QString& name, const QString& id = QString()) const;

  /**
   * @brief Return list of IonlineJobEdits
   *
   * Method is temporary!
   *
   * @return I stay owner of all pointers.
   */
  onlineJobEditOffers onlineJobEdits();
  QString onlineJobEditName(onlineJobEditOffer);

  bool isJobSupported(const QString& accountId, const QString& name) const;
  bool isJobSupported(const QString& accountId, const QStringList& names) const;
  bool isAnyJobSupported(const QString& accountId) const;

  onlineTaskConverter::convertType canConvert(const QString& originalTaskIid, const QString& convertTaskIid) const;
  onlineTaskConverter::convertType canConvert(const QString& originalTaskIid, const QStringList& convertTaskIids) const;

#if 0
  template<class T>
  onlineJobTyped<T> convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation, const QString& onlineJobId) const;
  template<class T>
  onlineJobTyped<T> convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation) const;
#endif

  /**
   * @brief Convert an onlineTask to another type
   *
   * @param original onlineJob to convert
   * @param convertTaskIid onlineTask iid you want to convert into
   * @param convertType OUT result of conversion. Note: this depends on original
   * @param userInformation OUT A translated html-string with information about the changes which were done
   * @param onlineJobId The id of the new onlineJob, if none is given original.id() is used
   */
  onlineJob convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation, const QString& onlineJobId) const;

  /**
   * @copydoc convert()
   */
  onlineJob convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation) const;

  /**
   * @brief Converts a onlineTask to best fitting type of a set of onlineTasks
   *
   * Will look for best conversion possible from original to any of convertTaskIids.
   *
   * @param original onlineJob to convert
   * @param convertTaskIids onlineTask-iids you want to convert into.
   * @param convertType OUT result of conversion. Note: this depends on original
   * @param userInformation OUT A translated html-string with information about the changes which were done
   * @param onlineJobId The id of the new onlineJob, if none is given original.id() is used
   */
  onlineJob convertBest(const onlineJob& original, const QStringList& convertTaskIids, onlineTaskConverter::convertType& convertType, QString& userInformation, const QString& onlineJobId) const;

  /**
   * @brief Convinient for convertBest() which crates an onlineJob with the same id as original.
   */
  onlineJob convertBest(const onlineJob& original, const QStringList& convertTaskIids, onlineTaskConverter::convertType& convertType, QString& userInformation) const;

  /**
   * @brief Request onlineTask::settings from plugin
   *
   * @return QSharedPointer to settings from plugin, can be a nullptr
   */
  template<class T>
  QSharedPointer<T> taskSettings(const QString& taskId, const QString& accountId) const;

  /**
   * @brief Request onlineTask::settings from plugin
   *
   * @see onlineTask::settings
   *
   * @param taskId onlineTask::name()
   * @param accountId MyMoneyAccount.id()
   * @return QSharedPointer to settings. QSharedPointer::isNull() is true if an error occurs
   * (e.g. plugin does not support the task).
   */
  QSharedPointer<IonlineTaskSettings> taskSettings(const QString& taskId, const QString& accountId) const;

  /**
   * @brief Check if the onlineTask system can do anything
   *
   * This is true if at least one plugin can process one of the available onlineTasks for at least one available account.
   */
  bool canSendAnyTask();

  /**
   * @brief Are there plugins and accounts to send a credit transfers?
   *
   * Like @r canSendAnyTask() but restricts the onlineTasks to credit transfers. This is useful
   * to disable the create credit transfer buttons.
   */
  bool canSendCreditTransfer();

  /**
   * @brief Are all preconditions set to edit the given job?
   */
  bool canEditOnlineJob(const onlineJob& job);

  /**
   * @brief See if a online task has a specified base
   *
   * This is usable if you want to see if e.g. taskIid is
   * of type creditTransfer
   */
  template<class baseTask>
  bool isInherited(const QString& taskIid) const;

  /**
   * @brief makes plugins loaded in KMyMoneyApp available here
   * @param plugins
   */
  void setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePluginExtended*>& plugins);

  /**
   * @brief updates online actions and should be called after plugin enable or disable
   */
  void updateActions();

Q_SIGNALS:
  /**
   * @brief Emitted if canSendAnyTask() changed
   *
   * At the moment it this signal can be sent even if the status did not change.
   */
  void canSendAnyTaskChanged(bool);

  /**
   * @brief Emitted if canSendCreditTransfer changed
   *
   * At the moment it this signal can be sent even if the status did not change.
   */
  void canSendCreditTransferChanged(bool);

public Q_SLOTS:
  /**
   * @brief Slot for plugins to make an onlineTask available.
   * @param task the task to register, I take ownership
   */
  void registerOnlineTask(onlineTask *const task);

  /**
   * @brief Slot for plugins to make an onlineTaskConverter available.
   * @param converter the converter to register, I take ownership
   */
  void registerOnlineTaskConverter(onlineTaskConverter *const converter);

  /**
   * @brief Check if the properties about available and sendable online tasks are still valid
   */
  void updateOnlineTaskProperties();

private:
  /**
   * Register all available online tasks
   */
  void registerAllOnlineTasks();

  /**
   * @brief Find onlinePlugin which is responsible for accountId
   * @param accountId
   * @return Pointer to onlinePluginExtended, do not delete.
   */
  KMyMoneyPlugin::OnlinePluginExtended* getOnlinePlugin(const QString& accountId) const;

  /**
   * @brief Creates an onlineTask by iid
   * @return pointer to task, caller gains ownership. Can be 0.
   */
  onlineTask* createOnlineTask(const QString& iid) const;

  /**
   * @brief Creates an onlineTask by its iid and xml data
   * @return pointer to task, caller gains ownership. Can be 0.
   */
  onlineTask* createOnlineTaskByXml(const QString& iid, const QDomElement& element) const;

  // Must be able to call createOnlineTaskByXml
  friend class onlineJob;

  // Must be able to call createOnlineTask
  template<class T>
  friend class onlineJobTyped;

  /**
   * @brief Get root instance of an onlineTask
   *
   * Returns a pointer from m_onlineTasks or tries to load/create
   * a approiate root element.
   *
   * Only createOnlineTask and createOnlineTaskByXml use it.
   *
   * @return A pointer, you do *not* gain ownership! Can be 0 if something went wrong.
   *
   * @internal Made to be forward compatible when onlineTask are loaded as plugins.
   */
  inline onlineTask* rootOnlineTask(const QString& name) const;

  /**
   * The key is the onlinePlugin's name
   */
  QMap<QString, KMyMoneyPlugin::OnlinePluginExtended*>* m_onlinePlugins;

  /**
   * The key is the name of the task
   */
  QMap<QString, onlineTask*> m_onlineTasks;

  /**
   * Key is the task the converter converts to
   */
  QMultiMap<QString, onlineTaskConverter*> m_onlineTaskConverter;

  /**
   * Intances of editors
   */
  QList<IonlineJobEdit*> m_onlineTaskEditors;

  bool m_inRegistration;
};

template<class T>
QSharedPointer<T> onlineJobAdministration::taskSettings(const QString& taskName, const QString& accountId) const
{
  IonlineTaskSettings::ptr settings = taskSettings(taskName, accountId);
  if (!settings.isNull()) {
    QSharedPointer<T> settingsFinal = settings.dynamicCast<T>();
    if (Q_LIKELY(!settingsFinal.isNull()))     // This can only happen if the onlinePlugin has a bug.
      return settingsFinal;
  }
  return QSharedPointer<T>();
}

template< class baseTask >
bool onlineJobAdministration::isInherited(const QString& taskIid) const
{
  return (dynamic_cast<baseTask*>(rootOnlineTask(taskIid)) != 0);
}

#if 0
template<class T>
onlineJobTyped<T> onlineJobAdministration::convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation, const QString& onlineJobId) const
{
  onlineJob job = convert(original, convertTaskIid, convertType, userInformation, onlineJobId);
  return onlineJobTyped<T>(job);
}

template<class T>
onlineJobTyped< T > onlineJobAdministration::convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation) const
{
  return convert<T>(original, convertTaskIid, convertType, userInformation, original.id());
}
#endif

#endif // ONLINEJOBADMINISTRATION_H
