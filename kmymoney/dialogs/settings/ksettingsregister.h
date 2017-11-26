/***************************************************************************
                          ksettingsregister.h
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

#ifndef KSETTINGSREGISTER_H
#define KSETTINGSREGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

namespace Ui { class KSettingsRegister; }

class KSettingsRegister : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsRegister)

public:
  explicit KSettingsRegister(QWidget* parent = nullptr);
  ~KSettingsRegister();

protected Q_SLOTS:
  void slotLoadNormal(const QString& text);
  void slotLoadReconcile(const QString& text);
  void slotLoadSearch(const QString& text);

private:
  Ui::KSettingsRegister *ui;

};
#endif

