/***************************************************************************
                          kbackupdialog.h  -  description
                             -------------------
    begin                : Mon Jun 4 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBACKUPDLG_H
#define KBACKUPDLG_H

#include <QDialog>
#include "ui_kbackupdlgdecl.h"

/**
  *@author Michael Edwardes
  */


class kbackupdlgdecl : public QDialog, public Ui::kbackupdlgdecl
{
public:
  kbackupdlgdecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

class KBackupDlg : public kbackupdlgdecl
{
  Q_OBJECT
private:
  void readConfig();
  void writeConfig();

protected slots:
  void chooseButtonClicked();

public:
  KBackupDlg(QWidget* parent);
  ~KBackupDlg();
};

#endif // KBACKUPDLG_H
