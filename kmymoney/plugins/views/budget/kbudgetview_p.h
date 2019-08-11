/***************************************************************************
                          kbudgetview.cpp
                          ---------------
    begin                : Thu Jan 10 2006
    copyright            : (C) 2006 by Darren Gould
    email                : darren_gould@gmx.de
                           Alvaro Soliverez <asoliverez@gmail.com>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
                           (C) 2019 Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KBUDGETVIEW_P_H
#define KBUDGETVIEW_P_H

#include "kbudgetview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>
#include <QMenu>
#include <QDate>
#include <QInputDialog>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KMessageBox>
#include <KConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbudgetview.h"
#include "kmymoneyviewbase_p.h"

#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyforecast.h"
#include "kbudgetvalues.h"
#include "kmymoneyutils.h"

#include "budgetsmodel.h"
#include "budgetviewproxymodel.h"
#include "kmymoneysettings.h"
#include "icons.h"
#include "menuenums.h"
#include "mymoneyenums.h"
#include "columnselector.h"

using namespace Icons;

#if 0
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
  explicit KBudgetListItem(QTreeWidget *parent, const MyMoneyBudget& budget):
    QTreeWidgetItem(parent),
    m_budget(budget)
  {
    setText(0, budget.name());
    setText(1, QString::fromLatin1("%1").arg(budget.budgetStart().year()));
    setFlags(flags() | Qt::ItemIsEditable);
  }
  ~KBudgetListItem() {}

  const MyMoneyBudget& budget() {
    return m_budget;
  }
  void setBudget(const MyMoneyBudget& budget) {
    m_budget = budget;
  }

private:
  MyMoneyBudget  m_budget;
};
#endif

class KBudgetViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KBudgetView)

public:
  explicit KBudgetViewPrivate(KBudgetView *qq)
    : KMyMoneyViewBasePrivate()
    , q_ptr(qq)
    , ui(new Ui::KBudgetView)
    , m_budgetProxyModel(nullptr)
    , m_inSelection(false)
    , m_budgetInEditing(false)
  {
  }

  ~KBudgetViewPrivate()
  {
    // remember the splitter settings for startup
    KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
    grp.writeEntry("KBudgetViewSplitterSize", ui->m_splitter->saveState());
    grp.sync();
    delete ui;
  }

  void init()
  {
    Q_Q(KBudgetView);
    ui->setupUi(q);

    ui->m_budgetList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_newButton->setIcon(Icons::get(Icon::BudgetNew));
    ui->m_renameButton->setIcon(Icons::get(Icon::BudgetRename));
    ui->m_deleteButton->setIcon(Icons::get(Icon::BudgetDelete));
    ui->m_updateButton->setIcon(Icons::get(Icon::DocumentSave));
    ui->m_resetButton->setIcon(Icons::get(Icon::EditUndo));
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    ui->m_budgetList->setModel(MyMoneyFile::instance()->budgetsModel());

/// @todo port to new model code
    // replace the standard proxy model
    m_budgetProxyModel = new BudgetViewProxyModel(q);
    ui->m_accountTree->setProxyModel(m_budgetProxyModel);

    auto columnSelector = new ColumnSelector(ui->m_accountTree, q->metaObject()->className());
    columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));
    columnSelector->setAlwaysHidden(QVector<int>({ AccountsModel::Column::Balance, AccountsModel::Column::PostedValue }));

    ui->m_accountTree->setModel(MyMoneyFile::instance()->accountsModel());
    // m_proxyModel->addAccountGroup(AccountsProxyModel::assetLiabilityEquity());

    columnSelector->setModel(m_budgetProxyModel);

    q->connect(m_budgetProxyModel, &BudgetViewProxyModel::balanceChanged, q, &KBudgetView::slotBudgetBalanceChanged);

    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KBudgetView::slotSelectAccount);

    q->connect(ui->m_budgetList, &QTableView::customContextMenuRequested, q, &KBudgetView::slotOpenContextMenu);
    q->connect(ui->m_budgetList->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KBudgetView::slotSelectBudget);

    /// @todo port to new model code
    // q->connect(ui->m_budgetList, &QTreeWidget::itemChanged, q, &KBudgetView::slotItemChanged);

    q->connect(ui->m_cbBudgetSubaccounts, &QAbstractButton::clicked, q, &KBudgetView::cb_includesSubaccounts_clicked);

    // connect the buttons to the actions. Make sure the enabled state
    // of the actions is reflected by the buttons
    q->connect(ui->m_renameButton, &QAbstractButton::clicked, q, &KBudgetView::slotStartRename);
    q->connect(ui->m_deleteButton, &QAbstractButton::clicked, q, &KBudgetView::slotDeleteBudget);

    q->connect(ui->m_budgetValue, &KBudgetValues::valuesChanged, q, &KBudgetView::slotBudgetedAmountChanged);

    q->connect(ui->m_newButton, &QAbstractButton::clicked, q, &KBudgetView::slotNewBudget);
    q->connect(ui->m_updateButton, &QAbstractButton::clicked, q, &KBudgetView::slotUpdateBudget);
    q->connect(ui->m_resetButton, &QAbstractButton::clicked, q, &KBudgetView::slotResetBudget);

    q->connect(ui->m_hideUnusedButton, &QAbstractButton::toggled, q, &KBudgetView::slotHideUnused);

    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_budgetProxyModel, &QSortFilterProxyModel::setFilterFixedString);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KBudgetView::selectByObject);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByVariant, q, &KBudgetView::selectByVariant);

    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KBudgetView::refresh);

    /// @todo cleanup
    #if 0
    m_budgetProxyModel = qobject_cast<BudgetViewProxyModel *>(ui->m_accountTree->init(View::Budget));
    m_proxyModel = m_budgetProxyModel;

    #endif

    // setup initial state
    updateButtonStates();

    auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
    ui->m_splitter->restoreState(grp.readEntry("KBudgetViewSplitterSize", QByteArray()));
    ui->m_splitter->setChildrenCollapsible(false);
  }

  QHash<eMenu::Action, bool> actionStates()
  {
    QHash<eMenu::Action, bool> actionStates;
    actionStates[eMenu::Action::NewBudget] = true;
    auto b = m_budgetList.size() >= 1 ? true : false;
    actionStates[eMenu::Action::DeleteBudget] = b;

    b = m_budgetList.size() == 1 ? true : false;
    actionStates[eMenu::Action::ChangeBudgetYear] = b;
    actionStates[eMenu::Action::CopyBudget] = b;
    actionStates[eMenu::Action::RenameBudget] = b;
    actionStates[eMenu::Action::BudgetForecast] = b;
    return actionStates;
  }

  void updateButtonStates()
  {
    const auto actionStates = KBudgetViewPrivate::actionStates();
    ui->m_newButton->setEnabled(actionStates[eMenu::Action::NewBudget]);
    ui->m_renameButton->setEnabled(actionStates[eMenu::Action::RenameBudget]);
    ui->m_deleteButton->setEnabled(actionStates[eMenu::Action::DeleteBudget]);
  }

  void askSave()
  {
    Q_Q(KBudgetView);
    // check if the content of a currently selected budget was modified
    // and ask to store the data
    if (ui->m_updateButton->isEnabled()) {
      if (KMessageBox::questionYesNo(q, i18n("<qt>Do you want to save the changes for <b>%1</b>?</qt>", m_budget.name()),
                                     i18n("Save changes")) == KMessageBox::Yes) {
        m_inSelection = true;
        q->slotUpdateBudget();
        m_inSelection = false;
      }
    }
  }

  void refreshHideUnusedButton()
  {
    ui->m_hideUnusedButton->setDisabled(m_budget.getaccounts().isEmpty());
  }

  void loadAccounts()
  {
    // if no budgets are selected, don't load the accounts
    // and clear out the previously shown list
    if (m_budget.id().isEmpty()) {
      ui->m_budgetValue->clear();
      ui->m_updateButton->setEnabled(false);
      ui->m_resetButton->setEnabled(false);
      return;
    }
    ui->m_updateButton->setEnabled(!(selectedBudget() == m_budget));
    ui->m_resetButton->setEnabled(!(selectedBudget() == m_budget));

    m_budgetProxyModel->setBudget(m_budget);
  }

  const MyMoneyBudget& selectedBudget() const
  {
    static MyMoneyBudget nullBudget;
    /// @todo port to new model code
#if 0
    QTreeWidgetItemIterator it_l(ui->m_budgetList, QTreeWidgetItemIterator::Selected);
    KBudgetListItem* item = dynamic_cast<KBudgetListItem*>(*it_l);
    if (item) {
      return item->budget();
    }
#endif
    return nullBudget;
  }

  void AccountEnter()
  {
    if (m_budget.id().isEmpty())
      return;
  }

  void clearSubBudgets(const QModelIndex &index)
  {
    const auto children = ui->m_accountTree->model()->rowCount(index);

    for (auto i = 0; i < children; ++i) {
      const auto childIdx = index.child(i, 0);
      const auto accountID = childIdx.data(eMyMoney::Model::Roles::IdRole).toString();
      m_budget.removeReference(accountID);
      clearSubBudgets(childIdx);
    }
  }

  bool collectSubBudgets(MyMoneyBudget::AccountGroup &destination, const QModelIndex &index) const
  {
    auto rc = false;
    const auto children = ui->m_accountTree->model()->rowCount(index);

    for (auto i = 0; i < children; ++i) {
      auto childIdx = index.child(i, 0);
      auto accountID = childIdx.data(eMyMoney::Model::Roles::IdRole).toString();
      MyMoneyBudget::AccountGroup auxAccount = m_budget.account(accountID);
      if (auxAccount.budgetLevel() != eMyMoney::Budget::Level::None
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

  /**
   * This method loads all available budgets into the budget list widget. If a budget is
   * currently selected it remains selected if it is still present.
   */
  void loadBudgets()
  {
  /// @todo port to new model code
  #if 0
    Q_Q(KBudgetView);
    m_budgetProxyModel->invalidate();

    // remember which item is currently selected
    QString id = m_budget.id();

    // clear the budget list
    ui->m_budgetList->clear();

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
      KBudgetListItem* item = new KBudgetListItem(ui->m_budgetList, *it);

      // create a list of unique years
      if (m_yearList.indexOf(QString::number((*it).budgetStart().year())) == -1)
        m_yearList += QString::number((*it).budgetStart().year());

      //sort the list by name
      ui->m_budgetList->sortItems((int)eAccountsModel::Column::Account, Qt::AscendingOrder);

      if (item->budget().id() == id) {
        m_budget = (*it);
        currentItem = item;
        item->setSelected(true);
      }
    }
    m_yearList.sort();

    if (currentItem) {
      ui->m_budgetList->setCurrentItem(currentItem);
    }

    // reset the status of the buttons
    ui->m_updateButton->setEnabled(false);
    ui->m_resetButton->setEnabled(false);

    // make sure the world around us knows what we have selected
    q->slotSelectBudget();
#endif
  }

  void ensureBudgetVisible(const QString& id)
  {
    /// @todo port to new model code
#if 0
    const auto widgetIt = QTreeWidgetItemIterator(ui->m_budgetList);
    while (*widgetIt) {
      const auto p = dynamic_cast<KBudgetListItem*>(*widgetIt);
      if ((p)->budget().id() == id) {
        ui->m_budgetList->scrollToItem((p), QAbstractItemView::PositionAtCenter);
        ui->m_budgetList->setCurrentItem(p, 0, QItemSelectionModel::ClearAndSelect);      // active item and deselect all others
      }
    }
#endif
  }

  KBudgetView*          q_ptr;
  Ui::KBudgetView*      ui;
  BudgetViewProxyModel* m_budgetProxyModel;

  MyMoneyBudget         m_budget;
  QMap<QString, ulong>  m_transactionCountMap;
  QStringList           m_yearList;
  QList<MyMoneyBudget>  m_budgetList;

  /**
    * Set if we are in the selection of a different budget
    **/
  bool                  m_inSelection;

  void adaptHideUnusedButton();

  static const int      m_iBudgetYearsAhead = 5;
  static const int      m_iBudgetYearsBack = 3;

  /**
    * This signals whether a budget is being edited
    **/
  bool                  m_budgetInEditing;
};

#endif
