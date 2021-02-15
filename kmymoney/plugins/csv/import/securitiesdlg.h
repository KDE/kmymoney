/*
    SPDX-FileCopyrightText: 2011 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
