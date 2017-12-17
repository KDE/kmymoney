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

#include <QWizard>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QString;

class MyMoneyAccount;
class MyMoneySchedule;
class MyMoneyAccountLoan;
class MyMoneyTransaction;

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
  *
  */
class KNewLoanWizardPrivate;
class KNewLoanWizard : public QWizard
{
  Q_OBJECT

  //TODO: find a way to make this not a friend class
  friend class AdditionalFeesWizardPage;
public:
  enum { Page_Intro, Page_EditIntro, Page_NewGeneralInfo,
         Page_EditSelection, Page_LoanAttributes,
         Page_EffectiveDate, Page_LendBorrow, Page_Name, Page_InterestType,
         Page_PreviousPayments, Page_RecordPayment, Page_VariableInterestDate,
         Page_PaymentEdit, Page_InterestEdit, Page_FirstPayment,
         Page_NewCalculateLoan, Page_PaymentFrequency,
         Page_InterestCalculation, Page_LoanAmount, Page_Interest,
         Page_Duration, Page_Payment, Page_FinalPayment,
         Page_CalculationOverview, Page_NewPayments, Page_InterestCategory,
         Page_AdditionalFees, Page_Schedule, Page_SummaryEdit,
         Page_AssetAccount, Page_Summary
       };

  explicit KNewLoanWizard(QWidget *parent = nullptr);
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
  MyMoneySchedule schedule() const;

  /**
    * This method returns the id of the account to/from which
    * the payout should be created. If the checkbox that allows
    * to skip the creation of this transaction is checked, this
    * method returns QString()
    *
    * @return id of account or empty QString
    */
  QString initialPaymentAccount() const;

  /**
    * This method returns the date of the payout transaction.
    * If the checkbox that allows to skip the creation of
    * this transaction is checked, this method returns QDate()
    *
    * @return selected date or invalid QDate if checkbox is selected.
    */
  QDate initialPaymentDate() const;

  bool validateCurrentPage();

  const MyMoneyAccountLoan account() const;

  /**
   * This method returns the id of the next page in the wizard.
   * It is overloaded here to support the dynamic nature of this wizard.
   *
   * @return id of the next page or -1 if there is no next page
   */
  int nextId() const;

protected Q_SLOTS:

  // void slotNewPayee(const QString&);
  void slotReloadEditWidgets();

Q_SIGNALS:
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
    * @param id A connected slot should store the id of the created object
    * in this variable
    */
  void createPayee(const QString& txt, QString& id);

protected:
  const QScopedPointer<KNewLoanWizardPrivate> d_ptr;
  KNewLoanWizard(KNewLoanWizardPrivate &dd, QWidget *parent);
   
private:
  Q_DISABLE_COPY(KNewLoanWizard)
  Q_DECLARE_PRIVATE(KNewLoanWizard)

private Q_SLOTS:
  void slotNewCategory(MyMoneyAccount &acc);
  void slotNewPayee(const QString& newnameBase, QString& id);
};

#endif
