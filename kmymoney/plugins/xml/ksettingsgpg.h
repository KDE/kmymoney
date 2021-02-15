/*
 * SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KSETTINGSGPG_H
#define KSETTINGSGPG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QShowEvent;

class KSettingsGpgPrivate;
class KSettingsGpg : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsGpg)

public:
  explicit KSettingsGpg(QWidget* parent = nullptr);
  ~KSettingsGpg();

public Q_SLOTS:
  void showEvent(QShowEvent * event) override;

protected Q_SLOTS:
  void slotStatusChanged(bool state);
  void slotIdChanged();
  void slotIdChanged(int idx);
  void slotKeyListChanged();

private:
  KSettingsGpgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsGpg)
};
#endif

