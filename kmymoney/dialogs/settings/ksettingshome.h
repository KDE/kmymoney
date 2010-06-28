/***************************************************************************
                          ksettingshome.h
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

#ifndef KSETTINGSHOME_H
#define KSETTINGSHOME_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingshomedecl.h"

class KSettingsHomeDecl : public QWidget, public Ui::KSettingsHomeDecl
{
public:
  KSettingsHomeDecl(QWidget *parent) : QWidget(parent) {
    setupUi(this);
  }
};


class KSettingsHome : public KSettingsHomeDecl
{
  Q_OBJECT

public:
  KSettingsHome(QWidget* parent = 0);
  ~KSettingsHome();

protected slots:
  void slotLoadItems(void);
  void slotUpdateItemList(void);
  void slotSelectHomePageItem();
  void slotMoveUp(void);
  void slotMoveDown(void);

private:
  bool m_noNeedToUpdateList;
};
#endif

