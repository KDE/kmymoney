/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSCOLORS_H
#define KSETTINGSCOLORS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui {
class KSettingsColors;
}

class KSettingsColors : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KSettingsColors)

public:
    explicit KSettingsColors(QWidget* parent = nullptr);
    ~KSettingsColors();

private Q_SLOTS:
    /**
      * This presets custom colors with system's color scheme
      */
    void slotCustomColorsToggled(bool);

private:
    Ui::KSettingsColors       *ui;
};
#endif

