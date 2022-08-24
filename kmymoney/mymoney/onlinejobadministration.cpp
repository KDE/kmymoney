/*
    SPDX-FileCopyrightText: 2013-2018 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019-2022 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
#include <KJsonUtils>
#include <KPluginMetaData>
#include <KPluginFactory>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneykeyvaluecontainer.h"
#include "onlinejobsmodel.h"
#include "onlinepluginextended.h"

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
    auto plugins = KPluginMetaData::findPlugins("kmymoney_plugins/onlinetasks", [](const KPluginMetaData& data) {
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
    if (task)
        return task->clone();
    return nullptr;
}

onlineTask* onlineJobAdministration::createOnlineTaskByXml(const QString& iid, const QDomElement& element) const
{
    onlineTask* task = rootOnlineTask(iid);
    if (task) {
        return task->createFromXml(element);
    }
    qWarning("In the file is a onlineTask for which I could not find the plugin ('%s')", qPrintable(iid));
    return new unavailableTask(element);
}

/**
 * @internal Using KPluginFactory to create the plugins seemed to be good idea. The drawback is that it does not support to create non QObjects directly.
 * This made this function way longer than needed and adds many checks.
 *
 * @fixme Delete created tasks
 */
onlineTask* onlineJobAdministration::rootOnlineTask(const QString& name) const
{
    auto plugins = KPluginMetaData::findPlugins("kmymoney_plugins/onlinetasks", [&name](const KPluginMetaData& data) {
        QJsonValue array = data.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Iids"];
        if (array.isArray())
            return (array.toVariant().toStringList().contains(name));
        return false;
    });

    if (plugins.isEmpty())
        return nullptr;

    if (plugins.length() != 1)
        qWarning() << "Multiple plugins which offer the online task \"" << name << "\" were found. Loading a random one.";

    auto pluginResult = KPluginFactory::instantiatePlugin<KMyMoneyPlugin::onlineTaskFactory>(plugins.first(), onlineJobAdministration::instance());
    if (!pluginResult) {
        qWarning() << "Could not load plugin for online task " << name << ", file name " << plugins.first().fileName() << ".";
        return nullptr;
    }
    auto taskFactory = pluginResult.plugin;

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

    m_onlineTaskConverter.insert(converter->convertedTask(), converter);
    qDebug() << "onlineTaskConverter available" << converter->convertedTask() << converter->convertibleTasks();
}

onlineJobAdministration::onlineJobEditOffers onlineJobAdministration::onlineJobEdits()
{
    auto plugins = KPluginMetaData::findPlugins("kmymoney_plugins/onlinetasks", [](const KPluginMetaData& data) {
        return !(data.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Editors"].isNull());
    });

    onlineJobAdministration::onlineJobEditOffers list;
    list.reserve(plugins.size());
    for(const KPluginMetaData& data: plugins) {
        QJsonArray editorsArray = data.rawData()["KMyMoney"].toObject()["OnlineTask"].toObject()["Editors"].toArray();
        for(QJsonValue entry: editorsArray) {
            if (!entry.toObject()["OnlineTaskIds"].isNull()) {
                list.append(onlineJobAdministration::onlineJobEditOffer{data.fileName(), KJsonUtils::readTranslatedString(entry.toObject(), "Name")});
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
    /// @todo optimize this loop to move the accounts to the outer loop
    for (KMyMoneyPlugin::OnlinePluginExtended* plugin : qAsConst(*m_onlinePlugins)) {
        QList<MyMoneyAccount> accounts;
        MyMoneyFile::instance()->accountList(accounts, QStringList(), true);
        for (const auto& account : qAsConst(accounts)) {
            if (account.hasOnlineMapping()) {
                for (const auto& onlineTaskIid : plugin->availableJobs(account.id())) {
                    if (m_onlineTasks.contains(onlineTaskIid)) {
                        return true;
                    }
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

    QList<MyMoneyAccount> accounts;
    MyMoneyFile::instance()->accountList(accounts, QStringList(), true);
    for (const auto& account : qAsConst(accounts)) {
        if (account.hasOnlineMapping()) {
            for (const onlineTask* task : qAsConst(m_onlineTasks)) {
                // Check if a online task has the correct type
                if (dynamic_cast<const creditTransfer*>(task) != 0) {
                    for (KMyMoneyPlugin::OnlinePluginExtended* plugin : qAsConst(*m_onlinePlugins)) {
                        if (plugin->availableJobs(account.id()).contains(task->taskName()))
                            return true;
                    }
                }
            }
        }
    }
    return false;
}

bool onlineJobAdministration::canEditOnlineJob(const QString& jobId)
{
    const auto idx = MyMoneyFile::instance()->onlineJobsModel()->indexById(jobId);
    if (idx.isValid()) {
        const auto taskIid = idx.data(eMyMoney::Model::OnlineJobTaskIidRole).toString();
        return (!taskIid.isEmpty() && m_onlineTasks.contains(taskIid));
    }
    return false;
}

void onlineJobAdministration::updateOnlineTaskProperties()
{
    emit canSendAnyTaskChanged(canSendAnyTask());
    emit canSendCreditTransferChanged(canSendCreditTransfer());
}
