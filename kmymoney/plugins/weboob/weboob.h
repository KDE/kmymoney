/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014-2015 Romain Bignon <romain@symlink.me>
 * Copyright (C) 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
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

#ifndef WEBOOB_HPP
#define WEBOOB_HPP

#include <memory>

#ifdef HAVE_CONFIG_H
#include <config-kmymoney.h>
#endif

#include "kmymoneyplugin.h"
#include "mymoneyaccount.h"
#include "mymoneykeyvaluecontainer.h"
#include "weboobext.h"

class Weboob : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::OnlinePlugin
{
  Q_OBJECT
  Q_INTERFACES(KMyMoneyPlugin::OnlinePlugin)

public:
  WeboobExt weboob;
  explicit Weboob(QObject *parent, const QVariantList &args);
  ~Weboob() override;

  void plug() override;
  void unplug() override;

  void protocols(QStringList& protocolList) const override;

  QWidget* accountConfigTab(const MyMoneyAccount& account, QString& tabName) override;

  MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) override;

  bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings) override;

  bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts = false) override;

protected Q_SLOTS:

  void gotAccount();

private:

  struct Private;
  /// \internal d-pointer instance.
  const std::unique_ptr<Private> d;
};

#endif /* WEBOOB_PLUGIN_HPP */
