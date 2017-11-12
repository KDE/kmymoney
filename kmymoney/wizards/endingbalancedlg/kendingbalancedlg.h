/***************************************************************************
                          kendingbalancedlg.h
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

#ifndef KENDINGBALANCEDLG_H
#define KENDINGBALANCEDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kendingbalancedlgdecl.h"

#include "mymoneyaccount.h"

class QDate;

class MyMoneyAccount;
class MyMoneyTransaction;

/**
  * This dialog is wizard based and used to enter additional
  * information required to start the reconciliation process.
  * This version implements the behaviour for checkings,
  * savings and credit card accounts.
  *
  * @author Thomas Baumgart
  */
class KEndingBalanceDlgDecl : public QWizard, public Ui::KEndingBalanceDlgDecl
{
public:
  KEndingBalanceDlgDecl(QWidget *parent) : QWizard(parent) {
    setupUi(this);
  }
};
class KEndingBalanceDlg : public KEndingBalanceDlgDecl
{
  Q_OBJECT
public:
  enum { Page_CheckingStart, Page_PreviousPostpone,
         Page_CheckingStatementInfo, Page_InterestChargeCheckings
       };

  explicit KEndingBalanceDlg(const MyMoneyAccount& account, QWidget *parent = 0);
  ~KEndingBalanceDlg();

  const MyMoneyMoney endingBalance() const;
  const MyMoneyMoney previousBalance() const;
  const QDate statementDate() const {
    return field("statementDate").toDate();
  };

  const MyMoneyTransaction interestTransaction();
  const MyMoneyTransaction chargeTransaction();

  /**
   * This method returns the id of the next page in the wizard.
   * It is overloaded here to support the dynamic nature of this wizard.
   *
   * @return id of the next page or -1 if there is no next page
   */
  int nextId() const;

protected:
  bool createTransaction(MyMoneyTransaction& t, const int sign, const MyMoneyMoney& amount, const QString& category, const QDate& date);
  const MyMoneyMoney adjustedReturnValue(const MyMoneyMoney& v) const;
  void createCategory(const QString& txt, QString& id, const MyMoneyAccount& parent);

protected slots:
  void slotReloadEditWidgets();
  void help();
  void slotCreateInterestCategory(const QString& txt, QString& id);
  void slotCreateChargesCategory(const QString& txt, QString& id);
  void accept();
  void slotUpdateBalances();

signals:
  /**
    * proxy signal for KMyMoneyPayeeCombo::createItem(const QString&, QString&)
    */
  void createPayee(const QString&, QString&);

  /**
    * emit when a category is about to be created
    */
  void createCategory(MyMoneyAccount& acc, const MyMoneyAccount& parent);

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;
};

#endif
