/***************************************************************************
                          ksettingsplugins.h
                             -------------------
    begin                : Thu Feb 12 2009
    copyright            : (C) 2009 Cristian Onet
    email                : onet.cristian@gmail.com
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

class KSettingsPlugins : public QWidget
{
  Q_OBJECT

public:

  KSettingsPlugins(QWidget* parent = 0);
  ~KSettingsPlugins();

public slots:
  void slotLoadPlugins();
  void slotSavePlugins();
  void slotDefaultsPlugins();
};

#endif // KSETTINGSPLUGINS_H
