/***************************************************************************
                          ksplittransactiondlg.h  -  description
                             -------------------
    begin                : Thu Jan 10 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KSPLITTRANSACTIONDLG_H
#define KSPLITTRANSACTIONDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <kmenu.h>
#include <kiconloader.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneymoney.h>
#include <mymoneyaccount.h>
#include <mymoneytransaction.h>


#include "ui_ksplittransactiondlgdecl.h"
#include "ui_ksplitcorrectiondlg.h"

class MyMoneyTag;

class KSplitCorrectionDlgDecl : public KDialog, public Ui::KSplitCorrectionDlgDecl
{
public:
  KSplitCorrectionDlgDecl(QWidget *parent) : KDialog(parent) {
    setupUi(this);
  }
};

/**
  * @author Thomas Baumgart
  */

class KSplitTransactionDlgDecl : public KDialog, public Ui::KSplitTransactionDlgDecl
{
public:
  KSplitTransactionDlgDecl(QWidget *parent) : KDialog(parent) {
    setupUi(this);
  }
};
class KSplitTransactionDlg : public KSplitTransactionDlgDecl
{
  Q_OBJECT

public:
  KSplitTransactionDlg(const MyMoneyTransaction& t,
                       const MyMoneySplit& s,
                       const MyMoneyAccount& acc,
                       const bool amountValid,
                       const bool deposit,
                       const MyMoneyMoney& calculatedValue,
                       const QMap<QString, MyMoneyMoney>& priceInfo,
                       QWidget* parent = 0);

  virtual ~KSplitTransactionDlg();

  /**
    * Using this method, an external object can retrieve the result
    * of the dialog.
    *
    * @return MyMoneyTransaction based on the transaction passes during
    *         the construction of this object and modified using the
    *         dialog.
    */
  const MyMoneyTransaction& transaction() const {
    return m_transaction;
  };

  /**
    * This method calculates the difference between the split that references
    * the account passed as argument to the constructor of this object and
    * all the other splits shown in the register of this dialog.
    *
    * @return difference as MyMoneyMoney object
    */
  MyMoneyMoney diffAmount();

  /**
    * This method calculates the sum of the splits shown in the register
    * of this dialog.
    *
    * @return sum of splits as MyMoneyMoney object
    */
  MyMoneyMoney splitsValue();

private:
  /**
    * This method updates the display of the sums below the register
    */
  void updateSums();

public slots:
  int exec();

protected slots:
  void accept();
  void reject();
  void slotClearAllSplits();
  void slotClearUnusedSplits();
  void slotSetTransaction(const MyMoneyTransaction& t);
  void slotCreateCategory(const QString& txt, QString& id);
  void slotUpdateButtons();
  void slotMergeSplits();
  void slotEditStarted();

  /// used internally to setup the initial size of all widgets
  void initSize();

signals:
  /**
    * This signal is sent out, when a new category needs to be created
    * Depending on the setting of either a payment or deposit, the parent
    * account will be preset to Expense or Income.
    *
    * @param account reference to account info. Will be filled by called slot
    * @param parent reference to parent account
    */
  void createCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * This signal is sent out, when a new tag needs to be created
    * @param txt The name of the tag to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createTag(const QString& txt, QString& id);

  /**
    * Signal is emitted, if any of the widgets enters (@a state equals @a true)
    *  or leaves (@a state equals @a false) object creation mode.
    *
    * @param state Enter (@a true) or leave (@a false) object creation
    */
  void objectCreation(bool state);

private:
  /**
    * This member keeps a copy of the current selected transaction
    */
  MyMoneyTransaction     m_transaction;

  /**
    * This member keeps a copy of the currently selected account
    */
  MyMoneyAccount         m_account;

  /**
    * This member keeps a copy of the currently selected split
    */
  MyMoneySplit           m_split;

  /**
    * This member keeps the precision for the values
    */
  int                    m_precision;

  /**
    * flag that shows that the amount specified in the constructor
    * should be used as fix value (true) or if it can be changed (false)
    */
  bool                   m_amountValid;

  /**
    * This member keeps track if the current transaction is of type
    * deposit (true) or withdrawal (false).
    */
  bool                   m_isDeposit;

  /**
    * This member keeps the amount that will be assigned to all the
    * splits that are marked 'will be calculated'.
    */
  MyMoneyMoney           m_calculatedValue;
};

#endif
