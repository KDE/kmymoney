/***************************************************************************
redefine.h
-------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : agander93@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#ifndef REDEFINEDLG_H
#define REDEFINEDLG_H

#include <QDialog>
#include <QPushButton>

#include "mymoneymoney.h"
#include "investmentwizardpage.h"
#include "mymoneystatement.h"
#include "ui_redefinedlgdecl.h"

class RedefineDlgDecl : public QWidget, public Ui::RedefineDlgDecl
{
public:
  RedefineDlgDecl() {
    setupUi(this);
  }
};

class RedefineDlg : public QDialog
{
  Q_OBJECT

public:
  RedefineDlg();
  ~RedefineDlg();

  QBrush           m_colorBrush;
  QBrush           m_colorBrushText;
  QBrush           m_errorBrush;
  QBrush           m_errorBrushText;
  QMap<InvestmentPage::columnTypeE, int>   m_colTypeNum;
  QMap<InvestmentPage::columnTypeE, QString> m_colTypeName;

  void             setColumnTypeNumber(QMap<InvestmentPage::columnTypeE, int> &colTypeNum);
  void             setColumnTypeName(QMap<InvestmentPage::columnTypeE, QString> &colTypeName);
  void             setValidActionTypes(const QList<MyMoneyStatement::Transaction::EAction> &validActionTypes);
  void             setColumnList(const QStringList& list);

  /**
  * This method calls buildOkTypeList() to identify the likely valid type,
  * based on the input values.  It then displays the transaction, allowing
  * the user to select the proper activity type.  This selection also is validated.
  */
  MyMoneyStatement::Transaction::EAction askActionType(const QString& info);

signals:

private:
  RedefineDlgDecl* m_widget;

  QPixmap          m_iconYes;
  QPixmap          m_iconNo;

  QPushButton*      m_buttonOK;
  QPushButton*      m_buttonCancel;

  MyMoneyStatement::Transaction::EAction  m_newType;

  QList<MyMoneyStatement::Transaction::EAction> m_validActionTypes;
  QList<MyMoneyStatement::Transaction::EAction> m_typesList;
  QStringList      m_columnList;

  int              m_maxCol;
  int              m_ret;

  /**
  * This method displays the transaction, highlighting the column containing the
  * dubious invewstment transaction type.  It displays a combobox, allowing the
  * user to select the proper type, based on m_validActionTypes.
  */
  void             displayLine(const QString& info);

private slots:
  /**
  * This method is called when the OK button is clicked.  The new investment type
  * is added to the transaction, and signal changedType() is raised to inform
  * InvestmentDlg of the new type.
  */
  void             slotAccepted();

  /**
  * This method is called when the user selects a new investment type, which replaces the
  * original one.  The OK button is then enabled.
  */
  void             slotNewActionSelected(const int& index);

  /**
  * This method is called if the user cancels the dialog, and returns KMessageBox::Cancel
  * (This return is used for compatibility with other InvestmentPage routine returns.)
  */
  void             slotRejected();
};

#endif // REDEFINEDLG_H
