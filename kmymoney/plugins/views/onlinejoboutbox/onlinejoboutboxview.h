/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ONLINEJOBOUTBOXVIEW_H
#define ONLINEJOBOUTBOXVIEW_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class KOnlineJobOutboxView;

class OnlineJobOutboxView : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit OnlineJobOutboxView(QObject *parent, const QVariantList &args);
#else
    explicit OnlineJobOutboxView(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~OnlineJobOutboxView();

    void plug(KXMLGUIFactory* guiFactory) final override;
    void unplug() final override;

private:
    KOnlineJobOutboxView* m_view;
};

#endif
