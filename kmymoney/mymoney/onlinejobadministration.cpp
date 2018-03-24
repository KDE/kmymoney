/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "onlinejobadministration.h"

// ----------------------------------------------------------------------------
// Std Includes

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QDebug>
#include <QPluginLoader>
#include <QJsonArray>


// ----------------------------------------------------------------------------
// KDE Includes
#include <KServiceTypeTrader>
#include <KPluginMetaData>
#include <KService>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneykeyvaluecontainer.h"
#include "plugins/onlinepluginextended.h"

#include "onlinetasks/unavailabletask/tasks/unavailabletask.h"
#include "onlinetasks/interfaces/tasks/credittransfer.h"
#include "tasks/onlinetask.h"

onlineJobAdministration::onlineJobAdministration(QObject *parent)
  : QObject(parent)
  , m_onlinePlugins(nullptr)
  , m_inRegistration(false)
{
}

onlineJobAdministration::~onlineJobAdministration()
{
  clearCaches();
}

onlineJobAdministration* onlineJobAdministration::instance()
{
  static onlineJobAdministration m_instance;
  return &m_instance;
}

void onlineJobAdministration::clearCaches()
{
  qDeleteAll(m_onlineTasks);
  m_onlineTasks.clear();
  qDeleteAll(m_onlineTaskConverter);
  m_onlineTaskConverter.clear();
}

KMyMoneyPlugin::OnlinePluginExtended* onlineJobAdministration::getOnlinePlugin(const QString& accountId) const
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);

  QMap<QString, KMyMoneyPlugin::OnlinePluginExtended*>::const_iterator it_p;
  it_p = m_onlinePlugins->constFind(acc.onlineBankingSettings().value("provider").toLower());

  if (it_p != m_onlinePlugins->constEnd()) {
    // plugin found, use it
    return *it_p;
  }
  return 0;
}

void onlineJobAdministration::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePluginExtended*>& plugins)
{
  m_onlinePlugins = &plugins;
  updateActions();
}

void onlineJobAdministration::updateActions()
{
  emit canSendAnyTaskChanged(canSendAnyTask());
  emit canSendCreditTransferChanged(canSendCreditTransfer());
}

QStringList onlineJobAdministration::availableOnlineTasks()
{
  auto plugins = KPluginLoader::findPlugins("kmymoney", [](const KPluginMetaData& data) {
    return !(data.rawData()["KMyMoney"].toObject()["OnlineTask"].isNull());
  });

  QStringList list;
  for(const KPluginMetaData& plugin: plugins) {
    QJsonValue array = plugin.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Iids"];
    if (array.isArray())
      list.append(array.toVariant().toStringList());
  }
  return list;
}

/**
 * @internal The real work is done here.
 */
bool onlineJobAdministration::isJobSupported(const QString& accountId, const QString& name) const
{
  if (!m_onlinePlugins)
    return false;
  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, *m_onlinePlugins) {
    if (plugin->availableJobs(accountId).contains(name))
      return true;
  }
  return false;
}

bool onlineJobAdministration::isJobSupported(const QString& accountId, const QStringList& names) const
{
  foreach (QString name, names) {
    if (isJobSupported(accountId, name))
      return true;
  }
  return false;
}

bool onlineJobAdministration::isAnyJobSupported(const QString& accountId) const
{
  if (accountId.isEmpty())
    return false;

  if (!m_onlinePlugins)
    return false;

  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, *m_onlinePlugins) {
    if (!(plugin->availableJobs(accountId).isEmpty()))
      return true;
  }
  return false;
}

onlineJob onlineJobAdministration::createOnlineJob(const QString& name, const QString& id) const
{
  return (onlineJob(createOnlineTask(name), id));
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
  qWarning("In the file is a onlineTask for which I could not find the plugin ('%s')", qPrintable(iid));
  return new unavailableTask(element);
}

onlineTask* onlineJobAdministration::createOnlineTaskFromSqlDatabase(const QString& iid, const QString& onlineTaskId, QSqlDatabase connection) const
{
  onlineTask* task = rootOnlineTask(iid);
  if (task != 0)
    return task->createFromSqlDatabase(connection, onlineTaskId);

  qWarning("In the file is a onlineTask for which I could not find the plugin ('%s')", qPrintable(iid));
  return 0;
}

/**
 * @interanl Using KPluginFactory to create the plugins seemed to be good idea. The drawback is that it does not support to create non QObjects directly.
 * This made this function way longer than needed and adds many checks.
 *
 * @fixme Delete created tasks
 */
onlineTask* onlineJobAdministration::rootOnlineTask(const QString& name) const
{
  auto plugins = KPluginLoader::findPlugins("kmymoney", [&name](const KPluginMetaData& data) {
    QJsonValue array = data.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Iids"];
    if (array.isArray())
      return (array.toVariant().toStringList().contains(name));
    return false;
  });

  if (plugins.isEmpty())
    return nullptr;

  if (plugins.length() != 1)
    qWarning() << "Multiple plugins which offer the online task \"" << name << "\" were found. Loading a random one.";

  // Load plugin
  std::unique_ptr<QPluginLoader> loader = std::unique_ptr<QPluginLoader>(new QPluginLoader{plugins.first().fileName()});
  QObject* plugin = loader->instance();
  if (!plugin) {
    qWarning() << "Could not load plugin for online task \"" << name << "\", file name \"" << plugins.first().fileName() << "\".";
    return nullptr;
  }

  // Cast to KPluginFactory
  KPluginFactory* pluginFactory = qobject_cast< KPluginFactory* >(plugin);
  if (!pluginFactory) {
    qWarning() << "Could not create plugin factory for online task \"" << name << "\", file name \"" << plugins.first().fileName() << "\".";
    return nullptr;
  }

  // Create onlineTaskFactory
  const QString pluginKeyword = plugins.first().rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["PluginKeyword"].toString();
  // Can create only objects which inherit from QObject directly
  QObject* taskFactoryObject = pluginFactory->create<QObject>(pluginKeyword, onlineJobAdministration::instance());
  KMyMoneyPlugin::onlineTaskFactory* taskFactory = qobject_cast< KMyMoneyPlugin::onlineTaskFactory* >(taskFactoryObject);
  if (!taskFactory) {
    qWarning() << "Could not create online task factory for online task \"" << name << "\", file name \"" << plugins.first().fileName() << "\".";
    return nullptr;
  }

  // Finally create task
  onlineTask* task = taskFactory->createOnlineTask(name);
  if (task)
    // Add to our cache as this is still used in several places
    onlineJobAdministration::instance()->registerOnlineTask(taskFactory->createOnlineTask(name));

  return task;
}

onlineTaskConverter::convertType onlineJobAdministration::canConvert(const QString& originalTaskIid, const QString& convertTaskIid) const
{
  return canConvert(originalTaskIid, QStringList(convertTaskIid));
}

onlineTaskConverter::convertType onlineJobAdministration::canConvert(const QString& originalTaskIid, const QStringList& convertTaskIids) const
{
  Q_ASSERT(false);
  //! @todo Make alive
  onlineTaskConverter::convertType bestConvertType = onlineTaskConverter::convertImpossible;
#if 0
  foreach (QString destinationName, destinationNames) {
    onlineTask::convertType type = canConvert(original, destinationName);
    if (type == onlineTask::convertionLossy)
      bestConvertType = onlineTask::convertionLossy;
    else if (type == onlineTask::convertionLoseless)
      return onlineTask::convertionLoseless;
  }
#else
  Q_UNUSED(originalTaskIid);
  Q_UNUSED(convertTaskIids);
#endif
  return bestConvertType;
}

/**
 * @todo if more than one converter offers the convert, use best
 */
onlineJob onlineJobAdministration::convert(const onlineJob& original, const QString& convertTaskIid, onlineTaskConverter::convertType& convertType, QString& userInformation, const QString& onlineJobId) const
{
  onlineJob newJob;

  QList<onlineTaskConverter*> converterList = m_onlineTaskConverter.values(convertTaskIid);
  foreach (onlineTaskConverter* converter, converterList) {
    if (converter->convertibleTasks().contains(original.taskIid())) {
      onlineTask* task = converter->convert(*original.task(), convertType, userInformation);
      Q_ASSERT_X(convertType != onlineTaskConverter::convertImpossible || task != 0, qPrintable("converter for " + converter->convertedTask()), "Converter returned convertType 'impossible' but return was not null_ptr.");
      if (task != 0) {
        newJob = onlineJob(task, onlineJobId);
        break;
      }
    }
  }

  return newJob;
}

onlineJob onlineJobAdministration::convertBest(const onlineJob& original, const QStringList& convertTaskIids, onlineTaskConverter::convertType& convertType, QString& userInformation) const
{
  return convertBest(original, convertTaskIids, convertType, userInformation, original.id());
}

onlineJob onlineJobAdministration::convertBest(const onlineJob& original, const QStringList& convertTaskIids, onlineTaskConverter::convertType& bestConvertType, QString& bestUserInformation, const QString& onlineJobId) const
{
  onlineJob bestConvert;
  bestConvertType = onlineTaskConverter::convertImpossible;
  bestUserInformation = QString();

  foreach (QString taskIid, convertTaskIids) {
    // Try convert
    onlineTaskConverter::convertType convertType = onlineTaskConverter::convertImpossible;
    QString userInformation;
    onlineJob convertJob = convert(original, taskIid, convertType, userInformation, onlineJobId);

    // Check if it was successful
    if (bestConvertType < convertType) {
      bestConvert = convertJob;
      bestUserInformation = userInformation;
      bestConvertType = convertType;
      if (convertType == onlineTaskConverter::convertionLoseless)
        break;
    }
  }

  return bestConvert;
}

void onlineJobAdministration::registerAllOnlineTasks()
{
  // avoid recursive entrance
  if (m_inRegistration)
    return;

  m_inRegistration = true;
  QStringList availableTasks = availableOnlineTasks();
  foreach (const auto& name, availableTasks) {
    onlineTask* const task = rootOnlineTask(name);
    Q_UNUSED(task);
  }
  m_inRegistration = false;
}

void onlineJobAdministration::registerOnlineTask(onlineTask *const task)
{
  if (Q_UNLIKELY(task == 0))
    return;

  const bool sendAnyTask = canSendAnyTask();
  const bool sendCreditTransfer = canSendCreditTransfer();

  m_onlineTasks.insert(task->taskName(), task);

  if (sendAnyTask != canSendAnyTask())
    emit canSendAnyTaskChanged(!sendAnyTask);
  if (sendCreditTransfer != canSendCreditTransfer())
    emit canSendCreditTransferChanged(!sendCreditTransfer);
}

void onlineJobAdministration::registerOnlineTaskConverter(onlineTaskConverter* const converter)
{
  if (Q_UNLIKELY(converter == 0))
    return;

  m_onlineTaskConverter.insertMulti(converter->convertedTask(), converter);
  qDebug() << "onlineTaskConverter available" << converter->convertedTask() << converter->convertibleTasks();
}

onlineJobAdministration::onlineJobEditOffers onlineJobAdministration::onlineJobEdits()
{
  auto plugins = KPluginLoader::findPlugins("kmymoney", [](const KPluginMetaData& data) {
    return !(data.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Editors"].isNull());
  });

  onlineJobAdministration::onlineJobEditOffers list;
  list.reserve(plugins.size());
  for(const KPluginMetaData& data: plugins) {
    QJsonArray editorsArray = data.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Editors"].toArray();
    for(QJsonValue entry: editorsArray) {
      if (!entry.toObject()["OnlineTaskIds"].isNull()) {
        list.append(onlineJobAdministration::onlineJobEditOffer{
            data.fileName(),
            entry.toObject()["PluginKeyword"].toString(),
            KPluginMetaData::readTranslatedString(entry.toObject(), "Name")
          });
      }
    }
  }
  return list;
}

IonlineTaskSettings::ptr onlineJobAdministration::taskSettings(const QString& taskName, const QString& accountId) const
{
  KMyMoneyPlugin::OnlinePluginExtended* plugin = getOnlinePlugin(accountId);
  if (plugin != 0)
    return (plugin->settings(accountId, taskName));
  return IonlineTaskSettings::ptr();
}

bool onlineJobAdministration::canSendAnyTask()
{
  if (!m_onlinePlugins)
    return false;

  if (m_onlineTasks.isEmpty()) {
    registerAllOnlineTasks();
  }

  // Check if any plugin supports a loaded online task
  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, *m_onlinePlugins) {
    QList<MyMoneyAccount> accounts;
    MyMoneyFile::instance()->accountList(accounts, QStringList(), true);
    foreach (MyMoneyAccount account, accounts) {
      foreach (QString onlineTaskIid, plugin->availableJobs(account.id())) {
        if (m_onlineTasks.contains(onlineTaskIid)) {
          return true;
        }
      }
    }
  }
  return false;
}

bool onlineJobAdministration::canSendCreditTransfer()
{
  if (!m_onlinePlugins)
    return false;

  if (m_onlineTasks.isEmpty()) {
    registerAllOnlineTasks();
  }

  foreach (onlineTask* task, m_onlineTasks) {
    // Check if a online task has the correct type
    if (dynamic_cast<creditTransfer*>(task) != 0) {
      foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, *m_onlinePlugins) {
        QList<MyMoneyAccount> accounts;
        MyMoneyFile::instance()->accountList(accounts, QStringList(), true);
        foreach (MyMoneyAccount account, accounts) {
          if (plugin->availableJobs(account.id()).contains(task->taskName()))
            return true;
        }
      }
    }
  }
  return false;
}

//! @fixme plugin query
bool onlineJobAdministration::canEditOnlineJob(const onlineJob& job)
{
  return (!job.taskIid().isEmpty() && !KServiceTypeTrader::self()->query(QLatin1String("KMyMoney/OnlineTaskUi"), QString("'%1' ~in [X-KMyMoney-onlineTaskIds]").arg(job.taskIid())).isEmpty());
}

void onlineJobAdministration::updateOnlineTaskProperties()
{
  emit canSendAnyTaskChanged(canSendAnyTask());
  emit canSendCreditTransferChanged(canSendCreditTransfer());
}
