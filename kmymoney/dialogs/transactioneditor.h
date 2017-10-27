/***************************************************************************
                             transactioneditor.h
                             ----------
    begin                : Wed Jun 07 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRANSACTIONEDITOR_H
#define TRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QWidget>
#include <QList>
#include <QEvent>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyschedule.h"
#include "transactioneditorcontainer.h"
#include "register.h"

class KMyMoneyCategory;

class TransactionEditor : public QObject
{
  Q_OBJECT
public:
  TransactionEditor();
  TransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);
  virtual ~TransactionEditor();

  /**
    * This method is used as a helper because virtual methods cannot be
    * called within a constructor. Thus setup() should be called immediately
    * after a TransactionEditor() object or one of its derivatives is
    * constructed. The parameter @a account identifies the account that
    * is currently opened in the calling ledger view.
    *
    * This account will not be included in category sets. The default is
    * no account so all will be shown. I have no idea anymore, what I
    * tried to say with the first sentence above. :(  Maybe this is crap.
    *
    * @param tabOrderWidgets QWidgetList which will be filled with the pointers
    *                        to the editWidgets in their tab order
    * @param account account that is currently shown in the calling ledger view
    * @param action default action (defaults to ActionNone).
    */
  void setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account = MyMoneyAccount(), KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone);

  /**
    * Enter the transactions into the ledger. In case of a newly created
    * transaction @a newId contains the assigned id. In case @a askForSchedule
    * is true (the default), the user will be asked if he wants to enter new
    * transactions with a post date in the future into the ledger or rather
    * create a schedule for them. In case  @a suppressBalanceWarnings is @p false
    * (the default) a warning will be displayed when the balance crosses the minimum
    * or maximum balance settings for the account.
    */
  virtual bool enterTransactions(QString& newId, bool askForSchedule = true, bool suppressBalanceWarnings = false);

  /**
    * This method creates a transaction based on the contents of the current widgets,
    * the splits in m_split in single selection mode or an existing transaction/split
    * and the contents of the widgets in multi selection mode.
    *
    * The split referencing the current account is returned as the first split in the
    * transaction's split list.
    *
    * @param t reference to created transaction
    * @param torig the original transaction
    * @param sorig the original split
    * @param skipPriceDialog if @p true the user will not be requested for price information
    *                        (defaults to @p false)
    *
    * @return @p false if aborted by user, @p true otherwise
    *
    * @note Usually not used directly. If unsure, use enterTransactions() instead.
    */
  virtual bool createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog = false) = 0;

  /**
    * This method returns information about the completeness of the data
    * entered. This can be used to control the availability of the
    * 'Enter transaction' action.
    *
    * @retval true if entering the transaction into the engine
    * @retval false if not enough information is present to enter the
    * transaction into the engine
    *
    * @param reason will be filled with a string about the reason why the
    *               completeness is not reached.  Empty if the return value
    *               is @c true.
    *
    * @sa transactionDataSufficient()
    */
  virtual bool isComplete(QString& reason) const = 0;

  /**
    * This method returns information if the editor is started with multiple transactions
    * being selected or not.
    *
    * @retval false only a single transaction was selected when the editor was started
    * @retval true multiple transactions were selected when the editor was started
    */
  virtual bool isMultiSelection() const {
    return m_transactions.count() > 1;
  }

  virtual bool fixTransactionCommodity(const MyMoneyAccount& account);

  virtual bool canAssignNumber() const;
  virtual void assignNextNumber();

  /**
    * Returns a pointer to the widget that should receive
    * the focus after the editor has been started.
    */
  virtual QWidget* firstWidget() const = 0;

  /**
    * Returns a pointer to a widget by name
    */
  QWidget* haveWidget(const QString& name) const;

  void setTransaction(const MyMoneyTransaction& t, const MyMoneySplit& s);

  bool eventFilter(QObject* o, QEvent* e);

  const MyMoneyAccount& account() const {
    return m_account;
  }

  void clearFinalWidgets();

  void addFinalWidget(const QWidget*);

  QString  m_memoText;
  QString  m_scheduleInfo;

  eMyMoney::Schedule::PaymentType m_paymentMethod;

public slots:
  void slotReloadEditWidgets();

  /**
    * The default implementation returns QDialog::Rejected
    */
  virtual int slotEditSplits();

  /**
    * Modify the account which the transaction should be based on. The
    * initial value for the account is passed during setup().
    *
    * @param id of the account to be used
    */
  virtual void slotUpdateAccount(const QString& id);

protected:
  virtual void createEditWidgets() = 0;
  virtual void setupFinalWidgets() = 0;
  virtual void loadEditWidgets(KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone) = 0;
  void setupCategoryWidget(KMyMoneyCategory* category, const QList<MyMoneySplit>& splits, QString& categoryId, const char* splitEditSlot, bool allowObjectCreation = true);
  void resizeForm();

  /**
   * This method sets the precision of the value widgets to reflect
   * the account in m_account. If m_account has no id, the precision
   * defaults to 2.
   */
  void setupPrecision();

protected slots:
  void slotUpdateButtonState();
  void slotUpdateMemoState();
  void slotUpdateAccount();
  void slotNumberChanged(const QString&);

signals:
  /**
    * This signal is sent out by the destructor to inform other entities
    * that editing has been finished. The parameter @a t contains the list
    * of transactions that were processed.
    */
  void finishEdit(const KMyMoneyRegister::SelectedTransactions& t);

  /**
    * This signal is sent out whenever enough data is present to enter the
    * transaction into the ledger. This signal can be used to control the
    * KAction which implements entering the transaction.
    *
    * @sa isComplete()
    *
    * @param state @a true if enough data is present, @a false otherwise.
    */
  void transactionDataSufficient(bool state);

  /**
    * This signal is sent out, when a new payee needs to be created
    * @sa KMyMoneyCombo::createItem()
    *
    * @param txt The name of the payee to be created
    * @param id A connected slot should store the id of the created object in this variable
    */
  void createPayee(const QString& txt, QString& id);

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
    * This signal is sent out, when a new security (e.g. stock )needs to be created
    * @a Parent should be the investment account under which the security account
    * will be created.
    *
    * @param account reference to account info. Will be filled by called slot
    * @param parent reference to parent account
    */
  void createSecurity(MyMoneyAccount& account, const MyMoneyAccount& parent);

  /**
    * Signal is emitted, if any of the widgets enters (@a state equals @a true)
    *  or leaves (@a state equals @a false) object creation mode.
    *
    * @param state Enter (@a true) or leave (@a false) object creation
    */
  void objectCreation(bool state);

  void statusMsg(const QString& txt);

  void statusProgress(int cnt, int base);

  /**
    * This signal is sent out for each newly added transaction
    *
    * @param date the post date of the newly created transaction
    */
  void lastPostDateUsed(const QDate& date);

  /**
    * This signal is sent out, if the user decides to schedule the transaction @a t
    * rather then adding it to the ledger right away.
    */
  void scheduleTransaction(const MyMoneyTransaction& t, eMyMoney::Schedule::Occurrence occurrence);

  /**
   * This signal is sent out, if the user double clicks the number field
   */
  void assignNumber();

  /**
   * This signal is sent out, if the user has pressed the ESC key.
   */
  void escapePressed();

  /**
   * This signal is sent out, if the user has pressed the Return or Enter
   * key and asks to end editing the transaction
   */
  void returnPressed();

  /**
   * This signal is sent out, if any of the balance warning levels
   * for @p account has been reached. @p msg contains the message text.
   * @p parent points to the parent widget to be used for the warning message box.
   */
  void balanceWarning(QWidget* parent, const MyMoneyAccount& account, const QString& msg);

  void operationTypeChanged(int index);

protected:
  QList<MyMoneySplit>                               m_splits;
  KMyMoneyRegister::SelectedTransactions            m_transactions;
  QList<const QWidget*>                             m_finalEditWidgets;
  TransactionEditorContainer*                       m_regForm;
  KMyMoneyRegister::Transaction*                    m_item;
  KMyMoneyRegister::QWidgetContainer                m_editWidgets;
  MyMoneyAccount                                    m_account;
  MyMoneyTransaction                                m_transaction;
  MyMoneySplit                                      m_split;
  QDate                                             m_lastPostDate;
  QMap<QString, MyMoneyMoney>                       m_priceInfo;
  KMyMoneyRegister::Action                          m_initialAction;
  bool                                              m_openEditSplits;
  bool                                              m_memoChanged;

private:
  /**
  *  If a new or an edited transaction has a valid number, keep it with the account
  */
  void keepNewNumber(const MyMoneyTransaction& tr);

};

class StdTransactionEditor : public TransactionEditor
{
  Q_OBJECT
public:
  StdTransactionEditor();
  StdTransactionEditor(TransactionEditorContainer* regForm, KMyMoneyRegister::Transaction* item, const KMyMoneyRegister::SelectedTransactions& list, const QDate& lastPostDate);
  ~StdTransactionEditor();

  bool isComplete(QString& reason) const;
  QWidget* firstWidget() const;

  /**
    * This method creates a transaction based on the contents of the current widgets,
    * the splits in m_split in single selection mode or an existing transaction/split
    * and the contents of the widgets in multi selection mode.
    *
    * The split referencing the current account is returned as the first split in the
    * transaction's split list.
    *
    * @param t reference to created transaction
    * @param torig the original transaction
    * @param sorig the original split
    * @param skipPriceDialog if @p true the user will not be requested for price information
    *                        (defaults to @p false)
    *
    * @return @p false if aborted by user, @p true otherwise
    *
    * @note Usually not used directly. If unsure, use enterTransactions() instead.
    */
  bool createTransaction(MyMoneyTransaction& t, const MyMoneyTransaction& torig, const MyMoneySplit& sorig, bool skipPriceDialog = false);

public slots:
  int slotEditSplits();
  void slotUpdateAmount(const QString&);

protected slots:
  void slotReloadEditWidgets();
  void slotUpdatePayment(const QString&);
  void slotUpdateDeposit(const QString&);
  void slotUpdateCategory(const QString&);
  void slotUpdatePayee(const QString&);
  //void slotUpdateTag(const QString&);
  void slotUpdateCashFlow(KMyMoneyRegister::CashFlowDirection);
  void slotCreateCategory(const QString&, QString&);
  void slotUpdateAction(int action);
  void slotUpdateAccount(const QString& id);

protected:
  /**
    * This method creates all necessary widgets for this transaction editor.
    * All signals will be connected to the relevant slots.
    */
  void createEditWidgets();

  /**
    * This method (re-)loads the widgets with the transaction information
    * contained in @a m_transaction and @a m_split.
    *
    * @param action preset the edit wigdets for @a action if no transaction
    *               is present
    */
  void loadEditWidgets(KMyMoneyRegister::Action action = KMyMoneyRegister::ActionNone);

  void setupCategoryWidget(QString&);
  void updateAmount(const MyMoneyMoney& value);
  bool isTransfer(const QString& accId1, const QString& accId2) const;

  void checkPayeeInSplit(MyMoneySplit& s, const QString& payeeId);

  /**
    * This method fills the editor widgets with the last transaction
    * that can be found for payee @a payeeId in the account @a m_account.
    */
  void autoFill(const QString& payeeId);

  /**
    * Extracts the amount of the transaction from the widgets depending
    * if form or register based input method is used.
    * Returns if an amount has been found in @a update.
    *
    * @param update pointer to update information flag
    * @return amount of transaction (deposit positive, payment negative)
    */
  MyMoneyMoney amountFromWidget(bool* update = 0) const;

  /**
    * Create or update a VAT split
    */
  void updateVAT(bool amountChanged = true);

  MyMoneyMoney removeVatSplit();

  /**
    * This method adds a VAT split to transaction @a tr if necessary.
    *
    * @param tr transaction that the split should be added to
    * @param amount Amount to be used for the calculation. Depending upon the
    *               setting of the resp. category, this value is treated as
    *               either gross or net value.
    * @retval false VAT split has not been added
    * @retval true VAT split has been added
    */
  bool addVatSplit(MyMoneyTransaction& tr, const MyMoneyMoney& amount);

  void setupFinalWidgets();

  /**
   * This method returns the sum of all splits of transaction @a t that
   * reference account m_account.
   */
  MyMoneyMoney shares(const MyMoneyTransaction& t) const;

private:
  MyMoneyMoney        m_shares;
  bool                m_inUpdateVat;
};


#endif
