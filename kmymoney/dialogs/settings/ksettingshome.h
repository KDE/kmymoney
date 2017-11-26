/***************************************************************************
                          ksettingshome.h
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

#ifndef KSETTINGSHOME_H
#define KSETTINGSHOME_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsHomePrivate;
class KSettingsHome : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsHome)

public:
  explicit KSettingsHome(QWidget* parent = nullptr);
  ~KSettingsHome();

protected Q_SLOTS:
  void slotLoadItems();
  void slotUpdateItemList();
  void slotSelectHomePageItem();
  void slotMoveUp();
  void slotMoveDown();

private:
  KSettingsHomePrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsHome)
};
#endif

