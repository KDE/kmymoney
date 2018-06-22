/***************************************************************************
                          kaccountsview.cpp
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017, 2018 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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

#include "onlinejobadministration.h"
#include "knewaccountwizard.h"
#include "kmymoneyutils.h"
#include "kmymoneysettings.h"
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
  connect(pActions[eMenu::Action::MapOnlineAccount],    &QAction::triggered, this, &KAccountsView::slotAccountMapOnline);
  connect(pActions[eMenu::Action::UnmapOnlineAccount],  &QAction::triggered, this, &KAccountsView::slotAccountUnmapOnline);
  connect(pActions[eMenu::Action::UpdateAccount],       &QAction::triggered, this, &KAccountsView::slotAccountUpdateOnline);
  connect(pActions[eMenu::Action::UpdateAllAccounts],   &QAction::triggered, this, &KAccountsView::slotAccountUpdateOnlineAll);
}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::executeCustomAction(eView::Action action)
{
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      {
        Q_D(KAccountsView);
        QTimer::singleShot(0, d->ui->m_accountTree, SLOT(setFocus()));
      }
      break;

    default:
      break;
  }
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
  d->m_proxyModel->setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts());
  d->m_proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
  if (KMyMoneySettings::showCategoriesInAccountsView()) {
    d->m_proxyModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense});
  } else {
    d->m_proxyModel->removeAccountType(eMyMoney::Account::Type::Income);
    d->m_proxyModel->removeAccountType(eMyMoney::Account::Type::Expense);
  }

  // reinitialize the default state of the hidden categories label
  d->m_haveUnusedCategories = false;
  d->ui->m_hiddenCategories->hide();  // hides label
  d->m_proxyModel->setHideUnusedIncomeExpenseAccounts(KMyMoneySettings::hideUnusedCategory());
}

void KAccountsView::showEvent(QShowEvent * event)
{
  Q_D(KAccountsView);
  if (!d->m_proxyModel)
    d->init();

  emit customActionRequested(View::Accounts, eView::Action::AboutToShow);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KAccountsView::updateActions(const MyMoneyObject& obj)
{
  Q_D(KAccountsView);

  const auto file = MyMoneyFile::instance();

  if (typeid(obj) != typeid(MyMoneyAccount) &&
      (obj.id().isEmpty() && d->m_currentAccount.id().isEmpty())) // do not disable actions that were already disabled)
    return;

  const auto& acc = static_cast<const MyMoneyAccount&>(obj);

  const QVector<eMenu::Action> actionsToBeDisabled {
        eMenu::Action::NewAccount, eMenu::Action::EditAccount, eMenu::Action::DeleteAccount,
        eMenu::Action::CloseAccount, eMenu::Action::ReopenAccount,
        eMenu::Action::ChartAccountBalance,
        eMenu::Action::UnmapOnlineAccount, eMenu::Action::MapOnlineAccount,
        eMenu::Action::UpdateAccount
  };

  for (const auto& a : actionsToBeDisabled)
    pActions[a]->setEnabled(false);

  pActions[eMenu::Action::NewAccount]->setEnabled(true);

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

void KAccountsView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
  switch(intent) {
    case eView::Intent::UpdateActions:
      updateActions(obj);
      break;

    case eView::Intent::OpenContextMenu:
      slotShowAccountMenu(static_cast<const MyMoneyAccount&>(obj));
      break;

    default:
      break;
  }
}

void KAccountsView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
  Q_D(KAccountsView);
  switch (intent) {
    case eView::Intent::UpdateNetWorth:
      if (variant.count() == 1 && d->m_proxyModel)
        slotNetWorthChanged(variant.first().value<MyMoneyMoney>());
      break;

    case eView::Intent::SetOnlinePlugins:
      if (variant.count() == 1)
        d->m_onlinePlugins = static_cast<QMap<QString, KMyMoneyPlugin::OnlinePlugin*>*>(variant.first().value<void*>());
      break;

    default:
      break;
  }
}

void KAccountsView::slotNewAccount()
{
  MyMoneyAccount account;
  account.setOpeningDate(KMyMoneySettings::firstFiscalDate());
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
  emit selectByObject(d->m_currentAccount, eView::Intent::None);
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
    emit selectByObject(MyMoneyAccount(), eView::Intent::None);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::error(this, i18n("Unable to delete account '%1'. Cause: %2", selectedAccountName, QString::fromLatin1(e.what())));
  }
}

void KAccountsView::slotCloseAccount()
{
  Q_D(KAccountsView);
  MyMoneyFileTransaction ft;
  try {
    d->m_currentAccount.setClosed(true);
    MyMoneyFile::instance()->modifyAccount(d->m_currentAccount);
    emit selectByObject(d->m_currentAccount, eView::Intent::None);
    ft.commit();
    if (KMyMoneySettings::hideClosedAccounts())
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
    emit selectByObject(d->m_currentAccount, eView::Intent::None);
    ft.commit();
  } catch (const MyMoneyException &) {
  }
}

void KAccountsView::slotChartAccountBalance()
{
  Q_D(KAccountsView);
  if (!d->m_currentAccount.id().isEmpty()) {
    emit customActionRequested(View::Accounts, eView::Action::ShowBalanceChart);
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

void KAccountsView::slotAccountUnmapOnline()
{
  Q_D(KAccountsView);
  // no account selected
  if (d->m_currentAccount.id().isEmpty())
    return;

  // not a mapped account
  if (!d->m_currentAccount.hasOnlineMapping())
    return;

  if (KMessageBox::warningYesNo(this, QString("<qt>%1</qt>").arg(i18n("Do you really want to remove the mapping of account <b>%1</b> to an online account? Depending on the details of the online banking method used, this action cannot be reverted.", d->m_currentAccount.name())), i18n("Remove mapping to online account")) == KMessageBox::Yes) {
    MyMoneyFileTransaction ft;
    try {
      d->m_currentAccount.setOnlineBankingSettings(MyMoneyKeyValueContainer());
      // delete the kvp that is used in MyMoneyStatementReader too
      // we should really get rid of it, but since I don't know what it
      // is good for, I'll keep it around. (ipwizard)
      d->m_currentAccount.deletePair("StatementKey");
      MyMoneyFile::instance()->modifyAccount(d->m_currentAccount);
      ft.commit();
      // The mapping could disable the online task system
      onlineJobAdministration::instance()->updateOnlineTaskProperties();
    } catch (const MyMoneyException &e) {
      KMessageBox::error(this, i18n("Unable to unmap account from online account: %1", QString::fromLatin1(e.what())));
    }
  }
  updateActions(d->m_currentAccount);
}

void KAccountsView::slotAccountMapOnline()
{
  Q_D(KAccountsView);
  // no account selected
  if (d->m_currentAccount.id().isEmpty())
    return;

  // already an account mapped
  if (d->m_currentAccount.hasOnlineMapping())
    return;

  // check if user tries to map a brokerageAccount
  if (d->m_currentAccount.name().contains(i18n(" (Brokerage)"))) {
    if (KMessageBox::warningContinueCancel(this, i18n("You try to map a brokerage account to an online account. This is usually not advisable. In general, the investment account should be mapped to the online account. Please cancel if you intended to map the investment account, continue otherwise"), i18n("Mapping brokerage account")) == KMessageBox::Cancel) {
      return;
    }
  }
  if (!d->m_onlinePlugins)
    return;

  // if we have more than one provider let the user select the current provider
  QString provider;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  switch (d->m_onlinePlugins->count()) {
    case 0:
      break;
    case 1:
      provider = d->m_onlinePlugins->begin().key();
      break;
    default: {
        QMenu popup(this);
        popup.setTitle(i18n("Select online banking plugin"));

        // Populate the pick list with all the provider
        for (it_p = d->m_onlinePlugins->constBegin(); it_p != d->m_onlinePlugins->constEnd(); ++it_p) {
          popup.addAction(it_p.key())->setData(it_p.key());
        }

        QAction *item = popup.actions()[0];
        if (item) {
          popup.setActiveAction(item);
        }

        // cancelled
        if ((item = popup.exec(QCursor::pos(), item)) == 0) {
          return;
        }

        provider = item->data().toString();
      }
      break;
  }

  if (provider.isEmpty())
    return;

  // find the provider
  it_p = d->m_onlinePlugins->constFind(provider.toLower());
  if (it_p != d->m_onlinePlugins->constEnd()) {
    // plugin found, call it
    MyMoneyKeyValueContainer settings;
    if ((*it_p)->mapAccount(d->m_currentAccount, settings)) {
      settings["provider"] = provider.toLower();
      MyMoneyAccount acc(d->m_currentAccount);
      acc.setOnlineBankingSettings(settings);
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->modifyAccount(acc);
        ft.commit();
        // The mapping could enable the online task system
        onlineJobAdministration::instance()->updateOnlineTaskProperties();
      } catch (const MyMoneyException &e) {
        KMessageBox::error(this, i18n("Unable to map account to online account: %1", QString::fromLatin1(e.what())));
      }
    }
  }
  updateActions(d->m_currentAccount);
}

void KAccountsView::slotAccountUpdateOnlineAll()
{
  Q_D(KAccountsView);
  QList<MyMoneyAccount> accList;
  MyMoneyFile::instance()->accountList(accList);
  QList<MyMoneyAccount>::iterator it_a;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;

  // remove all those from the list, that don't have a 'provider' or the
  // provider is not currently present
  for (it_a = accList.begin(); it_a != accList.end();) {
    if (!(*it_a).hasOnlineMapping()
        || d->m_onlinePlugins->find((*it_a).onlineBankingSettings().value("provider").toLower()) == d->m_onlinePlugins->end()) {
      it_a = accList.erase(it_a);
    } else
      ++it_a;
  }
  const QVector<eMenu::Action> disabledActions {eMenu::Action::UpdateAccount, eMenu::Action::UpdateAllAccounts};
  for (const auto& a : disabledActions)
    pActions[a]->setEnabled(false);

  // now work on the remaining list of accounts
  int cnt = accList.count() - 1;
  for (it_a = accList.begin(); it_a != accList.end(); ++it_a) {
    it_p = d->m_onlinePlugins->constFind((*it_a).onlineBankingSettings().value("provider").toLower());
    (*it_p)->updateAccount(*it_a, cnt != 0);
    --cnt;
  }

  // re-enable the disabled actions
  updateActions(d->m_currentAccount);
}

void KAccountsView::slotAccountUpdateOnline()
{
  Q_D(KAccountsView);
  // no account selected
  if (d->m_currentAccount.id().isEmpty())
    return;

  // no online account mapped
  if (!d->m_currentAccount.hasOnlineMapping())
    return;

  const QVector<eMenu::Action> disabledActions {eMenu::Action::UpdateAccount, eMenu::Action::UpdateAllAccounts};
  for (const auto& a : disabledActions)
    pActions[a]->setEnabled(false);

  // find the provider
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p;
  it_p = d->m_onlinePlugins->constFind(d->m_currentAccount.onlineBankingSettings().value("provider").toLower());
  if (it_p != d->m_onlinePlugins->constEnd()) {
    // plugin found, call it
    (*it_p)->updateAccount(d->m_currentAccount);
  }

  // re-enable the disabled actions
  updateActions(d->m_currentAccount);
}
