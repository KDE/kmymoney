/***************************************************************************
                          keditequityentrydlg.h  -  description
                             -------------------
    begin                : Sat Mar 6 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KEDITEQUITYENTRYDLG_H
#define KEDITEQUITYENTRYDLG_H


// ----------------------------------------------------------------------------
// QT Includes
#include <qdialog.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <k3listview.h>
#include <klocale.h>
#include <klineedit.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_keditequityentrydecl.h"
#include "mymoneysecurity.h"

/**
  * @author Kevin Tambascio
  */
class KEditEquityEntryDecl : public QDialog, public Ui::KEditEquityEntryDecl
{
public:
  KEditEquityEntryDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KEditEquityEntryDlg : public KEditEquityEntryDecl
{
  Q_OBJECT
public:
  KEditEquityEntryDlg(const MyMoneySecurity& selectedSecurity, QWidget *parent = NULL);
  ~KEditEquityEntryDlg();

  void updatedEquity(MyMoneySecurity& security) { security = m_selectedSecurity; }

protected slots:
  void slotOKClicked();
  void slotDataChanged(void);
  void slotSelectionChanged(Q3ListViewItem* item);

private slots:
  void slotTimerDone(void);

private:
  MyMoneySecurity m_selectedSecurity;
  bool m_changes;
};

#endif
