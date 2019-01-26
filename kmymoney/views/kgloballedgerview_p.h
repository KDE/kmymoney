/***************************************************************************
                          kgloballedgerview_p.h  -  description
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

#ifndef KGLOBALLEDGERVIEW_P_H
#define KGLOBALLEDGERVIEW_P_H

#include "kgloballedgerview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QFrame>
#include <QHBoxLayout>
#include <QList>
#include <QLabel>
#include <QEvent>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QToolTip>
#include <QMenu>
#include <QWidgetAction>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KToolBar>
#include <KPassivePopup>

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyviewbase_p.h"
#include "kendingbalancedlg.h"
#include "kfindtransactiondlg.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneyutils.h"
#include "mymoneyexception.h"
#include "mymoneymoney.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "kmymoneyaccountcombo.h"
#include "kbalancewarning.h"
#include "transactionmatcher.h"
#include "tabbar.h"
#include "register.h"
#include "transactioneditor.h"
#include "selectedtransactions.h"
#include "kmymoneysettings.h"
#include "registersearchline.h"
#include "scheduledtransaction.h"
#include "accountsmodel.h"
#include "models.h"
#include "mymoneyprice.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneysplit.h"
#include "mymoneypayee.h"
#include "mymoneytracer.h"
#include "transaction.h"
#include "transactionform.h"
#include "fancydategroupmarkers.h"
#include "widgetenums.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "menuenums.h"

#include <config-kmymoney.h>
#ifdef KMM_DEBUG
#include "mymoneyutils.h"
#endif

using namespace eMenu;
using namespace eMyMoney;

/**
  * helper class implementing an event filter to detect mouse button press
  * events on widgets outside a given set of widgets. This is used internally
  * to detect when to leave the edit mode.
  */
class MousePressFilter : public QObject
{
  Q_OBJECT
public:
  explicit MousePressFilter(QWidget* parent = nullptr) :
    QObject(parent),
    m_lastMousePressEvent(0),
    m_filterActive(true)
  {
  }

  /**
    * Add widget @p w to the list of possible parent objects. See eventFilter() how
    * they will be used.
    */
  void addWidget(QWidget* w)
  {
    m_parents.append(w);
  }

public Q_SLOTS:
  /**
    * This slot allows to activate/deactivate the filter. By default the
    * filter is active.
    *
    * @param state Allows to activate (@a true) or deactivate (@a false) the filter
    */
  void setFilterActive(bool state = true)
  {
    m_filterActive = state;
  }

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
  bool isChildOf(QWidget* child, QWidget* parent)
  {
    // QDialogs cannot be detected directly, but it can be assumed,
    // that events on a widget that do not have a parent widget within
    // our application are dialogs.
    if (!child->parentWidget())
      return true;

    while (child) {
      // if we are a child of the given parent, we have a match
      if (child == parent)
        return true;
      // if we are at the application level, we don't have a match
      if (child->inherits("KMyMoneyApp"))
        return false;
      // If one of the ancestors is a KPassivePopup or a KDialog or a popup widget then
      // it's as if it is a child of our own because these widgets could
      // appear during transaction entry (message boxes, completer widgets)
      if (dynamic_cast<KPassivePopup*>(child) ||
          ((child->windowFlags() & Qt::Popup) && /*child != kmymoney*/
           !child->parentWidget())) // has no parent, then it must be top-level window
        return true;
      child = child->parentWidget();
    }
    return false;
  }

  /**
    * Reimplemented from base class. Sends out the mousePressedOnExternalWidget() signal
    * if object @p o points to an object which is not a child widget of any added previously
    * using the addWidget() method. The signal is sent out only once for each event @p e.
    *
    * @param o pointer to QObject
    * @param e pointer to QEvent
    * @return always returns @a false
    */
  bool eventFilter(QObject* o, QEvent* e) final override
  {
    if (m_filterActive) {
      if (e->type() == QEvent::MouseButtonPress && !m_lastMousePressEvent) {
        QWidget* w = qobject_cast<QWidget*>(o);
        if (!w) {
          return QObject::eventFilter(o, e);
        }
        QList<QWidget*>::const_iterator it_w;
        for (it_w = m_parents.constBegin(); it_w != m_parents.constEnd(); ++it_w) {
          if (isChildOf(w, (*it_w))) {
            m_lastMousePressEvent = e;
            break;
          }
        }
        if (it_w == m_parents.constEnd()) {
          m_lastMousePressEvent = e;
          bool rc = false;
          emit mousePressedOnExternalWidget(rc);
        }
      }

      if (e->type() != QEvent::MouseButtonPress) {
        m_lastMousePressEvent = 0;
      }
    }
    return false;
  }

Q_SIGNALS:
  void mousePressedOnExternalWidget(bool&);

private:
  QList<QWidget*>      m_parents;
  QEvent*              m_lastMousePressEvent;
  bool                 m_filterActive;
};

class KGlobalLedgerViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KGlobalLedgerView)

public:
  explicit KGlobalLedgerViewPrivate(KGlobalLedgerView *qq) :
    q_ptr(qq),
    m_mousePressFilter(0),
    m_registerSearchLine(0),
    m_precision(2),
    m_recursion(false),
    m_showDetails(false),
    m_action(eWidgets::eRegister::Action::None),
    m_filterProxyModel(0),
    m_accountComboBox(0),
    m_balanceIsApproximated(false),
    m_toolbarFrame(nullptr),
    m_registerFrame(nullptr),
    m_buttonFrame(nullptr),
    m_formFrame(nullptr),
    m_summaryFrame(nullptr),
    m_register(nullptr),
    m_buttonbar(nullptr),
    m_leftSummaryLabel(nullptr),
    m_centerSummaryLabel(nullptr),
    m_rightSummaryLabel(nullptr),
    m_form(nullptr),
    m_needLoad(true),
    m_newAccountLoaded(true),
    m_inEditMode(false),
    m_transactionEditor(nullptr),
    m_balanceWarning(nullptr),
    m_moveToAccountSelector(nullptr),
    m_endingBalanceDlg(nullptr),
    m_searchDlg(nullptr)
  {
  }

  ~KGlobalLedgerViewPrivate()
  {
    delete m_moveToAccountSelector;
    delete m_endingBalanceDlg;
    delete m_searchDlg;
  }

  void init()
  {
    Q_Q(KGlobalLedgerView);
    m_needLoad = false;
    auto vbox = new QVBoxLayout(q);
    q->setLayout(vbox);
    vbox->setSpacing(6);
    vbox->setMargin(0);

    m_mousePressFilter = new MousePressFilter((QWidget*)q);
    m_action = eWidgets::eRegister::Action::None;

    // the proxy filter model
    m_filterProxyModel = new AccountNamesFilterProxyModel(q);
    m_filterProxyModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Equity});
    auto const model = Models::instance()->accountsModel();
    m_filterProxyModel->setSourceModel(model);
    m_filterProxyModel->setSourceColumns(model->getColumns());
    m_filterProxyModel->sort((int)eAccountsModel::Column::Account);

    // create the toolbar frame at the top of the view
    m_toolbarFrame = new QFrame();
    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_toolbarFrame);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(6);

    // the account selector widget
    m_accountComboBox = new KMyMoneyAccountCombo();
    m_accountComboBox->setModel(m_filterProxyModel);
    toolbarLayout->addWidget(m_accountComboBox);

    vbox->addWidget(m_toolbarFrame);
    toolbarLayout->setStretchFactor(m_accountComboBox, 60);
    // create the register frame
    m_registerFrame = new QFrame();
    QVBoxLayout* registerFrameLayout = new QVBoxLayout(m_registerFrame);
    registerFrameLayout->setContentsMargins(0, 0, 0, 0);
    registerFrameLayout->setSpacing(0);
    vbox->addWidget(m_registerFrame);
    vbox->setStretchFactor(m_registerFrame, 2);
    m_register = new KMyMoneyRegister::Register(m_registerFrame);
    m_register->setUsedWithEditor(true);
    registerFrameLayout->addWidget(m_register);
    m_register->installEventFilter(q);
    q->connect(m_register, &KMyMoneyRegister::Register::openContextMenu, q, &KGlobalLedgerView::slotTransactionsContextMenuRequested);
    q->connect(m_register, &KMyMoneyRegister::Register::transactionsSelected, q, &KGlobalLedgerView::slotUpdateSummaryLine);
    q->connect(m_register->horizontalHeader(), &QWidget::customContextMenuRequested, q, &KGlobalLedgerView::slotSortOptions);
    q->connect(m_register, &KMyMoneyRegister::Register::reconcileStateColumnClicked, q, &KGlobalLedgerView::slotToggleTransactionMark);

    // insert search line widget

    m_registerSearchLine = new KMyMoneyRegister::RegisterSearchLineWidget(m_register, m_toolbarFrame);
    toolbarLayout->addWidget(m_registerSearchLine);
    toolbarLayout->setStretchFactor(m_registerSearchLine, 100);
    // create the summary frame
    m_summaryFrame = new QFrame();
    QHBoxLayout* summaryFrameLayout = new QHBoxLayout(m_summaryFrame);
    summaryFrameLayout->setContentsMargins(0, 0, 0, 0);
    summaryFrameLayout->setSpacing(0);
    m_leftSummaryLabel = new QLabel(m_summaryFrame);
    m_centerSummaryLabel = new QLabel(m_summaryFrame);
    m_rightSummaryLabel = new QLabel(m_summaryFrame);
    summaryFrameLayout->addWidget(m_leftSummaryLabel);
    QSpacerItem* spacer = new QSpacerItem(20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
    summaryFrameLayout->addItem(spacer);
    summaryFrameLayout->addWidget(m_centerSummaryLabel);
    spacer = new QSpacerItem(20, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
    summaryFrameLayout->addItem(spacer);
    summaryFrameLayout->addWidget(m_rightSummaryLabel);
    vbox->addWidget(m_summaryFrame);

    // create the button frame
    m_buttonFrame = new QFrame(q);
    QVBoxLayout* buttonLayout = new QVBoxLayout(m_buttonFrame);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    vbox->addWidget(m_buttonFrame);
    m_buttonbar = new KToolBar(m_buttonFrame, 0, true);
    m_buttonbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buttonLayout->addWidget(m_buttonbar);

    m_buttonbar->addAction(pActions[eMenu::Action::NewTransaction]);
    m_buttonbar->addAction(pActions[eMenu::Action::DeleteTransaction]);
    m_buttonbar->addAction(pActions[eMenu::Action::EditTransaction]);
    m_buttonbar->addAction(pActions[eMenu::Action::EnterTransaction]);
    m_buttonbar->addAction(pActions[eMenu::Action::CancelTransaction]);
    m_buttonbar->addAction(pActions[eMenu::Action::AcceptTransaction]);
    m_buttonbar->addAction(pActions[eMenu::Action::MatchTransaction]);

    // create the transaction form frame
    m_formFrame = new QFrame(q);
    QVBoxLayout* frameLayout = new QVBoxLayout(m_formFrame);
    frameLayout->setContentsMargins(5, 5, 5, 5);
    frameLayout->setSpacing(0);
    m_form = new KMyMoneyTransactionForm::TransactionForm(m_formFrame);
    frameLayout->addWidget(m_form->getTabBar(m_formFrame));
    frameLayout->addWidget(m_form);
    m_formFrame->setFrameShape(QFrame::Panel);
    m_formFrame->setFrameShadow(QFrame::Raised);
    vbox->addWidget(m_formFrame);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KGlobalLedgerView::refresh);
    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KGlobalLedgerView::slotUpdateMoveToAccountMenu);
    q->connect(m_register, static_cast<void (KMyMoneyRegister::Register::*)(KMyMoneyRegister::Transaction *)>(&KMyMoneyRegister::Register::focusChanged), m_form, &KMyMoneyTransactionForm::TransactionForm::slotSetTransaction);
    q->connect(m_register, static_cast<void (KMyMoneyRegister::Register::*)()>(&KMyMoneyRegister::Register::focusChanged), q, &KGlobalLedgerView::updateLedgerActionsInternal);
//    q->connect(m_accountComboBox, &KMyMoneyAccountCombo::accountSelected, q, &KGlobalLedgerView::slotAccountSelected);
    q->connect(m_accountComboBox, &KMyMoneyAccountCombo::accountSelected, q, static_cast<void (KGlobalLedgerView::*)(const QString&)>(&KGlobalLedgerView::slotSelectAccount));
    q->connect(m_accountComboBox, &KMyMoneyAccountCombo::accountSelected, q, &KGlobalLedgerView::slotUpdateMoveToAccountMenu);
    q->connect(m_register, &KMyMoneyRegister::Register::transactionsSelected, q, &KGlobalLedgerView::slotTransactionsSelected);
    q->connect(m_register, &KMyMoneyRegister::Register::transactionsSelected, q, &KGlobalLedgerView::slotUpdateMoveToAccountMenu);
    q->connect(m_register, &KMyMoneyRegister::Register::editTransaction, q, &KGlobalLedgerView::slotEditTransaction);
    q->connect(m_register, &KMyMoneyRegister::Register::emptyItemSelected, q, &KGlobalLedgerView::slotNewTransaction);
    q->connect(m_register, &KMyMoneyRegister::Register::aboutToSelectItem, q, &KGlobalLedgerView::slotAboutToSelectItem);
    q->connect(m_mousePressFilter, &MousePressFilter::mousePressedOnExternalWidget, q, &KGlobalLedgerView::slotCancelOrEnterTransactions);

    q->connect(m_form, &KMyMoneyTransactionForm::TransactionForm::newTransaction, q, static_cast<void (KGlobalLedgerView::*)(eWidgets::eRegister::Action)>(&KGlobalLedgerView::slotNewTransactionForm));

    // setup mouse press filter
    m_mousePressFilter->addWidget(m_formFrame);
    m_mousePressFilter->addWidget(m_buttonFrame);
    m_mousePressFilter->addWidget(m_summaryFrame);
    m_mousePressFilter->addWidget(m_registerFrame);

    m_tooltipPosn = QPoint();
  }

  /**
    * This method reloads the account selection combo box of the
    * view with all asset and liability accounts from the engine.
    * If the account id of the current account held in @p m_accountId is
    * empty or if the referenced account does not exist in the engine,
    * the first account found in the list will be made the current account.
    */
  void loadAccounts()
  {
    const auto file = MyMoneyFile::instance();

    // check if the current account still exists and make it the
    // current account
    if (!m_lastSelectedAccountID.isEmpty()) {
      try {
        m_currentAccount = file->account(m_lastSelectedAccountID);
      } catch (const MyMoneyException &) {
        m_lastSelectedAccountID.clear();
        m_currentAccount = MyMoneyAccount();
        m_accountComboBox->setSelected(QString());
      }
    }

    // TODO: check why the invalidate is needed here
    m_filterProxyModel->invalidate();
    m_filterProxyModel->sort((int)eAccountsModel::Column::Account);
    m_filterProxyModel->setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts() && !KMyMoneySettings::showAllAccounts());
    m_filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    m_accountComboBox->expandAll();

    if (m_currentAccount.id().isEmpty()) {
      // find the first favorite account
      QModelIndexList list = m_filterProxyModel->match(m_filterProxyModel->index(0, 0),
                             (int)eAccountsModel::Role::Favorite,
                             QVariant(true),
                             1,
                             Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive | Qt::MatchRecursive));
      if (list.count() > 0) {
        QVariant accountId = list.front().data((int)eAccountsModel::Role::ID);
        if (accountId.isValid()) {
          m_currentAccount = file->account(accountId.toString());
        }
      }

      if (m_currentAccount.id().isEmpty()) {
        // there are no favorite accounts find any account
        list = m_filterProxyModel->match(m_filterProxyModel->index(0, 0),
                               Qt::DisplayRole,
                               QVariant(QString("*")),
                               -1,
                               Qt::MatchFlags(Qt::MatchWildcard | Qt::MatchRecursive));
        for (QModelIndexList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
          if (!it->parent().isValid())
            continue; // skip the top level accounts
          QVariant accountId = (*it).data((int)eAccountsModel::Role::ID);
          if (accountId.isValid()) {
            MyMoneyAccount a = file->account(accountId.toString());
            if (!a.isInvest() && !a.isClosed()) {
              m_currentAccount = a;
              break;
            }
          }
        }
      }
    }

    if (!m_currentAccount.id().isEmpty()) {
      m_accountComboBox->setSelected(m_currentAccount.id());
      try {
        m_precision = MyMoneyMoney::denomToPrec(m_currentAccount.fraction());
      } catch (const MyMoneyException &) {
        qDebug("Security %s for account %s not found", qPrintable(m_currentAccount.currencyId()), qPrintable(m_currentAccount.name()));
        m_precision = 2;
      }
    }
  }

  /**
    * This method clears the register, form, transaction list. See @sa m_register,
    * @sa m_transactionList
    */
  void clear()
  {
    // clear current register contents
    m_register->clear();

    // setup header font
    QFont font = KMyMoneySettings::listHeaderFontEx();
    QFontMetrics fm(font);
    int height = fm.lineSpacing() + 6;
    m_register->horizontalHeader()->setMinimumHeight(height);
    m_register->horizontalHeader()->setMaximumHeight(height);
    m_register->horizontalHeader()->setFont(font);

    // setup cell font
    font = KMyMoneySettings::listCellFontEx();
    m_register->setFont(font);

    // clear the form
    m_form->clear();

    // the selected transactions list
    m_transactionList.clear();

    // and the selected account in the combo box
    m_accountComboBox->setSelected(QString());

    // fraction defaults to two digits
    m_precision = 2;
  }

  void loadView()
  {
    MYMONEYTRACER(tracer);
    Q_Q(KGlobalLedgerView);

    // setup form visibility
    m_formFrame->setVisible(KMyMoneySettings::transactionForm());

    // no account selected
//    emit q->objectSelected(MyMoneyAccount());
    // no transaction selected
    KMyMoneyRegister::SelectedTransactions list;
    emit q->selectByVariant(QVariantList {QVariant::fromValue(list)}, eView::Intent::SelectRegisterTransactions);

    QMap<QString, bool> isSelected;
    QString focusItemId;
    QString backUpFocusItemId;  // in case the focus item is removed
    QString anchorItemId;
    QString backUpAnchorItemId; // in case the anchor item is removed

    if (!m_newAccountLoaded) {
      // remember the current selected transactions
      KMyMoneyRegister::RegisterItem* item = m_register->firstItem();
      for (; item; item = item->nextItem()) {
        if (item->isSelected()) {
          isSelected[item->id()] = true;
        }
      }
      // remember the item that has the focus
      storeId(m_register->focusItem(), focusItemId, backUpFocusItemId);
      // and the one that has the selection anchor
      storeId(m_register->anchorItem(), anchorItemId, backUpAnchorItemId);
    } else {
      m_registerSearchLine->searchLine()->clear();
    }

    // clear the current contents ...
    clear();

    // ... load the combobox widget and select current account ...
    loadAccounts();

    // ... setup the register columns ...
    m_register->setupRegister(m_currentAccount);

    // ... setup the form ...
    m_form->setupForm(m_currentAccount);

    if (m_currentAccount.id().isEmpty()) {
      // if we don't have an account we bail out
      q->setEnabled(false);
      return;
    }
    q->setEnabled(true);

    m_register->setUpdatesEnabled(false);

    // ... and recreate it
    KMyMoneyRegister::RegisterItem* focusItem = 0;
    KMyMoneyRegister::RegisterItem* anchorItem = 0;
    QMap<QString, MyMoneyMoney> actBalance, clearedBalance, futureBalance;
    QMap<QString, MyMoneyMoney>::iterator it_b;
    try {
      // setup the filter to select the transactions we want to display
      // and update the sort order
      QString sortOrder;
      QString key;
      QDate reconciliationDate = m_reconciliationDate;

      MyMoneyTransactionFilter filter(m_currentAccount.id());
      // if it's an investment account, we also take care of
      // the sub-accounts (stock accounts)
      if (m_currentAccount.accountType() == eMyMoney::Account::Type::Investment)
        filter.addAccount(m_currentAccount.accountList());

      if (isReconciliationAccount()) {
        key = "kmm-sort-reconcile";
        sortOrder = KMyMoneySettings::sortReconcileView();
        filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
        filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
      } else {
        filter.setDateFilter(KMyMoneySettings::startDate().date(), QDate());
        key = "kmm-sort-std";
        sortOrder = KMyMoneySettings::sortNormalView();
        if (KMyMoneySettings::hideReconciledTransactions()
            && !m_currentAccount.isIncomeExpense()) {
          filter.addState((int)eMyMoney::TransactionFilter::State::NotReconciled);
          filter.addState((int)eMyMoney::TransactionFilter::State::Cleared);
        }
      }
      filter.setReportAllSplits(true);

      // check if we have an account override of the sort order
      if (!m_currentAccount.value(key).isEmpty())
        sortOrder = m_currentAccount.value(key);

      // setup sort order
      m_register->setSortOrder(sortOrder);

      // retrieve the list from the engine
      MyMoneyFile::instance()->transactionList(m_transactionList, filter);

      emit q->slotStatusProgress(0, m_transactionList.count());

      // create the elements for the register
      QList<QPair<MyMoneyTransaction, MyMoneySplit> >::const_iterator it;
      QMap<QString, int>uniqueMap;
      int i = 0;
      for (it = m_transactionList.constBegin(); it != m_transactionList.constEnd(); ++it) {
        uniqueMap[(*it).first.id()]++;
        KMyMoneyRegister::Transaction* t = KMyMoneyRegister::Register::transactionFactory(m_register, (*it).first, (*it).second, uniqueMap[(*it).first.id()]);
        actBalance[t->split().accountId()] = MyMoneyMoney();
        emit q->slotStatusProgress(++i, 0);
        // if we're in reconciliation and the state is cleared, we
        // force the item to show in dimmed intensity to get a visual focus
        // on those items, that we need to work on
        if (isReconciliationAccount() && (*it).second.reconcileFlag() == eMyMoney::Split::State::Cleared) {
          t->setReducedIntensity(true);
        }
      }

      // create dummy entries for the scheduled transactions if sorted by postdate
      int period = KMyMoneySettings::schedulePreview();
      if (m_register->primarySortKey() == eWidgets::SortField::PostDate) {
        // show scheduled transactions which have a scheduled postdate
        // within the next 'period' days. In reconciliation mode, the
        // period starts on the statement date.
        QDate endDate = QDate::currentDate().addDays(period);
        if (isReconciliationAccount())
          endDate = reconciliationDate.addDays(period);
        QList<MyMoneySchedule> scheduleList = MyMoneyFile::instance()->scheduleList(m_currentAccount.id());
        while (!scheduleList.isEmpty()) {
          MyMoneySchedule& s = scheduleList.first();
          for (;;) {
            if (s.isFinished() || s.adjustedNextDueDate() > endDate) {
              break;
            }

            MyMoneyTransaction t(s.id(), KMyMoneyUtils::scheduledTransaction(s));
            if (s.isOverdue() && !KMyMoneySettings::showPlannedScheduleDates()) {
              // if the transaction is scheduled and overdue, it can't
              // certainly be posted in the past. So we take today's date
              // as the alternative
              t.setPostDate(s.adjustedDate(QDate::currentDate(), s.weekendOption()));
            } else {
              t.setPostDate(s.adjustedNextDueDate());
            }
            foreach (const auto split, t.splits()) {
              if (split.accountId() == m_currentAccount.id()) {
                new KMyMoneyRegister::StdTransactionScheduled(m_register, t, split, uniqueMap[t.id()]);
              }
            }
            // keep track of this payment locally (not in the engine)
            if (s.isOverdue() && !KMyMoneySettings::showPlannedScheduleDates()) {
              s.setLastPayment(QDate::currentDate());
            } else {
              s.setLastPayment(s.nextDueDate());
            }

            // if this is a one time schedule, we can bail out here as we're done
            if (s.occurrence() == eMyMoney::Schedule::Occurrence::Once)
              break;

            // for all others, we check if the next payment date is still 'in range'
            QDate nextDueDate = s.nextPayment(s.nextDueDate());
            if (nextDueDate.isValid()) {
              s.setNextDueDate(nextDueDate);
            } else {
              break;
            }
          }
          scheduleList.pop_front();
        }
      }

      // add the group markers
      m_register->addGroupMarkers();

      // sort the transactions according to the sort setting
      m_register->sortItems();

      // remove trailing and adjacent markers
      m_register->removeUnwantedGroupMarkers();

      // add special markers for reconciliation now so that they do not get
      // removed by m_register->removeUnwantedGroupMarkers(). Needs resorting
      // of items but that's ok.

      KMyMoneyRegister::StatementGroupMarker* statement = 0;
      KMyMoneyRegister::StatementGroupMarker* dStatement = 0;
      KMyMoneyRegister::StatementGroupMarker* pStatement = 0;

      if (isReconciliationAccount()) {
        switch (m_register->primarySortKey()) {
          case eWidgets::SortField::PostDate:
            statement = new KMyMoneyRegister::StatementGroupMarker(m_register, eWidgets::eRegister::CashFlowDirection::Deposit, reconciliationDate, i18n("Statement Details"));
            m_register->sortItems();
            break;
          case eWidgets::SortField::Type:
            dStatement = new KMyMoneyRegister::StatementGroupMarker(m_register, eWidgets::eRegister::CashFlowDirection::Deposit, reconciliationDate, i18n("Statement Deposit Details"));
            pStatement = new KMyMoneyRegister::StatementGroupMarker(m_register, eWidgets::eRegister::CashFlowDirection::Payment, reconciliationDate, i18n("Statement Payment Details"));
            m_register->sortItems();
            break;
          default:
            break;
        }
      }

      // we need at least the balance for the account we currently show
      actBalance[m_currentAccount.id()] = MyMoneyMoney();

      if (m_currentAccount.accountType() == eMyMoney::Account::Type::Investment)
        foreach (const auto accountID, m_currentAccount.accountList())
          actBalance[accountID] = MyMoneyMoney();

      // determine balances (actual, cleared). We do this by getting the actual
      // balance of all entered transactions from the engine and walk the list
      // of transactions backward. Also re-select a transaction if it was
      // selected before and setup the focus item.

      MyMoneyMoney factor(1, 1);
      if (m_currentAccount.accountGroup() == eMyMoney::Account::Type::Liability
          || m_currentAccount.accountGroup() == eMyMoney::Account::Type::Equity)
        factor = -factor;

      QMap<QString, int> deposits;
      QMap<QString, int> payments;
      QMap<QString, MyMoneyMoney> depositAmount;
      QMap<QString, MyMoneyMoney> paymentAmount;
      for (it_b = actBalance.begin(); it_b != actBalance.end(); ++it_b) {
        MyMoneyMoney balance = MyMoneyFile::instance()->balance(it_b.key());
        balance = balance * factor;
        clearedBalance[it_b.key()] =
          futureBalance[it_b.key()] =
            (*it_b) = balance;
        deposits[it_b.key()] = payments[it_b.key()] = 0;
        depositAmount[it_b.key()] = MyMoneyMoney();
        paymentAmount[it_b.key()] = MyMoneyMoney();
      }

      tracer.printf("total balance of %s = %s", qPrintable(m_currentAccount.name()), qPrintable(actBalance[m_currentAccount.id()].formatMoney("", 2)));
      tracer.printf("future balance of %s = %s", qPrintable(m_currentAccount.name()), qPrintable(futureBalance[m_currentAccount.id()].formatMoney("", 2)));
      tracer.printf("cleared balance of %s = %s", qPrintable(m_currentAccount.name()), qPrintable(clearedBalance[m_currentAccount.id()].formatMoney("", 2)));

      KMyMoneyRegister::RegisterItem* p = m_register->lastItem();
      focusItem = 0;

      // take care of possibly trailing scheduled transactions (bump up the future balance)
      while (p) {
        if (p->isSelectable()) {
          KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
          if (t && t->isScheduled()) {
            MyMoneyMoney balance = futureBalance[t->split().accountId()];
            const MyMoneySplit& split = t->split();
            // if this split is a stock split, we can't just add the amount of shares
            if (t->transaction().isStockSplit()) {
              balance = balance * split.shares();
            } else {
              balance += split.shares() * factor;
            }
            futureBalance[split.accountId()] = balance;
          } else if (t && !focusItem)
            focusItem = p;
        }
        p = p->prevItem();
      }

      p = m_register->lastItem();
      while (p) {
        KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
        if (t) {
          if (isSelected.contains(t->id()))
            t->setSelected(true);

          matchItemById(&focusItem, t, focusItemId, backUpFocusItemId);
          matchItemById(&anchorItem, t, anchorItemId, backUpAnchorItemId);

          const MyMoneySplit& split = t->split();
          MyMoneyMoney balance = futureBalance[split.accountId()];
          t->setBalance(balance);

          // if this split is a stock split, we can't just add the amount of shares
          if (t->transaction().isStockSplit()) {
            balance /= split.shares();
          } else {
            balance -= split.shares() * factor;
          }

          if (!t->isScheduled()) {
            if (isReconciliationAccount() && t->transaction().postDate() <= reconciliationDate && split.reconcileFlag() == eMyMoney::Split::State::Cleared) {
              if (split.shares().isNegative()) {
                payments[split.accountId()]++;
                paymentAmount[split.accountId()] += split.shares();
              } else {
                deposits[split.accountId()]++;
                depositAmount[split.accountId()] += split.shares();
              }
            }

            if (t->transaction().postDate() > QDate::currentDate()) {
              tracer.printf("Reducing actual balance by %s because %s/%s(%s) is in the future", qPrintable((split.shares() * factor).formatMoney("", 2)), qPrintable(t->transaction().id()), qPrintable(split.id()), qPrintable(t->transaction().postDate().toString(Qt::ISODate)));
              actBalance[split.accountId()] -= split.shares() * factor;
            }
          }
          futureBalance[split.accountId()] = balance;
        }
        p = p->prevItem();
      }

      clearedBalance[m_currentAccount.id()] = MyMoneyFile::instance()->clearedBalance(m_currentAccount.id(), reconciliationDate);

      tracer.printf("total balance of %s = %s", qPrintable(m_currentAccount.name()), qPrintable(actBalance[m_currentAccount.id()].formatMoney("", 2)));
      tracer.printf("future balance of %s = %s", qPrintable(m_currentAccount.name()), qPrintable(futureBalance[m_currentAccount.id()].formatMoney("", 2)));
      tracer.printf("cleared balance of %s = %s", qPrintable(m_currentAccount.name()), qPrintable(clearedBalance[m_currentAccount.id()].formatMoney("", 2)));

      // update statement information
      if (statement) {
        const QString aboutDeposits = i18np("%1 deposit (%2)", "%1 deposits (%2)",
                                            deposits[m_currentAccount.id()], depositAmount[m_currentAccount.id()].abs().formatMoney(m_currentAccount.fraction()));
        const QString aboutPayments = i18np("%1 payment (%2)", "%1 payments (%2)",
                                            payments[m_currentAccount.id()], paymentAmount[m_currentAccount.id()].abs().formatMoney(m_currentAccount.fraction()));

        statement->setText(i18nc("%1 is a string, e.g. 7 deposits; %2 is a string, e.g. 4 payments", "%1, %2", aboutDeposits, aboutPayments));
      }
      if (pStatement) {
        pStatement->setText(i18np("%1 payment (%2)", "%1 payments (%2)", payments[m_currentAccount.id()]
                                  , paymentAmount[m_currentAccount.id()].abs().formatMoney(m_currentAccount.fraction())));
      }
      if (dStatement) {
        dStatement->setText(i18np("%1 deposit (%2)", "%1 deposits (%2)", deposits[m_currentAccount.id()]
                                  , depositAmount[m_currentAccount.id()].abs().formatMoney(m_currentAccount.fraction())));
      }

      // add a last empty entry for new transactions
      // leave some information about the current account
      MyMoneySplit split;
      split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
      // make sure to use the value specified in the option during reconciliation
      if (isReconciliationAccount())
        split.setReconcileFlag(static_cast<eMyMoney::Split::State>(KMyMoneySettings::defaultReconciliationState()));
      KMyMoneyRegister::Register::transactionFactory(m_register, MyMoneyTransaction(), split, 0);

      m_register->updateRegister(true);

      if (focusItem) {
        // in case we have some selected items we just set the focus item
        // in other cases, we make the focusitem also the selected item
        if (anchorItem && (anchorItem != focusItem)) {
          m_register->setFocusItem(focusItem);
          m_register->setAnchorItem(anchorItem);
        } else
          m_register->selectItem(focusItem, true);
      } else {
        // just use the empty line at the end if nothing else exists in the ledger
        p = m_register->lastItem();
        m_register->setFocusItem(p);
        m_register->selectItem(p);
        focusItem = p;
      }

      updateSummaryLine(actBalance, clearedBalance);
      emit q->slotStatusProgress(-1, -1);

    } catch (const MyMoneyException &) {
      m_currentAccount = MyMoneyAccount();
      clear();
    }

    m_showDetails = KMyMoneySettings::showRegisterDetailed();

    // and tell everyone what's selected
    emit q->selectByObject(m_currentAccount, eView::Intent::None);
    KMyMoneyRegister::SelectedTransactions actualSelection(m_register);
    emit q->selectByVariant(QVariantList {QVariant::fromValue(actualSelection)}, eView::Intent::SelectRegisterTransactions);
  }

  void selectTransaction(const QString& id)
  {
    if (!id.isEmpty()) {
      KMyMoneyRegister::RegisterItem* p = m_register->lastItem();
      while (p) {
        KMyMoneyRegister::Transaction* t = dynamic_cast<KMyMoneyRegister::Transaction*>(p);
        if (t) {
          if (t->transaction().id() == id) {
            m_register->selectItem(t);
            m_register->ensureItemVisible(t);
            break;
          }
        }
        p = p->prevItem();
      }
    }
  }

  /**
   * @brief selects transactions for processing with slots
   * @param list of transactions
   * @return false if only schedule is to be selected
   */
  bool selectTransactions(const KMyMoneyRegister::SelectedTransactions list)
  {
    Q_Q(KGlobalLedgerView);
    // list can either contain a list of transactions or a single selected scheduled transaction
    // in the latter case, the transaction id is actually the one of the schedule. In order
    // to differentiate between the two, we just ask for the schedule. If we don't find one - because
    // we passed the id of a real transaction - then we know that fact.  We use the schedule here,
    // because the list of schedules is kept in a cache by MyMoneyFile. This way, we save some trips
    // to the backend which we would have to do if we check for the transaction.
    m_selectedTransactions.clear();
    auto sch = MyMoneySchedule();
    auto ret = true;

    m_accountGoto.clear();
    m_payeeGoto.clear();
    if (!list.isEmpty() && !list.first().isScheduled()) {
        m_selectedTransactions = list;
        if (list.count() == 1) {
            const MyMoneySplit& sp = m_selectedTransactions[0].split();
            if (!sp.payeeId().isEmpty()) {
                try {
                  auto payee = MyMoneyFile::instance()->payee(sp.payeeId());
                  if (!payee.name().isEmpty()) {
                      m_payeeGoto = payee.id();
                      auto name = payee.name();
                      name.replace(QRegExp("&(?!&)"), "&&");
                      pActions[Action::GoToPayee]->setText(i18n("Go to '%1'", name));
                    }
                } catch (const MyMoneyException &) {
                }
              }
            try {
              const auto& t = m_selectedTransactions[0].transaction();
              // search the first non-income/non-expense accunt and use it for the 'goto account'
              const auto& selectedTransactionSplit = m_selectedTransactions[0].split();
              foreach (const auto split, t.splits()) {
                  if (split.id() != selectedTransactionSplit.id()) {
                      auto acc = MyMoneyFile::instance()->account(split.accountId());
                      if (!acc.isIncomeExpense()) {
                          // for stock accounts we show the portfolio account
                          if (acc.isInvest()) {
                              acc = MyMoneyFile::instance()->account(acc.parentAccountId());
                            }
                          m_accountGoto = acc.id();
                          auto name = acc.name();
                          name.replace(QRegExp("&(?!&)"), "&&");
                          pActions[Action::GoToAccount]->setText(i18n("Go to '%1'", name));
                          break;
                        }
                    }
                }
            } catch (const MyMoneyException &) {
            }
          }
      } else if (!list.isEmpty()) {
        sch = MyMoneyFile::instance()->schedule(list.first().scheduleId());
        m_selectedTransactions.append(list.first());
        ret = false;
      }

    emit q->selectByObject(sch, eView::Intent::None);

    // make sure, we show some neutral menu entry if we don't have an object
    if (m_payeeGoto.isEmpty())
      pActions[Action::GoToPayee]->setText(i18n("Go to payee"));
    if (m_accountGoto.isEmpty())
      pActions[Action::GoToAccount]->setText(i18n("Go to account"));
    return ret;
  }

  /**
    * Returns @a true if setReconciliationAccount() has been called for
    * the current loaded account.
    *
    * @retval true current account is in reconciliation mode
    * @retval false current account is not in reconciliation mode
    */
  bool isReconciliationAccount() const
  {
    return m_currentAccount.id() == m_reconciliationAccount.id();
  }

  /**
    * Updates the values on the summary line beneath the register with
    * the given values. The contents shown differs between reconciliation
    * mode and normal mode.
    *
    * @param actBalance map of account indexed values to be used as actual balance
    * @param clearedBalance map of account indexed values to be used as cleared balance
    */
  void updateSummaryLine(const QMap<QString, MyMoneyMoney>& actBalance, const QMap<QString, MyMoneyMoney>& clearedBalance)
  {
    Q_Q(KGlobalLedgerView);
    const auto file = MyMoneyFile::instance();
    m_leftSummaryLabel->show();
    m_centerSummaryLabel->show();
    m_rightSummaryLabel->show();

    if (isReconciliationAccount()) {
      if (m_currentAccount.accountType() != eMyMoney::Account::Type::Investment) {
        m_leftSummaryLabel->setText(i18n("Statement: %1", m_endingBalance.formatMoney("", m_precision)));
        m_centerSummaryLabel->setText(i18nc("Cleared balance", "Cleared: %1", clearedBalance[m_currentAccount.id()].formatMoney("", m_precision)));
        m_totalBalance = clearedBalance[m_currentAccount.id()] - m_endingBalance;
      }
    } else {
      // update summary line in normal mode
      QDate reconcileDate = m_currentAccount.lastReconciliationDate();

      if (reconcileDate.isValid()) {
        m_leftSummaryLabel->setText(i18n("Last reconciled: %1", QLocale().toString(reconcileDate, QLocale::ShortFormat)));
      } else {
        m_leftSummaryLabel->setText(i18n("Never reconciled"));
      }

      QPalette palette = m_rightSummaryLabel->palette();
      palette.setColor(m_rightSummaryLabel->foregroundRole(), m_leftSummaryLabel->palette().color(q->foregroundRole()));
      if (m_currentAccount.accountType() != eMyMoney::Account::Type::Investment) {
        m_centerSummaryLabel->setText(i18nc("Cleared balance", "Cleared: %1", clearedBalance[m_currentAccount.id()].formatMoney("", m_precision)));
        m_totalBalance = actBalance[m_currentAccount.id()];
      } else {
        m_centerSummaryLabel->hide();
        MyMoneyMoney balance;
        MyMoneySecurity base = file->baseCurrency();
        QMap<QString, MyMoneyMoney>::const_iterator it_b;
        // reset the approximated flag
        m_balanceIsApproximated = false;
        for (it_b = actBalance.begin(); it_b != actBalance.end(); ++it_b) {
          MyMoneyAccount stock = file->account(it_b.key());
          QString currencyId = stock.currencyId();
          MyMoneySecurity sec = file->security(currencyId);
          MyMoneyMoney rate(1, 1);

          if (stock.isInvest()) {
            currencyId = sec.tradingCurrency();
            const MyMoneyPrice &priceInfo = file->price(sec.id(), currencyId);
            m_balanceIsApproximated |= !priceInfo.isValid();
            rate = priceInfo.rate(sec.tradingCurrency());
          }

          if (currencyId != base.id()) {
            const MyMoneyPrice &priceInfo = file->price(sec.tradingCurrency(), base.id());
            m_balanceIsApproximated |= !priceInfo.isValid();
            rate = (rate * priceInfo.rate(base.id())).convertPrecision(sec.pricePrecision());
          }
          balance += ((*it_b) * rate).convert(base.smallestAccountFraction());
        }
        m_totalBalance = balance;
      }
      m_rightSummaryLabel->setPalette(palette);
    }
    // determine the number of selected transactions
    KMyMoneyRegister::SelectedTransactions selection;
    m_register->selectedTransactions(selection);
    q->slotUpdateSummaryLine(selection);
  }

  /**
    * setup the default action according to the current account type
    */
  void setupDefaultAction()
  {
    switch (m_currentAccount.accountType()) {
      case eMyMoney::Account::Type::Asset:
      case eMyMoney::Account::Type::AssetLoan:
      case eMyMoney::Account::Type::Savings:
        m_action = eWidgets::eRegister::Action::Deposit;
        break;
      default:
        m_action = eWidgets::eRegister::Action::Withdrawal;
        break;
    }
  }

  // used to store the id of an item and the id of an immeadiate unselected sibling
  void storeId(KMyMoneyRegister::RegisterItem *item, QString &id, QString &backupId) {
    if (item) {
      // the id of the item
      id = item->id();
      // the id of the item's previous/next unselected item
      for (KMyMoneyRegister::RegisterItem *it = item->prevItem(); it != 0 && backupId.isEmpty(); it = it->prevItem()) {
        if (!it->isSelected()) {
          backupId = it->id();
        }
      }
      // if we didn't found previous unselected items search trough the next items
      for (KMyMoneyRegister::RegisterItem *it = item->nextItem(); it != 0 && backupId.isEmpty(); it = it->nextItem()) {
        if (!it->isSelected()) {
          backupId = it->id();
        }
      }
    }
  }

  // use to match an item by it's id or a backup id which has a lower precedence
  void matchItemById(KMyMoneyRegister::RegisterItem **item, KMyMoneyRegister::Transaction* t, QString &id, QString &backupId) {
    if (!backupId.isEmpty() && t->id() == backupId)
      *item = t;
    if (!id.isEmpty() && t->id() == id) {
      // we found the real thing there's no need for the backup anymore
      backupId.clear();
      *item = t;
    }
  }

  bool canProcessTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
  {
    if (m_register->focusItem() == 0)
      return false;

    bool rc = true;
    if (list.warnLevel() == KMyMoneyRegister::SelectedTransaction::OneAccountClosed) {
      // scan all splits for the first closed account
      QString closedAccount;
      foreach(const auto selectedTransaction, list) {
        foreach(const auto split, selectedTransaction.transaction().splits()) {
          const auto id = split.accountId();
          const auto acc = MyMoneyFile::instance()->account(id);
          if (acc.isClosed()) {
            closedAccount = acc.name();
            // we're done
            rc = false;
            break;
          }
        }
        if(!rc)
          break;
      }
      tooltip = i18n("Cannot process transactions in account %1, which is closed.", closedAccount);
      showTooltip(tooltip);
      return false;
    }

    if (!m_register->focusItem()->isSelected()) {
      tooltip = i18n("Cannot process transaction with focus if it is not selected.");
      showTooltip(tooltip);
      return false;
    }
    tooltip.clear();
    return !list.isEmpty();
  }

  void showTooltip(const QString msg) const
  {
    QToolTip::showText(m_tooltipPosn, msg);
  }

  bool createNewTransaction()
  {
    Q_Q(KGlobalLedgerView);
    auto rc = false;
    QString txt;
    if (q->canCreateTransactions(txt)) {
      rc = q->selectEmptyTransaction();
    }
    return rc;
  }

  TransactionEditor* startEdit(const KMyMoneyRegister::SelectedTransactions& list)
  {
    Q_Q(KGlobalLedgerView);
    TransactionEditor* editor = 0;
    QString txt;
    if (q->canEditTransactions(list, txt) || q->canCreateTransactions(txt)) {
      editor = q->startEdit(list);
    }
    return editor;
  }

  void doDeleteTransactions()
  {
    Q_Q(KGlobalLedgerView);
    KMyMoneyRegister::SelectedTransactions list = m_selectedTransactions;
    KMyMoneyRegister::SelectedTransactions::iterator it_t;
    int cnt = list.count();
    int i = 0;
    emit q->slotStatusProgress(0, cnt);
    MyMoneyFileTransaction ft;
    const auto file = MyMoneyFile::instance();
    try {
      it_t = list.begin();
      while (it_t != list.end()) {
        // only remove those transactions that do not reference a closed account
        if (!file->referencesClosedAccount((*it_t).transaction())) {
          file->removeTransaction((*it_t).transaction());
          // remove all those references in the list of selected transactions
          // that refer to the same transaction we just removed so that we
          // will not be caught by an exception later on (see bko #285310)
          KMyMoneyRegister::SelectedTransactions::iterator it_td = it_t;
          ++it_td;
          while (it_td != list.end()) {
            if ((*it_t).transaction().id() == (*it_td).transaction().id()) {
              it_td = list.erase(it_td);
              i++; // bump count of deleted transactions
            } else {
              ++it_td;
            }
          }
        }
        // need to ensure "nextCheckNumber" is still correct
        auto acc = file->account((*it_t).split().accountId());

        // the "lastNumberUsed" might have been the txn number deleted
        // so adjust it
        QString deletedNum = (*it_t).split().number();
        if (deletedNum == acc.value("lastNumberUsed")) {
          // decrement deletedNum and set new "lastNumberUsed"
          QString num = KMyMoneyUtils::getAdjacentNumber(deletedNum, -1);
          acc.setValue("lastNumberUsed", num);
          file->modifyAccount(acc);
        }

        list.erase(it_t);
        it_t = list.begin();
        emit q->slotStatusProgress(i++, 0);
      }
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(q, i18n("Unable to delete transaction(s)"), e.what());
    }
    emit q->slotStatusProgress(-1, -1);
  }

  void deleteTransactionEditor()
  {
    // make sure, we don't use the transaction editor pointer
    // anymore from now on
    auto p = m_transactionEditor;
    m_transactionEditor = nullptr;
    delete p;
  }

  void transactionUnmatch()
  {
    Q_Q(KGlobalLedgerView);
    KMyMoneyRegister::SelectedTransactions::const_iterator it;
    MyMoneyFileTransaction ft;
    try {
      for (it = m_selectedTransactions.constBegin(); it != m_selectedTransactions.constEnd(); ++it) {
        if ((*it).split().isMatched()) {
          TransactionMatcher matcher(m_currentAccount);
          matcher.unmatch((*it).transaction(), (*it).split());
        }
      }
      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(q, i18n("Unable to unmatch the selected transactions"), e.what());
    }
  }

  void transactionMatch()
  {
    Q_Q(KGlobalLedgerView);
    if (m_selectedTransactions.count() != 2)
      return;

    MyMoneyTransaction startMatchTransaction;
    MyMoneyTransaction endMatchTransaction;
    MyMoneySplit startSplit;
    MyMoneySplit endSplit;

    KMyMoneyRegister::SelectedTransactions::const_iterator it;
    KMyMoneyRegister::SelectedTransactions toBeDeleted;
    for (it = m_selectedTransactions.constBegin(); it != m_selectedTransactions.constEnd(); ++it) {
      if ((*it).transaction().isImported()) {
        if (endMatchTransaction.id().isEmpty()) {
          endMatchTransaction = (*it).transaction();
          endSplit = (*it).split();
          toBeDeleted << *it;
        } else {
          //This is a second imported transaction, we still want to merge
          startMatchTransaction = (*it).transaction();
          startSplit = (*it).split();
        }
      } else if (!(*it).split().isMatched()) {
        if (startMatchTransaction.id().isEmpty()) {
          startMatchTransaction = (*it).transaction();
          startSplit = (*it).split();
        } else {
          endMatchTransaction = (*it).transaction();
          endSplit = (*it).split();
          toBeDeleted << *it;
        }
      }
    }

  #if 0
    KMergeTransactionsDlg dlg(m_selectedAccount);
    dlg.addTransaction(startMatchTransaction);
    dlg.addTransaction(endMatchTransaction);
    if (dlg.exec() == QDialog::Accepted)
  #endif
    {
      MyMoneyFileTransaction ft;
      try {
        if (startMatchTransaction.id().isEmpty())
          throw MYMONEYEXCEPTION(QString::fromLatin1("No manually entered transaction selected for matching"));
        if (endMatchTransaction.id().isEmpty())
          throw MYMONEYEXCEPTION(QString::fromLatin1("No imported transaction selected for matching"));

        TransactionMatcher matcher(m_currentAccount);
        matcher.match(startMatchTransaction, startSplit, endMatchTransaction, endSplit, true);
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(q, i18n("Unable to match the selected transactions"), e.what());
      }
    }
  }

  /**
    * Mark the selected transactions as provided by @a flag. If
    * flag is @a MyMoneySplit::Unknown, the future state depends
    * on the current stat of the split's flag accoring to the
    * following table:
    *
    * - NotReconciled --> Cleared
    * - Cleared --> Reconciled
    * - Reconciled --> NotReconciled
    */
  void markTransaction(eMyMoney::Split::State flag)
  {
    Q_Q(KGlobalLedgerView);
    auto list = m_selectedTransactions;
    KMyMoneyRegister::SelectedTransactions::const_iterator it_t;
    auto cnt = list.count();
    auto i = 0;
    emit q->slotStatusProgress(0, cnt);
    MyMoneyFileTransaction ft;
    try {
      for (it_t = list.constBegin(); it_t != list.constEnd(); ++it_t) {
        // turn on signals before we modify the last entry in the list
        cnt--;
        MyMoneyFile::instance()->blockSignals(cnt != 0);

        // get a fresh copy
        auto t = MyMoneyFile::instance()->transaction((*it_t).transaction().id());
        auto sp = t.splitById((*it_t).split().id());
        if (sp.reconcileFlag() != flag) {
          if (flag == eMyMoney::Split::State::Unknown) {
            if (m_reconciliationAccount.id().isEmpty()) {
              // in normal mode we cycle through all states
              switch (sp.reconcileFlag()) {
                case eMyMoney::Split::State::NotReconciled:
                  sp.setReconcileFlag(eMyMoney::Split::State::Cleared);
                  break;
                case eMyMoney::Split::State::Cleared:
                  sp.setReconcileFlag(eMyMoney::Split::State::Reconciled);
                  break;
                case eMyMoney::Split::State::Reconciled:
                  sp.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                  break;
                default:
                  break;
              }
            } else {
              // in reconciliation mode we skip the reconciled state
              switch (sp.reconcileFlag()) {
                case eMyMoney::Split::State::NotReconciled:
                  sp.setReconcileFlag(eMyMoney::Split::State::Cleared);
                  break;
                case eMyMoney::Split::State::Cleared:
                  sp.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
                  break;
                default:
                  break;
              }
            }
          } else {
            sp.setReconcileFlag(flag);
          }

          t.modifySplit(sp);
          MyMoneyFile::instance()->modifyTransaction(t);
        }
        emit q->slotStatusProgress(i++, 0);
      }
      emit q->slotStatusProgress(-1, -1);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(q, i18n("Unable to modify transaction"), e.what());
    }
  }

  // move a stock transaction from one investment account to another
  void moveInvestmentTransaction(const QString& /*fromId*/,
      const QString& toId,
      const MyMoneyTransaction& tx)
  {
    MyMoneyAccount toInvAcc = MyMoneyFile::instance()->account(toId);
    MyMoneyTransaction t(tx);
    // first determine which stock we are dealing with.
    // fortunately, investment transactions have only one stock involved
    QString stockAccountId;
    QString stockSecurityId;
    MyMoneySplit s;
    foreach (const auto split, t.splits()) {
      stockAccountId = split.accountId();
      stockSecurityId =
        MyMoneyFile::instance()->account(stockAccountId).currencyId();
      if (!MyMoneyFile::instance()->security(stockSecurityId).isCurrency()) {
        s = split;
        break;
      }
    }
    // Now check the target investment account to see if it
    // contains a stock with this id
    QString newStockAccountId;
    foreach (const auto sAccount, toInvAcc.accountList()) {
      if (MyMoneyFile::instance()->account(sAccount).currencyId() ==
          stockSecurityId) {
        newStockAccountId = sAccount;
        break;
      }
    }
    // if it doesn't exist, we need to add it as a copy of the old one
    // no 'copyAccount()' function??
    if (newStockAccountId.isEmpty()) {
      MyMoneyAccount stockAccount =
        MyMoneyFile::instance()->account(stockAccountId);
      MyMoneyAccount newStock;
      newStock.setName(stockAccount.name());
      newStock.setNumber(stockAccount.number());
      newStock.setDescription(stockAccount.description());
      newStock.setInstitutionId(stockAccount.institutionId());
      newStock.setOpeningDate(stockAccount.openingDate());
      newStock.setAccountType(stockAccount.accountType());
      newStock.setCurrencyId(stockAccount.currencyId());
      newStock.setClosed(stockAccount.isClosed());
      MyMoneyFile::instance()->addAccount(newStock, toInvAcc);
      newStockAccountId = newStock.id();
    }
    // now update the split and the transaction
    s.setAccountId(newStockAccountId);
    t.modifySplit(s);
    MyMoneyFile::instance()->modifyTransaction(t);
  }

  void createTransactionMoveMenu()
  {
    Q_Q(KGlobalLedgerView);
    if (!m_moveToAccountSelector) {
      auto menu = pMenus[eMenu::Menu::MoveTransaction];
      if (menu ) {
        auto accountSelectorAction = new QWidgetAction(menu);
        m_moveToAccountSelector = new KMyMoneyAccountSelector(menu, 0, false);
        m_moveToAccountSelector->setObjectName("transaction_move_menu_selector");
        accountSelectorAction->setDefaultWidget(m_moveToAccountSelector);
        menu->addAction(accountSelectorAction);
        q->connect(m_moveToAccountSelector, &QObject::destroyed, q, &KGlobalLedgerView::slotObjectDestroyed);
        q->connect(m_moveToAccountSelector, &KMyMoneySelector::itemSelected, q, &KGlobalLedgerView::slotMoveToAccount);
      }
    }
  }

  QList<QPair<MyMoneyTransaction, MyMoneySplit> > automaticReconciliation(const MyMoneyAccount &account,
      const QList<QPair<MyMoneyTransaction, MyMoneySplit> > &transactions,
      const MyMoneyMoney &amount)
  {
    Q_Q(KGlobalLedgerView);
    static const int NR_OF_STEPS_LIMIT = 300000;
    static const int PROGRESSBAR_STEPS = 1000;
    QList<QPair<MyMoneyTransaction, MyMoneySplit> > result = transactions;

//    KMSTATUS(i18n("Running automatic reconciliation"));
    auto progressBarIndex = 0;
    q->slotStatusProgress(progressBarIndex, NR_OF_STEPS_LIMIT / PROGRESSBAR_STEPS);

    // optimize the most common case - all transactions should be cleared
    QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplitResult(result);
    MyMoneyMoney transactionsBalance;
    while (itTransactionSplitResult.hasNext()) {
      const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplitResult.next();
      transactionsBalance += transactionSplit.second.shares();
    }
    if (amount == transactionsBalance) {
      result = transactions;
      return result;
    }
    q->slotStatusProgress(progressBarIndex++, 0);
    // only one transaction is uncleared
    itTransactionSplitResult.toFront();
    int index = 0;
    while (itTransactionSplitResult.hasNext()) {
      const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplitResult.next();
      if (transactionsBalance - transactionSplit.second.shares() == amount) {
        result.removeAt(index);
        return result;
      }
      index++;
    }
    q->slotStatusProgress(progressBarIndex++, 0);

    // more than one transaction is uncleared - apply the algorithm
    result.clear();

    const auto& security = MyMoneyFile::instance()->security(account.currencyId());
    double precision = 0.1 / account.fraction(security);

    QList<MyMoneyMoney> sumList;
    sumList << MyMoneyMoney();

    QMap<MyMoneyMoney, QList<QPair<QString, QString> > > sumToComponentsMap;

    // compute the possible matches
    QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > it_ts(transactions);
    while (it_ts.hasNext()) {
      const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = it_ts.next();
      QListIterator<MyMoneyMoney> itSum(sumList);
      QList<MyMoneyMoney> tempList;
      while (itSum.hasNext()) {
        const MyMoneyMoney &sum = itSum.next();
        QList<QPair<QString, QString> > splitIds;
        splitIds << qMakePair<QString, QString>(transactionSplit.first.id(), transactionSplit.second.id());
        if (sumToComponentsMap.contains(sum)) {
          if (sumToComponentsMap.value(sum).contains(qMakePair<QString, QString>(transactionSplit.first.id(), transactionSplit.second.id()))) {
            continue;
          }
          splitIds.append(sumToComponentsMap.value(sum));
        }
        tempList << transactionSplit.second.shares() + sum;
        sumToComponentsMap[transactionSplit.second.shares() + sum] = splitIds;
        int size = sumToComponentsMap.size();
        if (size % PROGRESSBAR_STEPS == 0) {
          q->slotStatusProgress(progressBarIndex++, 0);
        }
        if (size > NR_OF_STEPS_LIMIT) {
          return result; // it's taking too much resources abort the algorithm
        }
      }
      QList<MyMoneyMoney> unionList;
      unionList.append(tempList);
      unionList.append(sumList);
      qSort(unionList);
      sumList.clear();
      MyMoneyMoney smallestSumFromUnion = unionList.first();
      sumList.append(smallestSumFromUnion);
      QListIterator<MyMoneyMoney> itUnion(unionList);
      while (itUnion.hasNext()) {
        MyMoneyMoney sumFromUnion = itUnion.next();
        if (smallestSumFromUnion < MyMoneyMoney(1 - precision / transactions.size())*sumFromUnion) {
          smallestSumFromUnion = sumFromUnion;
          sumList.append(sumFromUnion);
        }
      }
    }

    q->slotStatusProgress(NR_OF_STEPS_LIMIT / PROGRESSBAR_STEPS, 0);
    if (sumToComponentsMap.contains(amount)) {
      QListIterator<QPair<MyMoneyTransaction, MyMoneySplit> > itTransactionSplit(transactions);
      while (itTransactionSplit.hasNext()) {
        const QPair<MyMoneyTransaction, MyMoneySplit> &transactionSplit = itTransactionSplit.next();
        const QList<QPair<QString, QString> > &splitIds = sumToComponentsMap.value(amount);
        if (splitIds.contains(qMakePair<QString, QString>(transactionSplit.first.id(), transactionSplit.second.id()))) {
          result.append(transactionSplit);
        }
      }
    }

  #ifdef KMM_DEBUG
    qDebug("For the amount %s a number of %d possible sums where computed from the set of %d transactions: ",
           qPrintable(MyMoneyUtils::formatMoney(amount, security)), sumToComponentsMap.size(), transactions.size());
  #endif

    q->slotStatusProgress(-1, -1);
    return result;
  }

  KGlobalLedgerView   *q_ptr;
  MousePressFilter    *m_mousePressFilter;
  KMyMoneyRegister::RegisterSearchLineWidget* m_registerSearchLine;
//  QString              m_reconciliationAccount;
  QDate                m_reconciliationDate;
  MyMoneyMoney         m_endingBalance;
  int                  m_precision;
  bool                 m_recursion;
  bool                 m_showDetails;
  eWidgets::eRegister::Action m_action;

  // models
  AccountNamesFilterProxyModel *m_filterProxyModel;

  // widgets
  KMyMoneyAccountCombo* m_accountComboBox;

  MyMoneyMoney         m_totalBalance;
  bool                 m_balanceIsApproximated;
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
  MyMoneyAccount m_currentAccount;
  QString m_lastSelectedAccountID;

  MyMoneyAccount m_reconciliationAccount;

  /**
    * This member holds the transaction list
    */
  QList<QPair<MyMoneyTransaction, MyMoneySplit> >  m_transactionList;

  QLabel*                         m_leftSummaryLabel;
  QLabel*                         m_centerSummaryLabel;
  QLabel*                         m_rightSummaryLabel;

  KMyMoneyTransactionForm::TransactionForm* m_form;

  /**
    * This member holds the load state of page
    */
  bool                            m_needLoad;

  bool                            m_newAccountLoaded;
  bool                            m_inEditMode;

  QWidgetList                     m_tabOrderWidgets;
  QPoint                          m_tooltipPosn;
  KMyMoneyRegister::SelectedTransactions m_selectedTransactions;
  /**
    * This member keeps the date that was used as the last posting date.
    * It will be updated whenever the user modifies the post date
    * and is used to preset the posting date when new transactions are created.
    * This member is initialised to the current date when the program is started.
    */
  static QDate         m_lastPostDate;
  // pointer to the current transaction editor
  QPointer<TransactionEditor> m_transactionEditor;

  // id's that need to be remembered
  QString  m_accountGoto, m_payeeGoto;
  QString  m_lastPayeeEnteredId;
  QScopedPointer<KBalanceWarning> m_balanceWarning;
  KMyMoneyAccountSelector* m_moveToAccountSelector;

  // Reconciliation dialog
  KEndingBalanceDlg*    m_endingBalanceDlg;
  KFindTransactionDlg*  m_searchDlg;
};

#endif
