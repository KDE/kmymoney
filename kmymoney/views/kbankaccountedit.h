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

#ifndef KBANKACCOUNTEDIT_H
#define KBANKACCOUNTEDIT_H

// ----------------------------------------------------------------------------
// QT Includes
#include <QTableWidget>
#include <QString>
// ----------------------------------------------------------------------------
// KDE Includes

//#include <KListWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyaccount.h>
#include <mymoneyutils.h>
#include "accountsmodel.h"

#include "ui_kbankaccounteditdecl.h"


/**
  * This class implements the bank accounts editor.
  * It is supposed to be used as 
  * 1. input area and check for adhoc input while entering transactions
  * 2. input area and check in the first stage where only one account is possible per payee
  * 3. in a later phase where multiple accounts per payee are possible (=> account nick name & default account mark necessary)
  */
class KBankAccountEdit : public QWidget, public Ui::KBankAccountEditDecl
{
  Q_OBJECT

public:
  typedef enum {
    transactionOnly = 0,
    withName,
    allFields
  } editMode;
  typedef enum {
    national = 0,
    sepa,
    paypal
  } transactionSystemType;  // display value: National, Sepa, PayPal, data values in storage: national, sepa, paypal, see taSystem..To.. functions
  typedef enum {
    GuiText = 0,
    DataText
  } translation; // translate transactionSystemType to/from text for display or data
  KBankAccountEdit(QWidget *parent = 0, KBankAccountEdit::editMode editAccountMode = KBankAccountEdit::transactionOnly,
		   KBankAccountEdit::transactionSystemType transactionSystem = KBankAccountEdit::national
  );
  virtual ~KBankAccountEdit();
  /**
    * This method returns if bank account data is present
    *
    */
  bool isData();
  /**
    * This method returns if bank account data is changed by user
    *
    */
  bool isDataChanged();

  /**
    * This method checks if bank account data is either empty or data is OK
    *
    */
  bool isDataOk();

  /**
    * This method resets bank account data and displayed values
    *
    */
  void clearItemData();
  
// getter / setter
  void setAccountOwner(const QString& val);
  void setAccountNumber(const QString& val);
  void setBankCode(const QString& val);
  void setBankName(const QString& val);
  void setCountryCode(const QString& val);
  void setTransactionSystem(transactionSystemType val);
  void setTransactionSystem(const QString& val, translation translateFrom);
  void setAccountDataMinimal(const QString& accountOwner, const QString& accountNumber, 
			    const QString& bankCode, const QString& bankName,
			    const QString& countryCode, transactionSystemType = national);

  void setAccountName(const QString& val);
  void setIsDefaultAccount(const QString& val);
  void setIsDefaultAccount(bool val);
  void enableIsDefaultAccount(bool val);

  QString accountOwner() {return bankAccountOwnerNameEdit->text();};
  QString accountNumber() {return bankAccountNumberEdit->text();};
  QString bankCode() {return bankCodeEdit->text();};
  QString bankName() {return bankNameEdit->text();};
  QString transactionSystemText(translation translateFrom = GuiText);
  QString countryCode(){return countryCodeEdit->currentText();};
  QString accountName() {return accountNameEdit->text();};
  bool    isDefaultAccount() {return defaultAccountEdit->checkState() == Qt::Checked;};

  QString errorMsg() {return m_errorMsg;};
public slots:

  /**
    * Override the base class behaviour to include all updates that
    * happened in the meantime and restore the layout.
    */
//szo  void showEvent(QShowEvent * event);

  /**
    * update the account objects if their icon position has changed since
    * the last time.
    *
    * @ param action must be KMyMoneyView::preSave, otherwise this slot is a NOP.
    */
  //void slotUpdateIconPos(unsigned int action);

  /**
    * Reconcile an account
    *
    * @param acc reference to the account that is to be reconciled
    * @param reconciliationDate the date of the statement
    * @param endingBalance the ending balance noted on the statement
    */
//  void slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);

protected:


  /**
    * This method loads the accounts for the respective tab.
    *
    * @param tab which tab should be loaded
    */
//  void loadAccounts(AccountsViewTab tab);
//  void loadListView(void);
  /**
    * This method loads all the subaccounts recursively of a given root account
    *
    */
//szo? compile-Fehler  void loadAccountIconsIntoList(const MyMoneyAccount& parentAccount, KListWidget* listWidget);

private slots:
  void slotTextChanged();
  void slotBankCodeChanged();
  void slotTransActionSystemChanged(const QString & text);
signals:
  /**
    * This signal serves as proxy for KMyMoneyAccountTreeView::selectObject()
    *
    * @param obj const reference to object
    */
  void selectObject(const MyMoneyObject& obj);
  void dataChanged();

private:
  editMode m_editAccountMode;
  QString m_countryCode;
  transactionSystemType m_transactionSystem;

  // regex variables for format checks
  QRegExp rxOwnerName;
  QRegExp rxAccountNumber; 
  QRegExp rxBankCode;

  QString m_errorMsg; // error message found in in isDataOk
  /**
    * This method sets input masks and validators of editor fields depend on country and transaction system requirements
    *
    * @param countryCode two digit country code of the country where the ban resides
    * @param transactionSystem transcation system the account uses: currently supported national (e. g. for HBCI and OFX), planned: SEPA, Paypal
    */
  void switchEditorLayout(const transactionSystemType & transactionSystem);
  void setTransactionProperties(const QString& countryCode, const transactionSystemType transactionSystem = national);
  bool isBankCodeOk (transactionSystemType transactionSystem, bool signalErrorMsg);
  transactionSystemType taSystemTextToEnmum(const QString & text, translation translateFrom);
  QString taSystemEnmumToText(transactionSystemType taSystem, translation translateTo);
};

#endif
