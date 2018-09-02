/*
 * Copyright 2011  Allan Anderson <agander93@gmail.com>
 * Copyright 2016-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#ifndef SECURITIESDLG_H
#define SECURITIESDLG_H

#include  <QDialog>

namespace Ui
{
class SecuritiesDlg;
}

class QTableWidgetItem;
class SecuritiesDlg : public QDialog
{
  Q_OBJECT

public:
  SecuritiesDlg();
  ~SecuritiesDlg();

  Ui::SecuritiesDlg*   ui;

  /**
  * This method displays the security symbol and name. If symbol or name isn't empty
  * it blocks cell from editing respectively
  */
  void             displayLine(const QString symbol, const QString name);

private:
  typedef enum:int {ColumnStatus, ColumnSymbol, ColumnName} columnsE;
  QPushButton*     m_buttonOK;
  int              m_validRowCount;
  int              m_RowCount;

private Q_SLOTS:
  void             slotItemChanged(QTableWidgetItem* item);
};

#endif // SECURITIESDLG_H
