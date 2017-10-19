/***************************************************************************
                          knewbankdlg.h
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes
    email                : mte@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyinstitution.h"

#include "ui_knewbankdlgdecl.h"


class KNewBankDlgDecl : public QDialog, public Ui::KNewBankDlgDecl
{
public:
  explicit KNewBankDlgDecl(QWidget *parent) : QDialog(parent) {
    setupUi(this);
  }
};

/// This dialog lets the user create or edit an institution
class KNewBankDlg : public KNewBankDlgDecl
{
  Q_OBJECT

public:
  explicit KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent = 0);
  ~KNewBankDlg();
  const MyMoneyInstitution& institution();

protected slots:
  void okClicked();
  void institutionNameChanged(const QString &);

private:
  MyMoneyInstitution m_institution;

};

#endif
