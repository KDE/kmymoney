/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEJOBPLUGINMOCKUP_H
#define ONLINEJOBPLUGINMOCKUP_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"
#include "onlinepluginextended.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"

#include "mymoney/onlinejob.h"

/**
 * @short Mockup plugin which offers all online tasks
 */
class onlineJobPluginMockup : public KMyMoneyPlugin::OnlinePluginExtended
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::OnlinePluginExtended
               KMyMoneyPlugin::OnlinePlugin)

public:
  explicit onlineJobPluginMockup(QObject *parent, const QVariantList &args);
  ~onlineJobPluginMockup() override;

  void protocols(QStringList& protocolList) const override;
  QWidget* accountConfigTab(const MyMoneyAccount& account, QString& tabName) override;

  MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) override;
  bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings) override;

  bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts = false) override;

  QStringList availableJobs(QString accountId) const override;
  IonlineTaskSettings::ptr settings(QString accountId, QString taskName) override;
  void sendOnlineJob(QList< onlineJob >& jobs) override;
  void plug() override {}
  void unplug() override {}
};

#endif // ONLINEJOBPLUGINMOCKUP_H
