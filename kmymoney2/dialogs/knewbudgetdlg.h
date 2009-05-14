/***************************************************************************
                          knewbudgetdlg.h
                             -------------------
    begin                : Wed Jan 18 2006
    copyright            : (C) 2000-2004 by Darren Gould
    email                : darren_gould@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KNEWBUDGETDLG_H
#define KNEWBUDGETDLG_H

// ----------------------------------------------------------------------------
// QT Includes

class Q3ListViewItem;

// ----------------------------------------------------------------------------
// KDE Includes
#include <qdatetime.h>
#include <qcombobox.h>
#include <qlineedit.h>
// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewbudgetdlgdecl.h"


class KNewBudgetDlgDecl : public QDialog, public Ui::KNewBudgetDlgDecl
{
public:
  KNewBudgetDlgDecl( QWidget *parent ) : QDialog( parent ) {
    setupUi( this );
  }
};

class KNewBudgetDlg : public KNewBudgetDlgDecl
{
  Q_OBJECT
public:
  KNewBudgetDlg(QWidget* parent);
  ~KNewBudgetDlg();

  QString& getYear() {return m_year;};
  QString& getName() {return m_name;};

public slots:
    virtual void m_pbCancel_clicked();
    virtual void m_pbOk_clicked();

private:
  // the combobox should look m_icNextYears into the future
  static const int m_icFutureYears;
  static const int m_icPastYears;

  QString m_year;
  QString m_name;
};

#endif // KNEWBUDGETDLG_H
