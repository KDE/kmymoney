/*
    SPDX-FileCopyrightText: 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
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
    explicit ForecastView(QObject *parent, const QVariantList &args);
    ~ForecastView() override;

    void plug(KXMLGUIFactory* guiFactory) override;
    void unplug() override;

private:
    KForecastView* m_view;
};

#endif
