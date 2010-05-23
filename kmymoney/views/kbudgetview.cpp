/***************************************************************************
                          kbudgetview.cpp
                          ---------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
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

// ----------------------------------------------------------------------------
// QT Includes

#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QTabWidget>
#include <QCheckBox>
#include <QToolTip>
#include <QList>
#include <QResizeEvent>
#include <QTreeWidgetItemIterator>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kcalendarsystem.h>
#include <kcombobox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <kmymoneyglobalsettings.h>
#include <kmymoneytitlelabel.h>
#include <kmymoneyedit.h>
#include <kbudgetvalues.h>
#include "kmymoneyaccounttreebudget.h"
#include "knewbudgetdlg.h"
#include "kmymoney.h"

// *** KBudgetListItem Implementation ***
KBudgetListItem::KBudgetListItem(QTreeWidget *parent, const MyMoneyBudget& budget) :
    QTreeWidgetItem(parent),
    m_budget(budget)
{
  setText(0, budget.name());
  setText(1, QString("%1").arg(budget.budgetStart().year()));
  setFlags(flags() | Qt::ItemIsEditable);
  // allow in column rename
  //setRenameEnabled(0, true);
}

KBudgetListItem::~KBudgetListItem()
{
}

// *** KBudgetView Implementation ***
//TODO: This has to go to user settings
const int KBudgetView::m_iBudgetYearsAhead = 5;
const int KBudgetView::m_iBudgetYearsBack = 3;

KBudgetView::KBudgetView(QWidget *parent) :
    KBudgetViewDecl(parent),
    m_needReload(false),
    m_inSelection(false),
    m_budgetInEditing(false)
{
  m_accountTree->setSorting(-1);
  m_budgetList->setRootIsDecorated(false);
  m_budgetList->setContextMenuPolicy(Qt::CustomContextMenu);

  KGuiItem newButtonItem(QString(""),
                         KIcon("budget-add"),
                         i18n("Creates a new budget"),
                         i18n("Use this to create a new empty budget."));
  m_newButton->setGuiItem(newButtonItem);
  m_newButton->setToolTip(newButtonItem.toolTip());

  KGuiItem renameButtonItem(QString(""),
                            KIcon("budget-edit"),
                            i18n("Rename the current selected budget"),
                            i18n("Use this to start renaming the selected budget."));
  m_renameButton->setGuiItem(renameButtonItem);
  m_renameButton->setToolTip(renameButtonItem.toolTip());

  KGuiItem deleteButtonItem(QString(""),
                            KIcon("budget-delete"),
                            i18n("Delete the current selected budget"),
                            i18n("Use this to delete the selected budget."));
  m_deleteButton->setGuiItem(deleteButtonItem);
  m_deleteButton->setToolTip(deleteButtonItem.toolTip());

  KGuiItem updateButtonItem(QString(""),
                            KIcon("document-save"),
                            i18n("Accepts the entered values and stores the budget"),
                            i18n("Use this to store the modified data."));
  m_updateButton->setGuiItem(updateButtonItem);
  m_updateButton->setToolTip(updateButtonItem.toolTip());

  KGuiItem resetButtonItem(QString(""),
                           KIcon("edit-undo"),
                           i18n("Revert budget to last saved state"),
                           i18n("Use this to discard the modified data."));
  m_resetButton->setGuiItem(resetButtonItem);
  m_resetButton->setToolTip(resetButtonItem.toolTip());

  connect(m_budgetList, SIGNAL(customContextMenuRequested(const QPoint&)),
          this, SLOT(slotOpenContextMenu(const QPoint&)));
  connect(m_budgetList, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotRenameBudget(QTreeWidgetItem*)));
  connect(m_budgetList->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(slotSelectBudget()));

  connect(m_cbBudgetSubaccounts, SIGNAL(clicked()), this, SLOT(cb_includesSubaccounts_clicked()));

  connect(m_accountTree, SIGNAL(selectionChanged(Q3ListViewItem*)), this, SLOT(slotSelectAccount(Q3ListViewItem*)));
  connect(m_accountTree, SIGNAL(valueChanged()), this, SLOT(slotRefreshHideUnusedButton()));

  // connect the buttons to the actions. Make sure the enabled state
  // of the actions is reflected by the buttons
  connect(m_renameButton, SIGNAL(clicked()), kmymoney->action("budget_rename"), SLOT(trigger()));
  connect(m_deleteButton, SIGNAL(clicked()), kmymoney->action("budget_delete"), SLOT(trigger()));

  connect(m_budgetValue, SIGNAL(valuesChanged()), this, SLOT(slotBudgetedAmountChanged()));

  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewBudget()));
  connect(m_updateButton, SIGNAL(pressed()), this, SLOT(slotUpdateBudget()));
  connect(m_resetButton, SIGNAL(pressed()), this, SLOT(slotResetBudget()));

  connect(m_hideUnusedButton, SIGNAL(toggled(bool)), this, SLOT(slotHideUnused(bool)));

  // setup initial state
  m_newButton->setEnabled(kmymoney->action("budget_new")->isEnabled());
  m_renameButton->setEnabled(kmymoney->action("budget_rename")->isEnabled());
  m_deleteButton->setEnabled(kmymoney->action("budget_delete")->isEnabled());

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotRefreshView()));

  KConfigGroup grp = KGlobal::config()->group("Last Use Settings");
  QList<int> sizes = grp.readEntry("KBudgetViewSplitterSize", QList<int>());
  if (sizes.size() == 2)
    m_splitter->setSizes(sizes);
}

KBudgetView::~KBudgetView()
{
  // remember the splitter settings for startup
  KConfigGroup grp = KGlobal::config()->group("Last Use Settings");
  grp.writeEntry("KBudgetViewSplitterSize", m_splitter->sizes());
}

void KBudgetView::showEvent(QShowEvent * event)
{
  QTimer::singleShot(50, this, SLOT(slotRearrange()));
  m_accountTree->restoreLayout("Budget Account View Settings");
  if (m_needReload) {
    slotRefreshView();
  }
  KBudgetViewDecl::showEvent(event);
}

void KBudgetView::slotRearrange(void)
{
  resizeEvent(0);
}

void KBudgetView::resizeEvent(QResizeEvent* ev)
{
  // resize the register
  KBudgetViewDecl::resizeEvent(ev);
}

void KBudgetView::slotReloadView(void)
{
  ::timetrace("Start KBudgetView::slotReloadView");
  slotRearrange();
  ::timetrace("Done KBudgetView::slotReloadView");
}

void KBudgetView::loadBudgets(void)
{
  ::timetrace("Start KBudgetView::loadBudgets");

  // remember which item is currently selected
  QString id = m_budget.id();

  // clear the budget list
  m_budgetList->clear();
  m_budgetValue->clear();

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
    KBudgetListItem* item = new KBudgetListItem(m_budgetList, *it);

    // create a list of unique years
    if (m_yearList.indexOf(QString::number((*it).budgetStart().year())) == -1)
      m_yearList += QString::number((*it).budgetStart().year());

    //sort the list by name
    m_budgetList->sortItems(0, Qt::AscendingOrder);

    if (item->budget().id() == id) {
      m_budget = (*it);
      currentItem = item;
      item->setSelected(true);
    }
  }
  m_yearList.sort();

  if (currentItem) {
    m_budgetList->setCurrentItem(currentItem);
  }

  // reset the status of the buttons
  m_updateButton->setEnabled(false);
  m_resetButton->setEnabled(false);

  // make sure the world around us knows what we have selected
  slotSelectBudget();

  ::timetrace("End KBudgetView::loadBudgets");
}

void KBudgetView::ensureBudgetVisible(const QString& id)
{
  QTreeWidgetItemIterator widgetIt = QTreeWidgetItemIterator(m_budgetList);
  while (*widgetIt) {
    KBudgetListItem* p = dynamic_cast<KBudgetListItem*>(*widgetIt);
    if ((p)->budget().id() == id) {
      m_budgetList->scrollToItem((p), QAbstractItemView::PositionAtCenter);
      m_budgetList->setCurrentItem(p, 0, QItemSelectionModel::ClearAndSelect);      // active item and deselect all others
    }
  }
}

void KBudgetView::slotRefreshView(void)
{
  if (isVisible()) {
    if (m_inSelection)
      QTimer::singleShot(0, this, SLOT(slotRefreshView()));
    else {
      loadBudgets();
      m_needReload = false;
    }
  } else {
    m_needReload = true;
  }
}

void KBudgetView::loadAccounts(void)
{
  QMap<QString, bool> isOpen;

  ::timetrace("start load budget account view");

  // if no budgets are selected, don't load the accounts
  // and clear out the previously shown list
  if (m_budget.id().isEmpty()) {
    m_accountTree->clear();
    m_budgetValue->clear();
    m_updateButton->setEnabled(false);
    m_resetButton->setEnabled(false);
    ::timetrace("done load budgets view");
    return;
  }

  // remember the id of the current selected item
  KMyMoneyAccountTreeBaseItem *item = m_accountTree->selectedItem();
  QString selectedItemId = (item) ? item->id() : QString();

  // keep a map of all 'expanded' accounts
  Q3ListViewItemIterator it_lvi(m_accountTree);
  while (it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(it_lvi.current());
    if (item && item->isOpen()) {
      isOpen[item->id()] = true;
    }
    ++it_lvi;
  }

  // remember the upper left corner of the viewport
  QPoint startPoint = m_accountTree->viewportToContents(QPoint(0, 0));

  // clear the current contents and recreate it
  m_accountTree->clear();
  m_transactionCountMap.clear();

  // make sure, the pointers are not pointing to some deleted object
  m_incomeItem = m_expenseItem = 0;

  MyMoneyFile* file = MyMoneyFile::instance();

  m_transactionCountMap = file->transactionCountMap();
  m_accountTree->setBaseCurrency(file->baseCurrency());

  bool haveUnusedBudgets = false;

  // create the items
  try {
    const MyMoneySecurity& security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    const MyMoneyAccount& income = file->income();
    QStringList incSubAcctList = income.accountList();
    m_incomeItem = new KMyMoneyAccountTreeBudgetItem(m_accountTree, income, m_budget, security, i18n("Income"));
    haveUnusedBudgets |= loadSubAccounts(m_incomeItem, incSubAcctList, m_budget);
    m_incomeItem->setSelectable(false);

    const MyMoneyAccount& expense = file->expense();
    QStringList expSubAcctList = expense.accountList();
    m_expenseItem = new KMyMoneyAccountTreeBudgetItem(m_accountTree, expense, m_budget, security, i18n("Expense"));
    haveUnusedBudgets |= loadSubAccounts(m_expenseItem, expSubAcctList, m_budget);
    m_expenseItem->setSelectable(false);

  } catch (MyMoneyException *e) {
    kDebug(2) << "Problem in budgetview: " << e->what();
    delete e;
  }

  // scan through the list of accounts and re-expand those that were
  // expanded and re-select the one that was probably selected before
  it_lvi = Q3ListViewItemIterator(m_accountTree);
  while (it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeBaseItem*>(it_lvi.current());
    if (item) {
      if (item->id() == selectedItemId)
        m_accountTree->setSelected(item, true);
      if (isOpen.find(item->id()) != isOpen.end())
        item->setOpen(true);
    }
    ++it_lvi;
  }

  // reposition viewport
  m_accountTree->setContentsPos(startPoint.x(), startPoint.y());

  m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  m_resetButton->setEnabled(!(selectedBudget() == m_budget));

  ::timetrace("done load budgets view");
}


bool KBudgetView::loadSubAccounts(KMyMoneyAccountTreeBudgetItem* parent, QStringList& accountList, const MyMoneyBudget& budget)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  bool unused = false;

  //sort the subaccount list
  //FIXME this is just a hack to order the accounts
  if (!accountList.isEmpty()) {
    QMap<QString, MyMoneyAccount> accountMap;
    QList<MyMoneyAccount> alist;
    file->accountList(alist, accountList);
    accountList.clear();
    QList<MyMoneyAccount>::const_iterator it_ac;
    for (it_ac = alist.constBegin(); it_ac != alist.constEnd(); ++it_ac) {
      accountMap[(*it_ac).name()] = *it_ac;
    }
    QMap<QString, MyMoneyAccount>::const_iterator it_am;
    for (it_am = accountMap.constBegin(); it_am != accountMap.constEnd(); ++it_am) {
      accountList.prepend((*it_am).id()); //use prepend instead of append otherwise account show up in ascending order
    }
  }

  QStringList::const_iterator it_a;
  for (it_a = accountList.constBegin(); it_a != accountList.constEnd(); ++it_a) {
    const MyMoneyAccount& acc = file->account(*it_a);
    QList<MyMoneyPrice> prices;
    MyMoneySecurity security = file->baseCurrency();
    try {
      if (acc.isInvest()) {
        security = file->security(acc.currencyId());
        prices += file->price(acc.currencyId(), security.tradingCurrency());
        if (security.tradingCurrency() != file->baseCurrency().id()) {
          MyMoneySecurity sec = file->security(security.tradingCurrency());
          prices += file->price(sec.id(), file->baseCurrency().id());
        }
      } else if (acc.currencyId() != file->baseCurrency().id()) {
        if (acc.currencyId() != file->baseCurrency().id()) {
          security = file->security(acc.currencyId());
          prices += file->price(acc.currencyId(), file->baseCurrency().id());
        }
      }

    } catch (MyMoneyException *e) {
      kDebug(2) << Q_FUNC_INFO << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
      delete e;
    }

    QStringList subAcctList = acc.accountList();
    KMyMoneyAccountTreeBudgetItem *item = new KMyMoneyAccountTreeBudgetItem(parent, acc, budget, prices, security);
    unused |= loadSubAccounts(item, subAcctList, budget);

    // no child accounts and no value assigned to this account
    bool thisUnused = (!item->firstChild()) && (!budget.contains(acc.id()));

    // In case of a budget which is unused and we are requested to suppress
    // the display of those,
    if (acc.accountGroup() == MyMoneyAccount::Income
        || acc.accountGroup() == MyMoneyAccount::Expense) {
      if (m_hideUnusedButton->isEnabled() && m_hideUnusedButton->isChecked() && thisUnused) {
        unused = true;
        delete item;
      }
    }
  }
  return unused;
}

void KBudgetView::askSave(void)
{
  // check if the content of a currently selected budget was modified
  // and ask to store the data
  if (m_updateButton->isEnabled()) {
    if (KMessageBox::questionYesNo(this, QString("<qt>%1</qt>").arg(
                                     i18n("Do you want to save the changes for <b>%1</b>", m_budget.name())),
                                   i18n("Save changes")) == KMessageBox::Yes) {
      m_inSelection = true;
      slotUpdateBudget();
      m_inSelection = false;
    }
  }
}

void KBudgetView::slotRefreshHideUnusedButton(void)
{
  m_hideUnusedButton->setDisabled(m_budget.getaccounts().isEmpty());
}

void KBudgetView::slotSelectBudget(void)
{
  askSave();
  KBudgetListItem* item;
  m_budgetInEditing = false;

  QTreeWidgetItemIterator widgetIt = QTreeWidgetItemIterator(m_budgetList);
  if (m_budget.id().isEmpty()) {
    item = dynamic_cast<KBudgetListItem*>(*widgetIt);
    if (item) {
      m_budgetList->blockSignals(true);
      m_budgetList->setCurrentItem(item, QItemSelectionModel::ClearAndSelect);
      m_budgetList->blockSignals(false);
    }
  }

  m_accountTree->setEnabled(false);
  m_assignmentBox->setEnabled(false);
  m_budget = MyMoneyBudget();

  QTreeWidgetItemIterator it_l(m_budgetList, QTreeWidgetItemIterator::Selected);
  item = dynamic_cast<KBudgetListItem*>(*it_l);
  if (item) {
    m_budget = item->budget();
    m_accountTree->setEnabled(true);
  }

  slotRefreshHideUnusedButton();
  loadAccounts();

  QList<MyMoneyBudget> budgetList;
  if (!m_budget.id().isEmpty())
    budgetList << m_budget;
  emit selectObjects(budgetList);
}

void KBudgetView::slotHideUnused(bool toggled)
{
  // make sure we show all items for an empty budget
  bool prevState = !toggled;
  slotRefreshHideUnusedButton();
  if (prevState != m_hideUnusedButton->isChecked())
    loadAccounts();
}

const MyMoneyBudget& KBudgetView::selectedBudget(void) const
{
  static MyMoneyBudget nullBudget;

  QTreeWidgetItemIterator it_l(m_budgetList, QTreeWidgetItemIterator::Selected);
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(*it_l);
  if (item) {
    return item->budget();
  }
  return nullBudget;
}

KMyMoneyAccountTreeBudgetItem* KBudgetView::selectedAccount(void) const
{
  Q3ListViewItemIterator it_l(m_accountTree, Q3ListViewItemIterator::Selected);
  KMyMoneyAccountTreeBudgetItem* item = dynamic_cast<KMyMoneyAccountTreeBudgetItem*>(it_l.current());
  return item;
}

void KBudgetView::slotOpenContextMenu(const QPoint& p)
{
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(m_budgetList->itemAt(p));
  if (item)
    emit openContextMenu(item->budget());
  else
    emit openContextMenu(MyMoneyBudget());
}

void KBudgetView::slotStartRename(void)
{
  m_budgetInEditing = true;
  QTreeWidgetItemIterator it_l(m_budgetList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    m_budgetList->editItem(it_v, 0);
  }
}

// This variant is only called when a single budget is selected and renamed.
void KBudgetView::slotRenameBudget(QTreeWidgetItem* p)
{
  KBudgetListItem *pBudget = dynamic_cast<KBudgetListItem*>(p);

  //if there is no current item selected, exit
  if (m_budgetInEditing == false || !m_budgetList->currentItem() || p != m_budgetList->currentItem())
    return;

  m_budgetInEditing = false;

  //kDebug() << "[KPayeesView::slotRenamePayee]";
  // create a copy of the new name without appended whitespaces
  QString new_name = p->text(0);

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
      } catch (MyMoneyException *e) {
        // all ok, the name is unique
        delete e;
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

    } catch (MyMoneyException *e) {
      KMessageBox::detailedSorry(0, i18n("Unable to modify budget"),
                                 (e->what() + ' ' + i18n("thrown in") + ' ' + e->file() + ":%1").arg(e->line()));
      delete e;
    }
  } else {
    pBudget->setText(0, new_name);
  }
}

void KBudgetView::slotSelectAccount(Q3ListViewItem* item)
{
  if (item->listView() == m_accountTree) {
    m_assignmentBox->setEnabled(false);
    KMyMoneyAccountTreeBudgetItem *account = selectedAccount();
    m_assignmentBox->setEnabled(account != 0);

    if (account) {
      if (m_budget.id().isEmpty())
        return;

      QString id = account->id();
      m_leAccounts->setText(MyMoneyFile::instance()->accountToCategory(id));
      m_cbBudgetSubaccounts->setChecked(m_budget.account(id).budgetSubaccounts());
      m_accountTotal->setValue(m_budget.account(id).totalBalance());

      MyMoneyBudget::AccountGroup budgetAccount = m_budget.account(id);
      if (id != budgetAccount.id()) {
        budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
      }
      m_budgetValue->setBudgetValues(m_budget, budgetAccount);
    }
  }
}

void KBudgetView::slotBudgetedAmountChanged(void)
{
  if (m_budget.id().isEmpty())
    return;

  KMyMoneyAccountTreeBudgetItem *account;
  if ((account = selectedAccount()) == NULL)
    return;

  MyMoneyBudget::AccountGroup accountGroup = m_budget.account(account->id());
  accountGroup.setId(account->id());
  m_budgetValue->budgetValues(m_budget, accountGroup);
  m_budget.setAccount(accountGroup, account->id());

  account->setBudget(m_budget);
  m_accountTotal->setValue(accountGroup.totalBalance());

  m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  m_resetButton->setEnabled(!(selectedBudget() == m_budget));
}

void KBudgetView::AccountEnter()
{
  if (m_budget.id().isEmpty())
    return;

  //(ace) kCategoryWidget not currently defined
  KMyMoneyAccountTreeBudgetItem *item = NULL; //dynamic_cast<KMyMoneyAccountTreeBudgetItem*> (m_accountTree->findItem(m_leAccounts->selectedAccountId()));
  if (item) {
    m_accountTree->setCurrentItem(item);
    m_accountTree->setOpen(item, true);
  }
}

void KBudgetView::cb_includesSubaccounts_clicked()
{
  if (m_budget.id().isEmpty())
    return;

  if (selectedAccount() != 0) {
    QString accountID = selectedAccount()->id();
    // now, we get a reference to the accountgroup, to mofify its atribute,
    // and then put the resulting account group instead of the original

    MyMoneyBudget::AccountGroup auxAccount = m_budget.account(accountID);
    auxAccount.setBudgetSubaccounts(m_cbBudgetSubaccounts->isChecked());
    m_budget.setAccount(auxAccount, accountID);

    loadAccounts();
  }
}

void KBudgetView::slotNewBudget(void)
{
  askSave();
  kmymoney->action("budget_new")->trigger();
}

void KBudgetView::slotResetBudget(void)
{
  try {
    m_budget = MyMoneyFile::instance()->budget(m_budget.id());
    loadAccounts();
  } catch (MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to reset budget"),
                               (e->what() + ' ' + i18n("thrown in") + ' ' + e->file() + ":%1").arg(e->line()));
    delete e;
  }
}

void KBudgetView::slotUpdateBudget(void)
{
  MyMoneyFileTransaction ft;
  try {
    MyMoneyFile::instance()->modifyBudget(m_budget);
    ft.commit();
    slotRefreshHideUnusedButton();
  } catch (MyMoneyException *e) {
    KMessageBox::detailedSorry(0, i18n("Unable to modify budget"),
                               (e->what() + ' ' + i18n("thrown in") + ' ' + e->file() + ":%1").arg(e->line()));
    delete e;
  }
}

void KBudgetView::languageChange(void)
{
  KBudgetViewDecl::languageChange();

  m_newButton->setText(QString());
  m_renameButton->setText(QString());
  m_deleteButton->setText(QString());
  m_updateButton->setText(QString());
  m_resetButton->setText(QString());
}

#include "kbudgetview.moc"
