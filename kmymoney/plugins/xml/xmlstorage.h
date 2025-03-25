/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-FileCopyrightText: 2025 Thomas Baumgart <tbaumgart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XMLSTORAGE_H
#define XMLSTORAGE_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

#include <QLockFile>
#include <QUrl>

// Project Includes

#include "kmymoneyplugin.h"

class QIODevice;

class MyMoneyStorageMgr;
class MyMoneyXmlWriter;

class XMLStoragePrivate;
class XMLStorage : public KMyMoneyPlugin::Plugin, public KMyMoneyPlugin::StoragePlugin
{
    Q_OBJECT
    Q_INTERFACES(KMyMoneyPlugin::StoragePlugin)

public:
    explicit XMLStorage(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~XMLStorage() override;

    bool open(const QUrl &url) override;
    void close() override;
    bool save(const QUrl &url) override;
    bool saveAs() override;
    eKMyMoney::StorageType storageType() const override;
    QString fileExtension() const override;
    QUrl openUrl() const override;
    QString openErrorMessage() const override;

private:
    XMLStoragePrivate* d;
};

#endif
