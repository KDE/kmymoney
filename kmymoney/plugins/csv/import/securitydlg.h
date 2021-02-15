/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
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
