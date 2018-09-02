/*
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

#ifndef KSETTINGSICONS_H
#define KSETTINGSICONS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsIconsPrivate;
class KSettingsIcons : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsIcons)

public:
  explicit KSettingsIcons(QWidget* parent = nullptr);
  ~KSettingsIcons();

public Q_SLOTS:
  void slotResetTheme();

protected Q_SLOTS:
  void slotLoadTheme(const QString &theme);
  void slotSetTheme(const int &theme);

protected:
  void loadList();
private:
  KSettingsIconsPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsIcons)
};
#endif

