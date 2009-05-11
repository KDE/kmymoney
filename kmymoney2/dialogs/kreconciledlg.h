/***************************************************************************
                          kreconciledlg.h
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

#ifndef KRECONCILEDLG_H
#define KRECONCILEDLG_H

#include <qlabel.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <k3listview.h>
#include <qpushbutton.h>

#include "../mymoney/mymoneyfile.h"
#include "../mymoney/mymoneymoney.h"

#include "kreconciledlgdecl.h"

// This dialog is used for reconciliation.
class KReconcileDlg : public KReconcileDlgDecl  {
   Q_OBJECT
public: 
	KReconcileDlg(const MyMoneyMoney previousBal, const MyMoneyMoney endingBal, const QDate endingDate, MyMoneyAccount *accountIndex, const MyMoneyFile* file, QWidget *parent=0, const char *name=0);
	~KReconcileDlg();
//  void updateData(void);
  void resetData(const MyMoneyMoney previousBal, const MyMoneyMoney endingBal, const QDate endingDate, MyMoneyAccount *accountIndex, const MyMoneyFile* file);
  /** No descriptions */

protected:
  void resizeEvent(QResizeEvent*);

protected slots:
  void slotDebitSelected(Q3ListViewItem*, const QPoint&, int);
  void slotCreditSelected(Q3ListViewItem*, const QPoint&, int);
  void finishClicked();
  void cancelClicked();
  /** No descriptions */
  void editClicked();

signals:
  void reconcileFinished(bool);

private:
	MyMoneyMoney m_endingBalance;
	MyMoneyMoney m_previousBalance;
	MyMoneyMoney m_clearedBalance;
  MyMoneyMoney m_debitBalance;
  MyMoneyMoney m_creditBalance;
	
	MyMoneyFile* m_file;
	//MyMoneyBank m_bankIndex;
	MyMoneyAccount *m_accountIndex;

  QList<MyMoneyTransaction> m_debitsQList;
  QList<MyMoneyTransaction> m_creditsQList;
  QList<MyMoneyTransaction> m_reconciledTransactions;

  bool m_balanced;  // true when the account is balanced (determined by doDifference)

  QDate m_endingDate;

  void loadLists(void);
  void insertTransactions(void);
  void doDifference(void);
  /** No descriptions */
  bool inTransactions(MyMoneyTransaction *debittrans);
  /** No descriptions */
  bool inDebits(MyMoneyTransaction *transaction);
  /** No descriptions */
  bool inCredits(MyMoneyTransaction *transaction);

  void reloadLists();
  void clearReconcile();

public slots: // Public slots
  /** No descriptions */
  void slotTransactionChanged();
};

#endif
