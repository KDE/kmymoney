/*
 * Copyright 2010  Allan Anderson <agander93@gmail.com>
 * Copyright 2016-2017  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
  void             displayLine(const QStringList& colList, const QStringList& colHeaders, const qint8& typeCol);

  /**
  * This will add appropriate icons to cbActionTypes entries
  */
  void             iconifyActionTypesComboBox(const QList<eMyMoney::Transaction::Action>& validActionTypes);
private Q_SLOTS:
  void             slotActionSelected(int index);
};

#endif // TRANSACTIONDLG_H
