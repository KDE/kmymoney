/***************************************************************************
                          kbalancechartdlg  -  description
                             -------------------
    begin                : Mon Nov 26 2007
    copyright            : (C) 2007 by Thomas Baumgart
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

#ifndef KBALANCECHARTDLG_H
#define KBALANCECHARTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdialog.h>

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyAccount;

/**
 *	@author Thomas Baumgart <ipwizard@users.sourceforge.net>
 */
class KBalanceChartDlg : public KDialog
{
  Q_OBJECT
  public:
    explicit KBalanceChartDlg(const MyMoneyAccount& account, QWidget* parent = 0, const char* name = 0);
    ~KBalanceChartDlg();

};

#endif
