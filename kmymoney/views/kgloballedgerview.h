/***************************************************************************
                          kgloballedgerview.h  -  description
                             -------------------
    begin                : Sat Jul 13 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KGLOBALLEDGERVIEW_H
#define KGLOBALLEDGERVIEW_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"
#include "mymoneytransaction.h"
#include "mymoneysplit.h"

class MyMoneyAccount;
class MyMoneyReport;
class MyMoneySplit;
class MyMoneyTransaction;
class TransactionEditor;

namespace KMyMoneyRegister { class SelectedTransactions; }
namespace KMyMoneyRegister { class RegisterItem; }
namespace KMyMoneyRegister { class Transaction; }
namespace KMyMoneyTransactionForm { class TransactionForm; }
namespace eWidgets { namespace eRegister { enum class Action; } }
namespace eMyMoney { namespace Schedule { enum class Occurrence; } }

template <class T1, class T2> struct QPair;
template <typename T> class QList;

/**
  * @author Thomas Baumgart
  */
class KGlobalLedgerViewPrivate;
class KGlobalLedgerView : public KMyMoneyViewBase
{
  Q_OBJECT
public:
  explicit KGlobalLedgerView(QWidget *parent = nullptr);
  ~KGlobalLedgerView() override;

  void executeCustomAction(eView::Action action) override;
  void refresh();
  void updateActions(const MyMoneyObject& obj);
  void updateLedgerActions(const KMyMoneyRegister::SelectedTransactions& list);
  void updateLedgerActionsInternal();

  /**
    * This method returns the id of the currently selected account
    * or QString() if none is selected.
    */
  QString accountId() const;

  /**
    * Checks if new transactions can be created in the current context
    *
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the modify function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to create transactions (tooltip is not changed)
    * @retval false No, view does not support creation of transactions (tooltip is updated with message)
    */
  bool canCreateTransactions(QString& tooltip) const;

  /**
    * Checks if a list of transactions can be modified (edit/delete) in the current context
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the modify function is not available (in case
    *        of returning @c false)
    *
    * @retval true Yes, view allows to edit/delete transactions (tooltip is not changed)
    * @retval false No, view cannot edit/delete transactions (tooltip is updated with message)
    */
  bool canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  bool canDuplicateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /**
    * Checks if the list of transactions can be edited in the current context
    *
    * @param list list of selected transactions
    * @param tooltip reference to string receiving the tooltip text
    *        which explains why the edit function is not available (in case
    *        of returning @c false)
    *
    * @return @c true if edit operation is possible, @c false if not
    */
  bool canEditTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  TransactionEditor* startEdit(const KMyMoneyRegister::SelectedTransactions& list);

  /**
    * Method to prepare the ledger view to create a new transaction.
    * Returns if successful or not.
    *
    * retval true Empty transaction selected.
    * retval false Not successful (e.g. already editing a transaction)
    */
  bool selectEmptyTransaction();

public Q_SLOTS:
  /**
    * This slot is used to select the correct ledger view type for
    * the account specified by @p id in a specific mode.
    *
    * @param accountId Internal id used for the account to show
    * @param transactionId Internal id used for the transaction to select.
    *                      Default is QString() which will select the last
    *                      transaction in the ledger if not the same account
    *
    * @retval true selection of account referenced by @p id succeeded
    * @retval false selection of account failed
    */
  void slotSelectAccount(const QString& accountId);
  bool slotSelectAccount(const QString& accountId, const QString& transactionId);

  /**
    * This method is provided for convenience and acts as the method above.
    */
  void slotSelectAccount(const MyMoneyObject& acc);

  /**
   * Switch to reconciliation mode for account @a account.
   * If @a account is MyMoneyAccount() (the default), reconciliation mode
   * is turned off.
   *
   * @param account account for which reconciliation mode is activated.
   *                Default  is MyMoneyAccount().
   * @param reconciliationDate date of statement
   * @param endingBalance The calculated ending balance for the statement
   *                Default ist 0.
   */
  void slotSetReconcileAccount(const MyMoneyAccount& account, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);
  void slotSetReconcileAccount(const MyMoneyAccount& account, const QDate& reconciliationDate);
  void slotSetReconcileAccount(const MyMoneyAccount& account);
  void slotSetReconcileAccount();
  void slotShowTransactionMenu(const MyMoneySplit &sp);

  /**
   * Slot that should be entered after entering all due scheduled transactions
   * @param req is requester that made request to enter scheduled transactions
   * it's here to avoid reconciliation in case of random entering of scheduled transactions request
   */
  void slotContinueReconciliation();

  /**
    * Called, whenever the ledger view should pop up and a specific
    * transaction in an account should be shown. If @p transaction
    * is empty, the last transaction should be selected
    *
    * @param acc The ID of the account to be shown
    * @param transaction The ID of the transaction to be selected
    */
  void slotLedgerSelected(const QString& _accId, const QString& transaction);

  void slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent) override;
  void slotSelectByVariant(const QVariantList& variant, eView::Intent intent) override;

Q_SIGNALS:
  /**
    * This signal is emitted, when a new report has been generated.  A
    * 'generated' report is halfway between a default report and a custom
    * report.  It's created by the system in response to the user's
    * request, and it's usually filtered to be a little more specific
    * than the usual default reports.
    *
    * The proper behaviour when getting this signal is to switch to the
    * reports view and display the report.  But it should NOT be added
    * to the data file, unless the user customizes it further.  That's
    * because the user can always come back to the ledger UI to generate
    * the report again.
    *
    * @param report reference to MyMoneyReport object that contains the report
    *     details
    */
  void reportGenerated(const MyMoneyReport& report);

  /**
    * This signal is sent out, when the current selected transaction should
    * be marked different
    */
  void toggleReconciliationFlag();

protected:
  void showEvent(QShowEvent* event) override;
  void resizeEvent(QResizeEvent*) override;

  /**
    * This method handles the focus of the keyboard. When in edit mode
    * (m_inEditMode is true) the keyboard focus is handled
    * according to the widgets that are referenced in m_tabOrderWidgets.
    * If not in edit mode, the base class functionality is provided.
    *
    * @param next true if forward-tab, false if backward-tab was
    *             pressed by the user
    */
  bool focusNextPrevChild(bool next) override;

  bool eventFilter(QObject* o, QEvent* e) override;

private:
  Q_DECLARE_PRIVATE(KGlobalLedgerView)

private Q_SLOTS:

  void slotTransactionsContextMenuRequested();
  void slotLeaveEditMode(const KMyMoneyRegister::SelectedTransactions& list);

  void slotSortOptions();
  void slotToggleTransactionMark(KMyMoneyRegister::Transaction* t);

  void slotKeepPostDate(const QDate&);

  void slotAboutToSelectItem(KMyMoneyRegister::RegisterItem*, bool&);

  void slotUpdateSummaryLine(const KMyMoneyRegister::SelectedTransactions&);

  void slotMoveToAccount(const QString& id);
  void slotUpdateMoveToAccountMenu();
  void slotObjectDestroyed(QObject* o);
  void slotCancelOrEnterTransactions(bool& okToSelect);
  void slotNewSchedule(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence);
  void slotNewTransactionForm(eWidgets::eRegister::Action);

  void slotNewTransaction();
  void slotEditTransaction();
  void slotDeleteTransaction();
  void slotDuplicateTransaction(bool reverse = false);
  void slotEnterTransaction();

  /**
    * Accept the selected transactions that are marked as 'imported' and remove the flag
    */
  void slotAcceptTransaction();
  void slotCancelTransaction();
  void slotEditSplits();

  /**
   * This method takes the selected splits and checks that only one transaction (src)
   * has more than one split and all others have only a single one. It then copies the
   * splits of the @b src transaction to all others.
   */
  void slotCopySplits();
  void slotGoToPayee();
  void slotGoToAccount();
  void slotMatchTransactions();
  void slotCombineTransactions();
  void slotToggleReconciliationFlag();
  void slotMarkCleared();
  void slotMarkReconciled();
  void slotMarkNotReconciled();
  void slotSelectAllTransactions();
  void slotCreateScheduledTransaction();
  void slotAssignNumber();
  void slotCopyTransactionToClipboard();

  /**
    * Used to start reconciliation of account @a account. It switches the
    * ledger view into reconciliation mode and updates the view.
    *
    * @param account account which should be reconciled
    * @param reconciliationDate the statement date
    * @param endingBalance the ending balance entered for this account
    *
    * @retval true Reconciliation started
    * @retval false Account cannot be reconciled
    */
  void slotStartReconciliation();

  /**
    * Used to finish reconciliation of account @a account. It switches the
    * ledger view to normal mode and updates the view.
    *
    * @param account account which should be reconciled
    */
  void slotFinishReconciliation();
  void slotPostponeReconciliation();
  void slotOpenAccount();

  /**
    * Brings up a dialog to let the user search for specific transaction(s).  It then
    * opens a results window to display those transactions.
    */
  void slotFindTransaction();

  /**
    * Destroys a possibly open the search dialog
    */
  void slotCloseSearchDialog();

  void slotStatusMsg(const QString& txt);
  void slotStatusProgress(int cnt, int base);
  void slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& list);
};

#endif
