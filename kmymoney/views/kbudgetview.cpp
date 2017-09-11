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

#include <typeinfo>

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
#include <QTimer>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kguiitem.h>
#include <kcombobox.h>
#include <KSharedConfig>
#include <KActionCollection>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <kmymoneyglobalsettings.h>
#include <kmymoneytitlelabel.h>
#include <kmymoneyedit.h>
#include <kbudgetvalues.h>
#include "knewbudgetdlg.h"
#include "kmymoney.h"
#include "models.h"
#include <icons.h>

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

BudgetAccountsProxyModel::BudgetAccountsProxyModel(QObject *parent/* = 0*/) :
    AccountsViewFilterProxyModel(parent)
{
  addAccountGroup(QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Income, MyMoneyAccount::Expense});
}

/**
  * This function was reimplemented to add the data needed by the other columns that this model
  * is adding besides the columns of the @ref AccountsModel.
  */
QVariant BudgetAccountsProxyModel::data(const QModelIndex &index, int role) const
{
  if (!MyMoneyFile::instance()->storageAttached())
    return QVariant();
  const auto sourceColumn = m_mdlColumns->at(mapToSource(index).column());
  static QVector<AccountsModel::Columns> columnsToProcess {AccountsModel::TotalBalance, AccountsModel::TotalValue/*, AccountsModel::PostedValue*/, AccountsModel::Account};
  if (columnsToProcess.contains(sourceColumn)) {
        const auto ixAccount = mapToSource(BudgetAccountsProxyModel::index(index.row(), AccountsModel::Account, index.parent()));
        const auto account = ixAccount.data(AccountsModel::AccountRole).value<MyMoneyAccount>();
        auto const file = MyMoneyFile::instance();

        switch (role) {
          case Qt::DisplayRole:
            {
              switch (sourceColumn) {
                case AccountsModel::TotalBalance:
                  if (file->security(account.currencyId()) != file->baseCurrency())
                    return QVariant(MyMoneyUtils::formatMoney(accountBalance(account.id()), file->security(account.currencyId())));
                  else
                    return QVariant();
                case AccountsModel::TotalValue:
                  return QVariant(MyMoneyUtils::formatMoney(computeTotalValue(ixAccount), file->baseCurrency()));
                  // FIXME: Posted value doesn't correspond with total value without below code. Investigate why and wheather it matters.
                  //              case AccountsModel::PostedValue:
                  //                return QVariant(MyMoneyUtils::formatMoney(accountValue(account, accountBalance(account.id())), file->baseCurrency()));
                default:
                  break;
              }
            }
          case AccountsModel::AccountBalanceRole:
            if (file->security(account.currencyId()) != file->baseCurrency())
              return QVariant::fromValue(accountBalance(account.id()));
            else
              return QVariant();
          case AccountsModel::AccountTotalValueRole:
            return QVariant::fromValue(computeTotalValue(ixAccount));
          case AccountsModel::AccountValueRole:
            return QVariant::fromValue(accountValue(account, accountBalance(account.id())));
          default:
            break;
        }
  }
  return AccountsViewFilterProxyModel::data(index, role);
}

Qt::ItemFlags BudgetAccountsProxyModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = AccountsViewFilterProxyModel::flags(index);
  if (!index.parent().isValid())
    return flags & ~Qt::ItemIsSelectable;

  // check if any of the parent accounts has the 'include subaccounts'
  // flag set. If so, we don't allow selecting this account
  QModelIndex idx = index.parent();
  while (idx.isValid()) {
    QModelIndex source_idx = mapToSource(idx);
    QVariant accountData = sourceModel()->data(source_idx, AccountsModel::AccountRole);
    if (accountData.canConvert<MyMoneyAccount>()) {
      MyMoneyAccount account = accountData.value<MyMoneyAccount>();
      // find out if the account is budgeted
      MyMoneyBudget::AccountGroup budgetAccount = m_budget.account(account.id());
      if (budgetAccount.id() == account.id()) {
        if (budgetAccount.budgetSubaccounts()) {
          return flags & ~Qt::ItemIsEnabled;
        }
      }
    }
    idx = idx.parent();
  }
  return flags;
}

void BudgetAccountsProxyModel::setBudget(const MyMoneyBudget& budget)
{
  m_budget = budget;
  invalidate();
  checkBalance();
}

bool BudgetAccountsProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
  if (hideUnusedIncomeExpenseAccounts()) {
    const auto index = sourceModel()->index(source_row, AccountsModel::Account, source_parent);
    const auto accountData = sourceModel()->data(index, AccountsModel::AccountRole);
    if (accountData.canConvert<MyMoneyAccount>()) {
      const auto account = accountData.value<MyMoneyAccount>();
      MyMoneyMoney balance;
      // find out if the account is budgeted
      const auto budgetAccount = m_budget.account(account.id());
      if (budgetAccount.id() == account.id()) {
        balance = budgetAccount.balance();
        switch (budgetAccount.budgetLevel()) {
          case MyMoneyBudget::AccountGroup::eMonthly:
            balance *= MyMoneyMoney(12);
            break;
          default:
            break;
        }
      }
      if (!balance.isZero())
        return AccountsFilterProxyModel::filterAcceptsRow(source_row, source_parent);
    }
    for (auto i = 0; i < sourceModel()->rowCount(index); ++i) {
      if (filterAcceptsRow(i, index))
        return AccountsFilterProxyModel::filterAcceptsRow(i, index);
    }
    return false;
  }
  return AccountsFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

MyMoneyMoney BudgetAccountsProxyModel::accountBalance(const QString &accountId) const
{
  MyMoneyMoney balance;
  // find out if the account is budgeted
  MyMoneyBudget::AccountGroup budgetAccount = m_budget.account(accountId);
  if (budgetAccount.id() == accountId) {
    balance = budgetAccount.balance();
    switch (budgetAccount.budgetLevel()) {
      case MyMoneyBudget::AccountGroup::eMonthly:
        balance *= MyMoneyMoney(12);
        break;
      default:
        break;
    }
  }
  return balance;
}

MyMoneyMoney BudgetAccountsProxyModel::accountValue(const MyMoneyAccount &account, const MyMoneyMoney &balance) const
{
  return Models::instance()->accountsModel()->accountValue(account, balance);
}

MyMoneyMoney BudgetAccountsProxyModel::computeTotalValue(const QModelIndex &source_index) const
{
  auto model = sourceModel();
  auto account = model->data(source_index, AccountsModel::AccountRole).value<MyMoneyAccount>();
  auto totalValue = accountValue(account, accountBalance(account.id()));
  for (auto i = 0; i < model->rowCount(source_index); ++i)
    totalValue += computeTotalValue(model->index(i, AccountsModel::Account, source_index));
  return totalValue;
}

void BudgetAccountsProxyModel::checkBalance()
{
  // compute the balance
  QModelIndexList incomeList = match(index(0, 0),
                                     AccountsModel::AccountIdRole,
                                     MyMoneyFile::instance()->income().id(),
                                     1,
                                     Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  QModelIndexList expenseList = match(index(0, 0),
                                      AccountsModel::AccountIdRole,
                                      MyMoneyFile::instance()->expense().id(),
                                      1,
                                      Qt::MatchFlags(Qt::MatchExactly | Qt::MatchCaseSensitive));

  MyMoneyMoney balance;
  if (!incomeList.isEmpty() && !expenseList.isEmpty()) {
    QVariant incomeValue = data(incomeList.front(), AccountsModel::AccountTotalValueRole);
    QVariant expenseValue = data(expenseList.front(), AccountsModel::AccountTotalValueRole);

    if (incomeValue.isValid() && expenseValue.isValid()) {
      balance = incomeValue.value<MyMoneyMoney>() - expenseValue.value<MyMoneyMoney>();
    }
  }
  if (m_lastBalance != balance) {
    m_lastBalance = balance;
    emit balanceChanged(m_lastBalance);
  }
}

KBudgetView::KBudgetView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview) :
    QWidget(nullptr),
    m_kmymoney(kmymoney),
    m_kmymoneyview(kmymoneyview),
    m_needReload(false),
    m_needLoad(true),
    m_inSelection(false)
{
}

KBudgetView::~KBudgetView()
{
  if(!m_needLoad) {
    // remember the splitter settings for startup
    KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("KBudgetViewSplitterSize", m_splitter->saveState());
    grp.sync();
  }
}

KRecursiveFilterProxyModel *KBudgetView::getProxyModel()
{
  return m_filterProxyModel;
}

QList<AccountsModel::Columns> *KBudgetView::getProxyColumns()
{
  return m_accountTree->getColumns(KMyMoneyView::View::Budget);
}

void KBudgetView::setDefaultFocus()
{
  QTimer::singleShot(0, m_budgetList, SLOT(setFocus()));
}

bool KBudgetView::isLoaded()
{
  return !m_needLoad;
}

void KBudgetView::init()
{
  m_needLoad = false;
  setupUi(this);

  m_budgetList->setRootIsDecorated(false);
  m_budgetList->setContextMenuPolicy(Qt::CustomContextMenu);

  KGuiItem newButtonItem(QString(),
                         KMyMoneyUtils::overlayIcon(g_Icons[Icon::ViewTimeScheduleCalculus], g_Icons[Icon::ListAdd], Qt::TopRightCorner),
                         i18n("Creates a new budget"),
                         i18n("Use this to create a new empty budget."));
  KGuiItem::assign(m_newButton, newButtonItem);
  m_newButton->setToolTip(newButtonItem.toolTip());

  KGuiItem renameButtonItem(QString(),
                            KMyMoneyUtils::overlayIcon(g_Icons[Icon::ViewTimeScheduleCalculus], g_Icons[Icon::DocumentEdit]),
                            i18n("Rename the current selected budget"),
                            i18n("Use this to start renaming the selected budget."));
  KGuiItem::assign(m_renameButton, renameButtonItem);
  m_renameButton->setToolTip(renameButtonItem.toolTip());

  KGuiItem deleteButtonItem(QString(),
                            KMyMoneyUtils::overlayIcon(g_Icons[Icon::ViewTimeScheduleCalculus], g_Icons[Icon::EditDelete]),
                            i18n("Delete the current selected budget"),
                            i18n("Use this to delete the selected budget."));
  KGuiItem::assign(m_deleteButton, deleteButtonItem);
  m_deleteButton->setToolTip(deleteButtonItem.toolTip());

  KGuiItem updateButtonItem(QString(),
                            QIcon::fromTheme(g_Icons[Icon::DocumentSave]),
                            i18n("Accepts the entered values and stores the budget"),
                            i18n("Use this to store the modified data."));
  KGuiItem::assign(m_updateButton, updateButtonItem);
  m_updateButton->setToolTip(updateButtonItem.toolTip());

  KGuiItem resetButtonItem(QString(),
                           QIcon::fromTheme(g_Icons[Icon::EditUndo]),
                           i18n("Revert budget to last saved state"),
                           i18n("Use this to discard the modified data."));
  KGuiItem::assign(m_resetButton, resetButtonItem);
  m_resetButton->setToolTip(resetButtonItem.toolTip());

  m_collapseButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListCollapse]));
  m_expandButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListExpand]));

  m_filterProxyModel = new BudgetAccountsProxyModel(this);

  auto const model = Models::instance()->accountsModel();
  m_filterProxyModel->init(model, getProxyColumns());
  m_filterProxyModel->setFilterKeyColumn(-1);
  m_accountTree->init(m_filterProxyModel, model->getColumns());

  connect(this, &KBudgetView::openContextMenu, m_kmymoney, &KMyMoneyApp::slotShowBudgetContextMenu);
  connect(this, &KBudgetView::selectObjects, m_kmymoney, &KMyMoneyApp::slotSelectBudget);
  connect(m_kmymoney, &KMyMoneyApp::budgetRename, this, &KBudgetView::slotStartRename);
  connect(this, &KBudgetView::aboutToShow, m_kmymoneyview, &KMyMoneyView::aboutToChangeView);

  connect(m_filterProxyModel, &BudgetAccountsProxyModel::balanceChanged, this, &KBudgetView::slotBudgetBalanceChanged);

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, &QTreeView::collapsed, m_filterProxyModel, &AccountsViewFilterProxyModel::collapsed);
  connect(m_accountTree, &QTreeView::expanded, m_filterProxyModel, &AccountsViewFilterProxyModel::expanded);
  connect(m_accountTree, &KMyMoneyAccountTreeView::columnToggled , m_kmymoneyview, &KMyMoneyView::slotAccountTreeViewChanged);

  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), this, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectInstitution(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectInvestment(MyMoneyObject)));
  connect(m_accountTree, &KMyMoneyAccountTreeView::openContextMenu, m_kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), m_kmymoney, SLOT(slotAccountOpen(MyMoneyObject)));

  connect(m_budgetList, &QWidget::customContextMenuRequested,
          this, &KBudgetView::slotOpenContextMenu);
  connect(m_budgetList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KBudgetView::slotSelectBudget);
  connect(m_budgetList, &QTreeWidget::itemChanged, this, &KBudgetView::slotItemChanged);

  connect(m_cbBudgetSubaccounts, &QAbstractButton::clicked, this, &KBudgetView::cb_includesSubaccounts_clicked);

  // connect the buttons to the actions. Make sure the enabled state
  // of the actions is reflected by the buttons
  connect(m_renameButton, &QAbstractButton::clicked, kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetRename]), &QAction::trigger);
  connect(m_deleteButton, &QAbstractButton::clicked, kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetDelete]), &QAction::trigger);

  connect(m_budgetValue, &KBudgetValues::valuesChanged, this, &KBudgetView::slotBudgetedAmountChanged);

  connect(m_newButton, &QAbstractButton::clicked, this, &KBudgetView::slotNewBudget);
  connect(m_updateButton, &QAbstractButton::pressed, this, &KBudgetView::slotUpdateBudget);
  connect(m_resetButton, &QAbstractButton::pressed, this, &KBudgetView::slotResetBudget);

  connect(m_hideUnusedButton, &QAbstractButton::toggled, this, &KBudgetView::slotHideUnused);

  connect(m_collapseButton, &QAbstractButton::clicked, this, &KBudgetView::slotExpandCollapse);
  connect(m_expandButton, &QAbstractButton::clicked, this, &KBudgetView::slotExpandCollapse);

  // connect the two buttons to all required slots
  connect(m_collapseButton, &QAbstractButton::clicked, this, &KBudgetView::slotExpandCollapse);
  connect(m_collapseButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::collapseAll);
  connect(m_accountTree, &KMyMoneyAccountTreeView::collapsedAll, m_filterProxyModel, &AccountsViewFilterProxyModel::collapseAll);
  connect(m_expandButton, &QAbstractButton::clicked, this, &KBudgetView::slotExpandCollapse);
  connect(m_expandButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::expandAll);
  connect(m_accountTree, &KMyMoneyAccountTreeView::expandedAll, m_filterProxyModel, &AccountsViewFilterProxyModel::expandAll);

  connect(m_searchWidget, SIGNAL(textChanged(QString)), m_filterProxyModel, SLOT(setFilterFixedString(QString)));

  // setup initial state
  m_newButton->setEnabled(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetNew])->isEnabled());
  m_renameButton->setEnabled(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetRename])->isEnabled());
  m_deleteButton->setEnabled(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetDelete])->isEnabled());

  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KBudgetView::slotRefreshView);

  KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
  m_splitter->restoreState(grp.readEntry("KBudgetViewSplitterSize", QByteArray()));
  m_splitter->setChildrenCollapsible(false);
}


void KBudgetView::showEvent(QShowEvent * event)
{
  if (m_needLoad)
    init();

  emit aboutToShow();

  if (m_needReload) {
    slotRefreshView();
  }
  QWidget::showEvent(event);
}

void KBudgetView::loadBudgets()
{
  m_filterProxyModel->invalidate();

  // remember which item is currently selected
  QString id = m_budget.id();

  // clear the budget list
  m_budgetList->clear();

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

void KBudgetView::slotRefreshView()
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

void KBudgetView::loadAccounts()
{
  // if no budgets are selected, don't load the accounts
  // and clear out the previously shown list
  if (m_budget.id().isEmpty()) {
    m_budgetValue->clear();
    m_updateButton->setEnabled(false);
    m_resetButton->setEnabled(false);
    return;
  }
  m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  m_resetButton->setEnabled(!(selectedBudget() == m_budget));

  m_filterProxyModel->setBudget(m_budget);

  // and in case we need to show things expanded, we'll do so
//  if (KMyMoneyGlobalSettings::showAccountsExpanded()) {
//    m_accountTree->expandAll();
//  }

}

void KBudgetView::askSave()
{
  // check if the content of a currently selected budget was modified
  // and ask to store the data
  if (m_updateButton->isEnabled()) {
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
  m_hideUnusedButton->setDisabled(m_budget.getaccounts().isEmpty());
}

void KBudgetView::slotSelectBudget()
{
  askSave();
  KBudgetListItem* item;

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
  QModelIndex index = m_accountTree->currentIndex();
  if (index.isValid()) {
    MyMoneyAccount acc = m_accountTree->model()->data(index, AccountsModel::AccountRole).value<MyMoneyAccount>();
    slotSelectAccount(acc);
  } else {
    m_budgetValue->clear();
  }

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
    m_filterProxyModel->setHideUnusedIncomeExpenseAccounts(m_hideUnusedButton->isChecked());
}

const MyMoneyBudget& KBudgetView::selectedBudget() const
{
  static MyMoneyBudget nullBudget;

  QTreeWidgetItemIterator it_l(m_budgetList, QTreeWidgetItemIterator::Selected);
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(*it_l);
  if (item) {
    return item->budget();
  }
  return nullBudget;
}

void KBudgetView::slotOpenContextMenu(const QPoint& p)
{
  KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(m_budgetList->itemAt(p));
  if (item)
    emit openContextMenu(item->budget());
  else
    emit openContextMenu(MyMoneyBudget());
}

void KBudgetView::slotStartRename()
{
  QTreeWidgetItemIterator it_l(m_budgetList, QTreeWidgetItemIterator::Selected);
  QTreeWidgetItem* it_v;
  if ((it_v = *it_l) != 0) {
    m_budgetList->editItem(it_v, 0);
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
  m_assignmentBox->setEnabled(false);
  if (typeid(obj) != typeid(MyMoneyAccount))
    return;

  const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(obj);
  m_assignmentBox->setEnabled(true);

  if (m_budget.id().isEmpty())
    return;

  QString id = acc.id();
  m_leAccounts->setText(MyMoneyFile::instance()->accountToCategory(id));
  m_cbBudgetSubaccounts->setChecked(m_budget.account(id).budgetSubaccounts());
  m_accountTotal->setValue(m_budget.account(id).totalBalance());
  MyMoneyBudget::AccountGroup budgetAccount = m_budget.account(id);
  if (id != budgetAccount.id()) {
    budgetAccount.setBudgetLevel(MyMoneyBudget::AccountGroup::eMonthly);
  }
  m_budgetValue->setBudgetValues(m_budget, budgetAccount);
}

void KBudgetView::slotExpandCollapse()
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}

void KBudgetView::slotBudgetedAmountChanged()
{
  if (m_budget.id().isEmpty())
    return;

  QModelIndexList indexes = m_accountTree->selectionModel()->selectedIndexes();
  if (indexes.empty())
    return;
  QString accountID = indexes.front().data(AccountsModel::AccountIdRole).toString();

  MyMoneyBudget::AccountGroup accountGroup = m_budget.account(accountID);
  accountGroup.setId(accountID);
  m_budgetValue->budgetValues(m_budget, accountGroup);
  m_budget.setAccount(accountGroup, accountID);

  m_filterProxyModel->setBudget(m_budget);
  m_accountTotal->setValue(accountGroup.totalBalance());

  m_updateButton->setEnabled(!(selectedBudget() == m_budget));
  m_resetButton->setEnabled(!(selectedBudget() == m_budget));
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

  QModelIndexList indexes = m_accountTree->selectionModel()->selectedIndexes();
  if (!indexes.empty()) {
    QString accountID = indexes.front().data(AccountsModel::AccountIdRole).toString();
    // now, we get a reference to the accountgroup, to modify its attribute,
    // and then put the resulting account group instead of the original
    MyMoneyBudget::AccountGroup auxAccount = m_budget.account(accountID);
    auxAccount.setBudgetSubaccounts(m_cbBudgetSubaccounts->isChecked());

    // in case we turn the option on, we check that no subordinate account
    // has a budget. If we find some, we ask the user if he wants to move it
    // to the current account or leave things as they are
    if (m_cbBudgetSubaccounts->isChecked()) {
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
    m_filterProxyModel->setBudget(m_budget);
    m_budgetValue->setBudgetValues(m_budget, auxAccount);

    loadAccounts();
  }
}

void KBudgetView::clearSubBudgets(const QModelIndex &index)
{
  int children = m_accountTree->model()->rowCount(index);

  for (int i = 0; i < children; ++i) {
    QModelIndex childIdx = index.child(i, 0);
    QString accountID = childIdx.data(AccountsModel::AccountIdRole).toString();
    m_budget.removeReference(accountID);
    clearSubBudgets(childIdx);
  }
}

bool KBudgetView::collectSubBudgets(MyMoneyBudget::AccountGroup &destination, const QModelIndex &index) const
{
  bool rc = false;
  int children = m_accountTree->model()->rowCount(index);

  for (int i = 0; i < children; ++i) {
    QModelIndex childIdx = index.child(i, 0);
    QString accountID = childIdx.data(AccountsModel::AccountIdRole).toString();
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
  try {
    m_budget = MyMoneyFile::instance()->budget(m_budget.id());
    loadAccounts();
    QModelIndex index = m_accountTree->currentIndex();
    if (index.isValid()) {
      MyMoneyAccount acc = m_accountTree->model()->data(index, AccountsModel::AccountRole).value<MyMoneyAccount>();
      slotSelectAccount(acc);
    } else {
      m_budgetValue->clear();
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
  m_kmymoneyview->slotNetBalProChanged(balance, m_balanceLabel, KMyMoneyView::View::Budget);
}
