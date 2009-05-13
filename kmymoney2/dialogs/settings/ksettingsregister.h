/***************************************************************************
                          ksettingsregister.h
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

#ifndef KSETTINGSREGISTER_H
#define KSETTINGSREGISTER_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_ksettingsregisterdecl.h"

class KSettingsRegisterDecl : public QWidget, public Ui::KSettingsRegisterDecl
{
public:
  KSettingsRegisterDecl( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};


class KSettingsRegister : public KSettingsRegisterDecl
{
  Q_OBJECT

public:
  KSettingsRegister(QWidget* parent = 0);
  ~KSettingsRegister();

protected slots:
  void slotLoadNormal(void);
  void slotLoadReconcile(void);
  void slotLoadSearch(void);
};
#endif

