/***************************************************************************
                          kaccountsview.cpp
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#ifndef KACCOUNTSVIEW_P_H
#define KACCOUNTSVIEW_P_H

#include "kaccountsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountsview.h"
#include "kmymoneyaccountsviewbase_p.h"

#include "mymoneyexception.h"
#include "mymoneysplit.h"
#include "mymoneyschedule.h"
#include "mymoneytransaction.h"
#include "knewaccountdlg.h"
#include "keditloanwizard.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "accountsviewproxymodel.h"
#include "kmymoneyplugin.h"
#include "icons.h"
#include "mymoneyenums.h"
#include "menuenums.h"
#include "mymoneystatementreader.h"
#include "kmymoneyutils.h"

using namespace Icons;

class KAccountsViewPrivate : public KMyMoneyAccountsViewBasePrivate
{
  Q_DECLARE_PUBLIC(KAccountsView)

public:
  explicit KAccountsViewPrivate(KAccountsView *qq) :
    q_ptr(qq),
    ui(new Ui::KAccountsView),
    m_haveUnusedCategories(false),
    m_onlinePlugins(nullptr)
  {
  }

  ~KAccountsViewPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KAccountsView);
    m_accountTree = &ui->m_accountTree;

    // setup icons for collapse and expand button
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    m_proxyModel = ui->m_accountTree->init(View::Accounts);
    q->connect(m_proxyModel, &AccountsProxyModel::unusedIncomeExpenseAccountHidden, q, &KAccountsView::slotUnusedIncomeExpenseAccountHidden);
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KAccountsView::selectByObject);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByVariant, q, &KAccountsView::selectByVariant);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KAccountsView::refresh);
  }

  void editLoan()
  {
    Q_Q(KAccountsView);
    if (m_currentAccount.id().isEmpty())
      return;

    const auto file = MyMoneyFile::instance();
    if (file->isStandardAccount(m_currentAccount.id()))
      return;

    QPointer<KEditLoanWizard> wizard = new KEditLoanWizard(m_currentAccount);
    q->connect(wizard, &KEditLoanWizard::newCategory, q, &KAccountsView::slotNewCategory);
    q->connect(wizard, &KEditLoanWizard::createPayee, q, &KAccountsView::slotNewPayee);
    if (wizard->exec() == QDialog::Accepted && wizard != 0) {
      MyMoneySchedule sch;
      try {
        sch = file->schedule(m_currentAccount.value("schedule").toLatin1());
      } catch (const MyMoneyException &) {
        qDebug() << "schedule" << m_currentAccount.value("schedule").toLatin1() << "not found";
      }
      if (!(m_currentAccount == wizard->account())
          || !(sch == wizard->schedule())) {
        MyMoneyFileTransaction ft;
        try {
          file->modifyAccount(wizard->account());
          if (!sch.id().isEmpty()) {
            sch = wizard->schedule();
          }
          try {
            file->schedule(sch.id());
            file->modifySchedule(sch);
            ft.commit();
          } catch (const MyMoneyException &) {
            try {
              if(sch.transaction().splitCount() >= 2) {
                file->addSchedule(sch);
              }
              ft.commit();
            } catch (const MyMoneyException &e) {
              qDebug("Cannot add schedule: '%s'", e.what());
            }
          }
        } catch (const MyMoneyException &e) {
          qDebug("Unable to modify account %s: '%s'", qPrintable(m_currentAccount.name()),
                 e.what());
        }
      }
    }
    delete wizard;
  }

  void editAccount()
  {
    if (m_currentAccount.id().isEmpty())
      return;

    const auto file = MyMoneyFile::instance();
    if (file->isStandardAccount(m_currentAccount.id()))
      return;


    // set a status message so that the application can't be closed until the editing is done
    //        slotStatusMsg(caption);

    auto tid = file->openingBalanceTransaction(m_currentAccount);
    MyMoneyTransaction t;
    MyMoneySplit s0, s1;
    QPointer<KNewAccountDlg> dlg =
        new KNewAccountDlg(m_currentAccount, true, false, 0, i18n("Edit account '%1'", m_currentAccount.name()));

    if (!tid.isEmpty()) {
      try {
        t = file->transaction(tid);
        s0 = t.splitByAccount(m_currentAccount.id());
        s1 = t.splitByAccount(m_currentAccount.id(), false);
        dlg->setOpeningBalance(s0.shares());
        if (m_currentAccount.accountGroup() == eMyMoney::Account::Type::Liability) {
          dlg->setOpeningBalance(-s0.shares());
        }
      } catch (const MyMoneyException &e) {
        qDebug() << "Error retrieving opening balance transaction " << tid << ": " << e.what() << "\n";
        tid.clear();
      }
    }

    // check for online modules
    QMap<QString, KMyMoneyPlugin::OnlinePlugin *>::const_iterator it_plugin;
    if (m_onlinePlugins) {
      it_plugin = m_onlinePlugins->constEnd();
      const auto& kvp = m_currentAccount.onlineBankingSettings();
      if (!kvp["provider"].isEmpty()) {
        // if we have an online provider for this account, we need to check
        // that we have the corresponding plugin. If that exists, we ask it
        // to provide an additional tab for the account editor.
        it_plugin = m_onlinePlugins->constFind(kvp["provider"].toLower());
        if (it_plugin != m_onlinePlugins->constEnd()) {
          QString name;
          auto w = (*it_plugin)->accountConfigTab(m_currentAccount, name);
          dlg->addTab(w, name);
        }
      }
    }

    if (dlg != 0 && dlg->exec() == QDialog::Accepted) {
      try {
        MyMoneyFileTransaction ft;

        auto account = dlg->account();
        auto parent = dlg->parentAccount();
        if (m_onlinePlugins && it_plugin != m_onlinePlugins->constEnd()) {
          account.setOnlineBankingSettings((*it_plugin)->onlineBankingSettings(account.onlineBankingSettings()));
        }
        auto bal = dlg->openingBalance();
        if (m_currentAccount.accountGroup() == eMyMoney::Account::Type::Liability) {
          bal = -bal;
        }

        // we need to modify first, as reparent would override all other changes
        file->modifyAccount(account);
        if (account.parentAccountId() != parent.id()) {
          file->reparentAccount(account, parent);
        }
        if (!tid.isEmpty() && dlg->openingBalance().isZero()) {
          file->removeTransaction(t);

        } else if (!tid.isEmpty() && !dlg->openingBalance().isZero()) {
          s0.setShares(bal);
          s0.setValue(bal);
          t.modifySplit(s0);
          s1.setShares(-bal);
          s1.setValue(-bal);
          t.modifySplit(s1);
          t.setPostDate(account.openingDate());
          file->modifyTransaction(t);

        } else if (tid.isEmpty() && !dlg->openingBalance().isZero()) {
          file->createOpeningBalanceTransaction(m_currentAccount, bal);
        }

        ft.commit();

        // reload the account object as it might have changed in the meantime
        //            slotSelectAccount(file->account(account.id()));

      } catch (const MyMoneyException &e) {
        Q_Q(KAccountsView);
        KMessageBox::error(q, i18n("Unable to modify account '%1'. Cause: %2", m_currentAccount.name(), e.what()));
      }
    }

    delete dlg;
    //        ready();

  }

  enum CanCloseAccountCodeE {
    AccountCanClose = 0,        // can close the account
    AccountBalanceNonZero,      // balance is non zero
    AccountChildrenOpen,        // account has open children account
    AccountScheduleReference,   // account is referenced in a schedule
    AccountHasOnlineMapping,    // account has an online mapping
  };

  /**
    * This method checks, if an account can be closed or not. An account
    * can be closed if:
    *
    * - the balance is zero and
    * - all children are already closed and
    * - there is no unfinished schedule referencing the account
    * - and no online mapping is setup
    *
    * @param acc reference to MyMoneyAccount object in question
    * @retval true account can be closed
    * @retval false account cannot be closed
    */
  CanCloseAccountCodeE canCloseAccount(const MyMoneyAccount& acc)
  {
    // balance must be zero
    if (!acc.balance().isZero())
      return AccountBalanceNonZero;
    if (acc.hasOnlineMapping())
      return AccountHasOnlineMapping;

    // all children must be already closed
    foreach (const auto sAccount, acc.accountList()) {
      if (!MyMoneyFile::instance()->account(sAccount).isClosed()) {
        return AccountChildrenOpen;
      }
    }

    // there must be no unfinished schedule referencing the account
    QList<MyMoneySchedule> list = MyMoneyFile::instance()->scheduleList();
    QList<MyMoneySchedule>::const_iterator it_l;
    for (it_l = list.constBegin(); it_l != list.constEnd(); ++it_l) {
      if ((*it_l).isFinished())
        continue;
      if ((*it_l).hasReferenceTo(acc.id()))
        return AccountScheduleReference;
    }
    return AccountCanClose;
  }

  /**
   * This method checks if an account can be closed and enables/disables
   * the close account action
   * If disabled, it sets a tooltip explaning why it cannot be closed
   * @brief enableCloseAccountAction
   * @param acc reference to MyMoneyAccount object in question
   */
  void hintCloseAccountAction(const MyMoneyAccount& acc, QAction* a)
  {
    switch (canCloseAccount(acc)) {
      case AccountCanClose:
        a->setToolTip(QString());
        break;
      case AccountBalanceNonZero:
        a->setToolTip(i18n("The balance of the account must be zero before the account can be closed"));
        break;
      case AccountChildrenOpen:
        a->setToolTip(i18n("All subaccounts must be closed before the account can be closed"));
        break;
      case AccountScheduleReference:
        a->setToolTip(i18n("This account is still included in an active schedule"));
        break;
      case AccountHasOnlineMapping:
        a->setToolTip(i18n("This account is still mapped to an online account"));
        break;
    }
  }

  void accountsUpdateOnline(const QList<MyMoneyAccount>& accList)
  {
    Q_Q(KAccountsView);

    // block the update account actions for now so that we don't get here twice
    const QVector<eMenu::Action> disabledActions {eMenu::Action::UpdateAccount, eMenu::Action::UpdateAllAccounts};
    for (const auto& a : disabledActions)
      pActions[a]->setEnabled(false);

    // clear global message list
    MyMoneyStatementReader::clearResultMessages();

    // process all entries that have a mapped account and the 'provider' is available
    // we need to make sure that only the very last entry that matches sets the
    // 'moreAccounts' parameter in the call to updateAccount() to false
    auto processedAccounts = 0;

    for (auto it_provider = m_onlinePlugins->constBegin(); it_provider != m_onlinePlugins->constEnd(); ++it_provider) {
      auto nextAccount = accList.cend();
      for (auto it_a = accList.cbegin(); it_a != accList.cend(); ++it_a) {
        if ((*it_a).hasOnlineMapping()
        && (it_provider == m_onlinePlugins->constFind((*it_a).onlineBankingSettings().value("provider").toLower()))) {
          if (nextAccount != accList.cend()) {
            (*it_provider)->updateAccount(*nextAccount, true);
          }
          nextAccount = it_a;
          ++processedAccounts;
        }
      }
      // process a possible pending entry
      if (nextAccount != accList.cend()) {
        (*it_provider)->updateAccount(*nextAccount, false);
      }
    }

    // re-enable the disabled actions
    q->updateActions(m_currentAccount);

    KMyMoneyUtils::showStatementImportResult(MyMoneyStatementReader::resultMessages(), processedAccounts);
  }

  KAccountsView       *q_ptr;
  Ui::KAccountsView   *ui;
  bool                m_haveUnusedCategories;
  MyMoneyAccount      m_currentAccount;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* m_onlinePlugins;
};

#endif
