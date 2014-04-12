/***************************************************************************
                             kbankaccountssview.h
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

#ifndef KBANKACCOUNTSVIEW_H
#define KBANKACCOUNTSVIEW_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QTableWidget>
// ----------------------------------------------------------------------------
// KDE Includes

//#include <KListWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyutils.h>
#include "accountsmodel.h"

#include "ui_kbankaccountsviewdecl.h"
#include <QTableWidget>


/**
  * This class implements the bank accounts list and edit 'view'.
  */
class KBankAccountsView : public QWidget, public Ui::KBankAccountsView
{
  Q_OBJECT

public:
  KBankAccountsView(QWidget *parent = 0);
  virtual ~KBankAccountsView();

public slots:
  void slotLoadAccounts(void);

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime and restore the layout.
    */
  void showEvent(QShowEvent * event);

protected:

private slots:
  /**
    * slot to capture row changes
    *
    */
  void slotCurrentCellChanged ( int currentRow, int currentColumn, int previousRow, int previousColumn );
  /**
    * slot to do a (posted) row change
    *
    */
  void slotPostRowChange(int row);
  /**
    * add a empty bank account row or switch to an empty row
    *
    */
  void slotNewAccount(bool dummy);
  /**
    * delete the bank account that has focus
    *
    */
  void slotDelAccount(bool dummy);
signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTreeView::selectObject()
    *
    * @param obj const reference to object
    */
  void selectObject(const MyMoneyObject& obj);
  /**
    * This signal is required to induce a row change via the event queue
    *
    * @row int new where taht must be higlighted
    */
  void doRowChange(int row);

private:
  /**
    * This method checks Editor Data and transfers into row presentation (if possible). Return code says if transfer succeeded
    *
    */
  bool editorContentToRow(int row);
  /**
    * This method transfers account data of a row into the editor widget. It is expected that the data is correct and will pass checks of the editor
    *
    */
  void rowContentToEditor(int row);
  /**
    * This method sets input masks and validators of editor fields depend on country and transaction system requirements
    *
    * @param accountName nick name of this bank account in payee context
    * @param isDefaultAccount if this bank account is preferrred account of the payee
    * @param accountOwner the owner name of the account, required at least in some national transactions
    * @param accountNumber account number or IBAN
    * @param bankCode bank code or BIC
    * @param bankName bank name related to the bank code, required at least in some national transactions
    * @param countryCode two digit country code of the country where the ban resides
    * @param transactionSystem transcation system the account uses: currently supported national (e. g. for HBCI and OFX), SEPA, planned: Paypal
    */
  int bankAccountsAddRow(const QString& accountName, const bool isDefaultAccount, const QString& accountOwner, 
			 const QString& accountNumber, const QString& bankCode, const QString& bankName,
			 const QString& countryCode, const QString& transactionSystem);
  /**
    * This method translates boolean value isDefaultAccount into displayed value
    *
    */
  const QString isDefaultAccountText(const bool isDefaultAccount) {return (isDefaultAccount) ? QString("X") : QString(""); };
  
  QStringList bankAccountsHeader; // helper variable 
  int rowInEditor; // this must not be the row with focus, e. g. after a check of editor data failed
  
  // next int colSomething to enable change of column seq. easily in grid
  int colAccountName; 
  int colIsDefaultAccount;
  int colAccountOwner;
  int colAccountNumber;
  int colBankCode;
  int colBankName;
  int colTransactionSystem;
  int colCountryCode;
};

#endif
