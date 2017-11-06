/***************************************************************************
                          ksettingscolors.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowiczd@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSETTINGSCOLORS_H
#define KSETTINGSCOLORS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KSettingsColors; }

class KSettingsColors : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsColors)

public:
  explicit KSettingsColors(QWidget* parent = nullptr);
  ~KSettingsColors();

private slots:
  /**
    * This presets custom colors with system's color scheme
    */
  void slotCustomColorsToggled(bool);

private:
  Ui::KSettingsColors       *ui;
};
#endif

