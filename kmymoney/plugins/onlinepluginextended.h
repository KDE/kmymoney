/*

    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEPLUGINEXTENDED_H
#define ONLINEPLUGINEXTENDED_H

#include "kmymoneyplugin.h"

#include "mymoney/onlinejob.h"
#include "onlinetasks/interfaces/tasks/ionlinetasksettings.h"

class onlineJob;
class onlineTask;
class payeeIdentifierData;

namespace KMyMoneyPlugin
{

/**
 * @brief Interface between KMyMoney and Online Banking plugins for executing transactions
 *
 * This interface is under active development and will change often! Do not use it at the moment!
 *
 * @author Christian David (christian-david@web.de)
 */
class KMM_PLUGIN_EXPORT OnlinePluginExtended : public Plugin, public OnlinePlugin
{
    Q_OBJECT

public:
    OnlinePluginExtended(QObject* parent, const QVariantList& args);
    virtual ~OnlinePluginExtended() {}

    /**
     * @brief List onlineJobs supported by an account
     *
     * KMyMoney will use this function to ask the online plugin which online jobs it supports.
     * Later changes can be made public using the jobAvailable signals.
     *
     * @return A QStringList with supported onlineTask::name()s as values.
     */
    virtual QStringList availableJobs(QString accountId) const = 0;

    /**
     * @brief Get settings for onlineTask
     *
     * @see onlineTask::settings
     */
    virtual IonlineTaskSettings::ptr settings(QString accountId, QString taskName) = 0;

    /**
     * @brief Send onlineJobs to bank
     *
     * @param jobs Do not delete the onlineJob objects. You can edit them but expect them to be deleted after
     * you returned from this function.
     */
    virtual void sendOnlineJob(QList<onlineJob>& jobs) = 0;

    virtual void plug(KXMLGUIFactory* guiFactory) override = 0;
    virtual void unplug() override = 0;

Q_SIGNALS:
    /**
     * @brief Emit to make onlineJob available
     *
     * In case a onlineJob got available during runtime, emit one of these signals.
     */
    void jobAvailable(QString accountId, QString);
    void jobAvailable(QString accountId, QStringList);
    void jobUnavailable(QString accountId, QString);
    //void jobUnavailable( QString accountId );
};

class KMM_PLUGIN_EXPORT onlineTaskFactory
{
public:
    virtual onlineTask* createOnlineTask(const QString& taskId) const = 0;

    // Make g++ happy
    virtual ~onlineTaskFactory() {}
};

class KMM_PLUGIN_EXPORT payeeIdentifierDataFactory
{
public:
    virtual payeeIdentifierData* createPayeeIdentifier(const QString& payeeIdentifierIid) const = 0;
    // Make g+ happy
    virtual ~payeeIdentifierDataFactory() {}
};

} // namespace KMyMoneyPlugin

Q_DECLARE_INTERFACE(KMyMoneyPlugin::OnlinePluginExtended, "org.kmymoney.plugin.onlinepluginextended");
Q_DECLARE_INTERFACE(KMyMoneyPlugin::onlineTaskFactory, "org.kmymoney.plugin.onlinetaskfactory");
Q_DECLARE_INTERFACE(KMyMoneyPlugin::payeeIdentifierDataFactory, "org.kmymoney.plugin.payeeidentifierfactory");

#endif // ONLINEPLUGINEXTENDED_H
