/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcategoriesview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QBitArray>
#include <QMenu>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneyexception.h"
#include "kmymoneysettings.h"
#include "knewaccountdlg.h"
#include "kcategoryreassigndlg.h"
#include "mymoneyschedule.h"
#include "mymoneybudget.h"
#include "mymoneytransaction.h"
#include "mymoneytransactionfilter.h"
#include "mymoneyenums.h"
#include "storageenums.h"
#include "menuenums.h"
#include "mymoneymoney.h"
#include "accountdelegate.h"

using namespace Icons;

KCategoriesView::KCategoriesView(QWidget *parent) :
    KMyMoneyViewBase(*new KCategoriesViewPrivate(this), parent)
{
  Q_D(KCategoriesView);
  d->init();

  connect(pActions[eMenu::Action::NewCategory],    &QAction::triggered, this, &KCategoriesView::slotNewCategory);
  connect(pActions[eMenu::Action::EditCategory],   &QAction::triggered, this, &KCategoriesView::slotEditCategory);
  connect(pActions[eMenu::Action::DeleteCategory], &QAction::triggered, this, &KCategoriesView::slotDeleteCategory);

  d->ui->m_accountTree->setItemDelegate(new AccountDelegate(d->ui->m_accountTree));
  connect(MyMoneyFile::instance()->accountsModel(), &AccountsModel::profitLossChanged, this, &KCategoriesView::slotProfitLossChanged);

}

KCategoriesView::~KCategoriesView()
{
}

void KCategoriesView::slotSettingsChanged()
{
  Q_D(KCategoriesView);
  d->m_proxyModel->setHideClosedAccounts(!KMyMoneySettings::showAllAccounts());
  d->m_proxyModel->setHideEquityAccounts(true);
  d->m_proxyModel->setHideFavoriteAccounts(true);

  MyMoneyFile::instance()->accountsModel()->setColorScheme(AccountsModel::Positive, KMyMoneySettings::schemeColor(SchemeColor::Positive));
  MyMoneyFile::instance()->accountsModel()->setColorScheme(AccountsModel::Negative, KMyMoneySettings::schemeColor(SchemeColor::Negative));
}


void KCategoriesView::executeCustomAction(eView::Action action)
{
  Q_D(KCategoriesView);

  switch(action) {
    case eView::Action::SetDefaultFocus:
      QMetaObject::invokeMethod(d->ui->m_accountTree, "setFocus", Qt::QueuedConnection);
      break;

    default:
      break;
  }
}

void KCategoriesView::updateActions(const SelectedObjects& selections)
{
  /// @todo updateActions
  Q_D(KCategoriesView);
  const auto file = MyMoneyFile::instance();

  // check if there is anything todo and quit if not
  if (selections.selection(SelectedObjects::Account).count() < 1
    && d->m_currentCategory.id().isEmpty() ) {
    return;
  }

  pActions[eMenu::Action::NewCategory]->setEnabled(true);
  pActions[eMenu::Action::EditCategory]->setEnabled(false);
  pActions[eMenu::Action::DeleteCategory]->setEnabled(false);

  const auto accountIds = selections.selection(SelectedObjects::Account);
  if (accountIds.isEmpty()) {
    d->m_currentCategory = MyMoneyAccount();
    return;
  }
  const auto acc = file->accountsModel()->itemById(accountIds.at(0));
  auto b = file->isStandardAccount(acc.id()) ? false : true;

  QBitArray skip((int)eStorage::Reference::Count);
  switch (acc.accountType()) {
    case eMyMoney::Account::Type::Income:
    case eMyMoney::Account::Type::Expense:
      d->m_currentCategory = acc;
      pActions[eMenu::Action::EditCategory]->setEnabled(b);

      // enable delete action, if category/account itself is not referenced
      // by any object except accounts, because we want to allow
      // deleting of sub-categories. Also, we allow transactions, schedules and budgets
      // to be present because we can re-assign them during the delete process
      skip.fill(false);
      skip.setBit((int)eStorage::Reference::Transaction);
      skip.setBit((int)eStorage::Reference::Account);
      skip.setBit((int)eStorage::Reference::Schedule);
      skip.setBit((int)eStorage::Reference::Budget);
      pActions[eMenu::Action::DeleteCategory]->setEnabled(b && !file->isReferenced(acc, skip));
      break;

    default:
      d->m_currentCategory = MyMoneyAccount();
      break;
  }
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KCategoriesView::slotUnusedIncomeExpenseAccountHidden()
{
  Q_D(KCategoriesView);
  d->m_haveUnusedCategories = true;
  d->ui->m_hiddenCategories->setVisible(d->m_haveUnusedCategories);
}

void KCategoriesView::slotProfitLossChanged(const MyMoneyMoney &profit, bool isApproximate)
{
  Q_D(KCategoriesView);
  const auto formattedValue = profit.isNegative() ? d->formatViewLabelValue(-profit, KMyMoneySettings::schemeColor(SchemeColor::Negative))
                                                  : d->formatViewLabelValue(profit, KMyMoneySettings::schemeColor(SchemeColor::Positive));
  if (profit.isNegative())
    d->updateViewLabel(d->ui->m_totalProfitsLabel,
                           isApproximate ? i18nc("Approximate loss", "Loss: ~%1", formattedValue)
                                         : i18n("Loss: %1", formattedValue));
  else
    d->updateViewLabel(d->ui->m_totalProfitsLabel,
                           isApproximate ? i18nc("Approximate profit", "Profit: ~%1", formattedValue)
                                         : i18n("Profit: %1", formattedValue));
}

void KCategoriesView::slotNewCategory()
{
  Q_D(KCategoriesView);
  MyMoneyAccount parent;
  MyMoneyAccount account;

  // Preselect the parent account by looking at the current selected account/category
  if (!d->m_currentCategory.id().isEmpty() &&
      d->m_currentCategory.isIncomeExpense()) {
    try {
      parent = MyMoneyFile::instance()->account(d->m_currentCategory.id());
    } catch (const MyMoneyException &) {
    }
  }

  KNewAccountDlg::createCategory(account, parent);
}

void KCategoriesView::slotEditCategory()
{
  Q_D(KCategoriesView);
  if (d->m_currentCategory.id().isEmpty())
    return;

  const auto file = MyMoneyFile::instance();
  if (file->isStandardAccount(d->m_currentCategory.id()))
    return;

  switch (d->m_currentCategory.accountType()) {
    case eMyMoney::Account::Type::Income:
    case eMyMoney::Account::Type::Expense:
      break;
    default:
      return;
  }

  QPointer<KNewAccountDlg> dlg =
      new KNewAccountDlg(d->m_currentCategory, true, true, 0, i18n("Edit category '%1'", d->m_currentCategory.name()));

  dlg->setOpeningBalanceShown(false);
  dlg->setOpeningDateShown(false);

  if (dlg && dlg->exec() == QDialog::Accepted) {
    try {
      MyMoneyFileTransaction ft;

      auto account = dlg->account();
      auto parent = dlg->parentAccount();

      // we need to modify first, as reparent would override all other changes
      file->modifyAccount(account);
      if (account.parentAccountId() != parent.id())
        file->reparentAccount(account, parent);

      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::error(this, i18n("Unable to modify category '%1'. Cause: %2", d->m_currentCategory.name(), QString::fromLatin1(e.what())));
    }
  }

  delete dlg;
}

void KCategoriesView::slotDeleteCategory()
{
    Q_D(KCategoriesView);
    if (d->m_currentCategory.id().isEmpty())
      return;  // need an account ID

    const auto file = MyMoneyFile::instance();
    // can't delete standard accounts or account which still have transactions assigned
    if (file->isStandardAccount(d->m_currentCategory.id()))
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
    const auto hasReference = file->isReferenced(d->m_currentCategory, skip);

    // if we get here and still have transactions referencing the account, we
    // need to check with the user to possibly re-assign them to a different account
    auto needAskUser = true;

    MyMoneyFileTransaction ft;

    if (hasReference) {
      // show transaction reassignment dialog

      needAskUser = false;
      auto dlg = new KCategoryReassignDlg(this);
      auto categoryId = dlg->show(d->m_currentCategory);
      delete dlg; // and kill the dialog
      if (categoryId.isEmpty())
        return; // the user aborted the dialog, so let's abort as well

      auto newCategory = file->account(categoryId);
      try {
        {
          /*
            d->m_currentCategory.id() is the old id, categoryId the new one
            Now search all transactions and schedules that reference d->m_currentCategory.id()
            and replace that with categoryId.
          */
          // get the list of all transactions that reference the old account
          MyMoneyTransactionFilter filter(d->m_currentCategory.id());
          filter.setReportAllSplits(false);
          QList<MyMoneyTransaction> tlist;
          QList<MyMoneyTransaction>::iterator it_t;
          file->transactionList(tlist, filter);

          for (it_t = tlist.begin(); it_t != tlist.end(); ++it_t) {
            MyMoneyTransaction t = (*it_t);
            if (t.replaceId(categoryId, d->m_currentCategory.id()))
              file->modifyTransaction(t);
          }
        }
        // now fix all schedules
        {
          QList<MyMoneySchedule> slist = file->scheduleList(d->m_currentCategory.id());
          QList<MyMoneySchedule>::iterator it_s;

          for (it_s = slist.begin(); it_s != slist.end(); ++it_s) {
            MyMoneySchedule sch = (*it_s);
            if (sch.replaceId(categoryId, d->m_currentCategory.id())) {
              file->modifySchedule(sch);
            }
          }
        }
        // now fix all budgets
        {
          QList<MyMoneyBudget> blist = file->budgetList();
          QList<MyMoneyBudget>::const_iterator it_b;
          for (it_b = blist.constBegin(); it_b != blist.constEnd(); ++it_b) {
            if ((*it_b).hasReferenceTo(d->m_currentCategory.id())) {
              MyMoneyBudget b = (*it_b);
              MyMoneyBudget::AccountGroup fromBudget = b.account(d->m_currentCategory.id());
              MyMoneyBudget::AccountGroup toBudget = b.account(categoryId);
              toBudget += fromBudget;
              b.setAccount(toBudget, categoryId);
              b.removeReference(d->m_currentCategory.id());
              file->modifyBudget(b);
            }
          }
        }
      } catch (MyMoneyException &e) {
        KMessageBox::error(this, i18n("Unable to exchange category <b>%1</b> with category <b>%2</b>. Reason: %3", d->m_currentCategory.name(), newCategory.name(), QString::fromLatin1(e.what())));
        return;
      }
    }

    // retain the account name for a possible later usage in the error message box
    // since the account removal notifies the views the selected account can be changed
    // so we make sure by doing this that we display the correct name in the error message
    auto selectedAccountName = d->m_currentCategory.name();

    // at this point, we must not have a reference to the account
    // to be deleted anymore
    // special handling for categories to allow deleting of empty subcategories
    {
      // open a compound statement here to be able to declare variables
      // which would otherwise not work within a case label.

      // case A - only a single, unused category without subcats selected
      if (d->m_currentCategory.accountList().isEmpty()) {
        if (!needAskUser || (KMessageBox::questionYesNo(this, i18n("<qt>Do you really want to delete category <b>%1</b>?</qt>", selectedAccountName)) == KMessageBox::Yes)) {
          try {
            file->removeAccount(d->m_currentCategory);
            d->m_currentCategory.clearId();
            ft.commit();
          } catch (const MyMoneyException &e) {
            KMessageBox::error(this, i18n("<qt>Unable to delete category <b>%1</b>. Cause: %2</qt>", selectedAccountName, QString::fromLatin1(e.what())));
          }
        }
        return;
      }
      // case B - we have some subcategories, maybe the user does not want to
      //          delete them all, but just the category itself?
      auto parentAccount = file->account(d->m_currentCategory.parentAccountId());

      QStringList accountsToReparent;
      int result = KMessageBox::questionYesNoCancel(this,
                                                    i18n("<qt>Do you want to delete category <b>%1</b> with all its sub-categories or only "
                                                         "the category itself? If you only delete the category itself, all its sub-categories "
                                                         "will be made sub-categories of <b>%2</b>.</qt>", selectedAccountName, parentAccount.name()),
                                                    QString(),
                                                    KGuiItem(i18n("Delete all")),
                                                    KGuiItem(i18n("Just the category")));
      if (result == KMessageBox::Cancel)
        return; // cancel pressed? ok, no delete then...
      // "No" means "Just the category" and that means we need to reparent all subaccounts
      bool need_confirmation = false;
      // case C - User only wants to delete the category itself
      if (result == KMessageBox::No)
        accountsToReparent = d->m_currentCategory.accountList();
      else {
        // case D - User wants to delete all subcategories, now check all subcats of
        //          d->m_currentCategory and remember all that cannot be deleted and
        //          must be "reparented"
        foreach (const auto accountID, d->m_currentCategory.accountList()) {
          // reparent account if a transaction is assigned
          if (file->transactionCount(accountID) != 0)
            accountsToReparent.push_back(accountID);
          else if (!file->account(accountID).accountList().isEmpty()) {
            // or if we have at least one sub-account that is used for transactions
            if (!file->hasOnlyUnusedAccounts(file->account(accountID).accountList())) {
              accountsToReparent.push_back(accountID);
              //qDebug() << "subaccount not empty";
            }
          }
        }
        if (!accountsToReparent.isEmpty())
          need_confirmation = true;
      }
      if (!accountsToReparent.isEmpty() && need_confirmation) {
        if (KMessageBox::questionYesNo(this, i18n("<p>Some sub-categories of category <b>%1</b> cannot "
                                                  "be deleted, because they are still used. They will be made sub-categories of <b>%2</b>. Proceed?</p>", selectedAccountName, parentAccount.name())) != KMessageBox::Yes) {
          return; // user gets wet feet...
        }
      }
      // all good, now first reparent selected sub-categories
      try {
        auto parent = file->account(d->m_currentCategory.parentAccountId());
        for (QStringList::const_iterator it = accountsToReparent.constBegin(); it != accountsToReparent.constEnd(); ++it) {
          auto child = file->account(*it);
          file->reparentAccount(child, parent);
        }
        // reload the account because the sub-account list might have changed
        d->m_currentCategory = file->account(d->m_currentCategory.id());
        // now recursively delete remaining sub-categories
        file->removeAccountList(d->m_currentCategory.accountList());
        // don't forget to update d->m_currentCategory, because we still have a copy of
        // the old account list, which is no longer valid
        d->m_currentCategory = file->account(d->m_currentCategory.id());
      } catch (const MyMoneyException &e) {
        KMessageBox::error(this, i18n("<qt>Unable to delete a sub-category of category <b>%1</b>. Reason: %2</qt>", selectedAccountName, QString::fromLatin1(e.what())));
        return;
      }
    }
    // the category/account is deleted after the switch
    try {
      file->removeAccount(d->m_currentCategory);
      d->m_currentCategory.clearId();
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::error(this, i18n("Unable to delete category '%1'. Cause: %2", selectedAccountName, QString::fromLatin1(e.what())));
    }
}
