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


class KBudgetViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KBudgetView)

public:
  explicit KBudgetViewPrivate(KBudgetView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KBudgetView)
    , m_budgetProxyModel(nullptr)
    , m_contextMenu(nullptr)
    , m_actionCollection(nullptr)
  {
  }

  ~KBudgetViewPrivate()
  {
    // remember the splitter settings for startup
    // but only if we ever have been initialized
    if (m_budgetProxyModel) {
      KConfigGroup grp = KSharedConfig::openConfig()->group("Last Use Settings");
      grp.writeEntry("KBudgetViewSplitterSize", ui->m_splitter->saveState());
      grp.sync();
    }
    delete ui;
  }

  void init()
  {
    Q_Q(KBudgetView);
    ui->setupUi(q);

    ui->m_budgetList->setContextMenuPolicy(Qt::CustomContextMenu);

    // assign actions to tool buttons
    ui->m_newButton->setDefaultAction(m_actions[eMenu::BudgetAction::NewBudget]);
    ui->m_renameButton->setDefaultAction(m_actions[eMenu::BudgetAction::RenameBudget]);
    ui->m_deleteButton->setDefaultAction(m_actions[eMenu::BudgetAction::DeleteBudget]);

    ui->m_updateButton->setIcon(Icons::get(Icon::DocumentSave));
    ui->m_resetButton->setIcon(Icons::get(Icon::EditUndo));
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    ui->m_budgetList->setModel(MyMoneyFile::instance()->budgetsModel());

    // replace the standard proxy model
    m_budgetProxyModel = new BudgetViewProxyModel(q);
    m_budgetProxyModel->addAccountGroup(AccountsProxyModel::incomeExpense());
    ui->m_accountTree->setProxyModel(m_budgetProxyModel);

    auto columnSelector = new ColumnSelector(ui->m_accountTree, q->metaObject()->className());
    columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));
    columnSelector->setAlwaysHidden(QVector<int>({ AccountsModel::Column::Balance, AccountsModel::Column::PostedValue }));

    ui->m_accountTree->setModel(MyMoneyFile::instance()->accountsModel());

    columnSelector->setModel(m_budgetProxyModel);

    q->connect(m_budgetProxyModel, &BudgetViewProxyModel::balanceChanged, q, &KBudgetView::slotBudgetBalanceChanged);

    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestSelectionChange, q, &KBudgetView::slotAccountSelectionChanged );
    q->connect(ui->m_budgetList->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KBudgetView::slotSelectBudget);
    q->connect(ui->m_cbBudgetSubaccounts, &QAbstractButton::clicked, q, &KBudgetView::cb_includesSubaccounts_clicked);

    /// @todo possibly create a QAction and use a toolbutton for these
    q->connect(ui->m_updateButton, &QAbstractButton::clicked, q, &KBudgetView::slotUpdateBudget);
    q->connect(ui->m_resetButton, &QAbstractButton::clicked, q, &KBudgetView::slotResetBudget);
    q->connect(ui->m_hideUnusedButton, &QAbstractButton::toggled, q, &KBudgetView::slotHideUnused);

    q->connect(ui->m_budgetValue, &KBudgetValues::valuesChanged, q, &KBudgetView::slotBudgetedAmountChanged);
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_budgetProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    q->slotSettingsChanged();

    auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
    ui->m_splitter->restoreState(grp.readEntry("KBudgetViewSplitterSize", QByteArray()));
    ui->m_splitter->setChildrenCollapsible(false);
  }

  void askSave()
  {
    Q_Q(KBudgetView);
    // check if the content of a currently selected budget was modified
    // and ask to store the data
    if (ui->m_updateButton->isEnabled()) {
      if (KMessageBox::questionYesNo(q, i18n("<qt>Do you want to save the changes for <b>%1</b>?</qt>", m_budget.name()),
                                     i18n("Save changes")) == KMessageBox::Yes) {
        // m_inSelection = true;
        q->slotUpdateBudget();
        // m_inSelection = false;
      }
    }
  }

  void loadBudgetAccountsView()
  {
    // if no budgets are selected, don't load the accounts
    // and clear out the previously shown list
    if (m_budget.id().isEmpty()) {
      ui->m_budgetValue->clear();
      ui->m_updateButton->setEnabled(false);
      ui->m_resetButton->setEnabled(false);
      m_budgetProxyModel->setBudget(MyMoneyBudget());
      return;
    }
    ui->m_updateButton->setEnabled(!(selectedBudget() == m_budget));
    ui->m_resetButton->setEnabled(!(selectedBudget() == m_budget));
    m_budgetProxyModel->setBudget(m_budget);
  }

  MyMoneyBudget selectedBudget() const
  {
    MyMoneyBudget budget;
    auto currentIdx = ui->m_budgetList->selectionModel()->currentIndex();
    if (currentIdx.isValid() && !ui->m_budgetList->selectionModel()->selectedIndexes().isEmpty()) {
      const auto baseIdx = MyMoneyFile::baseModel()->mapToBaseSource( currentIdx );
      budget = MyMoneyFile::instance()->budgetsModel()->itemByIndex(baseIdx);
    }
    return budget;
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

  Ui::KBudgetView*                      ui;
  BudgetViewProxyModel*                 m_budgetProxyModel;
  MyMoneyBudget                         m_budget;
  QHash<eMenu::BudgetAction, QAction*>  m_actions;
  QMenu*                                m_contextMenu;
  KActionCollection*                    m_actionCollection;
};

#endif
