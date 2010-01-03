/***************************************************************************
                          kgeneratesql.h
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

#ifndef KGENERATESQLDLG_H
#define KGENERATESQLDLG_H

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
#include "ui_kgeneratesqldlgdecl.h"
#include "storage/mymoneystoragesql.h"
#include "storage/mymoneyseqaccessmgr.h"
#include "kguiutils.h"

class KGenerateSqlDlgDecl : public QWidget, public Ui::KGenerateSqlDlgDecl
{
public:
  KGenerateSqlDlgDecl() {
    setupUi( this );
  }
};

class KGenerateSqlDlg : public KDialog
{
Q_OBJECT
public:
  explicit KGenerateSqlDlg(QWidget *parent = 0);
  ~KGenerateSqlDlg();
  /**
    * execute the generation
  **/
  int exec();
public slots:
  void slotHelp();
  void slotdriverSelected();
  void slotcreateTables();
  void slotsaveSQL();
private:
  void initializeForm();

  KGenerateSqlDlgDecl* m_widget;
  QList<QString> m_supportedDrivers;
  MyMoneyDbDrivers m_map;
  kMandatoryFieldGroup* m_requiredFields;
  bool m_sqliteSelected;
  QString m_dbDriver;
  QString m_dbName;
  databaseTypeE m_dbType;
  MyMoneySeqAccessMgr* m_storage;
  bool m_mustDetachStorage;
};

#endif
