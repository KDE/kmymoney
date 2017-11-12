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

#include <QString>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes
class KToolBar;

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase.h"
#include "mymoneyaccount.h"
#include "registeritem.h"
#include "selectedtransactions.h"

class MyMoneyReport;
class MyMoneySplit;
class MyMoneyTransaction;
class TransactionEditor;
class QLabel;
class QFrame;

namespace KMyMoneyRegister { class Register; }
namespace KMyMoneyRegister { class Transaction; }
namespace KMyMoneyTransactionForm { class TransactionForm; }

/**
  * helper class implementing an event filter to detect mouse button press
  * events on widgets outside a given set of widgets. This is used internally
  * to detect when to leave the edit mode.
  */
class MousePressFilter : public QObject
{
  Q_OBJECT
public:
  explicit MousePressFilter(QWidget* parent = 0);

  /**
    * Add widget @p w to the list of possible parent objects. See eventFilter() how
    * they will be used.
    */
  void addWidget(QWidget* w);

public slots:
  /**
    * This slot allows to activate/deactivate the filter. By default the
    * filter is active.
    *
    * @param state Allows to activate (@a true) or deactivate (@a false) the filter
    */
  void setFilterActive(bool state = true);

  /**
    * This slot allows to activate/deactivate the filter. By default the
    * filter is active.
    *
    * @param state Allows to deactivate (@a true) or activate (@a false) the filter
    */
  void setFilterDeactive(bool state = false) {
    setFilterActive(!state);
  }

protected:
  /**
    * This method checks if the widget @p child is a child of
    * the widget @p parent and returns either @a true or @a false.
    *
    * @param child pointer to child widget
    * @param parent pointer to parent widget
    * @retval true @p child points to widget which has @p parent as parent or grand-parent
    * @retval false @p child points to a widget which is not related to @p parent
    */
  bool isChildOf(QWidget* child, QWidget* parent);

  /**
    * Reimplemented from base class. Sends out the mousePressedOnExternalWidget() signal
    * if object @p o points to an object which is not a child widget of any added previously
    * using the addWidget() method. The signal is sent out only once for each event @p e.
    *
    * @param o pointer to QObject
    * @param e pointer to QEvent
    * @return always returns @a false
    */
  bool eventFilter(QObject* o, QEvent* e);

signals:
  void mousePressedOnExternalWidget(bool&);

private:
  QList<QWidget*>      m_parents;
  QEvent*              m_lastMousePressEvent;
  bool                 m_filterActive;
};

/**
  * @author Thomas Baumgart
  */
class KGlobalLedgerView : public KMyMoneyViewBase
{
  Q_OBJECT
public:
  explicit KGlobalLedgerView(QWidget *parent = nullptr);
  ~KGlobalLedgerView();

  void setDefaultFocus() override;

  /**
    * This method returns the id of the currently selected account
    * or QString() if none is selected.
    */
  const QString accountId() const {
    return m_account.id();
  }

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
    * retval true Emtpy transaction selected.
    * retval false Not successful (e.g. already editing a transaction)
    */
  bool selectEmptyTransaction();

public slots:
  void showEvent(QShowEvent* event) override;

  /**
    * This method loads the view with data from the MyMoney engine.
    */
  void slotLoadView();

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
  bool slotSelectAccount(const QString& accountId, const QString& transactionId = QString());

  /**
    * This method is provided for convenience and acts as the method above.
    */
  bool slotSelectAccount(const MyMoneyObject& acc);

  /**
   * Switch to reconciliation mode for account @a account.
   * If @a account is MyMoneyAccount() (the default), reconciliation mode
   * is turned off.
   *
   * @param account account for which reconciliation mode is activated.
   *                Default  is MyMoneyAccount().
   * @param reconciliationDate date of statment
   * @param endingBalance The calculated ending balance for the statement
   *                Default ist 0.
   */
  void slotSetReconcileAccount(const MyMoneyAccount& account, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance);
  void slotSetReconcileAccount(const MyMoneyAccount& account, const QDate& reconciliationDate);
  void slotSetReconcileAccount(const MyMoneyAccount& account);
  void slotSetReconcileAccount();

  /**
    * Select all transactions in the ledger that are not hidden.
    */
  void slotSelectAllTransactions();

private:
  void showTooltip(const QString msg) const;

protected:
  /**
    * This method reloads the account selection combo box of the
    * view with all asset and liability accounts from the engine.
    * If the account id of the current account held in @p m_accountId is
    * empty or if the referenced account does not exist in the engine,
    * the first account found in the list will be made the current account.
    */
  void loadAccounts();

  /**
    * This method clears the register, form, transaction list. See @sa m_register,
    * @sa m_transactionList
    */
  void clear();

  void loadView();

  void resizeEvent(QResizeEvent*) override;

  void selectTransaction(const QString& id);

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

  /**
    * Returns @a true if setReconciliationAccount() has been called for
    * the current loaded account.
    *
    * @retval true current account is in reconciliation mode
    * @retval false current account is not in reconciliation mode
    */
  bool isReconciliationAccount() const;

  /**
    * Updates the values on the summary line beneath the register with
    * the given values. The contents shown differs between reconciliation
    * mode and normal mode.
    *
    * @param actBalance map of account indexed values to be used as actual balance
    * @param clearedBalance map of account indexed values to be used as cleared balance
    */
  void updateSummaryLine(const QMap<QString, MyMoneyMoney>& actBalance, const QMap<QString, MyMoneyMoney>& clearedBalance);

  /**
    * setup the default action according to the current account type
    */
  void setupDefaultAction();

protected slots:
  void slotLeaveEditMode(const KMyMoneyRegister::SelectedTransactions& list);
  void slotNewTransaction();
  void slotNewTransaction(KMyMoneyRegister::Action);

  void slotSortOptions();
  void slotToggleTransactionMark(KMyMoneyRegister::Transaction* t);

  void slotKeepPostDate(const QDate&);

  void slotAboutToSelectItem(KMyMoneyRegister::RegisterItem*, bool&);

  void slotUpdateSummaryLine(const KMyMoneyRegister::SelectedTransactions&);

protected:
  /**
    * This member keeps the date that was used as the last posting date.
    * It will be updated whenever the user modifies the post date
    * and is used to preset the posting date when new transactions are created.
    * This member is initialised to the current date when the program is started.
    */
  static QDate         m_lastPostDate;

private:
  /// \internal d-pointer class.
  class Private;
  /// \internal d-pointer instance.
  Private* const d;

  // frames
  QFrame*                       m_toolbarFrame;
  QFrame*                       m_registerFrame;
  QFrame*                       m_buttonFrame;
  QFrame*                       m_formFrame;
  QFrame*                       m_summaryFrame;

  // widgets
  KMyMoneyRegister::Register*   m_register;
  KToolBar*                     m_buttonbar;

  /**
    * This member holds the currently selected account
    */
  MyMoneyAccount m_account;

  /**
    * This member holds the transaction list
    */
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >  m_transactionList;

  QLabel*                         m_leftSummaryLabel;
  QLabel*                         m_centerSummaryLabel;
  QLabel*                         m_rightSummaryLabel;

  KMyMoneyTransactionForm::TransactionForm* m_form;

  bool                            m_needReload;

  /**
    * This member holds the load state of page
    */
  bool                            m_needLoad;

  bool                            m_newAccountLoaded;
  bool                            m_inEditMode;

  QWidgetList                     m_tabOrderWidgets;
  QPoint                          m_tooltipPosn;

signals:
  void accountSelected(const MyMoneyObject&);
  void transactionsSelected(const KMyMoneyRegister::SelectedTransactions&);
  void newTransaction();
  void startEdit();
  void endEdit();
  void cancelOrEndEdit(bool&);

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

  void openContextMenu();

  /**
    * This signal is sent out, when the current selected transaction should
    * be marked different
    */
  void toggleReconciliationFlag();

private:
  bool canProcessTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const;

  /** Initializes page and sets its load status to initialized
   */
  void init();
};

#endif
