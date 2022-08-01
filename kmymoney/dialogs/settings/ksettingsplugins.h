/*
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSPLUGINS_H
#define KSETTINGSPLUGINS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


class  KSettingsPluginsPrivate;
class KSettingsPlugins : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KSettingsPlugins)

public:
    explicit KSettingsPlugins(QWidget* parent = nullptr);
    ~KSettingsPlugins();

public Q_SLOTS:
    void slotResetToDefaults();
    void slotSavePluginConfiguration();

Q_SIGNALS:
    void changed(bool);
    void settingsChanged(const QString &dialogName);

private:
    KSettingsPluginsPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KSettingsPlugins)
};
#endif

