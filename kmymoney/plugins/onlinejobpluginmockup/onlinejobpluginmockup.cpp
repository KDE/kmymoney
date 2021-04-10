/*
    SPDX-FileCopyrightText: 2014-2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "onlinejobpluginmockup.h"

#include <QDebug>

#include <KPluginFactory>

#include "mymoneyfile.h"
#include "onlinejobadministration.h"
#include "mymoneyexception.h"

#include "onlinetasks/sepa/sepaonlinetransfer.h"
#include "sepacredittransfersettingsmockup.h"

onlineJobPluginMockup::onlineJobPluginMockup(QObject *parent, const QVariantList &args) :
    OnlinePluginExtended(parent, args)
{
    qDebug("onlineJobPluginMockup should be used during development only!");
}

onlineJobPluginMockup::~onlineJobPluginMockup()
{
}

void onlineJobPluginMockup::protocols(QStringList& protocolList) const
{
    protocolList << QLatin1String("Imaginary debugging protocol");
}

QWidget* onlineJobPluginMockup::accountConfigTab(const MyMoneyAccount&, QString&)
{
    return 0;
}

bool onlineJobPluginMockup::mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings)
{
    Q_UNUSED(acc);

    onlineBankingSettings.setValue("provider", objectName().toLower());
    return true;
}

MyMoneyKeyValueContainer onlineJobPluginMockup::onlineBankingSettings(const MyMoneyKeyValueContainer& current)
{
    MyMoneyKeyValueContainer nextKvp(current);
    nextKvp.setValue("provider", objectName().toLower());
    return nextKvp;
}

bool onlineJobPluginMockup::updateAccount(const MyMoneyAccount& acc, bool moreAccounts)
{
    Q_UNUSED(moreAccounts);
    if (acc.onlineBankingSettings().value("provider").toLower() == objectName().toLower())
        return true;
    return false;
}

QStringList onlineJobPluginMockup::availableJobs(QString accountId) const
{
    try {
        if (MyMoneyFile::instance()->account(accountId).onlineBankingSettings().value("provider").toLower() == objectName().toLower())
            return onlineJobAdministration::instance()->availableOnlineTasks();
    } catch (const MyMoneyException &) {
    }

    return QStringList();
}

IonlineTaskSettings::ptr onlineJobPluginMockup::settings(QString accountId, QString taskName)
{
    try {
        if (taskName == sepaOnlineTransfer::name() && MyMoneyFile::instance()->account(accountId).onlineBankingSettings().value("provider").toLower() == objectName().toLower())
            return IonlineTaskSettings::ptr(new sepaCreditTransferSettingsMockup);
    } catch (const MyMoneyException &) {
    }
    return IonlineTaskSettings::ptr();
}

void onlineJobPluginMockup::sendOnlineJob(QList< onlineJob >& jobs)
{
    foreach (const onlineJob& job, jobs) {
        qDebug() << "Pretend to send: " << job.taskIid() << job.id();
    }
}

K_PLUGIN_FACTORY_WITH_JSON(onlineJobPluginMockupFactory, "onlinejobpluginmockup.json", registerPlugin<onlineJobPluginMockup>();)

#include "onlinejobpluginmockup.moc"
