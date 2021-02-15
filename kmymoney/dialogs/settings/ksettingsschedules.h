/*
    SPDX-FileCopyrightText: 2005-2010 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSETTINGSSCHEDULES_H
#define KSETTINGSSCHEDULES_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsSchedulesPrivate;
class KSettingsSchedules : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsSchedules)

public:
  explicit KSettingsSchedules(QWidget* parent = nullptr);
  ~KSettingsSchedules();

public Q_SLOTS:
  void slotResetRegion();

protected Q_SLOTS:
  void slotLoadRegion(const QString &region);
  void slotSetRegion(const QString &region);

protected:
  void loadList();

private:
  KSettingsSchedulesPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsSchedules)
};

#endif
