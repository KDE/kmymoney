/***************************************************************************
                          knewloanwizard.h  -  description
                             -------------------
    begin                : Wed Oct 8 2003
    copyright            : (C) 2000-2003 by Thomas Baumgart
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

#ifndef KNEWLOANWIZARD_H
#define KNEWLOANWIZARD_H

// ----------------------------------------------------------------------------
// QT Includes

#include <qwidget.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include <../dialogs/knewloanwizarddecl.h>
#include <kmymoney/mymoneyscheduled.h>
#include <kmymoney/kmymoneyaccountselector.h>
#include <kmymoney/kmymoneydateinput.h>

/**
  * @author Thomas Baumgart
  */

/**
  * This class implementes a wizard for the creation of loan accounts.
  * The user is asked a set of questions and according to the answers
  * the respective MyMoneyAccount object can be requested from the
  * wizard when accept() has been called. A MyMoneySchedule is also
  * available to create a schedule entry for the payments to the newly
  * created loan.
  */
class KNewLoanWizard : public KNewLoanWizardDecl
{
  Q_OBJECT
public:
  KNewLoanWizard(QWidget *parent=0, const char *name=0);
  ~KNewLoanWizard();

  /**
    * This method returns the schedule for the payments. The account
    * where the amortization should be transferred to is the one
    * we currently try to create with this wizard. The appropriate split
    * will be returned as the first split of the transaction inside
    *
    * as parameter @p accountId as this is the account that was created
    * after this wizard was left via the accept() method.
    *
    * @return MyMoneySchedule object for payments
    */
  MyMoneySchedule schedule(void) const;

  /**
    * This method returns the id of the account to/from which
    * the payout should be created. If the checkbox that allows
    * to skip the creation of this transaction is checked, this
    * method returns QString()
    *
    * @return id of account or empty QString
    */
  QString initialPaymentAccount(void) const;

  /**
    * This method returns the date of the payout transaction.
    * If the checkbox that allows to skip the creation of
    * this transaction is checked, this method returns QDate()
    *
    * @return selected date or invalid QDate if checkbox is selected.
    */
  QDate initialPaymentDate(void) const;

protected:
  /**
    * This method returns the transaction that is stored within
    * the schedule. See schedule().
    *
    * @return MyMoneyTransaction object to be used within the schedule
    */
  MyMoneyTransaction transaction(void) const;

public slots:
  void next();

protected slots:
  void slotLiabilityLoan(void);
  void slotAssetLoan(void);
  virtual void slotCheckPageFinished(void);
  void slotPaymentsMade(void);
  void slotNoPaymentsMade(void);
  void slotRecordAllPayments(void);
  void slotRecordThisYearsPayments(void);
  void slotInterestOnPayment(void);
  void slotInterestOnReception(void);
  void slotCreateCategory(void);
  virtual void slotAdditionalFees(void);
  // void slotNewPayee(const QString&);
  void slotReloadEditWidgets(void);

protected:
  void loadComboBoxes(void);
  void loadAccountList(void);
  void resetCalculator(void);
  void updateLoanAmount(void);
  void updateInterestRate(void);
  void updateDuration(void);
  void updatePayment(void);
  void updateFinalPayment(void);
  void updateLoanInfo(void);
  QString updateTermWidgets(const long double v);
  void updatePeriodicPayment(void);
  void updateSummary(void);
  int calculateLoan(void);
  int term(void) const;

signals:
  /**
    * This signal is emitted, when a new category name has been
    * entered by the user and this name is not known as account
    * by the MyMoneyFile object.
    * Before the signal is emitted, a MyMoneyAccount is constructed
    * by this object and filled with the desired name. All other members
    * of MyMoneyAccount will remain in their default state. Upon return,
    * the connected slot should have created the object in the MyMoneyFile
    * engine and filled the member @p id.
    *
    * @param acc reference to MyMoneyAccount object that caries the name
    *            and will return information about the created category.
    */
  void newCategory(MyMoneyAccount& acc);

  /**
    * This signal is sent out, when a new payee needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the payee to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createPayee(const QString& txt, QString& id);

protected:
  MyMoneyAccountLoan  m_account;
  MyMoneyTransaction  m_transaction;
  MyMoneySplit        m_split;
};

#endif
