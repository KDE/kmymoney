/***************************************************************************
                          ksettingsgpg.h
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

public slots:
  void showEvent(QShowEvent * event) override;

protected slots:
  void slotStatusChanged(bool state);
  void slotIdChanged();
  void slotIdChanged(int idx);
  void slotKeyListChanged();

private:
  KSettingsGpgPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsGpg)
};
#endif

