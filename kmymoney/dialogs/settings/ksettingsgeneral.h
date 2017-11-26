/***************************************************************************
                          ksettingsgeneral.h
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

