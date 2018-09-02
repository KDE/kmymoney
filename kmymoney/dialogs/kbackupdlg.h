/*
 * Copyright 2001-2003  Michael Edwardes <mte@users.sourceforge.net>
 * Copyright 2001-2002  Felix Rodriguez <frodriguez@users.sourceforge.net>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
