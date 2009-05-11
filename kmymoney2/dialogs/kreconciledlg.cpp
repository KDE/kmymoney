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

#include <kmessagebox.h>
#include <kglobal.h>
#include <klocale.h>
#if QT_VERSION > 300
#include <kstandarddirs.h>
#else
#include <kstandarddirs.h>
#endif


#include "kreconciledlg.h"
//Added by qt3to4:
#include <QResizeEvent>

KReconcileDlg::KReconcileDlg(const MyMoneyMoney /* previousBal */, const MyMoneyMoney /* endingBal */,
  const QDate /* endingDate */, MyMoneyAccount* /*accountIndex */,
  const MyMoneyFile* /* file */, QWidget *parent, const char *name)
 : KReconcileDlgDecl(parent,name,true)
{
/*
  m_balanced = false;
  m_debitsQList.setAutoDelete(false);
  m_creditsQList.setAutoDelete(false);
  m_reconciledTransactions.setAutoDelete(false);

  m_file = file;
  m_accountIndex = accountIndex;
  m_endingBalance = endingBal;
  m_previousBalance = previousBal;
  m_clearedBalance.setAmount(0.0);
  m_debitBalance.setAmount(0.0);
  m_creditBalance.setAmount(0.0);
  m_endingDate = endingDate;
	

	totalCreditsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	totalDebitsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	previousLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	endingLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	differenceLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);

	
	debitListView->setRootIsDecorated(false);
	debitListView->addColumn(i18n("Date"));
	debitListView->addColumn(i18n("Number"));
	debitListView->addColumn(i18n("Payee"));
	debitListView->addColumn(i18n("Amount"));
	debitListView->addColumn(i18n("C"));
	debitListView->setMultiSelection(true);
  debitListView->setAllColumnsShowFocus(true);
  // never show a horizontal scroll bar
  debitListView->setHScrollBarMode(QScrollView::AlwaysOff);
	
	creditListView->setRootIsDecorated(false);
	creditListView->addColumn(i18n("Date"));
	creditListView->addColumn(i18n("Number"));
	creditListView->addColumn(i18n("Payee"));
	creditListView->addColumn(i18n("Amount"));
	creditListView->addColumn(i18n("C"));
	creditListView->setMultiSelection(true);
  creditListView->setAllColumnsShowFocus(true);
  // never show a horizontal scroll bar
  creditListView->setHScrollBarMode(QScrollView::AlwaysOff);

	endingLabel->setText(KGlobal::locale()->formatMoney(m_clearedBalance.amount(),""));
	
	previousLabel->setText(KGlobal::locale()->formatMoney(m_endingBalance.amount(),""));
	
	broughtForwardLabel->setText(KGlobal::locale()->formatMoney(m_previousBalance.amount(),""));

	totalCreditsLabel->setText(i18n("Deposits: ") + KGlobal::locale()->formatMoney(m_creditBalance.amount(),""));
	
	totalDebitsLabel->setText(i18n("Withdrawals: ") + KGlobal::locale()->formatMoney(m_debitBalance.amount(),""));


	loadLists();
	insertTransactions();
	
  connect(debitListView, SIGNAL(clicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotDebitSelected(QListViewItem*, const QPoint&, int)));
  connect(creditListView, SIGNAL(clicked(QListViewItem*, const QPoint&, int)), this, SLOT(slotCreditSelected(QListViewItem*, const QPoint&, int)));
	connect(buttonCancel, SIGNAL(clicked()), this, SLOT(cancelClicked()));
  connect(buttonOk, SIGNAL(clicked()), this, SLOT(finishClicked()));
  connect(buttonEdit, SIGNAL(clicked()), this, SLOT(editClicked()));

  doDifference();
*/
}

KReconcileDlg::~KReconcileDlg()
{
}

void KReconcileDlg::clearReconcile()
{
/*	
  MyMoneyTransaction *temp_transaction;
 	for(temp_transaction = m_creditsQList.first();temp_transaction;temp_transaction = m_creditsQList.next())
  {
    if(temp_transaction->state() == MyMoneyTransaction::Reconciled)
		{
     	temp_transaction->setState(MyMoneyTransaction::Unreconciled);
		}
  }
 	for(temp_transaction = m_debitsQList.first();temp_transaction;temp_transaction = m_debitsQList.next())
  {
    if(temp_transaction->state() == MyMoneyTransaction::Reconciled)
		{
     	temp_transaction->setState(MyMoneyTransaction::Unreconciled);
		}
  }
*/
}

void KReconcileDlg::loadLists(void)
{
/*
  // Load the internal transaaction lists
  m_creditsQList.clear();
  m_debitsQList.clear();

  unsigned int i=0;
  MyMoneyTransaction *transaction;
  for (i=0, transaction=m_accountIndex->transactionFirst(); transaction; transaction=m_accountIndex->transactionNext(), i++) {
    if (transaction->date()>m_endingDate)
      break;

    if (transaction->state()!=MyMoneyTransaction::Reconciled) {
      if (transaction->type() == MyMoneyTransaction::Debit) {
        transaction->setIndex(i);
        m_debitsQList.append(transaction);
      }
      else {
        transaction->setIndex(i);
        m_creditsQList.append(transaction);
      }
    }
  }
  resizeEvent(NULL);
*/
}

void KReconcileDlg::insertTransactions(void)
{
/*
  creditListView->clear();
  debitListView->clear();

  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
    (void) new KReconcileListItem(debitListView, it.current());
  }

  QListIterator<MyMoneyTransaction> it2(m_creditsQList);
  for ( ; it2.current(); ++it2) {
    (void) new KReconcileListItem(creditListView, it2.current());
  }

  resizeEvent(0);
*/
}

void KReconcileDlg::slotDebitSelected(Q3ListViewItem* /* item */, const QPoint&/*p*/, int/* col*/)
{
/*
    KReconcileListItem *reconcileItem = (KReconcileListItem*)item;
    if (!reconcileItem)
      return;

		// Changed double types to MyMoneyMoney because in the future
		// we will support mulitple currencies and have our own incompatable
		// internal format. (one that rounds properly and supports large numbers
		// e.g unsigned long.int with sign being a boolean).
//		MyMoneyMoney dblDebit = m_debitBalance;
//		MyMoneyMoney dblCleared = m_clearedBalance;
    MyMoneyMoney dblItem = reconcileItem->transaction()->amount();

    if(reconcileItem->isSelected())
		{
			m_debitBalance += dblItem;
			m_clearedBalance -= dblItem;
			reconcileItem->setReconciled(true);
			m_reconciledTransactions.append(reconcileItem->transaction());
		}
		else
		{
			m_debitBalance -= dblItem;
			m_clearedBalance += dblItem;
			reconcileItem->setReconciled(false);
			m_reconciledTransactions.remove(reconcileItem->transaction());
    }
		totalDebitsLabel->setText(i18n("Withdrawals: ") + KGlobal::locale()->formatMoney(m_debitBalance.amount(),""));

		endingLabel->setText(KGlobal::locale()->formatMoney(m_clearedBalance.amount(),""));

		doDifference();
*/
}

void KReconcileDlg::slotCreditSelected(Q3ListViewItem* /* item */, const QPoint&, int)
{
/*
    KReconcileListItem *reconcileItem = (KReconcileListItem*)item;
    if (!reconcileItem)
      return;

		// See above func
//		double dblCredit = m_creditBalance.amount();
//		double dblCleared = m_clearedBalance.amount();
    MyMoneyMoney dblItem = reconcileItem->transaction()->amount();
    if(reconcileItem->isSelected())
		{
  		m_creditBalance += dblItem;
			m_clearedBalance += dblItem;
			reconcileItem->setReconciled(true);
			m_reconciledTransactions.append(reconcileItem->transaction());
		}
		else
    {
  		m_creditBalance -= dblItem;
			m_clearedBalance -= dblItem;
			reconcileItem->setReconciled(false);
			m_reconciledTransactions.remove(reconcileItem->transaction());
    }
		
		totalCreditsLabel->setText(i18n("Deposits: ") + KGlobal::locale()->formatMoney(m_creditBalance.amount(),""));

		endingLabel->setText(KGlobal::locale()->formatMoney(m_clearedBalance.amount(),""));

		doDifference();
*/
}

void KReconcileDlg::doDifference(void)
{
/*
  MyMoneyMoney difference((m_previousBalance + m_clearedBalance)- m_endingBalance);

  differenceLabel->setText(KGlobal::locale()->formatMoney(difference.amount(),""));
  if (difference.isZero())
    m_balanced = true;
  else
    m_balanced = false;
*/
}

void KReconcileDlg::finishClicked(void)
{
/*
  if (!m_balanced) {
    if ((KMessageBox::questionYesNo(this, i18n("Account did not balance, are you sure ?")))==KMessageBox::No) {
			clearReconcile();
      return;
    }
  }
//	else
//  {
//  }
  emit reconcileFinished(true);
*/
}

/*
void KReconcileDlg::updateData(void)
{
  // Simply reload the list clearing the status.
  qDebug("In updateData");
  m_reconciledTransactions.clear();
  m_debitsQList.clear();
  m_creditsQList.clear();

  loadLists();
  insertTransactions();
  doDifference();
}
*/
void KReconcileDlg::cancelClicked()
{
/*
	clearReconcile();
	// Stop the transaction view from being refreshed on
	// cancel by passing false.
  emit reconcileFinished(false);
*/
}

void KReconcileDlg::resetData(const MyMoneyMoney /* previousBal */, const MyMoneyMoney /* endingBal */, const QDate /* endingDate */, MyMoneyAccount* /* accountIndex */, const MyMoneyFile* /* file */)
{
/*
  m_reconciledTransactions.clear();
  m_debitsQList.clear();
  m_creditsQList.clear();

  m_balanced = false;

	m_file = file;
  m_bankIndex = bankIndex;
	m_accountIndex = accountIndex;
  m_endingBalance = endingBal;
  m_previousBalance = previousBal;
  m_clearedBalance.setAmount(0.0);
  m_debitBalance.setAmount(0.0);
  m_creditBalance.setAmount(0.0);
  m_endingDate = endingDate;
	

	//totalCreditsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	//totalDebitsLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	//previousLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	//endingLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);
	//differenceLabel->setAlignment(AlignRight | AlignVCenter | ExpandTabs | SingleLine);

	
	endingLabel->setText(KGlobal::locale()->formatMoney(m_clearedBalance.amount(),""));
	
	previousLabel->setText(KGlobal::locale()->formatMoney(m_endingBalance.amount(),""));

	broughtForwardLabel->setText(KGlobal::locale()->formatMoney(m_previousBalance.amount(),""));

	totalCreditsLabel->setText(i18n("Deposits: ") + KGlobal::locale()->formatMoney(m_creditBalance.amount(),""));
	
	totalDebitsLabel->setText(i18n("Withdrawals: ") + KGlobal::locale()->formatMoney(m_debitBalance.amount(),""));

	loadLists();
	insertTransactions();
*/
}

void KReconcileDlg::slotTransactionChanged()
{
/*
	reloadLists();
	insertTransactions();
	show();
*/
}

/** No descriptions */
void KReconcileDlg::reloadLists()
{
/*
  unsigned int i=0;
  MyMoneyTransaction *transaction;
  for (i=0, transaction=m_accountIndex->transactionFirst(); transaction; transaction=m_accountIndex->transactionNext(), i++) {
    if (transaction->state()!=MyMoneyTransaction::Reconciled) {
      if (transaction->type() == MyMoneyTransaction::Debit) {
        transaction->setIndex(i);
        if(m_debitsQList.find(transaction) <  0)
        {
          m_debitsQList.append(transaction);
        }
      }
      else {
        transaction->setIndex(i);
        if(m_creditsQList.find(transaction) <  0)
        {
          m_creditsQList.append(transaction);
        }
      }
    }
  }


  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
  bool transactionFound = inTransactions(it.current());
  if(transactionFound == false)
    m_debitsQList.remove(it.current());
  }

  QListIterator<MyMoneyTransaction> it2(m_creditsQList);
  for ( ; it2.current(); ++it2) {
  bool transactionFound = inTransactions(it2.current());
  if(transactionFound == false)
    m_creditsQList.remove(it2.current());
  }
*/
}


/** No descriptions */
bool KReconcileDlg::inTransactions(MyMoneyTransaction * /*credittrans */)
{
/*
  MyMoneyTransaction *transaction;
  int i = 0;
  for ( i=0, transaction=m_accountIndex->transactionFirst(); transaction; transaction=m_accountIndex->transactionNext(), i++) {
    if( credittrans == transaction)
      return true;  	
  }
*/
  return false;
}

/*  Not used (yet?)
bool KReconcileDlg::inCredits(MyMoneyTransaction *transaction)
{
  QListIterator<MyMoneyTransaction> it(m_creditsQList);
  for ( ; it.current(); ++it) {
	if(transaction == it.current())
		return true;
  }

	return false;

}
*/

/*  Not used (yet?)
bool KReconcileDlg::inDebits(MyMoneyTransaction *transaction)
{
  QListIterator<MyMoneyTransaction> it(m_debitsQList);
  for ( ; it.current(); ++it) {
	if(transaction == it.current())
		return true;
  }

	return false;

}
*/

void KReconcileDlg::editClicked()
{
/*
  hide();
*/
}

void KReconcileDlg::resizeEvent(QResizeEvent* /* e */)
{
/*
  debitListView->setColumnWidth( 2, debitListView->visibleWidth()
    - debitListView->columnWidth(0)
    - debitListView->columnWidth(1)
    - debitListView->columnWidth(3)
    - debitListView->columnWidth(4));

  creditListView->setColumnWidth( 2, creditListView->visibleWidth()
    - creditListView->columnWidth(0)
    - creditListView->columnWidth(1)
    - creditListView->columnWidth(3)
    - creditListView->columnWidth(4));

  // call base class resizeEvent()
  KReconcileDlgDecl::resizeEvent(e);
*/
}

#include "kreconciledlg.moc"
