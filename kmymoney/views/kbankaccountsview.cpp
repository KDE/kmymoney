/***************************************************************************
                             kbankaccountssview.cpp
                             -------------------
    copyright            : (C) 2013 by Stephan Zodrow
    email                : szodrow@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kbankaccountsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QTabWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QPixmap>
#include <QLayout>
#include <QList>
#include <QDebug> // szo test only
#include <QMessageBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KListWidget>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <KToggleAction>
#include <fixx11h.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "kmymoneyaccounttreeview.h"
#include "models.h"
#include "payeeidentifier/ibanandbic/internationalaccountidentifier.h"
#include <payeeidentifier/nationalaccount/nationalaccountid.h>

KBankAccountsView::KBankAccountsView(QWidget *parent) :
    QWidget(parent)
{
  qDebug() << "KBankAccountsView Constructor";
  setupUi(this);
  QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  layoutWidget->setSizePolicy(sizePolicy);
  rowInEditor = -1;
  // set column sequence
  colAccountName = 0;
  colIsDefaultAccount = 1;
  colAccountOwner = 2;
  colAccountNumber = 3;
  colBankCode = 4;
  colBankName = 5;
  colTransactionSystem = 6;
  colCountryCode = 7;

  bankAccounts->setColumnCount(8);
  bankAccountsHeader<<i18n("Account Name")<<i18n("Default")<<i18n("Account Owner")<<i18n("Account Number")<<i18n("Bank Code")<<i18n("Bank Name")<<i18n("Account Type")<<i18n("Country");
  bankAccounts->setHorizontalHeaderLabels(bankAccountsHeader);
  bankAccounts->verticalHeader()->setVisible(false);
  bankAccounts->setEditTriggers(QAbstractItemView::NoEditTriggers);
  bankAccounts->setSelectionBehavior(QAbstractItemView::SelectRows);
  bankAccounts->setSelectionMode(QAbstractItemView::SingleSelection);
  bankAccounts->setShowGrid(false);
//    bankAccounts->setGeometry(QApplication::desktop()->screenGeometry());

  connect(pb_new, SIGNAL(clicked(bool)), this, SLOT(slotNewAccount(bool)));
  connect(pb_del, SIGNAL(clicked(bool)), this, SLOT(slotDelAccount(bool)));
  connect (bankAccounts, SIGNAL(currentCellChanged ( int , int , int , int  )), this, SLOT(slotCurrentCellChanged ( int , int , int , int  )));
  connect (this, SIGNAL(doRowChange ( int )), this, SLOT(slotPostRowChange ( int  )), Qt::QueuedConnection);  //queued is important
  bankAccounts->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
}

KBankAccountsView::~KBankAccountsView()
{
}

void KBankAccountsView::slotCurrentCellChanged ( int currentRow, int currentColumn, int previousRow, int previousColumn )
{
  qDebug() << "slotCurrentCellChanged: currentRow: " << currentRow << "previousRow: " << previousRow << "rowInEditor: " << rowInEditor;
  if (currentRow != previousRow // row change at all
      && currentRow != rowInEditor // avoid check after discarding below
     ) {
    if ( previousRow != -1 ) {
      if ( ! m_bankAccountEdit->isDataChanged() || m_bankAccountEdit->isDataOk() ) {
	editorContentToRow(previousRow);
	rowContentToEditor(currentRow);
      } else {
	QMessageBox msgBox;
	msgBox.setText(m_bankAccountEdit->errorMsg());
	msgBox.setInformativeText("Discard changes?");
	msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();
	if ( ret == QMessageBox::Discard ) {
	  rowContentToEditor(currentRow);
	} else {
	  qDebug() << "slotCurrentCellChanged: currentRow: " << bankAccounts->currentRow () << "rowInEditor: " << rowInEditor;
	  emit doRowChange(rowInEditor);
	}
      }
    }
  }
}

void KBankAccountsView::slotPostRowChange ( int row)
{
  qDebug() << "slotPostRowChange: row: " << row;
  bankAccounts->selectionModel()->clearSelection();
  bankAccounts->selectRow(row);
}

void KBankAccountsView::slotNewAccount(bool dummy)
{
  int currRow = bankAccounts->currentRow();
  if (bankAccounts->item(bankAccounts->rowCount() - 1, colAccountName)->text() != "" ) { // valid bank account entries have a name
    bankAccountsAddRow("", false, "", "", "", "", "", "Sepa");
    bankAccounts->selectRow(currRow);
    slotCurrentCellChanged(bankAccounts->rowCount() - 1, 0, currRow, 0);
    bankAccounts->selectRow(bankAccounts->rowCount() - 1);
  }
}

void KBankAccountsView::slotDelAccount(bool dummy)
{
  int currRow = bankAccounts->currentRow();
  bankAccounts->removeRow(currRow);
  if (bankAccounts->rowCount() > 0 ) {
    if (currRow == bankAccounts->rowCount()) {
      currRow --;
    }
    rowContentToEditor(currRow);
  } else {
    m_bankAccountEdit->clearItemData();
  }
}


int KBankAccountsView::bankAccountsAddRow(const QString& accountName, const bool isDefaultAccount, const QString& accountOwner, 
			 const QString& accountNumber, const QString& bankCode, const QString& bankName,
			 const QString& countryCode, const QString& transactionSystem)
{
  int row = bankAccounts->rowCount();

  qDebug() << "bankAccountsAddRow new row: " << row << " " << accountName;
  bankAccounts->setRowCount(row + 1);
  bankAccounts->setItem(row, colAccountName, new QTableWidgetItem(accountName) );
  bankAccounts->setItem(row, colIsDefaultAccount, new QTableWidgetItem(isDefaultAccountText(isDefaultAccount)) );
  bankAccounts->item(row, colIsDefaultAccount) ->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  bankAccounts->setItem(row, colAccountOwner, new QTableWidgetItem(accountOwner) );
  bankAccounts->setItem(row, colAccountNumber, new QTableWidgetItem(accountNumber) );
  bankAccounts->setItem(row, colBankCode, new QTableWidgetItem(bankCode) );
  bankAccounts->setItem(row, colBankName, new QTableWidgetItem(bankName) );
  bankAccounts->setItem(row, colTransactionSystem, new QTableWidgetItem(transactionSystem) );
  bankAccounts->setItem(row, colCountryCode, new QTableWidgetItem(countryCode) );
  if ( row == 0 ) 
    bankAccounts->setCurrentCell(0, 1); // focus needed, so that changes in 1st record are checked
  return row;
}



bool KBankAccountsView::editorContentToRow ( int row )
{
  qDebug() << "editorContentToRow: " << row << "accountNameEdit(edit): " << m_bankAccountEdit->accountName();
  bankAccounts->item(row, colAccountName)->setText( m_bankAccountEdit->accountName() );
  if (m_bankAccountEdit->isDefaultAccount() && bankAccounts->item(row, colIsDefaultAccount)->text() != isDefaultAccountText(true)) {
    for ( int i = 0; i< bankAccounts->rowCount(); i++ ) {
      if ( bankAccounts->item(i, colIsDefaultAccount)->text() == isDefaultAccountText(true) ) {
	bankAccounts->item(i, colIsDefaultAccount)->setText( isDefaultAccountText(false));
	exit;
      }
    }
  }
  bankAccounts->item(row, colIsDefaultAccount)->setText( isDefaultAccountText(m_bankAccountEdit->isDefaultAccount()));
  bankAccounts->item(row, colAccountOwner)->setText( m_bankAccountEdit->accountOwner() );
  bankAccounts->item(row, colAccountNumber)->setText( m_bankAccountEdit->accountNumber() );
  bankAccounts->item(row, colBankCode)->setText( m_bankAccountEdit->bankCode() );
  bankAccounts->item(row, colBankName)->setText( m_bankAccountEdit->bankName() );
  bankAccounts->item(row, colTransactionSystem)->setText( m_bankAccountEdit->transactionSystemText() );
  bankAccounts->item(row, colCountryCode)->setText( m_bankAccountEdit->countryCode() );
  return true;
}

void KBankAccountsView::rowContentToEditor ( int row )
{
  qDebug() << "rowContentToEditor: " << row << "accountNameEdit(tab): " << bankAccounts->itemAt(row, colAccountName)->text();
  if (bankAccounts->rowCount() == 0) 
    return;
  m_bankAccountEdit->setAccountName(bankAccounts->item(row, colAccountName)->text() );
  if (bankAccounts->item(row, colIsDefaultAccount)->text() == isDefaultAccountText(true) ) {
    m_bankAccountEdit->setIsDefaultAccount(true);
    m_bankAccountEdit->enableIsDefaultAccount(false);
  } else {
    m_bankAccountEdit->setIsDefaultAccount(false);
    m_bankAccountEdit->enableIsDefaultAccount(true);
  }
  m_bankAccountEdit->setAccountOwner(bankAccounts->item(row, colAccountOwner)->text() );
  m_bankAccountEdit->setAccountNumber (bankAccounts->item(row, colAccountNumber)->text() );
  m_bankAccountEdit->setBankCode(bankAccounts->item(row, colBankCode)->text() );
  m_bankAccountEdit->setBankName(bankAccounts->item(row, colBankName)->text() );
  m_bankAccountEdit->setTransactionSystem(bankAccounts->item(row, colTransactionSystem)->text(), KBankAccountEdit::GuiText );
  m_bankAccountEdit->setCountryCode(bankAccounts->item(row, colCountryCode)->text() );
  rowInEditor = row;
}

void KBankAccountsView::slotLoadAccounts( const MyMoneyPayee& payee )
{
  bankAccounts->clear();
  payeeIdentifier::constList identifiers = payee.payeeIdentifiers();
  
  if (identifiers.isEmpty())
    return;
  
  foreach( payeeIdentifier::constPtr ident, identifiers ) {
    if ( QSharedPointer<const internationalAccountIdentifier> ibanBic = ident.dynamicCast<const internationalAccountIdentifier>() ) {
      bankAccountsAddRow("", true, "", ibanBic->paperformatIban(), ibanBic->storedBic(), "", "", "Sepa");
    } else if ( QSharedPointer<const nationalAccountId> nationalAccount = ident.dynamicCast<const nationalAccountId>() ) {
      bankAccountsAddRow("", false, "", nationalAccount->accountNumber(), nationalAccount->bankCode(), "", "", "National");
    } else {
      bankAccountsAddRow(i18n("Plugin missing"), false, QString(), QString(), QString(), QString(), QString(), QString());
    }
  }
  rowContentToEditor(0);
}

void KBankAccountsView::showEvent(QShowEvent * event)
{
}

#include "kbankaccountsview.moc"
