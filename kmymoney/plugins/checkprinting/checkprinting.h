/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef CHECKPRINTING_H
#define CHECKPRINTING_H

#include <memory>

#include "kmymoneyplugin.h"

class KPluginInfo;
class QObject;
class CheckPrinting : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit CheckPrinting(QObject *parent, const QVariantList &args);
#else
    explicit CheckPrinting(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~CheckPrinting() override;

public Q_SLOTS:
    void updateConfiguration() override;
    void updateActions ( const SelectedObjects& selections ) override;

protected Q_SLOTS:
    void slotPrintCheck();

private:
    struct Private;
    std::unique_ptr<Private> d;
};

#endif

