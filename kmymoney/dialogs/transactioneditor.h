/*
 * Copyright 2007-2018  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017-2018  Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRANSACTIONEDITOR_H
#define TRANSACTIONEDITOR_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QWidgetList>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;

class TransactionEditorContainer;
class KMyMoneyCategory;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneyAccount;

template <typename T> class QList;

namespace eWidgets { namespace eRegister { enum class Action; } }

namespace KMyMoneyRegister {
  class SelectedTransactions;
  class Transaction;
}

namespace eMyMoney {
  namespace Schedule { enum class Occurrence;
                       enum class PaymentType; }
  namespace Split { enum class InvestmentTransactionType; }
                  }

class TransactionEditorPrivate;
class TransactionEditor : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(TransactionEditor)

public:
  TransactionEditor();
  explicit TransactionEditor(TransactionEditorPrivate &dd,
                             TransactionEditorContainer* regForm,
                             KMyMoneyRegister::Transaction* item,
                             const KMyMoneyRegister::SelectedTransactions& list,
                             const QDate& lastPostDate);
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
  void setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account, eWidgets::eRegister::Action action);
  void setup(QWidgetList& tabOrderWidgets, const MyMoneyAccount& account);
  void setup(QWidgetList& tabOrderWidgets);

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
  virtual bool isMultiSelection() const;

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

  bool eventFilter(QObject* o, QEvent* e) override;

  MyMoneyAccount account() const;

  void clearFinalWidgets();

  void addFinalWidget(const QWidget*);

  void setScheduleInfo(const QString& si);
  void setPaymentMethod(eMyMoney::Schedule::PaymentType pm);

  /**
   * This method returns if the editor is currently used to create a new transaction
   * or editing existing transaction(s).
   *
   * @returns @c true in case it creates a new transaction, @c false otherwise
   */
  bool createNewTransaction() const;

public Q_SLOTS:
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
  virtual void loadEditWidgets(eWidgets::eRegister::Action action) = 0;
  virtual void loadEditWidgets() = 0;
  void setupCategoryWidget(KMyMoneyCategory* category, const QList<MyMoneySplit>& splits, QString& categoryId, const char* splitEditSlot, bool allowObjectCreation = true);
  void resizeForm();

  /**
   * This method sets the precision of the value widgets to reflect
   * the account in m_account. If m_account has no id, the precision
   * defaults to 2.
   */
  void setupPrecision();

protected Q_SLOTS:
  void slotUpdateButtonState();
  void slotUpdateMemoState();
  void slotUpdateAccount();
  void slotNumberChanged(const QString&);

Q_SIGNALS:
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
  void escapePressed(int msec = 100);

  /**
   * This signal is sent out, if the user has pressed the Return or Enter
   * key and asks to end editing the transaction
   */
  void returnPressed(int msec = 100);

  /**
   * This signal is sent out, if any of the balance warning levels
   * for @p account has been reached. @p msg contains the message text.
   * @p parent points to the parent widget to be used for the warning message box.
   */
  void balanceWarning(QWidget* parent, const MyMoneyAccount& account, const QString& msg);

  void operationTypeChanged(int index);

protected:
  QScopedPointer<TransactionEditorPrivate> d_ptr;
  explicit TransactionEditor(TransactionEditorPrivate &dd);

protected Q_SLOTS:
  void slotNewPayee(const QString& newnameBase, QString& id);
  void slotNewTag(const QString& newnameBase, QString& id);
  void slotNewCategory(MyMoneyAccount& account, const MyMoneyAccount& parent);
  void slotNewInvestment(MyMoneyAccount& account, const MyMoneyAccount& parent);

private:
  Q_DECLARE_PRIVATE(TransactionEditor)
};

#endif
