/***************************************************************************
                           securitiesdlg.h
                         --------------------
begin                 : Sun Sept 11 2011
copyright             : (C) 2011 by Allan Anderson
email                 : agander93@gmail.com
copyright             : (C) 2016 by Łukasz Wojnilowicz
email                 : lukasz.wojnilowicz@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

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

private slots:
  void             slotItemChanged(QTableWidgetItem* item);
};

#endif // SECURITIESDLG_H
