/***************************************************************************
                           symboltabledlg.h
                         --------------------
begin                 : Sun Sept 11 2011
copyright             : (C) 2011 by Allan Anderson
email                 : agander93@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#ifndef SYMBOLTABLEDLG_H
#define SYMBOLTABLEDLG_H

#include <KDialog>

#include "ui_symboltabledlg.h"

class InvestmentProcessing;
class CSVDialog;

class SymbolTableDlgDecl : public QWidget, public Ui::SymbolTableDlgDecl
{
public:
  SymbolTableDlgDecl() {
    setupUi(this);
  }
};

class SymbolTableDlg : public KDialog
{
  Q_OBJECT

public:
  SymbolTableDlg();
  ~SymbolTableDlg();

  SymbolTableDlgDecl* m_widget;
  CSVDialog*       m_csvDialog;

  QString          m_securityName;

  /**
  * This method displays the investment symbol and name.  If it already exists, this is flagged.
  * If not, the user may edit the name.
  */
  void             displayLine(int& row, QString& symbol, const QString& name, bool& exists);

signals:
  /**
  * This signal is emitted when security names have been edited.
  */
  void             namesEdited();
  void             itemChanged(QTableWidgetItem*);

private:
  int              m_mainHeight;
  int              m_tableHeight;

  QList<QTableWidgetItem*>  m_selectedItems;

private slots:
  /**
  * This method is called when the OK button is clicked.  The edited investment names,
  * if any,  are returned to csvDialog().
  */
  void             slotAccepted();

  /**
  * This method is called if the user cancels the dialog, and returns KMessageBox::Cancel
  * (This return is used for compatibility with other InvestProcessing routine returns.)
  */
  void             slotRejected();

  void             slotItemChanged(QTableWidgetItem* item);

  /**
  * This method is called to enable the user to edit the name of an imported security.
  * On exit, the names are available for return to csvDialog().
  */
  void             slotEditSecurityCompleted();
};

#endif // SYMBOLTABLEDLG_H
