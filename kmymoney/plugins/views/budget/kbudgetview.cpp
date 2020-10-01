/***************************************************************************
                          kbudgetview.cpp
                          ---------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
                           (C) 2019 Thomas Baumgart <tbaumgart@kde.org>
                           (C) 2020 Robert Szczesiak <dev.rszczesiak@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kbudgetview_p.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

using namespace Icons;

KBudgetView::KBudgetView(QWidget *parent) :
    KMyMoneyViewBase(*new KBudgetViewPrivate(this), parent)
{
}

KBudgetView::KBudgetView(KBudgetViewPrivate &dd, QWidget *parent)
    : KMyMoneyViewBase(dd, parent)
{
}

KBudgetView::~KBudgetView()
{
}

void KBudgetView::slotSettingsChanged()
{
  Q_D(KBudgetView);
  if (d->m_budgetProxyModel) {
    d->m_budgetProxyModel->setColorScheme(AccountsModel::Positive, KMyMoneySettings::schemeColor(SchemeColor::Positive));
    d->m_budgetProxyModel->setColorScheme(AccountsModel::Negative, KMyMoneySettings::schemeColor(SchemeColor::Negative));
  }

}

void KBudgetView::showEvent(QShowEvent * event)
{
  Q_D(KBudgetView);
  if (!d->m_budgetProxyModel) {
    d->init();
    slotSelectBudget();
  }
  emit customActionRequested(View::Budget, eView::Action::AboutToShow);

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KBudgetView::executeCustomAction(eView::Action action)
{
  switch(action) {
    case eView::Action::SetDefaultFocus:
      {
        Q_D(KBudgetView);
        QMetaObject::invokeMethod(d->ui->m_budgetList, "setFocus", Qt::QueuedConnection);
      }
      break;

    default:
      break;
  }
}

void KBudgetView::slotNewBudget()
{
  Q_D(KBudgetView);
  auto date = QDate::currentDate();
  date.setDate(date.year(), KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());
  auto newname = i18nc("@item:intable Your budgets, %1 budget year", "Budget %1", QString::number(date.year()));

  MyMoneyBudget budget;

  // make sure we have a unique name
  try {
    // Exception thrown when the name is not found
    int i = 1;
    while (!MyMoneyFile::instance()->budgetByName(newname).id().isEmpty())
      newname = i18nc("@item:intable Your bugets, %1 budget year, %2 unique index", "Budget %1 (%2)", QString::number(date.year()), i++);
  } catch (const MyMoneyException &) {
    // all ok, the name is unique
  }

  MyMoneyFileTransaction ft;
  try {
    budget.setName(newname);
    budget.setBudgetStart(date);

    MyMoneyFile::instance()->addBudget(budget);
    ft.commit();
    // select the newly created budget
    d->ui->m_budgetList->setCurrentIndex(MyMoneyFile::instance()->budgetsModel()->indexById(budget.id()));
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to add budget"), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotDeleteBudget()
{
  Q_D(KBudgetView);
  const QModelIndexList indexes = d->ui->m_budgetList->selectionModel()->selectedIndexes();
  if (indexes.isEmpty())
    return; // shouldn't happen

  auto file = MyMoneyFile::instance();

  // get confirmation from user
  QString prompt;
  if (indexes.count() == 2) {
    const auto budgetName = indexes.first().data(eMyMoney::Model::BudgetNameRole).toString();
    prompt = i18n("<p>Do you really want to remove the budget <b>%1</b>?</p>", budgetName);
  } else
    prompt = i18n("Do you really want to remove all selected budgets?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Budget")) == KMessageBox::No)
    return;

  try {
    MyMoneyFileTransaction ft;
    // now loop over all selected d->m_budgetList and remove them
    // we first collect all budget ids because the indexes will
    // become invalid during the removal
    QStringList budgetIds;
    for (const auto& idx: indexes) {
      budgetIds << idx.data(eMyMoney::Model::IdRole).toString();
    }
    for (const auto& id : qAsConst(budgetIds)) {
      auto budget = file->budget(id);
      file->removeBudget(budget);
    }
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to remove budget."), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotCopyBudget()
{
  Q_D(KBudgetView);
  const QModelIndexList indexes = d->ui->m_budgetList->selectionModel()->selectedIndexes();
  if (indexes.count() == 2) {
    const auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {

      MyMoneyBudget budget = file->budget(indexes.first().data(eMyMoney::Model::IdRole).toString());;
      budget.clearId();
      budget.setName(i18n("Copy of %1", budget.name()));

      file->addBudget(budget);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to add budget"), QString::fromLatin1(e.what()));
    }
  }
}

void KBudgetView::slotBudgetForecast()
{
  Q_D(KBudgetView);
  if (d->ui->m_budgetList->selectionModel()->selectedIndexes().count() == 2) {
    auto file = MyMoneyFile::instance();
    MyMoneyFileTransaction ft;
    try {
      auto idx = d->ui->m_budgetList->selectionModel()->selectedIndexes().first();
      idx = MyMoneyFile::baseModel()->mapToBaseSource(idx);
      if (idx.isValid()) {
        auto budget = file->budgetsModel()->itemByIndex(idx);
        bool calcBudget = budget.getaccounts().count() == 0;
        if (!calcBudget) {
          if (KMessageBox::warningContinueCancel(0, i18n("The current budget already contains data. Continuing will replace all current values of this budget."), i18nc("Warning message box", "Warning")) == KMessageBox::Continue)
            calcBudget = true;
        }

        if (calcBudget) {
          QDate historyStart;
          QDate historyEnd;
          QDate budgetStart;
          QDate budgetEnd;

          budgetStart = budget.budgetStart();
          budgetEnd = budgetStart.addYears(1).addDays(-1);
          historyStart = budgetStart.addYears(-1);
          historyEnd = budgetEnd.addYears(-1);

          MyMoneyForecast forecast = KMyMoneyUtils::forecast();
          forecast.createBudget(budget, historyStart, historyEnd, budgetStart, budgetEnd, true);

          MyMoneyFile::instance()->modifyBudget(budget);
          ft.commit();
          // load updated data
          slotSelectBudget();
        }
      }
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify budget."), QString::fromLatin1(e.what()));
    }
  }
}

void KBudgetView::slotResetBudget()
{
  Q_D(KBudgetView);
  try {
    d->ui->m_budgetValue->clear();
    d->m_budget = MyMoneyFile::instance()->budget(d->m_budget.id());
    d->ui->m_updateButton->setEnabled(false);
    slotSelectBudget();
    d->loadBudgetAccountsView();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to reset budget"), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotUpdateBudget()
{
  Q_D(KBudgetView);
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->modifyBudget(d->m_budget);
    ft.commit();
    d->ui->m_updateButton->setEnabled(false);
    // load updated data
    slotSelectBudget();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to modify budget"), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotStartRename()
{
  Q_D(KBudgetView);
  if (d->ui->m_budgetList->selectionModel()->selectedIndexes().count() == 2) {
    auto row = d->ui->m_budgetList->selectionModel()->selectedIndexes().first().row();
    auto idx = d->ui->m_budgetList->model()->index(row, 0);
    d->ui->m_budgetList->setCurrentIndex(idx);
    d->ui->m_budgetList->edit(idx);
  }
}

void KBudgetView::slotOpenContextMenu(const QPoint&)
{
  Q_D(KBudgetView);

  typedef void(KBudgetView::*KBudgetViewFunc)();
  struct actionInfo {
    KBudgetViewFunc callback;
    QString         text;
    Icon            icon;
    bool            enabled;
  };
  const auto actionStates = d->actionStates();

  const QVector<actionInfo> actionInfos {
    {&KBudgetView::slotNewBudget,        i18n("New budget"),               Icon::BudgetNew,    actionStates[eMenu::Action::NewBudget]},
    {&KBudgetView::slotStartRename,      i18n("Rename budget"),            Icon::BudgetRename, actionStates[eMenu::Action::RenameBudget]},
    {&KBudgetView::slotDeleteBudget,     i18n("Delete budget"),            Icon::BudgetRemove, actionStates[eMenu::Action::DeleteBudget]},
    {&KBudgetView::slotCopyBudget,       i18n("Copy budget"),              Icon::BudgetCopy,   actionStates[eMenu::Action::CopyBudget]},
    {&KBudgetView::slotBudgetForecast,   i18n("Budget based on forecast"), Icon::Forecast,     actionStates[eMenu::Action::BudgetForecast]}
  };
  auto menu = new QMenu(i18nc("Menu header", "Budget options"));
  for (const auto& info : actionInfos) {
    auto a = menu->addAction(Icons::get(info.icon), info.text, this, info.callback);
    a->setEnabled(info.enabled);
  }
  menu->exec(QCursor::pos());
}

void KBudgetView::slotSelectAccount(const MyMoneyObject &obj, eView::Intent intent)
{
  Q_UNUSED(intent)
  Q_D(KBudgetView);
  d->ui->m_assignmentBox->setEnabled(false);
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  d->ui->m_assignmentBox->setEnabled(true);

  if (d->m_budget.id().isEmpty())
    return;

  QString id = acc.id();
  d->ui->m_leAccounts->setText(MyMoneyFile::instance()->accountToCategory(id));
  d->ui->m_cbBudgetSubaccounts->setChecked(d->m_budget.account(id).budgetSubaccounts());
  d->ui->m_accountTotal->setValue(d->m_budget.account(id).totalBalance());
  MyMoneyBudget::AccountGroup budgetAccount = d->m_budget.account(id);
  if (id != budgetAccount.id()) {
    budgetAccount.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
  }
  d->ui->m_budgetValue->setBudgetValues(d->m_budget, budgetAccount);
}

void KBudgetView::slotBudgetedAmountChanged()
{
  Q_D(KBudgetView);
  if (d->m_budget.id().isEmpty())
    return;

  const auto indexes = d->ui->m_accountTree->selectionModel()->selectedIndexes();
  if (indexes.isEmpty())
    return;
  QString accountID = indexes.front().data(eMyMoney::Model::Roles::IdRole).toString();

  MyMoneyBudget::AccountGroup accountGroup = d->m_budget.account(accountID);
  accountGroup.setId(accountID);
  d->ui->m_budgetValue->budgetValues(d->m_budget, accountGroup);
  d->m_budget.setAccount(accountGroup, accountID);

  d->m_budgetProxyModel->setBudget(d->m_budget);
  d->ui->m_accountTotal->setValue(accountGroup.totalBalance());

  d->ui->m_updateButton->setEnabled(!(d->selectedBudget() == d->m_budget));
  d->ui->m_resetButton->setEnabled(!(d->selectedBudget() == d->m_budget));
}

void KBudgetView::cb_includesSubaccounts_clicked()
{
  Q_D(KBudgetView);
  if (d->m_budget.id().isEmpty())
    return;

  QModelIndexList indexes = d->ui->m_accountTree->selectionModel()->selectedIndexes();
  if (!indexes.empty()) {
    QString accountID = indexes.front().data(eMyMoney::Model::Roles::IdRole).toString();
    // now, we get a reference to the accountgroup, to modify its attribute,
    // and then put the resulting account group instead of the original
    MyMoneyBudget::AccountGroup auxAccount = d->m_budget.account(accountID);
    auxAccount.setBudgetSubaccounts(d->ui->m_cbBudgetSubaccounts->isChecked());

    // in case we turn the option on, we check that no subordinate account
    // has a budget. If we find some, we ask the user if he wants to move it
    // to the current account or leave things as they are
    if (d->ui->m_cbBudgetSubaccounts->isChecked()) {
      // TODO: asking the user needs to be added. So long, we assume yes
      if (1) {
        MyMoneyBudget::AccountGroup subAccount;
        if (d->collectSubBudgets(subAccount, indexes.front())) {
          // we found a sub-budget somewhere
          // so we add those figures found and
          // clear the subaccounts
          auxAccount += subAccount;
          d->clearSubBudgets(indexes.front());
        }

        if (auxAccount.budgetLevel() == eMyMoney::Budget::Level::None) {
          MyMoneyBudget::PeriodGroup period;
          auxAccount.addPeriod(d->m_budget.budgetStart(), period);
          auxAccount.setBudgetLevel(eMyMoney::Budget::Level::Monthly);
        }
      }
    }

    d->m_budget.setAccount(auxAccount, accountID);
    d->m_budgetProxyModel->setBudget(d->m_budget);
    d->ui->m_budgetValue->setBudgetValues(d->m_budget, auxAccount);

    d->loadBudgetAccountsView();
  }
}

void KBudgetView::slotBudgetBalanceChanged(const MyMoneyMoney &balance)
{
  Q_D(KBudgetView);
  const auto formattedValue = balance.isNegative() ? d->formatViewLabelValue(-balance, KMyMoneySettings::schemeColor(SchemeColor::Negative))
                                                   : d->formatViewLabelValue(balance, KMyMoneySettings::schemeColor(SchemeColor::Positive));
  d->updateViewLabel(d->ui->m_balanceLabel,
                         balance.isNegative() ? i18nc("Profit/Loss", "Loss: %1", formattedValue)
                                              : i18nc("Profit/Loss", "Profit: %1", formattedValue));
}

void KBudgetView::slotSelectBudget()
{
  Q_D(KBudgetView);
  d->askSave();

  d->ui->m_accountTree->setEnabled(false);
  d->ui->m_assignmentBox->setEnabled(false);

  d->m_budget = d->selectedBudget();
  d->ui->m_accountTree->setDisabled(d->m_budget.id().isEmpty());

  if (!d->m_budget.id().isEmpty()) {
    d->loadBudgetAccountsView();
    const auto idx = d->ui->m_accountTree->currentIndex();
    if (idx.isValid()) {
      const auto baseIdx = MyMoneyFile::baseModel()->mapToBaseSource(idx);
      const auto account = MyMoneyFile::instance()->accountsModel()->itemByIndex(baseIdx);
      slotSelectAccount(account, eView::Intent::None);
    } else {
      d->ui->m_budgetValue->clear();
    }
  }

  d->actionStates();
  d->updateButtonStates();
}

void KBudgetView::slotHideUnused(bool toggled)
{
  Q_D(KBudgetView);
  // make sure we show all items for an empty budget
  const auto prevState = !toggled;
  if (prevState != d->ui->m_hideUnusedButton->isChecked())
    d->m_budgetProxyModel->setHideUnusedIncomeExpenseAccounts(d->ui->m_hideUnusedButton->isChecked());
}
