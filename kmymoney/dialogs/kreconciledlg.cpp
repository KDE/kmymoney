/***************************************************************************
                          kreconciledlg.cpp
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

#include "kreconciledlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QResizeEvent>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

// ----------------------------------------------------------------------------
// Project Includes

KReconcileDlg::KReconcileDlg(const MyMoneyMoney /* previousBal */, const MyMoneyMoney /* endingBal */,
  const QDate /* endingDate */, MyMoneyAccount* /*accountIndex */,
  const MyMoneyFile* /* file */, QWidget *parent)
 : KReconcileDlgDecl(parent)
{
  setModal( true );
}

KReconcileDlg::~KReconcileDlg()
{
}

void KReconcileDlg::clearReconcile()
{
}

void KReconcileDlg::loadLists(void)
{
}

void KReconcileDlg::insertTransactions(void)
{
}

void KReconcileDlg::slotDebitSelected(Q3ListViewItem* /* item */, const QPoint&/*p*/, int/* col*/)
{
}

void KReconcileDlg::slotCreditSelected(Q3ListViewItem* /* item */, const QPoint&, int)
{
}

void KReconcileDlg::doDifference(void)
{
}

void KReconcileDlg::finishClicked(void)
{
}

void KReconcileDlg::cancelClicked()
{
}

void KReconcileDlg::resetData(const MyMoneyMoney /* previousBal */, const MyMoneyMoney /* endingBal */, const QDate /* endingDate */, MyMoneyAccount* /* accountIndex */, const MyMoneyFile* /* file */)
{
}

void KReconcileDlg::slotTransactionChanged()
{
}

/** No descriptions */
void KReconcileDlg::reloadLists()
{
}


/** No descriptions */
bool KReconcileDlg::inTransactions(MyMoneyTransaction * /*credittrans */)
{
  return false;
}

void KReconcileDlg::editClicked()
{
}

void KReconcileDlg::resizeEvent(QResizeEvent* /* e */)
{
}

#include "kreconciledlg.moc"
