/*
 * Copyright 2005-2010  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

