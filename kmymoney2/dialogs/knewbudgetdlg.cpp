/***************************************************************************
                          knewbudgetdlg.cpp
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

// ----------------------------------------------------------------------------
// QT Includes

#include <qcheckbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <k3listview.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes
#include "knewbudgetdlg.h"

const int KNewBudgetDlg::m_icFutureYears = 5;
const int KNewBudgetDlg::m_icPastYears = 2;

KNewBudgetDlg::KNewBudgetDlg(QWidget* parent, const char *name) :
  KNewBudgetDlgDecl(parent, name)
{
  QStringList slYear;
  QDate dToday = QDate::currentDate();
  int iYear = dToday.year();

  for (int i=0; i<=m_icFutureYears; i++)
    m_cbYear->insertItem( QString::number(iYear++) );

  iYear = dToday.year();
  for (int i=0; i<=m_icFutureYears; i++)
    m_cbYear->insertItem( QString::number(--iYear) );
}

KNewBudgetDlg::~KNewBudgetDlg()
{
}

void KNewBudgetDlg::m_pbCancel_clicked()
{
  reject();
}

void KNewBudgetDlg::m_pbOk_clicked()
{
  // force focus change to update all data
  m_pbOk->setFocus();

  if (m_leBudgetName->displayText().isEmpty())
  {
    KMessageBox::information(this, i18n("Please specify a budget name"));
    m_leBudgetName->setFocus();
    return;
  }

  m_year = m_cbYear->currentText();
  m_name = m_leBudgetName->displayText();

  accept();
}


#include "knewbudgetdlg.moc"
