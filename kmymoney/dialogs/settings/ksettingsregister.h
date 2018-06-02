/*
 * Copyright 2005-2007  Thomas Baumgart <tbaumgart@kde.org>
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

