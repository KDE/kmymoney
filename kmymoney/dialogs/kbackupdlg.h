/***************************************************************************
                          kbackupdialog.h  -  description
                             -------------------
    begin                : Mon Jun 4 2001
    copyright            : (C) 2001 by Michael Edwardes
    email                : mte@users.sourceforge.net
                             Javier Campos Morales <javi_c@ctv.es>
                             Felix Rodriguez <frodriguez@mail.wesleyan.edu>
                             (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

namespace Ui { class KBackupDlg; }

/**
  *@author Michael Edwardes
  */

class KBackupDlg : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(KBackupDlg)

public:
  explicit KBackupDlg(QWidget* parent = nullptr);
  ~KBackupDlg();

  QString mountPoint() const;
  bool mountCheckBox() const;

protected Q_SLOTS:
  void chooseButtonClicked();

private:
  Ui::KBackupDlg *ui;
  void readConfig();
  void writeConfig();
};

#endif // KBACKUPDLG_H
