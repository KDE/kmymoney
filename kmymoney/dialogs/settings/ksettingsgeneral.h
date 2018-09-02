/*
 * Copyright 2005-2008  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef KSETTINGSGENERAL_H
#define KSETTINGSGENERAL_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsGeneralPrivate;
class KSettingsGeneral : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsGeneral)

public:
  explicit KSettingsGeneral(QWidget* parent = nullptr);
  ~KSettingsGeneral();

protected Q_SLOTS:
  void slotChooseLogPath();
  void slotLoadStartDate(const QDate&);
  void slotUpdateLogTypes();

protected:
  void showEvent(QShowEvent* event) override;

public Q_SLOTS:
  void slotUpdateEquitiesVisibility();

private:
  KSettingsGeneralPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsGeneral)
};
#endif

