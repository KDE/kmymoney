/***************************************************************************
                          ksettingsschedules.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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
