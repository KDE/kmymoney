/***************************************************************************
                          kselectdatabase.h
                             -------------------
    copyright            : (C) 2005 by Tony Bloomfield <tonybloom@users.sourceforge.net>

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
#include <QLineEdit>
#include <QLabel>

// ----------------------------------------------------------------------------
// KDE Includes
#include <kurl.h>
#include <kdialog.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "ui_kselectdatabasedlgdecl.h"
#include "storage/mymoneystoragesql.h"
#include "kguiutils.h"


class KSelectDatabaseDlgDecl : public QWidget, public Ui::KSelectDatabaseDlgDecl
{
public:
  KSelectDatabaseDlgDecl() {
    setupUi( this );
  }
};

class KSelectDatabaseDlg : public KDialog
{
Q_OBJECT
public:
  explicit KSelectDatabaseDlg(int openMode, KUrl openURL = KUrl(), QWidget *parent = 0);
  ~KSelectDatabaseDlg();
  /**
    * Check whether we have required database drivers
    * @return - false, no drivers available, true, can proceed
  **/
  bool checkDrivers();
  /** Return URL of database
    * @return - pseudo-URL of database selected by user
  **/
  const KUrl selectedURL();
  /** Execute the database selection dialog
    * @return - as QDialog::exec()
  **/
  int exec();
public slots:
  void slotDriverSelected(QListWidgetItem *driver);
  void slotHelp();
  void slotGenerateSQL();
private:
  KSelectDatabaseDlgDecl* m_widget;
  int m_mode;
  KUrl m_url;
  QList<QString> m_supportedDrivers;
  MyMoneyDbDrivers m_map;
  kMandatoryFieldGroup* m_requiredFields;
};

#endif
