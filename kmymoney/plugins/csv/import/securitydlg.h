/*
 * Copyright 2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef SECURITYDLG_H
#define SECURITYDLG_H

#include <QDialog>

namespace Ui
{
class SecurityDlg;
}

class QPushButton;
class SecurityDlg : public QDialog
{
  Q_OBJECT

public:
  SecurityDlg();
  ~SecurityDlg();

  Ui::SecurityDlg*   ui;

  /**
  * This method initializes securities combobox.
  */
  void             initializeSecurities(const QString &presetSymbol, const QString &presetName);

  QString          security();
  QString          name();
  QString          symbol();
  int              dontAsk();

private:
  QPushButton*     m_buttonOK;

private Q_SLOTS:
  void             slotIndexChanged(int index);
  void             slotEditingFinished();
};

#endif // SECURITYDLG_H
