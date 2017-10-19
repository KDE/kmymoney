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
// Std Includes

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kgeneratesqldlgdecl.h"

class QDialogButtonBox;
class MyMoneyDbDriver;
class MyMoneySeqAccessMgr;
class kMandatoryFieldGroup;
class KGenerateSqlDlgDecl : public QWidget, public Ui::KGenerateSqlDlgDecl
{
public:
  KGenerateSqlDlgDecl() {
    setupUi(this);
  }
};

class KGenerateSqlDlg : public QDialog
{
  Q_OBJECT
public:
  explicit KGenerateSqlDlg(QWidget *parent = 0);
  ~KGenerateSqlDlg();
  /**
   * execute the generation
   */
  int exec();
public slots:
  void slotHelp();
  void slotdriverSelected();
  void slotcreateTables();
  void slotsaveSQL();
private:
  void initializeForm();
  QString selectedDriver();

  KGenerateSqlDlgDecl* m_widget;
  QDialogButtonBox* m_buttonBox;
  QPushButton* m_createTablesButton;
  QPushButton* m_saveSqlButton;

  QList<QString> m_supportedDrivers;
  //MyMoneyDbDrivers m_map;
  std::unique_ptr<kMandatoryFieldGroup> m_requiredFields;
  bool m_sqliteSelected;
  QExplicitlySharedDataPointer<MyMoneyDbDriver> m_dbDriver;
  QString m_dbName;
  MyMoneySeqAccessMgr* m_storage;
  bool m_mustDetachStorage;
};

#endif
