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
