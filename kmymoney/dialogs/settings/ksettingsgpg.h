/***************************************************************************
                          ksettingsgpg.h
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

#ifndef KSETTINGSGPG_H
#define KSETTINGSGPG_H

// ----------------------------------------------------------------------------
// QT Includes

class QShowEvent;

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsgpgdecl.h"

class KSettingsGpgDecl : public QWidget, public Ui::KSettingsGpgDecl
{
public:
  KSettingsGpgDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};

class KSettingsGpg : public KSettingsGpgDecl
{
  Q_OBJECT

public:
  KSettingsGpg(QWidget* parent = 0);
  ~KSettingsGpg();

public slots:
  void showEvent(QShowEvent * event);

protected slots:
  void slotStatusChanged(bool state);
  void slotIdChanged();
  void slotKeyListChanged();

private:
  int        m_checkCount;
  bool       m_needCheckList;
  bool       m_listOk;
};
#endif

