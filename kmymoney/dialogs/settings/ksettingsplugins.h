/*
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

#ifndef KSETTINGSPLUGINS_H
#define KSETTINGSPLUGINS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KPluginSelector;

class  KSettingsPluginsPrivate;
class KSettingsPlugins : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsPlugins)

public:
  explicit KSettingsPlugins(QWidget* parent = nullptr);
  ~KSettingsPlugins();

public Q_SLOTS:
  void slotResetToDefaults();
  void slotSavePluginConfiguration();

Q_SIGNALS:
  void changed(bool);
  void settingsChanged(const QString &dialogName);

private:
  KSettingsPluginsPrivate* const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsPlugins)

private Q_SLOTS:
  void slotPluginsSelectionChanged(bool changed);
};
#endif

