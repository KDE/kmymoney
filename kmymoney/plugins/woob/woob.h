/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WOOB_H
#define WOOB_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

class WoobPrivate;
class Woob : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::OnlinePlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::OnlinePlugin)

public:
    explicit Woob(QObject *parent, const QVariantList &args);
    ~Woob() override;

    void plug() override;
    void unplug() override;

    void protocols(QStringList& protocolList) const override;

    QWidget* accountConfigTab(const MyMoneyAccount& account, QString& tabName) override;

    MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) override;

    bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings) override;

    bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts = false) override;

private:
    Q_DECLARE_PRIVATE(Woob)
    WoobPrivate * const d_ptr;

private Q_SLOTS:
    void gotAccount();
};

#endif
