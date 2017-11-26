/***************************************************************************
transactiondlg.h
-------------------
begin                 : Sat Jan 01 2010
copyright             : (C) 2010 by Allan Anderson
email                 : agander93@gmail.com
copyright             : (C) 2016 by Łukasz Wojniłowicz
email                 : lukasz.wojnilowicz@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

#ifndef TRANSACTIONDLG_H
#define TRANSACTIONDLG_H

#include <QDialog>

#include "mymoneystatement.h"

namespace Ui
{
class TransactionDlg;
}

class TransactionDlg : public QDialog
{
  Q_OBJECT

public:
  TransactionDlg(const QStringList& colList, const QStringList& colHeaders, const qint8& typeCol,
              const QList<MyMoneyStatement::Transaction::EAction>& validActionTypes);
  ~TransactionDlg();

  Ui::TransactionDlg*   ui;
  QBrush             m_colorBrush;
  QBrush             m_colorBrushText;
  QBrush             m_errorBrush;
  QBrush             m_errorBrushText;
  MyMoneyStatement::Transaction::EAction getActionType();

private:
  QPixmap           m_iconYes;
  QPixmap           m_iconNo;

  QPushButton*      m_buttonOK;
  QPushButton*      m_buttonCancel;

  QList<MyMoneyStatement::Transaction::EAction> m_validActionTypes;
  QList<MyMoneyStatement::Transaction::EAction> m_actionTypes;

  QStringList      m_columnList;

  int              m_typeColumn;

  void             updateWindowSize();

  /**
  * This method displays the transaction line in tableWidget
  */
  void             displayLine(const QStringList& colList, const QStringList& colHeaders, const qint8& typeCol);

  /**
  * This will add appropriate icons to cbActionTypes entries
  */
  void             iconifyActionTypesComboBox(const QList<MyMoneyStatement::Transaction::EAction>& validActionTypes);
private Q_SLOTS:
  void             slotActionSelected(int index);
};

#endif // TRANSACTIONDLG_H
