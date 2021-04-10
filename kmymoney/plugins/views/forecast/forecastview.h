/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2021 Dawid Wróbel <me@dawidwrobel.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FORECASTVIEW_H
#define FORECASTVIEW_H

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// QT Includes

// Project Includes

#include "kmymoneyplugin.h"

class KForecastView;

class ForecastView : public KMyMoneyPlugin::Plugin
{
    Q_OBJECT

public:
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 77, 0)
    explicit ForecastView(QObject *parent, const QVariantList &args);
#else
    explicit ForecastView(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
#endif
    ~ForecastView() override;

    void plug(KXMLGUIFactory* guiFactory) override;
    void unplug() override;

private:
    KForecastView* m_view;
};

#endif
