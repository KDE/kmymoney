/*
    SPDX-FileCopyrightText: 2005 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ksettingsfonts.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsfonts.h"

KSettingsFonts::KSettingsFonts(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::KSettingsFonts)
{
  ui->setupUi(this);
}

KSettingsFonts::~KSettingsFonts()
{
  delete ui;
}
