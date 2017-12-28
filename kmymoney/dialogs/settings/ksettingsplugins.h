/***************************************************************************
                          ksettingsplugins.h
                             -------------------
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

