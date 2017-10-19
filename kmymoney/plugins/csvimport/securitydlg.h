/***************************************************************************
                           securitydlg.h
                         --------------------
begin                 : Sat Jan 15 2017
copyright             : (C) 2017 by Łukasz Wojnilowicz
email                 : lukasz.wojnilowicz@gmail.com
****************************************************************************/

/***************************************************************************

*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published      *
*   by the Free Software Foundation; either version 2 of the License,      *
*   or  (at your option) any later version.                                *
*                                                                          *
****************************************************************************/

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

private slots:
  void             slotIndexChanged(int index);
  void             slotEditingFinished();
};

#endif // SECURITYDLG_H
