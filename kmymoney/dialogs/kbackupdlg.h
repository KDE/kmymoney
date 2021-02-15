/*
 * SPDX-FileCopyrightText: 2001-2003 Michael Edwardes <mte@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2001-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
 * SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

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
  bool mountCheckBoxChecked() const;

protected Q_SLOTS:
  void chooseButtonClicked();

private:
  Ui::KBackupDlg *ui;
  void readConfig();
  void writeConfig();
};

#endif // KBACKUPDLG_H
