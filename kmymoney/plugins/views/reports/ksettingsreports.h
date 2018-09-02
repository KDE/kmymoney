/***************************************************************************
                          ksettingsreports.h
                             -------------------
    copyright            : (C) 2010 by Bernd Gonsior
    email                : bernd.gonsior@googlemail.com
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

#ifndef KSETTINGSREPORTS_H
#define KSETTINGSREPORTS_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KSettingsReportsPrivate;
class KSettingsReports : public QWidget
{
  Q_OBJECT
  Q_DISABLE_COPY(KSettingsReports)

public:
  explicit KSettingsReports(QWidget* parent = nullptr);
  ~KSettingsReports();

protected Q_SLOTS:
  void slotCssUrlSelected(const QUrl&);
  void slotEditingFinished();

private:
  KSettingsReportsPrivate * const d_ptr;
  Q_DECLARE_PRIVATE(KSettingsReports)
};
#endif
