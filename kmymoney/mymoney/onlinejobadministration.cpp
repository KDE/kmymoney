/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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
// QT Includes

#include <QtCore/QList>
#include <QtCore/QDebug>
#include <QtCore/QScopedPointer>


// ----------------------------------------------------------------------------
// KDE Includes
#include <KServiceTypeTrader>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoney/mymoneyfile.h"
#include "mymoney/mymoneyaccount.h"
#include "mymoney/mymoneykeyvaluecontainer.h"
#include "plugins/onlinepluginextended.h"

#include "onlinetasks/unavailabletask/tasks/unavailabletask.h"
#include "onlinetasks/interfaces/tasks/credittransfer.h"

onlineJobAdministration::onlineJobAdministration(QObject *parent) :
    QObject(parent)
{
}

onlineJobAdministration::~onlineJobAdministration()
{
// Will be done somewhere else
//  qDeleteAll(m_onlinePlugins);
  qDeleteAll(m_onlineTasks);
  qDeleteAll(m_onlineTaskConverter);
}

KMyMoneyPlugin::OnlinePluginExtended* onlineJobAdministration::getOnlinePlugin(const QString& accountId) const
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(accountId);

  QMap<QString, KMyMoneyPlugin::OnlinePluginExtended*>::const_iterator it_p;
  it_p = m_onlinePlugins.constFind(acc.onlineBankingSettings().value("provider"));

  if (it_p != m_onlinePlugins.constEnd()) {
    // plugin found, use it
    return *it_p;
  }
  return 0;
}

void onlineJobAdministration::addPlugin(const QString& pluginName, KMyMoneyPlugin::OnlinePluginExtended *plugin)
{
  const bool sendAnyTask = canSendAnyTask();
  const bool sendCreditTransfer = canSendCreditTransfer();

  m_onlinePlugins.insert(pluginName, plugin);

  if (!sendAnyTask && canSendAnyTask())
    emit canSendAnyTaskChanged(true);
  if (!sendCreditTransfer && canSendCreditTransfer())
    emit canSendCreditTransferChanged(true);
}

QStringList onlineJobAdministration::availableOnlineTasks()
{
  QStringList list;
  foreach (onlineTask* task, m_onlineTasks) {
    list.append(task->taskName());
  }
  return list;
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

  foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, m_onlinePlugins) {
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

/**
 * @TODO Need a technique to handle tasks were the plugin was not loaded.
 * There could be a new dummy task which is linked statically. If the original task could not be loaded the dummy is used.
 */
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

onlineTask* onlineJobAdministration::rootOnlineTask(const QString& name) const
{
  return m_onlineTasks.value(name);
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

void onlineJobAdministration::registerOnlineTask(onlineTask *const task)
{
  if (Q_UNLIKELY(task == 0))
    return;

  const bool sendAnyTask = canSendAnyTask();
  const bool sendCreditTransfer = canSendCreditTransfer();

  m_onlineTasks.insert(task->taskName(), task);
  qDebug() << "onlineTask available" << task->taskName();

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
  return KServiceTypeTrader::self()->query(QLatin1String("KMyMoney/OnlineTaskUi"));
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
  QList<MyMoneyAccount> accounts;
  MyMoneyFile::instance()->accountList(accounts, QStringList(), true);
  foreach (MyMoneyAccount account, accounts) {
    if (account.hasOnlineMapping()) {
      // Check if any plugin supports a loaded online task
      foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, m_onlinePlugins) {
        foreach (QString onlineTaskIid, plugin->availableJobs(account.id())) {
          if (m_onlineTasks.contains(onlineTaskIid))
            return true;
        }
      }
    }
  }
  return false;
}

bool onlineJobAdministration::canSendCreditTransfer()
{
  QList<MyMoneyAccount> accounts;
  MyMoneyFile::instance()->accountList(accounts, QStringList(), true);
  foreach(MyMoneyAccount account, accounts) {
    if (account.hasOnlineMapping()) {
      foreach (onlineTask* task, m_onlineTasks) {
        // Check if a online task has the correct type
        if (!dynamic_cast<const creditTransfer*>(task))
          continue;
        foreach (KMyMoneyPlugin::OnlinePluginExtended* plugin, m_onlinePlugins) {
          if (plugin->availableJobs(account.id()).contains(task->taskName()))
            return true;
        }
      }
    }
  }
  return false;
}

bool onlineJobAdministration::canEditOnlineJob(const onlineJob& job)
{
  return (!job.taskIid().isEmpty() && !KServiceTypeTrader::self()->query(QLatin1String("KMyMoney/OnlineTaskUi"), QString("'%1' ~in [X-KMyMoney-onlineTaskIds]").arg(job.taskIid())).isEmpty());
}

void onlineJobAdministration::updateOnlineTaskProperties()
{
  emit canSendAnyTaskChanged(canSendAnyTask());
  emit canSendCreditTransferChanged(canSendCreditTransfer());
}
