/***************************************************************************
                          ksettingsschedules.h
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsschedulesdecl.h"

class KSettingsSchedulesDecl : public QWidget, public Ui::KSettingsSchedulesDecl
{
public:
  KSettingsSchedulesDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};


class KSettingsSchedules : public KSettingsSchedulesDecl
{
  Q_OBJECT

public:
  KSettingsSchedules(QWidget* parent = 0);
  ~KSettingsSchedules();

public slots:
  void slotResetRegion();

protected slots:
  void slotLoadRegion(const QString &region);
  void slotSetRegion(const QString &region);

protected:
  void loadList();

private:
  QMap<QString, QString> m_regionMap;
};

#endif
