/***************************************************************************
                          ksettingsfonts.h
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

#ifndef KSETTINGSFONTS_H
#define KSETTINGSFONTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KSettingsFonts; }

class KSettingsFonts : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsFonts)

public:
  explicit KSettingsFonts(QWidget* parent = nullptr);
  ~KSettingsFonts();

private:
  Ui::KSettingsFonts *ui;
};
#endif

