/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Christian Dávid <christian-david@web.de>
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

#include "onlinejobpluginmockup.h"

#include <QDebug>

#include <KPluginFactory>

#include "mymoneyfile.h"
#include "onlinejobadministration.h"
#include "mymoneyexception.h"

#include "onlinetasks/sepa/sepaonlinetransfer.h"
#include "sepacredittransfersettingsmockup.h"

onlineJobPluginMockup::onlineJobPluginMockup(QObject *parent, const QVariantList &args) :
  OnlinePluginExtended(parent, "onlinejobpluginmockup")
{
  Q_UNUSED(args);
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

QStringList onlineJobPluginMockup::availableJobs(QString accountId)
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
