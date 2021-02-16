/*
    SPDX-FileCopyrightText: 2010 Allan Anderson <agander93@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
  TransactionDlg(const QStringList& colList, const QStringList& colHeaders, const int typeCol,
              const QList<eMyMoney::Transaction::Action>& validActionTypes);
  ~TransactionDlg();

  Ui::TransactionDlg*   ui;
  QBrush             m_colorBrush;
  QBrush             m_colorBrushText;
  QBrush             m_errorBrush;
  QBrush             m_errorBrushText;
  eMyMoney::Transaction::Action getActionType();

private:
  QPixmap           m_iconYes;
  QPixmap           m_iconNo;

  QPushButton*      m_buttonOK;
  QPushButton*      m_buttonCancel;

  QList<eMyMoney::Transaction::Action> m_validActionTypes;
  QList<eMyMoney::Transaction::Action> m_actionTypes;

  QStringList      m_columnList;

  int              m_typeColumn;

  void             updateWindowSize();

  /**
  * This method displays the transaction line in tableWidget
  */
  void             displayLine(const QStringList& colList, const QStringList& colHeaders, const int typeCol);

  /**
  * This will add appropriate icons to cbActionTypes entries
  */
  void             iconifyActionTypesComboBox(const QList<eMyMoney::Transaction::Action>& validActionTypes);
private Q_SLOTS:
  void             slotActionSelected(int index);
};

#endif // TRANSACTIONDLG_H
