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

#include "kaccountsview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QBitArray>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "knewaccountwizard.h"
#include "kbalancechartdlg.h"
#include "kmymoneyutils.h"
#include "kmymoneyglobalsettings.h"
#include "storageenums.h"
#include "menuenums.h"

using namespace Icons;

KAccountsView::KAccountsView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KAccountsViewPrivate(this), parent)
{
  connect(pActions[eMenu::Action::NewAccount],          &QAction::triggered, this, &KAccountsView::slotNewAccount);
  connect(pActions[eMenu::Action::EditAccount],         &QAction::triggered, this, &KAccountsView::slotEditAccount);
  connect(pActions[eMenu::Action::DeleteAccount],       &QAction::triggered, this, &KAccountsView::slotDeleteAccount);
  connect(pActions[eMenu::Action::CloseAccount],        &QAction::triggered, this, &KAccountsView::slotCloseAccount);
  connect(pActions[eMenu::Action::ReopenAccount],       &QAction::triggered, this, &KAccountsView::slotReopenAccount);
  connect(pActions[eMenu::Action::ChartAccountBalance], &QAction::triggered, this, &KAccountsView::slotChartAccountBalance);
}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::setDefaultFocus()
{
  Q_D(KAccountsView);
  QTimer::singleShot(0, d->ui->m_accountTree, SLOT(setFocus()));
}

void KAccountsView::refresh()
{
  Q_D(KAccountsView);
  if (!isVisible()) {
    d->m_needsRefresh = true;
    return;
  }
  d->m_needsRefresh = false;
  // TODO: check why the invalidate is needed here
  d->m_proxyModel->invalidate();
  d->m_proxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts());
  d->m_proxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  if (KMyMoneyGlobalSettings::showCategoriesInAccountsView()) {
    d->m_proxyModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense});
  } else {
    d->m_proxyModel->removeAccountType(eMyMoney::Account::Type::Income);
    d->m_proxyModel->removeAccountType(eMyMoney::Account::Type::Expense);
  }

  // reinitialize the default state of the hidden categories label
  d->m_haveUnusedCategories = false;
  d->ui->m_hiddenCategories->hide();  // hides label
  d->m_proxyModel->setHideUnusedIncomeExpenseAccounts(KMyMoneyGlobalSettings::hideUnusedCategory());
}

void KAccountsView::showEvent(QShowEvent * event)
{
  Q_D(KAccountsView);
  if (!d->m_proxyModel)
    d->init();

  emit aboutToShow(View::Accounts);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KAccountsView::updateActions(const MyMoneyObject& obj)
{
  Q_D(KAccountsView);
  if (typeid(obj) != typeid(MyMoneyAccount) &&
      (obj.id().isEmpty() && d->m_currentAccount.id().isEmpty())) // do not disable actions that were already disabled)
    return;

  const auto& acc = static_cast<const MyMoneyAccount&>(obj);

  const QVector<eMenu::Action> actionsToBeDisabled {
        eMenu::Action::NewAccount, eMenu::Action::EditAccount, eMenu::Action::DeleteAccount,
        eMenu::Action::CloseAccount, eMenu::Action::ReopenAccount,
        eMenu::Action::ChartAccountBalance,
        eMenu::Action::UnmapOnlineAccount, eMenu::Action::MapOnlineAccount, eMenu::Action::UpdateAccount
  };

  for (const auto& a : actionsToBeDisabled)
    pActions[a]->setEnabled(false);

  pActions[eMenu::Action::NewAccount]->setEnabled(true);

  const auto file = MyMoneyFile::instance();
  if (acc.id().isEmpty()) {
    d->m_currentAccount = MyMoneyAccount();
    return;
  } else if (file->isStandardAccount(acc.id())) {
    d->m_currentAccount = acc;
    return;
  }
  d->m_currentAccount = acc;

  switch (acc.accountGroup()) {
    case eMyMoney::Account::Type::Asset:
    case eMyMoney::Account::Type::Liability:
    case eMyMoney::Account::Type::Equity:
    {
      pActions[eMenu::Action::EditAccount]->setEnabled(true);
      pActions[eMenu::Action::DeleteAccount]->setEnabled(!file->isReferenced(acc));

      auto b = acc.isClosed() ? true : false;
      pActions[eMenu::Action::ReopenAccount]->setEnabled(b);
      pActions[eMenu::Action::CloseAccount]->setEnabled(!b);

      if (!acc.isClosed()) {
        b = (d->canCloseAccount(acc) == KAccountsViewPrivate::AccountCanClose) ? true : false;
        pActions[eMenu::Action::CloseAccount]->setEnabled(b);
        d->hintCloseAccountAction(acc, pActions[eMenu::Action::CloseAccount]);
      }

      pActions[eMenu::Action::ChartAccountBalance]->setEnabled(true);

      if (d->m_currentAccount.hasOnlineMapping()) {
        pActions[eMenu::Action::UnmapOnlineAccount]->setEnabled(true);

        if (d->m_onlinePlugins) {
          // check if provider is available
          QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
          it_p = d->m_onlinePlugins->constFind(d->m_currentAccount.onlineBankingSettings().value(QLatin1String("provider")).toLower());
          if (it_p != d->m_onlinePlugins->constEnd()) {
            QStringList protocols;
            (*it_p)->protocols(protocols);
            if (protocols.count() > 0) {
              pActions[eMenu::Action::UpdateAccount]->setEnabled(true);
            }
          }
        }

      } else {
        pActions[eMenu::Action::MapOnlineAccount]->setEnabled(d->m_onlinePlugins && !d->m_onlinePlugins->isEmpty());
      }

      break;
    }
    default:
      break;
  }

  QBitArray skip((int)eStorage::Reference::Count);
  if (!d->m_currentAccount.id().isEmpty()) {
    if (!file->isStandardAccount(d->m_currentAccount.id())) {
      switch (d->m_currentAccount.accountGroup()) {
        case eMyMoney::Account::Type::Asset:
        case eMyMoney::Account::Type::Liability:
        case eMyMoney::Account::Type::Equity:

          break;

        default:
          break;
      }
    }
  }

}

void KAccountsView::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins)
{
  Q_D(KAccountsView);
  d->m_onlinePlugins = &plugins;
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KAccountsView::slotUnusedIncomeExpenseAccountHidden()
{
  Q_D(KAccountsView);
  d->m_haveUnusedCategories = true;
  d->ui->m_hiddenCategories->setVisible(d->m_haveUnusedCategories);
}

void KAccountsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
  Q_D(KAccountsView);
  d->netBalProChanged(netWorth, d->ui->m_totalProfitsLabel, View::Accounts);
}

void KAccountsView::slotShowAccountMenu(const MyMoneyAccount& acc)
{
  Q_UNUSED(acc);
  pMenus[eMenu::Menu::Account]->exec(QCursor::pos());
}

void KAccountsView::slotNewAccount()
{
  MyMoneyAccount account;
  account.setOpeningDate(KMyMoneyGlobalSettings::firstFiscalDate());
  NewAccountWizard::Wizard::newAccount(account);
}

void KAccountsView::slotEditAccount()
{
  Q_D(KAccountsView);

  switch (d->m_currentAccount.accountType()) {
    case eMyMoney::Account::Type::Loan:
    case eMyMoney::Account::Type::AssetLoan:
      d->editLoan();
      break;
    default:
      d->editAccount();
      break;
  }
  emit objectSelected(d->m_currentAccount);
}

void KAccountsView::slotDeleteAccount()
{
  Q_D(KAccountsView);
  if (d->m_currentAccount.id().isEmpty())
    return;  // need an account ID

  const auto file = MyMoneyFile::instance();
  // can't delete standard accounts or account which still have transactions assigned
  if (file->isStandardAccount(d->m_currentAccount.id()))
    return;

  // check if the account is referenced by a transaction or schedule
  QBitArray skip((int)eStorage::Reference::Count);
  skip.fill(false);
  skip.setBit((int)eStorage::Reference::Account);
  skip.setBit((int)eStorage::Reference::Institution);
  skip.setBit((int)eStorage::Reference::Payee);
  skip.setBit((int)eStorage::Reference::Tag);
  skip.setBit((int)eStorage::Reference::Security);
  skip.setBit((int)eStorage::Reference::Currency);
  skip.setBit((int)eStorage::Reference::Price);
  if (file->isReferenced(d->m_currentAccount, skip))
    return;

  MyMoneyFileTransaction ft;

  // retain the account name for a possible later usage in the error message box
  // since the account removal notifies the views the selected account can be changed
  // so we make sure by doing this that we display the correct name in the error message
  auto selectedAccountName = d->m_currentAccount.name();

  try {
    file->removeAccount(d->m_currentAccount);
    d->m_currentAccount.clearId();
    emit objectSelected(MyMoneyAccount());
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::error(this, i18n("Unable to delete account '%1'. Cause: %2", selectedAccountName, e.what()));
  }
}

void KAccountsView::slotCloseAccount()
{
  Q_D(KAccountsView);
  MyMoneyFileTransaction ft;
  try {
    d->m_currentAccount.setClosed(true);
    MyMoneyFile::instance()->modifyAccount(d->m_currentAccount);
    emit objectSelected(d->m_currentAccount);
    ft.commit();
    if (KMyMoneyGlobalSettings::hideClosedAccounts())
      KMessageBox::information(this, i18n("<qt>You have closed this account. It remains in the system because you have transactions which still refer to it, but it is not shown in the views. You can make it visible again by going to the View menu and selecting <b>Show all accounts</b> or by deselecting the <b>Do not show closed accounts</b> setting.</qt>"), i18n("Information"), "CloseAccountInfo");
  } catch (const MyMoneyException &) {
  }
}

void KAccountsView::slotReopenAccount()
{
  Q_D(KAccountsView);
  const auto file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;
  try {
    auto& acc = d->m_currentAccount;
    while (acc.isClosed()) {
      acc.setClosed(false);
      file->modifyAccount(acc);
      acc = file->account(acc.parentAccountId());
    }
    emit objectSelected(d->m_currentAccount);
    ft.commit();
  } catch (const MyMoneyException &) {
  }
}

void KAccountsView::slotChartAccountBalance()
{
  Q_D(KAccountsView);
  if (!d->m_currentAccount.id().isEmpty()) {
    QPointer<KBalanceChartDlg> dlg = new KBalanceChartDlg(d->m_currentAccount, this);
    dlg->exec();
    delete dlg;
  }
}

void KAccountsView::slotNewCategory()
{
  Q_D(KAccountsView);
  KNewAccountDlg::newCategory(d->m_currentAccount, MyMoneyAccount());
}

void KAccountsView::slotNewPayee(const QString& nameBase, QString& id)
{
  KMyMoneyUtils::newPayee(nameBase, id);
}
