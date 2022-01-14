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
    explicit OnlineJobOutboxView(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~OnlineJobOutboxView();

    void plug(KXMLGUIFactory* guiFactory) final override;
    void unplug() final override;

private:
    KOnlineJobOutboxView* m_view;
};

#endif
