/***************************************************************************
                          kmymoneyfileinfodlg.h  -  description
                             -------------------
    begin                : Sun Oct  9 2005
    copyright            : (C) 2005 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYFILEINFODLG_H
#define KMYMONEYFILEINFODLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "../dialogs/kmymoneyfileinfodlgdecl.h"

/**
  * @author Thomas Baumgart
  */

class KMyMoneyFileInfoDlg : public KMyMoneyFileInfoDlgDecl
{
   Q_OBJECT
public: 
  KMyMoneyFileInfoDlg(QWidget *parent=0, const char *name=0);
  virtual ~KMyMoneyFileInfoDlg();
};

#endif
