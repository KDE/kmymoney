/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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

