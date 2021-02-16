/*
    SPDX-FileCopyrightText: 2006 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2006 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kbudgetview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"

using namespace Icons;

KBudgetView::KBudgetView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KBudgetViewPrivate(this), parent)
{
  Q_D(KBudgetView);
  d->m_inSelection = false;
}

KBudgetView::KBudgetView(KBudgetViewPrivate &dd, QWidget *parent)
    : KMyMoneyAccountsViewBase(dd, parent)
{
}

KBudgetView::~KBudgetView()
{
}

void KBudgetView::showEvent(QShowEvent * event)
{
  Q_D(KBudgetView);
  if (!d->m_proxyModel)
    d->init();

  emit customActionRequested(View::Budget, eView::Action::AboutToShow);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KBudgetView::executeCustomAction(eView::Action action)
{
  Q_D(KBudgetView);
  switch(action) {
    case eView::Action::Refresh:
      refresh();
      break;

    case eView::Action::SetDefaultFocus:
      QTimer::singleShot(0, d->ui->m_budgetList, SLOT(setFocus()));
      break;

    default:
      break;
  }
}

void KBudgetView::refresh()
{
  Q_D(KBudgetView);
  if (isVisible()) {
    if (d->m_inSelection)
      QTimer::singleShot(0, this, SLOT(refresh()));
    else {
      d->loadBudgets();
      d->m_needsRefresh = false;
    }
  } else {
    d->m_needsRefresh = true;
  }
}

void KBudgetView::slotNewBudget()
{
  Q_D(KBudgetView);
  d->askSave();
  auto date = QDate::currentDate();
  date.setDate(date.year(), KMyMoneySettings::firstFiscalMonth(), KMyMoneySettings::firstFiscalDay());
  auto newname = i18n("Budget %1", date.year());

  MyMoneyBudget budget;

  // make sure we have a unique name
  try {
    int i = 1;
    // Exception thrown when the name is not found
    while (1) {
      MyMoneyFile::instance()->budgetByName(newname);
      newname = i18n("Budget %1 %2", date.year(), i++);
    }
  } catch (const MyMoneyException &) {
    // all ok, the name is unique
  }

  MyMoneyFileTransaction ft;
  try {
    budget.setName(newname);
    budget.setBudgetStart(date);

    MyMoneyFile::instance()->addBudget(budget);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to add budget"), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotDeleteBudget()
{
  Q_D(KBudgetView);
  if (d->m_budgetList.isEmpty())
    return; // shouldn't happen

  auto file = MyMoneyFile::instance();

  // get confirmation from user
  QString prompt;
  if (d->m_budgetList.size() == 1)
    prompt = i18n("<p>Do you really want to remove the budget <b>%1</b>?</p>", d->m_budgetList.front().name());
  else
    prompt = i18n("Do you really want to remove all selected budgets?");

  if (KMessageBox::questionYesNo(this, prompt, i18n("Remove Budget")) == KMessageBox::No)
    return;

  try {
    MyMoneyFileTransaction ft;
    // now loop over all selected d->m_budgetList and remove them
    for (const auto& budget : d->m_budgetList)
      file->removeBudget(budget);
    ft.commit();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to remove budget."), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotCopyBudget()
{
  Q_D(KBudgetView);
  if (d->m_budgetList.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = d->m_budgetList.first();
      budget.clearId();
      budget.setName(i18n("Copy of %1", budget.name()));

      MyMoneyFile::instance()->addBudget(budget);
      ft.commit();
    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to add budget"), QString::fromLatin1(e.what()));
    }
  }
}

void KBudgetView::slotChangeBudgetYear()
{
  Q_D(KBudgetView);
  if (d->m_budgetList.size() == 1) {
    QStringList years;
    int current = 0;
    bool haveCurrent = false;
    MyMoneyBudget budget = *(d->m_budgetList.begin());
    for (int i = (QDate::currentDate().year() - 3); i < (QDate::currentDate().year() + 5); ++i) {
      years << QString::fromLatin1("%1").arg(i);
      if (i == budget.budgetStart().year()) {
        haveCurrent = true;
      }
      if (!haveCurrent)
        ++current;
    }
    if (!haveCurrent)
      current = 0;
    bool ok = false;

    auto yearString = QInputDialog::getItem(this, i18n("Select year"), i18n("Budget year"), years, current, false, &ok);

    if (ok) {
      int year = yearString.toInt(0, 0);
      QDate newYear = QDate(year, budget.budgetStart().month(), budget.budgetStart().day());
      if (newYear != budget.budgetStart()) {
        MyMoneyFileTransaction ft;
        try {
          budget.setBudgetStart(newYear);
          MyMoneyFile::instance()->modifyBudget(budget);
          ft.commit();
        } catch (const MyMoneyException &e) {
          KMessageBox::detailedSorry(this, i18n("Unable to modify budget."), QString::fromLatin1(e.what()));
        }
      }
    }
  }
}

void KBudgetView::slotBudgetForecast()
{
  Q_D(KBudgetView);
  if (d->m_budgetList.size() == 1) {
    MyMoneyFileTransaction ft;
    try {
      MyMoneyBudget budget = d->m_budgetList.first();
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
    d->m_budget = MyMoneyFile::instance()->budget(d->m_budget.id());
    d->loadAccounts();
    const auto index = d->ui->m_accountTree->currentIndex();
    if (index.isValid()) {
      const auto acc = d->ui->m_accountTree->model()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
      slotSelectAccount(acc, eView::Intent::None);
    } else {
      d->ui->m_budgetValue->clear();
    }

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
    d->refreshHideUnusedButton();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(this, i18n("Unable to modify budget"), QString::fromLatin1(e.what()));
  }
}

void KBudgetView::slotStartRename()
{
  Q_D(KBudgetView);
  QTreeWidgetItemIterator it_l(d->ui->m_budgetList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    d->ui->m_budgetList->editItem(it_v, 0);
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
    {&KBudgetView::slotDeleteBudget,     i18n("Delete budget"),            Icon::BudgetDelete, actionStates[eMenu::Action::DeleteBudget]},
    {&KBudgetView::slotCopyBudget,       i18n("Copy budget"),              Icon::BudgetCopy,   actionStates[eMenu::Action::CopyBudget]},
    {&KBudgetView::slotChangeBudgetYear, i18n("Change budget year"),       Icon::Calendar,     actionStates[eMenu::Action::ChangeBudgetYear]},
    {&KBudgetView::slotBudgetForecast,   i18n("Budget based on forecast"), Icon::Forecast,     actionStates[eMenu::Action::BudgetForecast]}
  };
  auto menu = new QMenu(i18nc("Menu header", "Budget options"));
  for (const auto& info : actionInfos) {
    auto a = menu->addAction(Icons::get(info.icon), info.text, this, info.callback);
    a->setEnabled(info.enabled);
  }
  menu->exec(QCursor::pos());
}

void KBudgetView::slotItemChanged(QTreeWidgetItem* p, int col)
{
  // if we don't have an item we actually don't care about it
  if (!p)
    return;

  auto pBudget = dynamic_cast<KBudgetListItem*>(p);
  if (!pBudget)
    return;
  if (col == 1) {
    pBudget->setText(1, QString().setNum(pBudget->budget().budgetStart().year()));
    return;
  }

  // create a copy of the new name without leading and trailing whitespaces
  QString new_name = p->text(0).trimmed();

  if (pBudget->budget().name() != new_name) {
    MyMoneyFileTransaction ft;
    try {
      // check if we already have a budget with the new name
      try {
        // this function call will throw an exception, if the budget
        // hasn't been found.
        MyMoneyFile::instance()->budgetByName(new_name);
        // the name already exists, ask the user whether he's sure to keep the name
        if (KMessageBox::questionYesNo(this,
                                       i18n("A budget with the name '%1' already exists. It is not advisable to have "
                                            "multiple budgets with the same identification name. Are you sure you would like "
                                            "to rename the budget?", new_name)) != KMessageBox::Yes) {
          p->setText(0, pBudget->budget().name());
          return;
        }
      } catch (const MyMoneyException &) {
        // all ok, the name is unique
      }

      MyMoneyBudget b = pBudget->budget();
      b.setName(new_name);
      // don't use pBudget beyond this point as it will change due to call to modifyBudget
      pBudget = 0;

      MyMoneyFile::instance()->modifyBudget(b);

      // the above call to modifyBudget will reload the view so
      // all references and pointers to the view have to be
      // re-established. You cannot use pBudget beyond this point!!!
      ft.commit();

    } catch (const MyMoneyException &e) {
      KMessageBox::detailedSorry(this, i18n("Unable to modify budget"), QString::fromLatin1(e.what()));
    }
  } else {
    pBudget->setText(0, new_name);
  }
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
  if (indexes.empty())
    return;
  QString accountID = indexes.front().data((int)eAccountsModel::Role::ID).toString();

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
    QString accountID = indexes.front().data((int)eAccountsModel::Role::ID).toString();
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

    d->loadAccounts();
  }
}

void KBudgetView::slotBudgetBalanceChanged(const MyMoneyMoney &balance)
{
  Q_D(KBudgetView);
  d->netBalProChanged(balance, d->ui->m_balanceLabel, View::Budget);
}

void KBudgetView::slotSelectBudget()
{
  Q_D(KBudgetView);
  d->askSave();
  KBudgetListItem* item;

  QTreeWidgetItemIterator widgetIt = QTreeWidgetItemIterator(d->ui->m_budgetList);
  if (d->m_budget.id().isEmpty()) {
    item = dynamic_cast<KBudgetListItem*>(*widgetIt);
    if (item) {
      d->ui->m_budgetList->blockSignals(true);
      d->ui->m_budgetList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
      d->ui->m_budgetList->blockSignals(false);
    }
  }

  d->ui->m_accountTree->setEnabled(false);
  d->ui->m_assignmentBox->setEnabled(false);
  d->m_budget = MyMoneyBudget();

  QTreeWidgetItemIterator it_l(d->ui->m_budgetList, QTreeWidgetItemIterator::Selected);
  item = dynamic_cast<KBudgetListItem*>(*it_l);
  if (item) {
    d->m_budget = item->budget();
    d->ui->m_accountTree->setEnabled(true);
  }

  d->refreshHideUnusedButton();
  d->loadAccounts();
  const auto index = d->ui->m_accountTree->currentIndex();
  if (index.isValid()) {
    const auto acc = d->ui->m_accountTree->model()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
    slotSelectAccount(acc, eView::Intent::None);
  } else {
    d->ui->m_budgetValue->clear();
  }

  d->m_budgetList.clear();
  if (!d->m_budget.id().isEmpty())
    d->m_budgetList << d->m_budget;
  d->actionStates();
  d->updateButtonStates();
}

void KBudgetView::slotHideUnused(bool toggled)
{
  Q_D(KBudgetView);
  // make sure we show all items for an empty budget
  const auto prevState = !toggled;
  d->refreshHideUnusedButton();
  if (prevState != d->ui->m_hideUnusedButton->isChecked())
    d->m_budgetProxyModel->setHideUnusedIncomeExpenseAccounts(d->ui->m_hideUnusedButton->isChecked());
}
