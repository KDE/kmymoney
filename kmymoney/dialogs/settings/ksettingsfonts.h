/*
 * SPDX-FileCopyrightText: 2005 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KSETTINGSFONTS_H
#define KSETTINGSFONTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KSettingsFonts; }

class KSettingsFonts : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsFonts)

public:
  explicit KSettingsFonts(QWidget* parent = nullptr);
  ~KSettingsFonts();

private:
  Ui::KSettingsFonts *ui;
};
#endif

