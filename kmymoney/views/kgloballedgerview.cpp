/***************************************************************************
                          kgloballedgerview.cpp  -  description
                             -------------------
    begin                : Wed Jul 26 2006
    copyright            : (C) 2006 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kgloballedgerview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
#include <QList>
#include <QLabel>
#include <QEvent>
#include <QApplication>
#include <QTimer>
#include <QMenu>
#include <QClipboard>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KToolBar>
#include <KActionCollection>
#include <KXmlGuiWindow>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "kmymoneyaccountcombo.h"
#include "kmymoneypayeecombo.h"
#include "keditscheduledlg.h"
#include "kendingbalancedlg.h"
#include "register.h"
#include "transactioneditor.h"
#include "selectedtransactions.h"
#include "kmymoneysettings.h"
#include "registersearchline.h"
#include "kfindtransactiondlg.h"
#include "accountsmodel.h"
#include "models.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneysplit.h"
#include "transaction.h"
#include "transactionform.h"
#include "widgetenums.h"
#include "mymoneyenums.h"
#include "menuenums.h"

using namespace eMenu;

QDate KGlobalLedgerViewPrivate::m_lastPostDate;

KGlobalLedgerView::KGlobalLedgerView(QWidget *parent) :
  KMyMoneyViewBase(*new KGlobalLedgerViewPrivate(this), parent)
{
  const QHash<Action, std::function<void()>> actionConnections {
    {Action::NewTransaction,            [this](){ KGlobalLedgerView::slotNewTransaction(); }},
    {Action::EditTransaction,           [this](){ KGlobalLedgerView::slotEditTransaction(); }},
    {Action::DeleteTransaction,         [this](){ KGlobalLedgerView::slotDeleteTransaction(); }},
    {Action::DuplicateTransaction,      [this](){ KGlobalLedgerView::slotDuplicateTransaction(); }},
    {Action::AddReversingTransaction,   [this](){ KGlobalLedgerView::slotDuplicateTransaction(true); }},
    {Action::EnterTransaction,          [this](){ KGlobalLedgerView::slotEnterTransaction(); }},
    {Action::AcceptTransaction,         [this](){ KGlobalLedgerView::slotAcceptTransaction(); }},
    {Action::CancelTransaction,         [this](){ KGlobalLedgerView::slotCancelTransaction(); }},
    {Action::EditSplits,                [this](){ KGlobalLedgerView::slotEditSplits(); }},
    {Action::CopySplits,                [this](){ KGlobalLedgerView::slotCopySplits(); }},
    {Action::GoToPayee,                 [this](){ KGlobalLedgerView::slotGoToPayee(); }},
    {Action::GoToAccount,               [this](){ KGlobalLedgerView::slotGoToAccount(); }},
    {Action::MatchTransaction,          [this](){ KGlobalLedgerView::slotMatchTransactions(); }},
    {Action::CombineTransactions,       [this](){ KGlobalLedgerView::slotCombineTransactions(); }},
    {Action::ToggleReconciliationFlag,  [this](){ KGlobalLedgerView::slotToggleReconciliationFlag(); }},
    {Action::MarkCleared,               [this](){ KGlobalLedgerView::slotMarkCleared(); }},
    {Action::MarkReconciled,            [this](){ KGlobalLedgerView::slotMarkReconciled(); }},
    {Action::MarkNotReconciled,         [this](){ KGlobalLedgerView::slotMarkNotReconciled(); }},
    {Action::SelectAllTransactions,     [this](){ KGlobalLedgerView::slotSelectAllTransactions(); }},
    {Action::NewScheduledTransaction,   [this](){ KGlobalLedgerView::slotCreateScheduledTransaction(); }},
    {Action::AssignTransactionsNumber,  [this](){ KGlobalLedgerView::slotAssignNumber(); }},
    {Action::StartReconciliation,       [this](){ KGlobalLedgerView::slotStartReconciliation(); }},
    {Action::FinishReconciliation,      [this](){ KGlobalLedgerView::slotFinishReconciliation(); }},
    {Action::PostponeReconciliation,    [this](){ KGlobalLedgerView::slotPostponeReconciliation(); }},
    {Action::OpenAccount,               [this](){ KGlobalLedgerView::slotOpenAccount(); }},
    {Action::EditFindTransaction,       [this](){ KGlobalLedgerView::slotFindTransaction(); }},
  };

  for (auto a = actionConnections.cbegin(); a != actionConnections.cend(); ++a)
    connect(pActions[a.key()], &QAction::triggered, this, a.value());

  KXmlGuiWindow* mw = KMyMoneyUtils::mainWindow();
  KStandardAction::copy(this, &KGlobalLedgerView::slotCopyTransactionToClipboard,  mw->actionCollection());

  Q_D(KGlobalLedgerView);
  d->m_balanceWarning.reset(new KBalanceWarning(this));
}

KGlobalLedgerView::~KGlobalLedgerView()
{
}

void KGlobalLedgerView::executeCustomAction(eView::Action action)
{
  Q_D(KGlobalLedgerView);
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      // delay the setFocus call until the event loop is running
      QMetaObject::invokeMethod(d->m_registerSearchLine->searchLine(), "setFocus", Qt::QueuedConnection);
      break;

    case eView::Action::DisableViewDepenedendActions:
      pActions[Action::SelectAllTransactions]->setEnabled(false);
      break;

    case eView::Action::InitializeAfterFileOpen:
      d->m_lastSelectedAccountID.clear();
      d->m_currentAccount = MyMoneyAccount();
      if (d->m_accountComboBox) {
        d->m_accountComboBox->setSelected(QString());
      }
      break;

    case eView::Action::CleanupBeforeFileClose:
      if (d->m_inEditMode) {
        d->deleteTransactionEditor();
      }
      break;

    default:
      break;
  }
}

void KGlobalLedgerView::refresh()
{
  Q_D(KGlobalLedgerView);
  if (isVisible()) {
    if (!d->m_inEditMode) {
      setUpdatesEnabled(false);
      d->loadView();
      setUpdatesEnabled(true);
      d->m_needsRefresh = false;
      // force a new account if the current one is empty
      d->m_newAccountLoaded = d->m_currentAccount.id().isEmpty();
    }
  } else {
    d->m_needsRefresh = true;
  }
}

void KGlobalLedgerView::showEvent(QShowEvent* event)
{
  if (MyMoneyFile::instance()->storageAttached()) {
    Q_D(KGlobalLedgerView);
    if (d->m_needLoad)
      d->init();

    emit customActionRequested(View::Ledgers, eView::Action::AboutToShow);

    if (d->m_needsRefresh) {
      if (!d->m_inEditMode) {
        setUpdatesEnabled(false);
        d->loadView();
        setUpdatesEnabled(true);
        d->m_needsRefresh = false;
        d->m_newAccountLoaded = false;
      }

    } else {
      if (!d->m_lastSelectedAccountID.isEmpty()) {
        try {
          const auto acc = MyMoneyFile::instance()->account(d->m_lastSelectedAccountID);
          slotSelectAccount(acc.id());
        } catch (const MyMoneyException &) {
          d->m_lastSelectedAccountID.clear();                                               // account is invalid
        }
      } else {
        slotSelectAccount(d->m_accountComboBox->getSelected());
      }

      KMyMoneyRegister::SelectedTransactions list(d->m_register);
      updateLedgerActions(list);
      emit selectByVariant(QVariantList {QVariant::fromValue(list)}, eView::Intent::SelectRegisterTransactions);
    }
  }

  pActions[Action::SelectAllTransactions]->setEnabled(true);
  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KGlobalLedgerView::updateActions(const MyMoneyObject& obj)
{
  Q_D(KGlobalLedgerView);
//  if (typeid(obj) != typeid(MyMoneyAccount) &&
//      (obj.id().isEmpty() && d->m_currentAccount.id().isEmpty())) // do not disable actions that were already disabled))
//    return;

  const auto& acc = static_cast<const MyMoneyAccount&>(obj);

  const QVector<Action> actionsToBeDisabled {
        Action::StartReconciliation,
        Action::FinishReconciliation,
        Action::PostponeReconciliation,
        Action::OpenAccount,
        Action::NewTransaction
  };

  for (const auto& a : actionsToBeDisabled)
    pActions[a]->setEnabled(false);

  auto b = acc.isClosed() ? false : true;
  pMenus[Menu::MoveTransaction]->setEnabled(b);

  QString tooltip;
  pActions[Action::NewTransaction]->setEnabled(canCreateTransactions(tooltip) || !isVisible());
  pActions[Action::NewTransaction]->setToolTip(tooltip);

  const auto file = MyMoneyFile::instance();
  if (!acc.id().isEmpty() && !file->isStandardAccount(acc.id())) {
    switch (acc.accountGroup()) {
      case eMyMoney::Account::Type::Asset:
      case eMyMoney::Account::Type::Liability:
      case eMyMoney::Account::Type::Equity:
        pActions[Action::OpenAccount]->setEnabled(true);
        if (acc.accountGroup() != eMyMoney::Account::Type::Equity) {
          if (d->m_reconciliationAccount.id().isEmpty()) {
            pActions[Action::StartReconciliation]->setEnabled(true);
            pActions[Action::StartReconciliation]->setToolTip(i18n("Reconcile"));
          } else {
            auto tip = i18n("Reconcile - disabled because you are currently reconciling <b>%1</b>", d->m_reconciliationAccount.name());
            pActions[Action::StartReconciliation]->setToolTip(tip);
            if (!d->m_transactionEditor) {
              pActions[Action::FinishReconciliation]->setEnabled(acc.id() == d->m_reconciliationAccount.id());
              pActions[Action::PostponeReconciliation]->setEnabled(acc.id() == d->m_reconciliationAccount.id());
            }
          }
        }
        break;
      case eMyMoney::Account::Type::Income :
      case eMyMoney::Account::Type::Expense :
        pActions[Action::OpenAccount]->setEnabled(true);
        break;
      default:
        break;
    }
  }

  d->m_currentAccount = acc;
//  slotSelectAccount(acc);
}

void KGlobalLedgerView::updateLedgerActions(const KMyMoneyRegister::SelectedTransactions& list)
{
  Q_D(KGlobalLedgerView);

  d->selectTransactions(list);
  updateLedgerActionsInternal();
}

void KGlobalLedgerView::updateLedgerActionsInternal()
{
  Q_D(KGlobalLedgerView);
  const QVector<Action> actionsToBeDisabled {
    Action::EditTransaction, Action::EditSplits, Action::EnterTransaction,
    Action::CancelTransaction, Action::DeleteTransaction, Action::MatchTransaction,
    Action::AcceptTransaction, Action::DuplicateTransaction, Action::AddReversingTransaction, Action::ToggleReconciliationFlag, Action::MarkCleared,
    Action::GoToAccount, Action::GoToPayee, Action::AssignTransactionsNumber, Action::NewScheduledTransaction,
    Action::CombineTransactions, Action::CopySplits,
  };

  for (const auto& a : actionsToBeDisabled)
    pActions[a]->setEnabled(false);

  const auto file = MyMoneyFile::instance();

  pActions[Action::MatchTransaction]->setText(i18nc("Button text for match transaction", "Match"));
//  pActions[Action::TransactionNew]->setToolTip(i18n("Create a new transaction"));

  pMenus[Menu::MoveTransaction]->setEnabled(false);
  pMenus[Menu::MarkTransaction]->setEnabled(false);
  pMenus[Menu::MarkTransactionContext]->setEnabled(false);

  if (!d->m_selectedTransactions.isEmpty() && !d->m_selectedTransactions.first().isScheduled()) {
    // enable 'delete transaction' only if at least one of the
    // selected transactions does not reference a closed account
    bool enable = false;
    KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
    for (it_t = d->m_selectedTransactions.constBegin(); (enable == false) && (it_t != d->m_selectedTransactions.constEnd()); ++it_t) {
      enable = !(*it_t).transaction().id().isEmpty() && !file->referencesClosedAccount((*it_t).transaction());
    }
    pActions[Action::DeleteTransaction]->setEnabled(enable);

    if (!d->m_transactionEditor) {
      QString tooltip = i18n("Duplicate the current selected transactions");
      pActions[Action::DuplicateTransaction]->setEnabled(canDuplicateTransactions(d->m_selectedTransactions, tooltip) && !d->m_selectedTransactions[0].transaction().id().isEmpty());
      pActions[Action::DuplicateTransaction]->setToolTip(tooltip);

      tooltip = i18n("Add reversing transactions to the currently selected");
      pActions[Action::AddReversingTransaction]->setEnabled(canDuplicateTransactions(d->m_selectedTransactions, tooltip) && !d->m_selectedTransactions[0].transaction().id().isEmpty());
      pActions[Action::AddReversingTransaction]->setToolTip(tooltip);

      if (canEditTransactions(d->m_selectedTransactions, tooltip)) {
        pActions[Action::EditTransaction]->setEnabled(true);
        // editing splits is allowed only if we have one transaction selected
        if (d->m_selectedTransactions.count() == 1) {
          pActions[Action::EditSplits]->setEnabled(true);
        }
        if (d->m_currentAccount.isAssetLiability() && d->m_currentAccount.accountType() != eMyMoney::Account::Type::Investment) {
          pActions[Action::NewScheduledTransaction]->setEnabled(d->m_selectedTransactions.count() == 1);
        }
      }
      pActions[Action::EditTransaction]->setToolTip(tooltip);

      if (!d->m_currentAccount.isClosed())
        pMenus[Menu::MoveTransaction]->setEnabled(true);

      pMenus[Menu::MarkTransaction]->setEnabled(true);
      pMenus[Menu::MarkTransactionContext]->setEnabled(true);

      // Allow marking the transaction if at least one is selected
      pActions[Action::MarkCleared]->setEnabled(true);
      pActions[Action::MarkReconciled]->setEnabled(true);
      pActions[Action::MarkNotReconciled]->setEnabled(true);
      pActions[Action::ToggleReconciliationFlag]->setEnabled(true);

      if (!d->m_accountGoto.isEmpty())
        pActions[Action::GoToAccount]->setEnabled(true);
      if (!d->m_payeeGoto.isEmpty())
        pActions[Action::GoToPayee]->setEnabled(true);

      // Matching is enabled as soon as one regular and one imported transaction is selected
      int matchedCount = 0;
      int importedCount = 0;
      KMyMoneyRegister::SelectedTransactions::const_iterator it;
      for (it = d->m_selectedTransactions.constBegin(); it != d->m_selectedTransactions.constEnd(); ++it) {
        if ((*it).transaction().isImported())
          ++importedCount;
        if ((*it).split().isMatched())
          ++matchedCount;
      }

      if (d->m_selectedTransactions.count() == 2 /* && pActions[Action::TransactionEdit]->isEnabled() */) {
        pActions[Action::MatchTransaction]->setEnabled(true);
      }
      if (importedCount != 0 || matchedCount != 0)
        pActions[Action::AcceptTransaction]->setEnabled(true);
      if (matchedCount != 0) {
        pActions[Action::MatchTransaction]->setEnabled(true);
        pActions[Action::MatchTransaction]->setText(i18nc("Button text for unmatch transaction", "Unmatch"));
        pActions[Action::MatchTransaction]->setIcon(QIcon("process-stop"));
      }

      if (d->m_selectedTransactions.count() > 1) {
        pActions[Action::CombineTransactions]->setEnabled(true);
      }
      if (d->m_selectedTransactions.count() >= 2) {
        int singleSplitTransactions = 0;
        int multipleSplitTransactions = 0;
        foreach (const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
          switch (st.transaction().splitCount()) {
            case 0:
              break;
            case 1:
              singleSplitTransactions++;
              break;
            default:
              multipleSplitTransactions++;
              break;
          }
        }
        if (singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
          pActions[Action::CopySplits]->setEnabled(true);
        }
      }
      if (d->m_selectedTransactions.count() >= 2) {
        int singleSplitTransactions = 0;
        int multipleSplitTransactions = 0;
        foreach(const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
          switch(st.transaction().splitCount()) {
            case 0:
              break;
            case 1:
              singleSplitTransactions++;
              break;
            default:
              multipleSplitTransactions++;
              break;
          }
        }
        if(singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
          pActions[Action::CopySplits]->setEnabled(true);
        }
      }
    } else {
      pActions[Action::AssignTransactionsNumber]->setEnabled(d->m_transactionEditor->canAssignNumber());
      pActions[Action::NewTransaction]->setEnabled(false);
      pActions[Action::DeleteTransaction]->setEnabled(false);
      QString reason;
      pActions[Action::EnterTransaction]->setEnabled(d->m_transactionEditor->isComplete(reason));
      //FIXME: Port to KDE4
      // the next line somehow worked in KDE3 but does not have
      // any influence under KDE4
      ///  Works for me when 'reason' is set. Allan
      pActions[Action::EnterTransaction]->setToolTip(reason);
      pActions[Action::CancelTransaction]->setEnabled(true);
    }
  }
}

void KGlobalLedgerView::slotAboutToSelectItem(KMyMoneyRegister::RegisterItem* item, bool& okToSelect)
{
  Q_UNUSED(item);
  slotCancelOrEnterTransactions(okToSelect);
}

void KGlobalLedgerView::slotUpdateSummaryLine(const KMyMoneyRegister::SelectedTransactions& selection)
{
  Q_D(KGlobalLedgerView);
  if (selection.count() > 1) {
    MyMoneyMoney balance;
    foreach (const KMyMoneyRegister::SelectedTransaction& t, selection) {
      if (!t.isScheduled()) {
        balance += t.split().shares();
      }
    }
    d->m_rightSummaryLabel->setText(QString("%1: %2").arg(QChar(0x2211), balance.formatMoney("", d->m_precision)));

  } else {
    if (d->isReconciliationAccount()) {
      d->m_rightSummaryLabel->setText(i18n("Difference: %1", d->m_totalBalance.formatMoney("", d->m_precision)));

    } else {
      if (d->m_currentAccount.accountType() != eMyMoney::Account::Type::Investment) {
        d->m_rightSummaryLabel->setText(i18n("Balance: %1", d->m_totalBalance.formatMoney("", d->m_precision)));
        bool showNegative = d->m_totalBalance.isNegative();
        if (d->m_currentAccount.accountGroup() == eMyMoney::Account::Type::Liability && !d->m_totalBalance.isZero())
          showNegative = !showNegative;
        if (showNegative) {
          QPalette palette = d->m_rightSummaryLabel->palette();
          palette.setColor(d->m_rightSummaryLabel->foregroundRole(), KMyMoneySettings::schemeColor(SchemeColor::Negative));
          d->m_rightSummaryLabel->setPalette(palette);
        }
      } else {
        d->m_rightSummaryLabel->setText(i18n("Investment value: %1%2",
                                             d->m_balanceIsApproximated ? "~" : "",
                                             d->m_totalBalance.formatMoney(MyMoneyFile::instance()->baseCurrency().tradingSymbol(), d->m_precision)));
      }
    }
  }
}

void KGlobalLedgerView::resizeEvent(QResizeEvent* ev)
{
  if (MyMoneyFile::instance()->storageAttached()) {
    Q_D(KGlobalLedgerView);
    if (d->m_needLoad)
      d->init();

    d->m_register->resize((int)eWidgets::eTransaction::Column::Detail);
    d->m_form->resize((int)eWidgets::eTransactionForm::Column::Value1);
  }
  KMyMoneyViewBase::resizeEvent(ev);
}

void KGlobalLedgerView::slotSetReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
{
  Q_D(KGlobalLedgerView);
  if(d->m_needLoad)
    d->init();

  if (d->m_reconciliationAccount.id() != acc.id()) {
    // make sure the account is selected
    if (!acc.id().isEmpty())
      slotSelectAccount(acc.id());

    d->m_reconciliationAccount = acc;
    d->m_reconciliationDate = reconciliationDate;
    d->m_endingBalance = endingBalance;
    if (acc.accountGroup() == eMyMoney::Account::Type::Liability)
      d->m_endingBalance = -endingBalance;

    d->m_newAccountLoaded = true;

    if (acc.id().isEmpty()) {
      d->m_buttonbar->removeAction(pActions[Action::PostponeReconciliation]);
      d->m_buttonbar->removeAction(pActions[Action::FinishReconciliation]);
    } else {
      d->m_buttonbar->addAction(pActions[Action::PostponeReconciliation]);
      d->m_buttonbar->addAction(pActions[Action::FinishReconciliation]);
      // when we start reconciliation, we need to reload the view
      // because no data has been changed. When postponing or finishing
      // reconciliation, the data change in the engine takes care of updating
      // the view.
      refresh();
    }
  }
}

void KGlobalLedgerView::slotSetReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate)
{
  slotSetReconcileAccount(acc, reconciliationDate, MyMoneyMoney());
}

void KGlobalLedgerView::slotSetReconcileAccount(const MyMoneyAccount& acc)
{
  slotSetReconcileAccount(acc, QDate(), MyMoneyMoney());
}

void KGlobalLedgerView::slotSetReconcileAccount()
{
  slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
}

void KGlobalLedgerView::slotShowTransactionMenu(const MyMoneySplit& sp)
{
  Q_UNUSED(sp)
  pMenus[Menu::Transaction]->exec(QCursor::pos());
}

void KGlobalLedgerView::slotContinueReconciliation()
{
  Q_D(KGlobalLedgerView);
  const auto file = MyMoneyFile::instance();
  MyMoneyAccount account;

  try {
    account = file->account(d->m_currentAccount.id());
    // get rid of previous run.
    delete d->m_endingBalanceDlg;
    d->m_endingBalanceDlg = new KEndingBalanceDlg(account, this);
    if (account.isAssetLiability()) {

      if (d->m_endingBalanceDlg->exec() == QDialog::Accepted) {
        if (KMyMoneySettings::autoReconciliation()) {
          MyMoneyMoney startBalance = d->m_endingBalanceDlg->previousBalance();
          MyMoneyMoney endBalance = d->m_endingBalanceDlg->endingBalance();
          QDate endDate = d->m_endingBalanceDlg->statementDate();

          QList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
          MyMoneyTransactionFilter filter(account.id());
          filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
          filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
          filter.setDateFilter(QDate(), endDate);
          filter.setConsiderCategory(false);
          filter.setReportAllSplits(true);
          file->transactionList(transactionList, filter);
          QList<QPair<MyMoneyTransaction, MyMoneySplit> > result = d->automaticReconciliation(account, transactionList, endBalance - startBalance);

          if (!result.empty()) {
            QString message = i18n("KMyMoney has detected transactions matching your reconciliation data.\nWould you like KMyMoney to clear these transactions for you?");
            if (KMessageBox::questionYesNo(this,
                                           message,
                                           i18n("Automatic reconciliation"),
                                           KStandardGuiItem::yes(),
                                           KStandardGuiItem::no(),
                                           "AcceptAutomaticReconciliation") == KMessageBox::Yes) {
              // mark the transactions cleared
              KMyMoneyRegister::SelectedTransactions oldSelection = d->m_selectedTransactions;
              d->m_selectedTransactions.clear();
              QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplitResult(result);
              while (itTransactionSplitResult.hasNext()) {
                const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplitResult.next();
                d->m_selectedTransactions.append(KMyMoneyRegister::SelectedTransaction(transactionSplit.first, transactionSplit.second, QString()));
              }
              // mark all transactions in d->m_selectedTransactions as 'Cleared'
              d->markTransaction(eMyMoney::Split::State::Cleared);
              d->m_selectedTransactions = oldSelection;
            }
          }
        }

        if (!file->isStandardAccount(account.id()) &&
            account.isAssetLiability()) {
          if (!isVisible())
            emit customActionRequested(View::Ledgers, eView::Action::SwitchView);
          Models::instance()->accountsModel()->slotReconcileAccount(account, d->m_endingBalanceDlg->statementDate(), d->m_endingBalanceDlg->endingBalance());
          slotSetReconcileAccount(account, d->m_endingBalanceDlg->statementDate(), d->m_endingBalanceDlg->endingBalance());

          // check if the user requests us to create interest
          // or charge transactions.
          auto ti = d->m_endingBalanceDlg->interestTransaction();
          auto tc = d->m_endingBalanceDlg->chargeTransaction();
          MyMoneyFileTransaction ft;
          try {
            if (ti != MyMoneyTransaction()) {
              MyMoneyFile::instance()->addTransaction(ti);
            }
            if (tc != MyMoneyTransaction()) {
              MyMoneyFile::instance()->addTransaction(tc);
            }
            ft.commit();

          } catch (const MyMoneyException &e) {
            qWarning("interest transaction not stored: '%s'", e.what());
          }

          // reload the account object as it might have changed in the meantime
          d->m_reconciliationAccount = file->account(account.id());
          updateActions(d->m_currentAccount);
          updateLedgerActionsInternal();
          //            slotUpdateActions();
        }
      }
    }
  } catch (const MyMoneyException &) {
  }
}

void KGlobalLedgerView::slotLedgerSelected(const QString& _accId, const QString& transaction)
{
  auto acc = MyMoneyFile::instance()->account(_accId);
  QString accId(_accId);

  switch (acc.accountType()) {
    case Account::Type::Stock:
      // if a stock account is selected, we show the
      // the corresponding parent (investment) account
      acc = MyMoneyFile::instance()->account(acc.parentAccountId());
      accId = acc.id();
      // intentional fall through

    case Account::Type::Checkings:
    case Account::Type::Savings:
    case Account::Type::Cash:
    case Account::Type::CreditCard:
    case Account::Type::Loan:
    case Account::Type::Asset:
    case Account::Type::Liability:
    case Account::Type::AssetLoan:
    case Account::Type::Income:
    case Account::Type::Expense:
    case Account::Type::Investment:
    case Account::Type::Equity:
      if (!isVisible())
        emit customActionRequested(View::Ledgers, eView::Action::SwitchView);
      slotSelectAccount(accId, transaction);
      break;

    case Account::Type::CertificateDep:
    case Account::Type::MoneyMarket:
    case Account::Type::Currency:
      qDebug("No ledger view available for account type %d", (int)acc.accountType());
      break;

    default:
      qDebug("Unknown account type %d in KMyMoneyView::slotLedgerSelected", (int)acc.accountType());
      break;
  }
}

void KGlobalLedgerView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
  switch(intent) {
    case eView::Intent::UpdateActions:
      updateActions(obj);
      break;

    case eView::Intent::FinishEnteringOverdueScheduledTransactions:
      slotContinueReconciliation();
      break;

    case eView::Intent::SynchronizeAccountInLedgersView:
      slotSelectAccount(obj);
      break;

    default:
      break;
  }
}

void KGlobalLedgerView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
  switch(intent) {
    case eView::Intent::ShowTransaction:
      if (variant.count() == 2)
        slotLedgerSelected(variant.at(0).toString(), variant.at(1).toString());
      break;
    case eView::Intent::SelectRegisterTransactions:
      if (variant.count() == 1)
        updateLedgerActions(variant.at(0).value<KMyMoneyRegister::SelectedTransactions>());
      break;
    default:
      break;
  }
}

void KGlobalLedgerView::slotSelectAccount(const MyMoneyObject& obj)
{
  Q_D(KGlobalLedgerView);
  if (typeid(obj) != typeid(MyMoneyAccount))
    return/* false */;

  d->m_lastSelectedAccountID = obj.id();
}

void KGlobalLedgerView::slotSelectAccount(const QString& id)
{
  slotSelectAccount(id, QString());
}

bool KGlobalLedgerView::slotSelectAccount(const QString& id, const QString& transactionId)
{
  Q_D(KGlobalLedgerView);
  auto rc = true;

  if (!id.isEmpty()) {
    if (d->m_currentAccount.id() != id) {
      try {
        d->m_currentAccount = MyMoneyFile::instance()->account(id);
        // if a stock account is selected, we show the
        // the corresponding parent (investment) account
        if (d->m_currentAccount.isInvest()) {
          d->m_currentAccount = MyMoneyFile::instance()->account(d->m_currentAccount.parentAccountId());
        }
        d->m_lastSelectedAccountID = d->m_currentAccount.id();
        d->m_newAccountLoaded = true;
        refresh();
      } catch (const MyMoneyException &) {
        qDebug("Unable to retrieve account %s", qPrintable(id));
        rc = false;
      }
    } else {
      // we need to refresh m_account.m_accountList, a child could have been deleted
      d->m_currentAccount = MyMoneyFile::instance()->account(id);

      emit selectByObject(d->m_currentAccount, eView::Intent::None);
      emit selectByObject(d->m_currentAccount, eView::Intent::SynchronizeAccountInInvestmentView);
    }
    d->selectTransaction(transactionId);
  }
  return rc;
}

bool KGlobalLedgerView::selectEmptyTransaction()
{
  Q_D(KGlobalLedgerView);
  bool rc = false;

  if (!d->m_inEditMode) {
    // in case we don't know the type of transaction to be created,
    // have at least one selected transaction and the id of
    // this transaction is not empty, we take it as template for the
    // transaction to be created
    KMyMoneyRegister::SelectedTransactions list(d->m_register);
    if ((d->m_action == eWidgets::eRegister::Action::None) && (!list.isEmpty()) && (!list[0].transaction().id().isEmpty())) {
      // the new transaction to be created will have the same type
      // as the one that currently has the focus
      KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(d->m_register->focusItem());
      if (t)
        d->m_action = t->actionType();
      d->m_register->clearSelection();
    }

    // if we still don't have an idea which type of transaction
    // to create, we use the default.
    if (d->m_action == eWidgets::eRegister::Action::None) {
      d->setupDefaultAction();
    }

    d->m_register->selectItem(d->m_register->lastItem());
    d->m_register->updateRegister();
    rc = true;
  }
  return rc;
}

TransactionEditor* KGlobalLedgerView::startEdit(const KMyMoneyRegister::SelectedTransactions& list)
{
  Q_D(KGlobalLedgerView);
  // we use the warnlevel to keep track, if we have to warn the
  // user that some or all splits have been reconciled or if the
  // user cannot modify the transaction if at least one split
  // has the status frozen. The following value are used:
  //
  // 0 - no sweat, user can modify
  // 1 - user should be warned that at least one split has been reconciled
  //     already
  // 2 - user will be informed, that this transaction cannot be changed anymore

  int warnLevel = list.warnLevel();
  Q_ASSERT(warnLevel < 2);  // otherwise the edit action should not be enabled

  switch (warnLevel) {
    case 0:
      break;

    case 1:
      if (KMessageBox::warningContinueCancel(this,
                                             i18n(
                                               "At least one split of the selected transactions has been reconciled. "
                                               "Do you wish to continue to edit the transactions anyway?"
                                               ),
                                             i18n("Transaction already reconciled"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                                             "EditReconciledTransaction") == KMessageBox::Cancel) {
        warnLevel = 2;
      }
      break;

    case 2:
      KMessageBox::sorry(this,
                         i18n("At least one split of the selected transactions has been frozen. "
                              "Editing the transactions is therefore prohibited."),
                         i18n("Transaction already frozen"));
      break;

    case 3:
      KMessageBox::sorry(this,
                         i18n("At least one split of the selected transaction references an account that has been closed. "
                              "Editing the transactions is therefore prohibited."),
                         i18n("Account closed"));
      break;
  }

  if (warnLevel > 1) {
    d->m_register->endEdit();
    return 0;
  }


  TransactionEditor* editor = 0;
  KMyMoneyRegister::Transaction* item = dynamic_cast<KMyMoneyRegister::Transaction*>(d->m_register->focusItem());

  if (item) {
    // in case the current focus item is not selected, we move the focus to the first selected transaction
    if (!item->isSelected()) {
      KMyMoneyRegister::RegisterItem* p;
      for (p = d->m_register->firstItem(); p; p = p->nextItem()) {
        KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
        if (t && t->isSelected()) {
          d->m_register->setFocusItem(t);
          item = t;
          break;
        }
      }
    }

    // decide, if we edit in the register or in the form
    TransactionEditorContainer* parent;
    if (d->m_formFrame->isVisible())
      parent = d->m_form;
    else {
      parent = d->m_register;
    }

    editor = item->createEditor(parent, list, KGlobalLedgerViewPrivate::m_lastPostDate);

    // check that we use the same transaction commodity in all selected transactions
    // if not, we need to update this in the editor's list. The user can also bail out
    // of this operation which means that we have to stop editing here.
    if (editor) {
      if (!editor->fixTransactionCommodity(d->m_currentAccount)) {
        // if the user wants to quit, we need to destroy the editor
        // and bail out
        delete editor;
        editor = 0;
      }
    }

    if (editor) {
      if (parent == d->m_register) {
        // make sure, the height of the table is correct
        d->m_register->updateRegister(KMyMoneySettings::ledgerLens() | !KMyMoneySettings::transactionForm());
      }

      d->m_inEditMode = true;
      connect(editor, &TransactionEditor::transactionDataSufficient, pActions[Action::EnterTransaction], &QAction::setEnabled);
      connect(editor, &TransactionEditor::returnPressed, pActions[Action::EnterTransaction], &QAction::trigger);
      connect(editor, &TransactionEditor::escapePressed, pActions[Action::CancelTransaction], &QAction::trigger);

      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, editor, &TransactionEditor::slotReloadEditWidgets);
      connect(editor, &TransactionEditor::finishEdit, this, &KGlobalLedgerView::slotLeaveEditMode);
      connect(editor, &TransactionEditor::objectCreation, d->m_mousePressFilter, &MousePressFilter::setFilterDeactive);
      connect(editor, &TransactionEditor::lastPostDateUsed, this, &KGlobalLedgerView::slotKeepPostDate);

      // create the widgets, place them in the parent and load them with data
      // setup tab order
      d->m_tabOrderWidgets.clear();
      editor->setup(d->m_tabOrderWidgets, d->m_currentAccount, d->m_action);

      Q_ASSERT(!d->m_tabOrderWidgets.isEmpty());

      // install event filter in all taborder widgets
      QWidgetList::const_iterator it_w = d->m_tabOrderWidgets.constBegin();
      for (; it_w != d->m_tabOrderWidgets.constEnd(); ++it_w) {
        (*it_w)->installEventFilter(this);
      }
      // Install a filter that checks if a mouse press happened outside
      // of one of our own widgets.
      qApp->installEventFilter(d->m_mousePressFilter);

      // Check if the editor has some preference on where to set the focus
      // If not, set the focus to the first widget in the tab order
      QWidget* focusWidget = editor->firstWidget();
      if (!focusWidget)
        focusWidget = d->m_tabOrderWidgets.first();

      // for some reason, this only works reliably if delayed a bit
      QTimer::singleShot(10, focusWidget, SLOT(setFocus()));

      // preset to 'I have no idea which type to create' for the next round.
      d->m_action = eWidgets::eRegister::Action::None;
    }
  }
  return editor;
}

void KGlobalLedgerView::slotTransactionsContextMenuRequested()
{
  Q_D(KGlobalLedgerView);
  auto transactions = d->m_selectedTransactions;
  updateLedgerActionsInternal();
//  emit transactionsSelected(d->m_selectedTransactions); // that should select MyMoneySchedule in KScheduledView
  if (!transactions.isEmpty() && transactions.first().isScheduled())
    emit selectByObject(MyMoneyFile::instance()->schedule(transactions.first().scheduleId()), eView::Intent::OpenContextMenu);
  else
    slotShowTransactionMenu(MyMoneySplit());
}

void KGlobalLedgerView::slotLeaveEditMode(const KMyMoneyRegister::SelectedTransactions& list)
{
  Q_D(KGlobalLedgerView);
  d->m_inEditMode = false;
  qApp->removeEventFilter(d->m_mousePressFilter);

  // a possible focusOut event may have removed the focus, so we
  // install it back again.
  d->m_register->focusItem()->setFocus(true);

  // if we come back from editing a new item, we make sure that
  // we always select the very last known transaction entry no
  // matter if the transaction has been created or not.

  if (list.count() && list[0].transaction().id().isEmpty()) {
    // block signals to prevent some infinite loops that might occur here.
    d->m_register->blockSignals(true);
    d->m_register->clearSelection();
    KMyMoneyRegister::RegisterItem* p = d->m_register->lastItem();
    if (p && p->prevItem())
      p = p->prevItem();
    d->m_register->selectItem(p);
    d->m_register->updateRegister(true);
    d->m_register->blockSignals(false);
    // we need to update the form manually as sending signals was blocked
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
    if (t)
      d->m_form->slotSetTransaction(t);
  } else {
    if (!KMyMoneySettings::transactionForm()) {
      // update the row height of the transactions because it might differ between viewing/editing mode when not using the transaction form
      d->m_register->blockSignals(true);
      d->m_register->updateRegister(true);
      d->m_register->blockSignals(false);
    }
  }
  d->m_needsRefresh = true; // TODO: Why transaction in view doesn't update without this?
  if (d->m_needsRefresh)
    refresh();

  d->m_register->endEdit();
  d->m_register->setFocus();
}

bool KGlobalLedgerView::focusNextPrevChild(bool next)
{
  Q_D(KGlobalLedgerView);
  bool  rc = false;
  // qDebug() << "----------------------------------------------------------";
  // qDebug() << "KGlobalLedgerView::focusNextPrevChild, editmode=" << d->m_inEditMode;
  if (d->m_inEditMode) {
    QWidget *w = 0;

    w = qApp->focusWidget();
    int currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
    const auto startIndex = currentWidgetIndex;
    // qDebug() << "Focus is at currentWidgetIndex" <<  currentWidgetIndex << w->objectName();
    do {
      while (w && currentWidgetIndex == -1) {
        // qDebug() << w->objectName() << "not in list, use parent";
        w = w->parentWidget();
        currentWidgetIndex = d->m_tabOrderWidgets.indexOf(w);
      }
      // qDebug() << "Focus is at currentWidgetIndex" <<  currentWidgetIndex << w->objectName();

      if (currentWidgetIndex != -1) {
        // if(w) qDebug() << "tab order is at" << w->objectName();
        currentWidgetIndex += next ? 1 : -1;
        if (currentWidgetIndex < 0)
          currentWidgetIndex = d->m_tabOrderWidgets.size() - 1;
        else if (currentWidgetIndex >= d->m_tabOrderWidgets.size())
          currentWidgetIndex = 0;

        w = d->m_tabOrderWidgets[currentWidgetIndex];
        // qDebug() << "currentWidgetIndex" <<  currentWidgetIndex << w->objectName() << w->isVisible();

        if (((w->focusPolicy() & Qt::TabFocus) == Qt::TabFocus) && w->isVisible() && w->isEnabled()) {
          // qDebug() << "Set focus to" << w->objectName();
          w->setFocus(next ? Qt::TabFocusReason: Qt::BacktabFocusReason);
          rc = true;
          break;
        }
      } else {
        break;
      }
    } while(currentWidgetIndex != startIndex);
  } else
    rc = KMyMoneyViewBase::focusNextPrevChild(next);
  return rc;
}

bool KGlobalLedgerView::eventFilter(QObject* o, QEvent* e)
{
  Q_D(KGlobalLedgerView);
  bool rc = false;
  //  Need to capture mouse position here as QEvent::ToolTip is too slow
  d->m_tooltipPosn = QCursor::pos();

  if (e->type() == QEvent::KeyPress) {
    if (d->m_inEditMode) {
      // qDebug("object = %s, key = %d", o->className(), k->key());
      if (o == d->m_register) {
        // we hide all key press events from the register
        // while editing a transaction
        rc = true;
      }
    }
  }

  if (!rc)
    rc = KMyMoneyViewBase::eventFilter(o, e);

  return rc;
}

void KGlobalLedgerView::slotSortOptions()
{
  Q_D(KGlobalLedgerView);
  QPointer<KSortOptionDlg> dlg = new KSortOptionDlg(this);

  QString key;
  QString sortOrder, def;
  if (d->isReconciliationAccount()) {
    key = "kmm-sort-reconcile";
    def = KMyMoneySettings::sortReconcileView();
  } else {
    key = "kmm-sort-std";
    def = KMyMoneySettings::sortNormalView();
  }

  // check if we have an account override of the sort order
  if (!d->m_currentAccount.value(key).isEmpty())
    sortOrder = d->m_currentAccount.value(key);

  QString oldOrder = sortOrder;

  dlg->setSortOption(sortOrder, def);

  if (dlg->exec() == QDialog::Accepted) {
    if (dlg != 0) {
      sortOrder = dlg->sortOption();
      if (sortOrder != oldOrder) {
        if (sortOrder.isEmpty()) {
          d->m_currentAccount.deletePair(key);
        } else {
          d->m_currentAccount.setValue(key, sortOrder);
        }
        MyMoneyFileTransaction ft;
        try {
          MyMoneyFile::instance()->modifyAccount(d->m_currentAccount);
          ft.commit();
        } catch (const MyMoneyException &e) {
          qDebug("Unable to update sort order for account '%s': %s", qPrintable(d->m_currentAccount.name()), e.what());
        }
      }
    }
  }
  delete dlg;
}

void KGlobalLedgerView::slotToggleTransactionMark(KMyMoneyRegister::Transaction* /* t */)
{
  Q_D(KGlobalLedgerView);
  if (!d->m_inEditMode) {
    slotToggleReconciliationFlag();
  }
}

void KGlobalLedgerView::slotKeepPostDate(const QDate& date)
{
  KGlobalLedgerViewPrivate::m_lastPostDate = date;
}

QString KGlobalLedgerView::accountId() const
{
  Q_D(const KGlobalLedgerView);
  return d->m_currentAccount.id();
}

bool KGlobalLedgerView::canCreateTransactions(QString& tooltip) const
{
  Q_D(const KGlobalLedgerView);
  bool rc = true;

  if (d->m_currentAccount.id().isEmpty()) {
    tooltip = i18n("Cannot create transactions when no account is selected.");
    rc = false;
  }
  if (d->m_currentAccount.accountGroup() == eMyMoney::Account::Type::Income
      || d->m_currentAccount.accountGroup() == eMyMoney::Account::Type::Expense) {
    tooltip = i18n("Cannot create transactions in the context of a category.");
    d->showTooltip(tooltip);
    rc = false;
  }
  if (d->m_currentAccount.isClosed()) {
    tooltip = i18n("Cannot create transactions in a closed account.");
    d->showTooltip(tooltip);
    rc = false;
  }
  return rc;
}

bool KGlobalLedgerView::canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  Q_D(const KGlobalLedgerView);
  return d->canProcessTransactions(list, tooltip) && list.canModify();
}

bool KGlobalLedgerView::canDuplicateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  Q_D(const KGlobalLedgerView);
  return d->canProcessTransactions(list, tooltip) && list.canDuplicate();
}

bool KGlobalLedgerView::canEditTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  Q_D(const KGlobalLedgerView);
  // check if we can edit the list of transactions. We can edit, if
  //
  //   a) no mix of standard and investment transactions exist
  //   b) if a split transaction is selected, this is the only selection
  //   c) none of the splits is frozen
  //   d) the transaction having the current focus is selected

  // check for d)
  if (!d->canProcessTransactions(list, tooltip))
    return false;
  // check for c)
  if (list.warnLevel() == 2) {
    tooltip = i18n("Cannot edit transactions with frozen splits.");
    d->showTooltip(tooltip);
    return false;
  }

  bool rc = true;
  int investmentTransactions = 0;
  int normalTransactions = 0;

  if (d->m_currentAccount.accountGroup() == eMyMoney::Account::Type::Income
      || d->m_currentAccount.accountGroup() == eMyMoney::Account::Type::Expense) {
    tooltip = i18n("Cannot edit transactions in the context of a category.");
    d->showTooltip(tooltip);
    rc = false;
  }

  if (d->m_currentAccount.isClosed()) {
    tooltip = i18n("Cannot create or edit any transactions in Account %1 as it is closed", d->m_currentAccount.name());
    d->showTooltip(tooltip);
    rc = false;
  }

  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  QString action;
  for (it_t = list.begin(); rc && it_t != list.end(); ++it_t) {
    if ((*it_t).transaction().id().isEmpty()) {
      tooltip.clear();
      rc = false;
      continue;
    }

    if (KMyMoneyUtils::transactionType((*it_t).transaction()) == KMyMoneyUtils::InvestmentTransaction) {
      if (action.isEmpty()) {
        action = (*it_t).split().action();
        continue;
      }
      if (action == (*it_t).split().action()) {
        continue;
      } else {
        tooltip = (i18n("Cannot edit mixed investment action/type transactions together."));
        d->showTooltip(tooltip);
        rc = false;
        break;
      }
    }

    if (KMyMoneyUtils::transactionType((*it_t).transaction()) == KMyMoneyUtils::InvestmentTransaction)
      ++investmentTransactions;
    else
      ++normalTransactions;

    // check for a)
    if (investmentTransactions != 0 && normalTransactions != 0) {
      tooltip = i18n("Cannot edit investment transactions and non-investment transactions together.");
      d->showTooltip(tooltip);
      rc = false;
      break;
    }

    // check for b) but only for normalTransactions
    if ((*it_t).transaction().splitCount() > 2 && normalTransactions != 0) {
      if (list.count() > 1) {
        tooltip = i18n("Cannot edit multiple split transactions at once.");
        d->showTooltip(tooltip);
        rc = false;
        break;
      }
    }
  }

  // check for multiple transactions being selected in an investment account
  // we do not allow editing in this case: https://bugs.kde.org/show_bug.cgi?id=240816
  // later on, we might allow to edit investment transactions of the same type
  ///  Can now disable the following check.

  /*  if (rc == true && investmentTransactions > 1) {
      tooltip = i18n("Cannot edit multiple investment transactions at once");
      rc = false;
    }*/

  // now check that we have the correct account type for investment transactions
  if (rc == true && investmentTransactions != 0) {
    if (d->m_currentAccount.accountType() != eMyMoney::Account::Type::Investment) {
      tooltip = i18n("Cannot edit investment transactions in the context of this account.");
      rc = false;
    }
  }
  return rc;
}

void KGlobalLedgerView::slotMoveToAccount(const QString& id)
{
  Q_D(KGlobalLedgerView);
  // close the menu, if it is still open
  if (pMenus[Menu::Transaction]->isVisible())
    pMenus[Menu::Transaction]->close();

  if (!d->m_selectedTransactions.isEmpty()) {
    const auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
      foreach (const auto selection, d->m_selectedTransactions) {
        if (d->m_currentAccount.accountType() == eMyMoney::Account::Type::Investment) {
          d->moveInvestmentTransaction(d->m_currentAccount.id(), id, selection.transaction());
        } else {
          // we get the data afresh from the engine as
          // it might have changed by a previous iteration
          // in this loop. Use case: two splits point to
          // the same account and both are selected.
          auto tid = selection.transaction().id();
          auto sid = selection.split().id();
          auto t = file->transaction(tid);
          auto s = t.splitById(sid);
          s.setAccountId(id);
          t.modifySplit(s);
          file->modifyTransaction(t);
        }
      }
      ft.commit();
    } catch (const MyMoneyException &) {
    }
  }
}

void KGlobalLedgerView::slotUpdateMoveToAccountMenu()
{
  Q_D(KGlobalLedgerView);
  d->createTransactionMoveMenu();

  // in case we were not able to create the selector, we
  // better get out of here. Anything else would cause
  // a crash later on (accountSet.load)
  if (!d->m_moveToAccountSelector)
    return;

  if (!d->m_currentAccount.id().isEmpty()) {
    AccountSet accountSet;
    if (d->m_currentAccount.accountType() == eMyMoney::Account::Type::Investment) {
      accountSet.addAccountType(eMyMoney::Account::Type::Investment);
    } else if (d->m_currentAccount.isAssetLiability()) {

      accountSet.addAccountType(eMyMoney::Account::Type::Checkings);
      accountSet.addAccountType(eMyMoney::Account::Type::Savings);
      accountSet.addAccountType(eMyMoney::Account::Type::Cash);
      accountSet.addAccountType(eMyMoney::Account::Type::AssetLoan);
      accountSet.addAccountType(eMyMoney::Account::Type::CertificateDep);
      accountSet.addAccountType(eMyMoney::Account::Type::MoneyMarket);
      accountSet.addAccountType(eMyMoney::Account::Type::Asset);
      accountSet.addAccountType(eMyMoney::Account::Type::Currency);
      accountSet.addAccountType(eMyMoney::Account::Type::CreditCard);
      accountSet.addAccountType(eMyMoney::Account::Type::Loan);
      accountSet.addAccountType(eMyMoney::Account::Type::Liability);
    } else if (d->m_currentAccount.isIncomeExpense()) {
      accountSet.addAccountType(eMyMoney::Account::Type::Income);
      accountSet.addAccountType(eMyMoney::Account::Type::Expense);
    }

    accountSet.load(d->m_moveToAccountSelector);
    // remove those accounts that we currently reference
    // with the selected items
    foreach (const auto selection, d->m_selectedTransactions) {
      d->m_moveToAccountSelector->removeItem(selection.split().accountId());
    }
    // remove those accounts from the list that are denominated
    // in a different currency
    auto list = d->m_moveToAccountSelector->accountList();
    QList<QString>::const_iterator it_a;
    for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
      auto acc = MyMoneyFile::instance()->account(*it_a);
      if (acc.currencyId() != d->m_currentAccount.currencyId())
        d->m_moveToAccountSelector->removeItem((*it_a));
    }
  }
}

void KGlobalLedgerView::slotObjectDestroyed(QObject* o)
{
  Q_D(KGlobalLedgerView);
  if (o == d->m_moveToAccountSelector) {
    d->m_moveToAccountSelector = nullptr;
  }
}

void KGlobalLedgerView::slotCancelOrEnterTransactions(bool& okToSelect)
{
  Q_D(KGlobalLedgerView);
  static bool oneTime = false;
  if (!oneTime) {
    oneTime = true;
    auto dontShowAgain = "CancelOrEditTransaction";
    // qDebug("KMyMoneyApp::slotCancelOrEndEdit");
    if (d->m_transactionEditor) {
      if (KMyMoneySettings::focusChangeIsEnter() && pActions[Action::EnterTransaction]->isEnabled()) {
        slotEnterTransaction();
        if (d->m_transactionEditor) {
          // if at this stage the editor is still there that means that entering the transaction was cancelled
          // for example by pressing cancel on the exchange rate editor so we must stay in edit mode
          okToSelect = false;
        }
      } else {
        // okToSelect is preset to true if a cancel of the dialog is useful and false if it is not
        int rc;
        KGuiItem noGuiItem = KStandardGuiItem::save();
        KGuiItem yesGuiItem = KStandardGuiItem::discard();
        KGuiItem cancelGuiItem = KStandardGuiItem::cont();

        // if the transaction can't be entered make sure that it can't be entered by pressing no either
        if (!pActions[Action::EnterTransaction]->isEnabled()) {
          noGuiItem.setEnabled(false);
          noGuiItem.setToolTip(pActions[Action::EnterTransaction]->toolTip());
        }

        // in case we have a new transaction and cannot save it we simply cancel
        if (!pActions[Action::EnterTransaction]->isEnabled() && d->m_transactionEditor && d->m_transactionEditor->createNewTransaction()) {
          rc = KMessageBox::Yes;

        } else if (okToSelect == true) {
          rc = KMessageBox::warningYesNoCancel(this, i18n("<p>Please select what you want to do: discard the changes, save the changes or continue to edit the transaction.</p><p>You can also set an option to save the transaction automatically when e.g. selecting another transaction.</p>"), i18n("End transaction edit"), yesGuiItem, noGuiItem, cancelGuiItem, dontShowAgain);

        } else {
          rc = KMessageBox::warningYesNo(this, i18n("<p>Please select what you want to do: discard or save the changes.</p><p>You can also set an option to save the transaction automatically when e.g. selecting another transaction.</p>"), i18n("End transaction edit"), yesGuiItem, noGuiItem, dontShowAgain);
        }

        switch (rc) {
          case KMessageBox::Yes:
            slotCancelTransaction();
            break;
          case KMessageBox::No:
            slotEnterTransaction();
            // make sure that we'll see this message the next time no matter
            // if the user has chosen the 'Don't show again' checkbox
            KMessageBox::enableMessage(dontShowAgain);
            if (d->m_transactionEditor) {
              // if at this stage the editor is still there that means that entering the transaction was cancelled
              // for example by pressing cancel on the exchange rate editor so we must stay in edit mode
              okToSelect = false;
            }
            break;
          case KMessageBox::Cancel:
            // make sure that we'll see this message the next time no matter
            // if the user has chosen the 'Don't show again' checkbox
            KMessageBox::enableMessage(dontShowAgain);
            okToSelect = false;
            break;
        }
      }
    }
    oneTime = false;
  }
}

void KGlobalLedgerView::slotNewSchedule(const MyMoneyTransaction& _t, eMyMoney::Schedule::Occurrence occurrence)
{
  KEditScheduleDlg::newSchedule(_t, occurrence);
}

void KGlobalLedgerView::slotNewTransactionForm(eWidgets::eRegister::Action id)
{
  Q_D(KGlobalLedgerView);
  if (!d->m_inEditMode) {
    d->m_action = id;
    // since we jump here via code, we have to make sure to react only
    // if the action is enabled
    if (pActions[Action::NewTransaction]->isEnabled()) {
      if (d->createNewTransaction()) {
        d->m_transactionEditor = d->startEdit(d->m_selectedTransactions);
        if (d->m_transactionEditor) {
          KMyMoneyMVCCombo::setSubstringSearchForChildren(this/*d->m_myMoneyView*/, !KMyMoneySettings::stringMatchFromStart());
          KMyMoneyPayeeCombo* payeeEdit = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_transactionEditor->haveWidget("payee"));
          if (payeeEdit && !d->m_lastPayeeEnteredId.isEmpty()) {
            // in case we entered a new transaction before and used a payee,
            // we reuse it here. Save the text to the edit widget, select it
            // so that hitting any character will start entering another payee.
            payeeEdit->setSelectedItem(d->m_lastPayeeEnteredId);
            payeeEdit->lineEdit()->selectAll();
          }
          if (d->m_transactionEditor) {
            connect(d->m_transactionEditor.data(), &TransactionEditor::statusProgress, this, &KGlobalLedgerView::slotStatusProgress);
            connect(d->m_transactionEditor.data(), &TransactionEditor::statusMsg, this, &KGlobalLedgerView::slotStatusMsg);
            connect(d->m_transactionEditor.data(), &TransactionEditor::scheduleTransaction, this, &KGlobalLedgerView::slotNewSchedule);
          }
          updateLedgerActionsInternal();
//          emit transactionsSelected(d->m_selectedTransactions);
        }
      }
    }
  }
}

void KGlobalLedgerView::slotNewTransaction()
{
  // in case the view is not visible ...
  if (!isVisible()) {
    // we switch to it
    pActions[Action::ShowLedgersView]->activate(QAction::ActionEvent::Trigger);
    QString tooltip;
    if (!canCreateTransactions(tooltip)) {
      // and inform the user via a dialog about the reason
      // why a transaction cannot be created
      KMessageBox::sorry(this, tooltip);
      return;
    }
  }
  slotNewTransactionForm(eWidgets::eRegister::Action::None);
}

void KGlobalLedgerView::slotEditTransaction()
{
  Q_D(KGlobalLedgerView);
  // qDebug("KMyMoneyApp::slotTransactionsEdit()");
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (pActions[Action::EditTransaction]->isEnabled()) {
    // as soon as we edit a transaction, we don't remember the last payee entered
    d->m_lastPayeeEnteredId.clear();
    d->m_transactionEditor = d->startEdit(d->m_selectedTransactions);
    KMyMoneyMVCCombo::setSubstringSearchForChildren(this/*d->m_myMoneyView*/, !KMyMoneySettings::stringMatchFromStart());
    updateLedgerActionsInternal();
  }
}

void KGlobalLedgerView::slotDeleteTransaction()
{
  Q_D(KGlobalLedgerView);
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if (!pActions[Action::DeleteTransaction]->isEnabled())
    return;
  if (d->m_selectedTransactions.isEmpty())
    return;
  if (d->m_selectedTransactions.warnLevel() == 1) {
    if (KMessageBox::warningContinueCancel(this,
                                           i18n("At least one split of the selected transactions has been reconciled. "
                                                "Do you wish to delete the transactions anyway?"),
                                           i18n("Transaction already reconciled")) == KMessageBox::Cancel)
      return;
  }
  auto msg =
      i18np("Do you really want to delete the selected transaction?",
            "Do you really want to delete all %1 selected transactions?",
            d->m_selectedTransactions.count());

  if (KMessageBox::questionYesNo(this, msg, i18n("Delete transaction")) == KMessageBox::Yes) {
    //KMSTATUS(i18n("Deleting transactions"));
    d->doDeleteTransactions();
  }
}

void KGlobalLedgerView::slotDuplicateTransaction(bool reverse)
{
  Q_D(KGlobalLedgerView);
  // since we may jump here via code, we have to make sure to react only
  // if the action is enabled
  if (pActions[Action::DuplicateTransaction]->isEnabled()) {
    KMyMoneyRegister::SelectedTransactions selectionList = d->m_selectedTransactions;
    KMyMoneyRegister::SelectedTransactions::iterator it_t;

    int i = 0;
    int cnt = d->m_selectedTransactions.count();
    //    KMSTATUS(i18n("Duplicating transactions"));
    emit selectByVariant(QVariantList {QVariant(0), QVariant(cnt)}, eView::Intent::ReportProgress);
    MyMoneyFileTransaction ft;
    MyMoneyTransaction lt;
    try {
      foreach (const auto selection, selectionList) {
        auto t = selection.transaction();
        // wipe out any reconciliation information
        for (auto& split : t.splits()) {
          split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
          split.setReconcileDate(QDate());
          split.setBankID(QString());
        }
        // clear invalid data
        t.setEntryDate(QDate());
        t.clearId();

        if (reverse)
            // reverse transaction
            t.reverse();
        else
            // set the post date to today
            t.setPostDate(QDate::currentDate());

        MyMoneyFile::instance()->addTransaction(t);
        lt = t;
        emit selectByVariant(QVariantList {QVariant(i++), QVariant(0)}, eView::Intent::ReportProgress);
      }
      ft.commit();

      // select the new transaction in the ledger
      if (!d->m_currentAccount.id().isEmpty())
        slotLedgerSelected(d->m_currentAccount.id(), lt.id());
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to duplicate transaction(s)"), QString::fromLatin1(e.what()));
    }
    // switch off the progress bar
    emit selectByVariant(QVariantList {QVariant(-1), QVariant(-1)}, eView::Intent::ReportProgress);
  }
}

void KGlobalLedgerView::slotEnterTransaction()
{
  Q_D(KGlobalLedgerView);
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (pActions[Action::EnterTransaction]->isEnabled()) {
    // disable the action while we process it to make sure it's processed only once since
    // d->m_transactionEditor->enterTransactions(newId) will run QCoreApplication::processEvents
    // we could end up here twice which will cause a crash slotUpdateActions() will enable the action again
    pActions[Action::EnterTransaction]->setEnabled(false);
    if (d->m_transactionEditor) {
      QString accountId = d->m_currentAccount.id();
      QString newId;
      connect(d->m_transactionEditor.data(), &TransactionEditor::balanceWarning, d->m_balanceWarning.data(), &KBalanceWarning::slotShowMessage);
      if (d->m_transactionEditor->enterTransactions(newId)) {
        KMyMoneyPayeeCombo* payeeEdit = dynamic_cast<KMyMoneyPayeeCombo*>(d->m_transactionEditor->haveWidget("payee"));
        if (payeeEdit && !newId.isEmpty()) {
          d->m_lastPayeeEnteredId = payeeEdit->selectedItem();
        }
        d->deleteTransactionEditor();
      }
      if (!newId.isEmpty()) {
        slotLedgerSelected(accountId, newId);
      }
    }
    updateLedgerActionsInternal();
  }
}

void KGlobalLedgerView::slotAcceptTransaction()
{
  Q_D(KGlobalLedgerView);
  KMyMoneyRegister::SelectedTransactions list = d->m_selectedTransactions;
  KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
  int cnt = list.count();
  int i = 0;
  emit selectByVariant(QVariantList {QVariant(0), QVariant(cnt)}, eView::Intent::ReportProgress);
  MyMoneyFileTransaction ft;
  try {
    for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
      // reload transaction in case it got changed during the course of this loop
      MyMoneyTransaction t = MyMoneyFile::instance()->transaction((*it_t).transaction().id());
      if (t.isImported()) {
        t.setImported(false);
        if (!d->m_currentAccount.id().isEmpty()) {
          foreach (const auto split, t.splits()) {
            if (split.accountId() == d->m_currentAccount.id()) {
              if (split.reconcileFlag() == eMyMoney::Split::State::NotReconciled) {
                MyMoneySplit s = split;
                s.setReconcileFlag(eMyMoney::Split::State::Cleared);
                t.modifySplit(s);
              }
            }
          }
        }
        MyMoneyFile::instance()->modifyTransaction(t);
      }
      if ((*it_t).split().isMatched()) {
        // reload split in case it got changed during the course of this loop
        MyMoneySplit s = t.splitById((*it_t).split().id());
        TransactionMatcher matcher(d->m_currentAccount);
        matcher.accept(t, s);
      }
      emit selectByVariant(QVariantList {QVariant(i++), QVariant(0)}, eView::Intent::ReportProgress);
    }
    emit selectByVariant(QVariantList {QVariant(-1), QVariant(-1)}, eView::Intent::ReportProgress);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to accept transaction"), QString::fromLatin1(e.what()));
  }
}

void KGlobalLedgerView::slotCancelTransaction()
{
  Q_D(KGlobalLedgerView);
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (pActions[Action::CancelTransaction]->isEnabled()) {
    // make sure, we block the enter function
    pActions[Action::EnterTransaction]->setEnabled(false);
    // qDebug("KMyMoneyApp::slotTransactionsCancel");
    d->deleteTransactionEditor();
    updateLedgerActions(d->m_selectedTransactions);
    emit selectByVariant(QVariantList {QVariant::fromValue(d->m_selectedTransactions)}, eView::Intent::SelectRegisterTransactions);
  }
}

void KGlobalLedgerView::slotEditSplits()
{
  Q_D(KGlobalLedgerView);
  // since we jump here via code, we have to make sure to react only
  // if the action is enabled
  if (pActions[Action::EditSplits]->isEnabled()) {
    // as soon as we edit a transaction, we don't remember the last payee entered
    d->m_lastPayeeEnteredId.clear();
    d->m_transactionEditor = d->startEdit(d->m_selectedTransactions);
    updateLedgerActions(d->m_selectedTransactions);
    emit selectByVariant(QVariantList {QVariant::fromValue(d->m_selectedTransactions)}, eView::Intent::SelectRegisterTransactions);

    if (d->m_transactionEditor) {
      KMyMoneyMVCCombo::setSubstringSearchForChildren(this/*d->m_myMoneyView*/, !KMyMoneySettings::stringMatchFromStart());
      if (d->m_transactionEditor->slotEditSplits() == QDialog::Accepted) {
        MyMoneyFileTransaction ft;
        try {
          QString id;
          connect(d->m_transactionEditor.data(), &TransactionEditor::balanceWarning, d->m_balanceWarning.data(), &KBalanceWarning::slotShowMessage);
          d->m_transactionEditor->enterTransactions(id);
          ft.commit();
        } catch (const MyMoneyException &e) {
          KMessageBox::detailedSorry(this, i18n("Unable to modify transaction"), QString::fromLatin1(e.what()));
        }
      }
    }
    d->deleteTransactionEditor();
    updateLedgerActions(d->m_selectedTransactions);
    emit selectByVariant(QVariantList {QVariant::fromValue(d->m_selectedTransactions)}, eView::Intent::SelectRegisterTransactions);
  }
}

void KGlobalLedgerView::slotCopyTransactionToClipboard()
{
  Q_D(KGlobalLedgerView);

  // suppress copy transactions if view not visible
  // or in edit mode
  if (!isVisible() || d->m_inEditMode)
    return;

  // format transactions into text
  QString txt;
  const auto file = MyMoneyFile::instance();
  const auto acc = file->account(d->m_lastSelectedAccountID);
  const auto currency = file->currency(acc.currencyId());

  foreach (const auto& st, d->m_selectedTransactions) {
    if (!txt.isEmpty() || (d->m_selectedTransactions.count() > 1)) {
      txt += QStringLiteral("----------------------------\n");
    }
    try {
      const auto& s = st.split();
      // Date
      txt += i18n("Date: %1", st.transaction().postDate().toString(Qt::DefaultLocaleShortDate));
      txt += QStringLiteral("\n");
      // Payee
      QString payee = i18nc("Name for unknown payee", "Unknown");
      if (!s.payeeId().isEmpty()) {
        payee = file->payee(s.payeeId()).name();
      }
      txt += i18n("Payee: %1", payee);
      txt += QStringLiteral("\n");
      // Amount
      txt += i18n("Amount: %1", s.value().formatMoney(currency.tradingSymbol(),  MyMoneyMoney::denomToPrec(acc.fraction(currency))));
      txt += QStringLiteral("\n");
      // Memo
      txt += i18n("Memo: %1", s.memo());
      txt += QStringLiteral("\n");

    } catch (MyMoneyException &) {
      qDebug() << "Cannot copy transaction" << st.transaction().id() << "to clipboard";
    }
  }
  if (d->m_selectedTransactions.count() > 1) {
    txt += QStringLiteral("----------------------------\n");
  }

  if (!txt.isEmpty()) {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(txt);
  }
}

void KGlobalLedgerView::slotCopySplits()
{
  Q_D(KGlobalLedgerView);
  const auto file = MyMoneyFile::instance();

  if (d->m_selectedTransactions.count() >= 2) {
    int singleSplitTransactions = 0;
    int multipleSplitTransactions = 0;
    KMyMoneyRegister::SelectedTransaction selectedSourceTransaction;
    foreach (const auto& st, d->m_selectedTransactions) {
      switch (st.transaction().splitCount()) {
        case 0:
          break;
        case 1:
          singleSplitTransactions++;
          break;
        default:
          selectedSourceTransaction = st;
          multipleSplitTransactions++;
          break;
      }
    }
    if (singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
      MyMoneyFileTransaction ft;
      try {
        const auto& sourceTransaction = selectedSourceTransaction.transaction();
        const auto& sourceSplit = selectedSourceTransaction.split();
        foreach (const KMyMoneyRegister::SelectedTransaction& st, d->m_selectedTransactions) {
          auto t = st.transaction();

          // don't process the source transaction
          if (sourceTransaction.id() == t.id()) {
            continue;
          }

          const auto& baseSplit = st.split();

          if (t.splitCount() == 1) {
            foreach (const auto& split, sourceTransaction.splits()) {
              // Don't copy the source split, as we already have that
              // as part of the destination transaction
              if (split.id() == sourceSplit.id()) {
                continue;
              }

              MyMoneySplit sp(split);
              // clear the ID and reconciliation state
              sp.clearId();
              sp.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
              sp.setReconcileDate(QDate());

              // in case it is a simple transaction consisting of two splits,
              // we can adjust the share and value part of the second split we
              // just created. We need to keep a possible price in mind in case
              // of different currencies
              if (sourceTransaction.splitCount() == 2) {
                sp.setValue(-baseSplit.value());
                sp.setShares(-(baseSplit.shares() * baseSplit.price()));
              }
              t.addSplit(sp);
            }
            // check if we need to add/update a VAT assignment
            file->updateVAT(t);

            // and store the modified transaction
            file->modifyTransaction(t);
          }
        }
        ft.commit();
      } catch (const MyMoneyException &) {
        qDebug() << "transactionCopySplits() failed";
      }
    }
  }
}

void KGlobalLedgerView::slotGoToPayee()
{
  Q_D(KGlobalLedgerView);
  if (!d->m_payeeGoto.isEmpty()) {
    try {
      QString transactionId;
      if (d->m_selectedTransactions.count() == 1) {
        transactionId = d->m_selectedTransactions[0].transaction().id();
      }
      // make sure to pass copies, as d->myMoneyView->slotPayeeSelected() overrides
      // d->m_payeeGoto and d->m_currentAccount while calling slotUpdateActions()
      QString payeeId = d->m_payeeGoto;
      QString accountId = d->m_currentAccount.id();
      emit selectByVariant(QVariantList {QVariant(payeeId), QVariant(accountId), QVariant(transactionId)}, eView::Intent::ShowPayee);
//      emit openPayeeRequested(payeeId, accountId, transactionId);
    } catch (const MyMoneyException &) {
    }
  }
}

void KGlobalLedgerView::slotGoToAccount()
{
  Q_D(KGlobalLedgerView);
  if (!d->m_accountGoto.isEmpty()) {
    try {
      QString transactionId;
      if (d->m_selectedTransactions.count() == 1) {
        transactionId = d->m_selectedTransactions[0].transaction().id();
      }
      // make sure to pass a copy, as d->myMoneyView->slotLedgerSelected() overrides
      // d->m_accountGoto while calling slotUpdateActions()
      slotLedgerSelected(d->m_accountGoto, transactionId);
    } catch (const MyMoneyException &) {
    }
  }
}

void KGlobalLedgerView::slotMatchTransactions()
{
  Q_D(KGlobalLedgerView);
  // if the menu action is retrieved it can contain an '&' character for the accelerator causing the comparison to fail if not removed
  QString transactionActionText = pActions[Action::MatchTransaction]->text();
  transactionActionText.remove('&');
  if (transactionActionText == i18nc("Button text for match transaction", "Match"))
    d->transactionMatch();
  else
    d->transactionUnmatch();
}

void KGlobalLedgerView::slotCombineTransactions()
{
  qDebug("slotTransactionCombine() not implemented yet");
}

void KGlobalLedgerView::slotToggleReconciliationFlag()
{
  Q_D(KGlobalLedgerView);
  d->markTransaction(eMyMoney::Split::State::Unknown);
}

void KGlobalLedgerView::slotMarkCleared()
{
  Q_D(KGlobalLedgerView);
  d->markTransaction(eMyMoney::Split::State::Cleared);
}

void KGlobalLedgerView::slotMarkReconciled()
{
  Q_D(KGlobalLedgerView);
  d->markTransaction(eMyMoney::Split::State::Reconciled);
}

void KGlobalLedgerView::slotMarkNotReconciled()
{
  Q_D(KGlobalLedgerView);
  d->markTransaction(eMyMoney::Split::State::NotReconciled);
}

void KGlobalLedgerView::slotSelectAllTransactions()
{
  Q_D(KGlobalLedgerView);
  if(d->m_needLoad)
    d->init();

  d->m_register->clearSelection();
  KMyMoneyRegister::RegisterItem* p = d->m_register->firstItem();
  while (p) {
    KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
    if (t) {
      if (t->isVisible() && t->isSelectable() && !t->isScheduled() && !t->id().isEmpty()) {
        t->setSelected(true);
      }
    }
    p = p->nextItem();
  }
  // this is here only to re-paint the items without selecting anything because the data (including the selection) is not really held in the model right now
  d->m_register->selectAll();

  // inform everyone else about the selected items
  KMyMoneyRegister::SelectedTransactions list(d->m_register);
  updateLedgerActions(list);
  emit selectByVariant(QVariantList {QVariant::fromValue(list)}, eView::Intent::SelectRegisterTransactions);
}

void KGlobalLedgerView::slotCreateScheduledTransaction()
{
   Q_D(KGlobalLedgerView);
  if (d->m_selectedTransactions.count() == 1) {
    // make sure to have the current selected split as first split in the schedule
    MyMoneyTransaction t = d->m_selectedTransactions[0].transaction();
    MyMoneySplit s = d->m_selectedTransactions[0].split();
    QString splitId = s.id();
    s.clearId();
    s.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
    s.setReconcileDate(QDate());
    t.removeSplits();
    t.addSplit(s);
    foreach (const auto split, d->m_selectedTransactions[0].transaction().splits()) {
      if (split.id() != splitId) {
        MyMoneySplit s0 = split;
        s0.clearId();
        s0.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
        s0.setReconcileDate(QDate());
        t.addSplit(s0);
      }
    }
    KEditScheduleDlg::newSchedule(t, eMyMoney::Schedule::Occurrence::Monthly);
  }
}

void KGlobalLedgerView::slotAssignNumber()
{
  Q_D(KGlobalLedgerView);
  if (d->m_transactionEditor)
    d->m_transactionEditor->assignNextNumber();
}

void KGlobalLedgerView::slotStartReconciliation()
{
  Q_D(KGlobalLedgerView);

  // we cannot reconcile standard accounts
  if (!MyMoneyFile::instance()->isStandardAccount(d->m_currentAccount.id()))
    emit selectByObject(d->m_currentAccount, eView::Intent::StartEnteringOverdueScheduledTransactions);
  // asynchronous call to KScheduledView::slotEnterOverdueSchedules is made here
  // after that all activity should be continued in KGlobalLedgerView::slotContinueReconciliation()
}

void KGlobalLedgerView::slotFinishReconciliation()
{
  Q_D(KGlobalLedgerView);
  const auto file = MyMoneyFile::instance();

  if (!d->m_reconciliationAccount.id().isEmpty()) {
    // retrieve list of all transactions that are not reconciled or cleared
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > transactionList;
    MyMoneyTransactionFilter filter(d->m_reconciliationAccount.id());
    filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
    filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
    filter.setDateFilter(QDate(), d->m_endingBalanceDlg->statementDate());
    filter.setConsiderCategory(false);
    filter.setReportAllSplits(true);
    file->transactionList(transactionList, filter);

    auto balance = MyMoneyFile::instance()->balance(d->m_reconciliationAccount.id(), d->m_endingBalanceDlg->statementDate());
    MyMoneyMoney actBalance, clearedBalance;
    actBalance = clearedBalance = balance;

    // walk the list of transactions to figure out the balance(s)
    for (auto it = transactionList.constBegin(); it != transactionList.constEnd(); ++it) {
      if ((*it).second.reconcileFlag() == eMyMoney::Split::State::NotReconciled) {
        clearedBalance -= (*it).second.shares();
      }
    }

    if (d->m_endingBalanceDlg->endingBalance() != clearedBalance) {
      auto message = i18n("You are about to finish the reconciliation of this account with a difference between your bank statement and the transactions marked as cleared.\n"
                             "Are you sure you want to finish the reconciliation?");
      if (KMessageBox::questionYesNo(this, message, i18n("Confirm end of reconciliation"), KStandardGuiItem::yes(), KStandardGuiItem::no()) == KMessageBox::No)
        return;
    }

    MyMoneyFileTransaction ft;

    // refresh object
    d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());

    // Turn off reconciliation mode
//    Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
//    slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
//    d->m_myMoneyView->finishReconciliation(d->m_reconciliationAccount);

    // only update the last statement balance here, if we haven't a newer one due
    // to download of online statements.
    if (d->m_reconciliationAccount.value("lastImportedTransactionDate").isEmpty()
        || QDate::fromString(d->m_reconciliationAccount.value("lastImportedTransactionDate"), Qt::ISODate) < d->m_endingBalanceDlg->statementDate()) {
      d->m_reconciliationAccount.setValue("lastStatementBalance", d->m_endingBalanceDlg->endingBalance().toString());
      // in case we override the last statement balance here, we have to make sure
      // that we don't show the online balance anymore, as it might be different
      d->m_reconciliationAccount.deletePair("lastImportedTransactionDate");
    }
    d->m_reconciliationAccount.setLastReconciliationDate(d->m_endingBalanceDlg->statementDate());

    // keep a record of this reconciliation
    d->m_reconciliationAccount.addReconciliation(d->m_endingBalanceDlg->statementDate(), d->m_endingBalanceDlg->endingBalance());

    d->m_reconciliationAccount.deletePair("lastReconciledBalance");
    d->m_reconciliationAccount.deletePair("statementBalance");
    d->m_reconciliationAccount.deletePair("statementDate");

    try {
      // update the account data
      file->modifyAccount(d->m_reconciliationAccount);

      /*
      // collect the list of cleared splits for this account
      filter.clear();
      filter.addAccount(d->m_reconciliationAccount.id());
      filter.addState(eMyMoney::TransactionFilter::Cleared);
      filter.setConsiderCategory(false);
      filter.setReportAllSplits(true);
      file->transactionList(transactionList, filter);
      */

      // walk the list of transactions/splits and mark the cleared ones as reconciled

      for (auto it = transactionList.begin(); it != transactionList.end(); ++it) {
        MyMoneySplit sp = (*it).second;
        // skip the ones that are not marked cleared
        if (sp.reconcileFlag() != eMyMoney::Split::State::Cleared)
          continue;

        // always retrieve a fresh copy of the transaction because we
        // might have changed it already with another split
        MyMoneyTransaction t = file->transaction((*it).first.id());
        sp.setReconcileFlag(eMyMoney::Split::State::Reconciled);
        sp.setReconcileDate(d->m_endingBalanceDlg->statementDate());
        t.modifySplit(sp);

        // update the engine ...
        file->modifyTransaction(t);

        // ... and the list
        (*it) = qMakePair(t, sp);
      }
      ft.commit();

      // reload account data from engine as the data might have changed in the meantime
      d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());

      /**
        * This signal is emitted when an account has been successfully reconciled
        * and all transactions are updated in the engine. It can be used by plugins
        * to create reconciliation reports.
        *
        * @param account the account data
        * @param date the reconciliation date as provided through the dialog
        * @param startingBalance the starting balance as provided through the dialog
        * @param endingBalance the ending balance as provided through the dialog
        * @param transactionList reference to QList of QPair containing all
        *        transaction/split pairs processed by the reconciliation.
        */
      emit selectByVariant(QVariantList {
                             QVariant::fromValue(d->m_reconciliationAccount),
                             QVariant::fromValue(d->m_endingBalanceDlg->statementDate()),
                             QVariant::fromValue(d->m_endingBalanceDlg->previousBalance()),
                             QVariant::fromValue(d->m_endingBalanceDlg->endingBalance()),
                             QVariant::fromValue(transactionList)
                           }, eView::Intent::AccountReconciled);

    } catch (const MyMoneyException &) {
      qDebug("Unexpected exception when setting cleared to reconcile");
    }
    // Turn off reconciliation mode
    Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
    slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
  }
  // Turn off reconciliation mode
  d->m_reconciliationAccount = MyMoneyAccount();
  updateActions(d->m_currentAccount);
  updateLedgerActionsInternal();
  d->loadView();
//  slotUpdateActions();
}

void KGlobalLedgerView::slotPostponeReconciliation()
{
  Q_D(KGlobalLedgerView);
  MyMoneyFileTransaction ft;
  const auto file = MyMoneyFile::instance();

  if (!d->m_reconciliationAccount.id().isEmpty()) {
    // refresh object
    d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());

    // Turn off reconciliation mode
//    Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
//    slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
//    d->m_myMoneyView->finishReconciliation(d->m_reconciliationAccount);

    d->m_reconciliationAccount.setValue("lastReconciledBalance", d->m_endingBalanceDlg->previousBalance().toString());
    d->m_reconciliationAccount.setValue("statementBalance", d->m_endingBalanceDlg->endingBalance().toString());
    d->m_reconciliationAccount.setValue("statementDate", d->m_endingBalanceDlg->statementDate().toString(Qt::ISODate));

    try {
      file->modifyAccount(d->m_reconciliationAccount);
      ft.commit();
      d->m_reconciliationAccount = MyMoneyAccount();
      updateActions(d->m_currentAccount);
      updateLedgerActionsInternal();
//      slotUpdateActions();
    } catch (const MyMoneyException &) {
      qDebug("Unexpected exception when setting last reconcile info into account");
      ft.rollback();
      d->m_reconciliationAccount = file->account(d->m_reconciliationAccount.id());
    }
    // Turn off reconciliation mode
    Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
    slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
    d->loadView();
  }
}

void KGlobalLedgerView::slotOpenAccount()
{
  Q_D(KGlobalLedgerView);
  if (!MyMoneyFile::instance()->isStandardAccount(d->m_currentAccount.id()))
    slotLedgerSelected(d->m_currentAccount.id(), QString());
}

void KGlobalLedgerView::slotFindTransaction()
{
  Q_D(KGlobalLedgerView);
  if (!d->m_searchDlg) {
    d->m_searchDlg = new KFindTransactionDlg(this);
    connect(d->m_searchDlg, &QObject::destroyed, this, &KGlobalLedgerView::slotCloseSearchDialog);
    connect(d->m_searchDlg, &KFindTransactionDlg::transactionSelected,
            this, &KGlobalLedgerView::slotLedgerSelected);
  }
  d->m_searchDlg->show();
  d->m_searchDlg->raise();
  d->m_searchDlg->activateWindow();
}

void KGlobalLedgerView::slotCloseSearchDialog()
{
  Q_D(KGlobalLedgerView);
  if (d->m_searchDlg)
    d->m_searchDlg->deleteLater();
  d->m_searchDlg = nullptr;
}

void KGlobalLedgerView::slotStatusMsg(const QString& txt)
{
  emit selectByVariant(QVariantList {QVariant(txt)}, eView::Intent::ReportProgressMessage);
}

void KGlobalLedgerView::slotStatusProgress(int cnt, int base)
{
  emit selectByVariant(QVariantList {QVariant(cnt), QVariant(base)}, eView::Intent::ReportProgress);
}

void KGlobalLedgerView::slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& list)
{
  updateLedgerActions(list);
  emit selectByVariant(QVariantList {QVariant::fromValue(list)}, eView::Intent::SelectRegisterTransactions);
}
