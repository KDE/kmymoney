/***************************************************************************
                          ksettingsicons.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz
    email                : lukasz.wojnilowicz@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

