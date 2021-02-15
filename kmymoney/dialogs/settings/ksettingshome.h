/*
    SPDX-FileCopyrightText: 2005-2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSHOME_H
#define KSETTINGSHOME_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsHomePrivate;
class KSettingsHome : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsHome)

public:
  explicit KSettingsHome(QWidget* parent = nullptr);
  ~KSettingsHome();

protected Q_SLOTS:
  void slotLoadItems();
  void slotUpdateItemList();
  void slotSelectHomePageItem();
  void slotMoveUp();
  void slotMoveDown();

private:
  KSettingsHomePrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsHome)
};
#endif

