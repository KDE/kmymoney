/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ksettingscolors.h"
#include "kmymoneysettings.h"

#include "ui_ksettingscolors.h"

KSettingsColors::KSettingsColors(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::KSettingsColors)
{
    ui->setupUi(this);
}

KSettingsColors::~KSettingsColors()
{
    delete ui;
}
