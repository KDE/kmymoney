/*
    SPDX-FileCopyrightText: 2014-2015 Romain Bignon <romain@symlink.me>
    SPDX-FileCopyrightText: 2014-2015 Florent Fourcot <weboob@flo.fourcot.fr>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WEBOOB_H
#define WEBOOB_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyAccount;
class MyMoneyKeyValueContainer;

class WeboobPrivate;
class Weboob : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::OnlinePlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::OnlinePlugin)

public:
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit Weboob(QObject *parent, const QVariantList &args);
#else
    explicit Weboob(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~Weboob() override;

    void plug(KXMLGUIFactory* guiFactory) override;
    void unplug() override;

    void protocols(QStringList& protocolList) const override;

    QWidget* accountConfigTab(const MyMoneyAccount& account, QString& tabName) override;

    MyMoneyKeyValueContainer onlineBankingSettings(const MyMoneyKeyValueContainer& current) override;

    bool mapAccount(const MyMoneyAccount& acc, MyMoneyKeyValueContainer& onlineBankingSettings) override;

    bool updateAccount(const MyMoneyAccount& acc, bool moreAccounts = false) override;

private:
    Q_DECLARE_PRIVATE(Weboob)
    WeboobPrivate * const d_ptr;

private Q_SLOTS:
    void gotAccount();
};

#endif
