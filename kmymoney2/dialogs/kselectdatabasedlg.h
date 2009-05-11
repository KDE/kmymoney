/***************************************************************************
                          kselectdatabase.h
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield
    author               : Tony Bloomfield
    email                : tonybloom@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSELECTDATABASEDLG_H
#define KSELECTDATABASEDLG_H

// ----------------------------------------------------------------------------
// QT Includes
#include <q3listbox.h>
#include <qlineedit.h>
#include <qlabel.h>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kurl.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "kselectdatabasedlgdecl.h"
#include "../mymoney/storage/mymoneystoragesql.h"
#include "kmymoney/kguiutils.h"

class KSelectDatabaseDlg : public KSelectDatabaseDlgDecl
{
Q_OBJECT
public:
  KSelectDatabaseDlg(QWidget *parent = 0, const char *name = 0);
  KSelectDatabaseDlg(KUrl openURL, QWidget *parent = 0, const char *name = 0);
  ~KSelectDatabaseDlg();
  /** Set the mode of this dialog
    * @param - openMode (IO_ReadWrite = open database; IO_WriteOnly = saveas database)
  **/
  void setMode(int openMode);
  /** Return URL of database
    * @return - pseudo-URL of database selected by user
  **/
  const KUrl selectedURL();

public slots:
  void slotDriverSelected(Q3ListBoxItem *driver);
  void slotHelp();
  void slotGenerateSQL();
private:
  int m_mode;
  MyMoneyDbDrivers m_map;
  kMandatoryFieldGroup* m_requiredFields;
};

#endif
