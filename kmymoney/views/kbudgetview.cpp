/***************************************************************************
                          kbudgetview.cpp
                          ---------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
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

#include "kbudgetview.h"
#include "kbudgetview_p.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KConfig>
#include <KMessageBox>
#include <KSharedConfig>
#include <KActionCollection>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbudgetview.h"

#include "mymoneyfile.h"
#include "kmymoneyedit.h"
#include "kbudgetvalues.h"
#include "kmymoney.h"
#include "modelenums.h"
#include "viewenums.h"

using namespace Icons;

/**
  * @author Darren Gould
  * @author Thomas Baumgart
  *
  * This class represents an item in the budgets list view.
  */
class KBudgetListItem : public QTreeWidgetItem
{
public:
  /**
    * Constructor to be used to construct a budget entry object.
    *
    * @param parent pointer to the QTreeWidget object this entry should be
    *               added to.
    * @param budget const reference to MyMoneyBudget for which
    *               the QTreeWidget entry is constructed
    */
  KBudgetListItem(QTreeWidget *parent, const MyMoneyBudget& budget);
  ~KBudgetListItem();

  const MyMoneyBudget& budget() {
    return m_budget;
  };
  void setBudget(const MyMoneyBudget& budget) {
    m_budget = budget;
  }

private:
  MyMoneyBudget  m_budget;
};

// *** KBudgetListItem Implementation ***
KBudgetListItem::KBudgetListItem(QTreeWidget *parent, const MyMoneyBudget& budget) :
    QTreeWidgetItem(parent),
    m_budget(budget)
{
  setText(0, budget.name());
  setText(1, QString("%1").arg(budget.budgetStart().year()));
  setFlags(flags() | Qt::ItemIsEditable);
}

KBudgetListItem::~KBudgetListItem()
{
}

// *** KBudgetView Implementation ***
//TODO: This has to go to user settings
const int KBudgetView::m_iBudgetYearsAhead = 5;
const int KBudgetView::m_iBudgetYearsBack = 3;

KBudgetView::KBudgetView(QWidget *parent) :
    KMyMoneyAccountsViewBase(*new KBudgetViewPrivate(this), parent),
    m_inSelection(false)
{
}

KBudgetView::KBudgetView(KBudgetViewPrivate &dd, QWidget *parent)
    : KMyMoneyAccountsViewBase(dd, parent)
{
}

KBudgetView::~KBudgetView()
{
  Q_D(KBudgetView);
  if(d->m_proxyModel) {
    // remember the splitter settings for startup
    KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("KBudgetViewSplitterSize", d->ui->m_splitter->saveState());
    grp.sync();
  }
}

void KBudgetView::setDefaultFocus()
{
  Q_D(KBudgetView);
  QTimer::singleShot(0, d->ui->m_budgetList, SLOT(setFocus()));
}

void KBudgetView::showEvent(QShowEvent * event)
{
  Q_D(KBudgetView);
  if (!d->m_proxyModel)
    d->init();

  emit aboutToShow(View::Budget);

  if (d->m_needsRefresh)
    refresh();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KBudgetView::loadBudgets()
{
  Q_D(KBudgetView);
  d->m_budgetProxyModel->invalidate();

  // remember which item is currently selected
  QString id = m_budget.id();

  // clear the budget list
  d->ui->m_budgetList->clear();

  // add the correct years to the drop down list
  QDate date = QDate::currentDate();
  int iStartYear = date.year() - m_iBudgetYearsBack;

  m_yearList.clear();
  for (int i = 0; i < m_iBudgetYearsAhead + m_iBudgetYearsBack; i++)
    m_yearList += QString::number(iStartYear + i);

  KBudgetListItem* currentItem = 0;

  QList<MyMoneyBudget> list = MyMoneyFile::instance()->budgetList();
  QList<MyMoneyBudget>::ConstIterator it;
  for (it = list.constBegin(); it != list.constEnd(); ++it) {
    KBudgetListItem* item = new KBudgetListItem(d->ui->m_budgetList, *it);

    // create a list of unique years
    if (m_yearList.indexOf(QString::number((*it).budgetStart().year())) == -1)
      m_yearList += QString::number((*it).budgetStart().year());

    //sort the list by name
    d->ui->m_budgetList->sortItems((int)eAccountsModel::Column::Account, Qt::AscendingOrder);

    if (item->budget().id() == id) {
      m_budget = (*it);
      currentItem = item;
      item->setSelected(true);
    }
  }
  m_yearList.sort();

  if (currentItem) {
    d->ui->m_budgetList->setCurrentItem(currentItem);
  }

  // reset the status of the buttons
  d->ui->m_updateButton->setEnabled(false);
  d->ui->m_resetButton->setEnabled(false);

  // make sure the world around us knows what we have selected
  slotSelectBudget();
}

void KBudgetView::ensureBudgetVisible(const QString& id)
{
  Q_D(KBudgetView);
  const auto widgetIt = QTreeWidgetItemIterator(d->ui->m_budgetList);
  while (*widgetIt) {
    const auto p = dynamic_cast<KBudgetListItem*>(*widgetIt);
    if ((p)->budget().id() == id) {
      d->ui->m_budgetList->scrollToItem((p), QAbstractItemView::PositionAtCenter);
      d->ui->m_budgetList->setCurrentItem(p, 0, QItemSelectionModel::ClearAndSelect);      // active item and deselect all others
    }
  }
}

void KBudgetView::refresh()
{
  Q_D(KBudgetView);
  if (isVisible()) {
    if (m_inSelection)
      QTimer::singleShot(0, this, SLOT(refresh()));
    else {
      loadBudgets();
      d->m_needsRefresh = false;
    }
  } else {
    d->m_needsRefresh = true;
  }
}

void KBudgetView::loadAccounts()
{
  Q_D(KBudgetView);
  // if no budgets are selected, don't load the accounts
  // and clear out the previously shown list
  if (m_budget.id().isEmpty()) {
    d->ui->m_budgetValue->clear();
    d->ui->m_updateButton->setEnabled(false);
    d->ui->m_resetButton->setEnabled(false);
    return;
  }
  d->ui->m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  d->ui->m_resetButton->setEnabled(!(selectedBudget() == m_budget));

  d->m_budgetProxyModel->setBudget(m_budget);
}

void KBudgetView::askSave()
{
  Q_D(KBudgetView);
  // check if the content of a currently selected budget was modified
  // and ask to store the data
  if (d->ui->m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
                                     i18n("Do you want to save the changes for <b>%1</b>?", m_budget.name())),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      m_inSelection = true;
      slotUpdateBudget();
      m_inSelection = false;
    }
  }
}

void KBudgetView::slotRefreshHideUnusedButton()
{
  Q_D(KBudgetView);
  d->ui->m_hideUnusedButton->setDisabled(m_budget.getaccounts().isEmpty());
}

void KBudgetView::slotSelectBudget()
{
  Q_D(KBudgetView);
  askSave();
  KBudgetListItem* item;

  QTreeWidgetItemIterator widgetIt = QTreeWidgetItemIterator(d->ui->m_budgetList);
  if (m_budget.id().isEmpty()) {
    item = dynamic_cast<KBudgetListItem*>(*widgetIt);
    if (item) {
      d->ui->m_budgetList->blockSignals(true);
      d->ui->m_budgetList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
      d->ui->m_budgetList->blockSignals(false);
    }
  }

  d->ui->m_accountTree->setEnabled(false);
  d->ui->m_assignmentBox->setEnabled(false);
  m_budget = MyMoneyBudget();

  QTreeWidgetItemIterator it_l(d->ui->m_budgetList, QTreeWidgetItemIterator::Selected);
  item = dynamic_cast<KBudgetListItem*>(*it_l);
  if (item) {
    m_budget = item->budget();
    d->ui->m_accountTree->setEnabled(true);
  }

  slotRefreshHideUnusedButton();
  loadAccounts();
  const auto index = d->ui->m_accountTree->currentIndex();
  if (index.isValid()) {
    const auto acc = d->ui->m_accountTree->model()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
    slotSelectAccount(acc);
  } else {
    d->ui->m_budgetValue->clear();
  }

  QList<MyMoneyBudget> budgetList;
  if (!m_budget.id().isEmpty())
    budgetList << m_budget;
  emit selectObjects(budgetList);
}

void KBudgetView::slotHideUnused(bool toggled)
{
  Q_D(KBudgetView);
  // make sure we show all items for an empty budget
  const auto prevState = !toggled;
  slotRefreshHideUnusedButton();
  if (prevState != d->ui->m_hideUnusedButton->isChecked())
    d->m_budgetProxyModel->setHideUnusedIncomeExpenseAccounts(d->ui->m_hideUnusedButton->isChecked());
}

const MyMoneyBudget& KBudgetView::selectedBudget() const
{
  Q_D(const KBudgetView);
  static MyMoneyBudget nullBudget;

  QTreeWidgetItemIterator it_l(d->ui->m_budgetList, QTreeWidgetItemIterator::Selected);
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(*it_l);
  if (item) {
    return item->budget();
  }
  return nullBudget;
}

void KBudgetView::slotOpenContextMenu(const QPoint& p)
{
  Q_D(KBudgetView);
  auto item = dynamic_cast<KBudgetListItem*>(d->ui->m_budgetList->itemAt(p));
  if (item)
    emit openContextMenu(item->budget());
  else
    emit openContextMenu(MyMoneyBudget());
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

void KBudgetView::slotItemChanged(QTreeWidgetItem* p, int col)
{
  // if we don't have an item we actually don't care about it
  if (!p)
    return;

  KBudgetListItem *pBudget = dynamic_cast<KBudgetListItem*>(p);
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
      KMessageBox::detailedSorry(0, i18n("Unable to modify budget"),
                                 i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
    }
  } else {
    pBudget->setText(0, new_name);
  }
}

void KBudgetView::slotSelectAccount(const MyMoneyObject &obj)
{
  Q_D(KBudgetView);
  d->ui->m_assignmentBox->setEnabled(false);
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  d->ui->m_assignmentBox->setEnabled(true);

  if (m_budget.id().isEmpty())
    return;

  QString id = acc.id();
  d->ui->m_leAccounts->setText(MyMoneyFile::instance()->accountToCategory(id));
  d->ui->m_cbBudgetSubaccounts->setChecked(m_budget.account(id).budgetSubaccounts());
  d->ui->m_accountTotal->setValue(m_budget.account(id).totalBalance());
  MyMoneyBudget::AccountGroup budgetAccount = m_budget.account(id);
  if (id != budgetAccount.id()) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
  }
  d->ui->m_budgetValue->setBudgetValues(m_budget, budgetAccount);
}

void KBudgetView::slotBudgetedAmountChanged()
{
  if (m_budget.id().isEmpty())
    return;
  Q_D(KBudgetView);

  const auto indexes = d->ui->m_accountTree->selectionModel()->selectedIndexes();
  if (indexes.empty())
    return;
  QString accountID = indexes.front().data((int)eAccountsModel::Role::ID).toString();

  MyMoneyBudget::AccountGroup accountGroup = m_budget.account(accountID);
  accountGroup.setId(accountID);
  d->ui->m_budgetValue->budgetValues(m_budget, accountGroup);
  m_budget.setAccount(accountGroup, accountID);

  d->m_budgetProxyModel->setBudget(m_budget);
  d->ui->m_accountTotal->setValue(accountGroup.totalBalance());

  d->ui->m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  d->ui->m_resetButton->setEnabled(!(selectedBudget() == m_budget));
}

void KBudgetView::AccountEnter()
{
  if (m_budget.id().isEmpty())
    return;
}

void KBudgetView::cb_includesSubaccounts_clicked()
{
  if (m_budget.id().isEmpty())
    return;
  Q_D(KBudgetView);

  QModelIndexList indexes = d->ui->m_accountTree->selectionModel()->selectedIndexes();
  if (!indexes.empty()) {
    QString accountID = indexes.front().data((int)eAccountsModel::Role::ID).toString();
    // now, we get a reference to the accountgroup, to modify its attribute,
    // and then put the resulting account group instead of the original
    MyMoneyBudget::AccountGroup auxAccount = m_budget.account(accountID);
    auxAccount.setBudgetSubaccounts(d->ui->m_cbBudgetSubaccounts->isChecked());

    // in case we turn the option on, we check that no subordinate account
    // has a budget. If we find some, we ask the user if he wants to move it
    // to the current account or leave things as they are
    if (d->ui->m_cbBudgetSubaccounts->isChecked()) {
      // TODO: asking the user needs to be added. So long, we assume yes
      if (1) {
        MyMoneyBudget::AccountGroup subAccount;
        if (collectSubBudgets(subAccount, indexes.front())) {
          // we found a sub-budget somewhere
          // so we add those figures found and
          // clear the subaccounts
          auxAccount += subAccount;
          clearSubBudgets(indexes.front());
        }

        if (auxAccount.budgetLevel() == MyMoneyBudget::AccountGroup::eNone) {
          MyMoneyBudget::PeriodGroup period;
          auxAccount.addPeriod(m_budget.budgetStart(), period);
          auxAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
        }
      }
    }

    m_budget.setAccount(auxAccount, accountID);
    d->m_budgetProxyModel->setBudget(m_budget);
    d->ui->m_budgetValue->setBudgetValues(m_budget, auxAccount);

    loadAccounts();
  }
}

void KBudgetView::clearSubBudgets(const QModelIndex &index)
{
  Q_D(KBudgetView);
  const auto children = d->ui->m_accountTree->model()->rowCount(index);

  for (auto i = 0; i < children; ++i) {
    const auto childIdx = index.child(i, 0);
    const auto accountID = childIdx.data((int)eAccountsModel::Role::ID).toString();
    m_budget.removeReference(accountID);
    clearSubBudgets(childIdx);
  }
}

bool KBudgetView::collectSubBudgets(MyMoneyBudget::AccountGroup &destination, const QModelIndex &index) const
{
  Q_D(const KBudgetView);
  auto rc = false;
  const auto children = d->ui->m_accountTree->model()->rowCount(index);

  for (auto i = 0; i < children; ++i) {
    auto childIdx = index.child(i, 0);
    auto accountID = childIdx.data((int)eAccountsModel::Role::ID).toString();
    MyMoneyBudget::AccountGroup auxAccount = m_budget.account(accountID);
    if (auxAccount.budgetLevel() != MyMoneyBudget::AccountGroup::eNone
        && !auxAccount.isZero()) {
      rc = true;
      // add the subaccount
      // TODO: deal with budgets in different currencies
      //    https://bugs.kde.org/attachment.cgi?id=54813 contains a demo file
      destination += auxAccount;
    }
    rc |= collectSubBudgets(destination, childIdx);
  }
  return rc;
}

void KBudgetView::slotNewBudget()
{
  askSave();
  kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetNew])->trigger();
}

void KBudgetView::slotResetBudget()
{
  Q_D(KBudgetView);
  try {
    m_budget = MyMoneyFile::instance()->budget(m_budget.id());
    loadAccounts();
    const auto index = d->ui->m_accountTree->currentIndex();
    if (index.isValid()) {
      const auto acc = d->ui->m_accountTree->model()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
      slotSelectAccount(acc);
    } else {
      d->ui->m_budgetValue->clear();
    }

  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Unable to reset budget"),
                               i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}

void KBudgetView::slotUpdateBudget()
{
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->modifyBudget(m_budget);
    ft.commit();
    slotRefreshHideUnusedButton();
  } catch (const MyMoneyException &e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify budget"),
                               i18n("%1 thrown in %2:%3", e.what(), e.file(), e.line()));
  }
}

void KBudgetView::slotBudgetBalanceChanged(const MyMoneyMoney &balance)
{
  Q_D(KBudgetView);
  d->netBalProChanged(balance, d->ui->m_balanceLabel, View::Budget);
}
