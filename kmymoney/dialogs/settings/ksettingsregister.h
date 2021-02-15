/*
    SPDX-FileCopyrightText: 2005-2007 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSREGISTER_H
#define KSETTINGSREGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KSettingsRegister; }

class KSettingsRegister : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsRegister)

public:
  explicit KSettingsRegister(QWidget* parent = nullptr);
  ~KSettingsRegister();

protected Q_SLOTS:
  void slotLoadNormal(const QString& text);
  void slotLoadReconcile(const QString& text);
  void slotLoadSearch(const QString& text);

private:
  Ui::KSettingsRegister *ui;

};
#endif

