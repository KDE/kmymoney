/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef GNCIMPORTER_H
#define GNCIMPORTER_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class MyMoneyGncReader;

class GNCImporter : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::StoragePlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::StoragePlugin)

public:
    explicit GNCImporter(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~GNCImporter() override;

    bool open(const QUrl &url) override;
    bool save(const QUrl &url) override;
    bool saveAs() override;

    eKMyMoney::StorageType storageType() const override;
    QString fileExtension() const override;
    QUrl openUrl() const override;
};

#endif
