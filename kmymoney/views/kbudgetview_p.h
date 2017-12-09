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

#ifndef KBUDGETVIEW_P_H
#define KBUDGETVIEW_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KActionCollection>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kbudgetview.h"
#include "kmymoneyaccountsviewbase_p.h"

#include "budgetviewproxymodel.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "icons.h"

using namespace Icons;

namespace Ui {
  class KBudgetView;
}
class KBudgetViewPrivate : public KMyMoneyAccountsViewBasePrivate
{
  Q_DECLARE_PUBLIC(KBudgetView)

public:
  explicit KBudgetViewPrivate(KBudgetView *qq) :
    q_ptr(qq),
    ui(new Ui::KBudgetView)
  {
  }

  ~KBudgetViewPrivate()
  {
  }

  void init()
  {
    Q_Q(KBudgetView);
    ui->setupUi(q);
    m_accountTree = &ui->m_accountTree;

    ui->m_budgetList->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_newButton->setIcon(Icons::get(Icon::BudgetNew));
    ui->m_renameButton->setIcon(Icons::get(Icon::BudgetRename));
    ui->m_deleteButton->setIcon(Icons::get(Icon::BudgetDelete));
    ui->m_updateButton->setIcon(Icons::get(Icon::DocumentSave));
    ui->m_resetButton->setIcon(Icons::get(Icon::EditUndo));
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    m_budgetProxyModel = qobject_cast<BudgetViewProxyModel *>(ui->m_accountTree->init(View::Budget));
    m_proxyModel = m_budgetProxyModel;

    q->connect(m_budgetProxyModel, &BudgetViewProxyModel::balanceChanged, q, &KBudgetView::slotBudgetBalanceChanged);

    q->connect(ui->m_accountTree, SIGNAL(selectObject(MyMoneyObject)), q, SLOT(slotSelectAccount(MyMoneyObject)));

    q->connect(ui->m_budgetList, &QWidget::customContextMenuRequested,
               q, &KBudgetView::slotOpenContextMenu);
    q->connect(ui->m_budgetList->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KBudgetView::slotSelectBudget);
    q->connect(ui->m_budgetList, &QTreeWidget::itemChanged, q, &KBudgetView::slotItemChanged);

    q->connect(ui->m_cbBudgetSubaccounts, &QAbstractButton::clicked, q, &KBudgetView::cb_includesSubaccounts_clicked);

    // connect the buttons to the actions. Make sure the enabled state
    // of the actions is reflected by the buttons
    q->connect(ui->m_renameButton, &QAbstractButton::clicked, kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetRename]), &QAction::trigger);
    q->connect(ui->m_deleteButton, &QAbstractButton::clicked, kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetDelete]), &QAction::trigger);

    q->connect(ui->m_budgetValue, &KBudgetValues::valuesChanged, q, &KBudgetView::slotBudgetedAmountChanged);

    q->connect(ui->m_newButton, &QAbstractButton::clicked, q, &KBudgetView::slotNewBudget);
    q->connect(ui->m_updateButton, &QAbstractButton::clicked, q, &KBudgetView::slotUpdateBudget);
    q->connect(ui->m_resetButton, &QAbstractButton::clicked, q, &KBudgetView::slotResetBudget);

    q->connect(ui->m_hideUnusedButton, &QAbstractButton::toggled, q, &KBudgetView::slotHideUnused);

    q->connect(ui->m_searchWidget, SIGNAL(textChanged(QString)), m_budgetProxyModel, SLOT(setFilterFixedString(QString)));

    // setup initial state
    ui->m_newButton->setEnabled(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetNew])->isEnabled());
    ui->m_renameButton->setEnabled(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetRename])->isEnabled());
    ui->m_deleteButton->setEnabled(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::BudgetDelete])->isEnabled());

    auto grp = KSharedConfig::openConfig()->group("Last Use Settings");
    ui->m_splitter->restoreState(grp.readEntry("KBudgetViewSplitterSize", QByteArray()));
    ui->m_splitter->setChildrenCollapsible(false);
  }

  KBudgetView          *q_ptr;
  Ui::KBudgetView      *ui;
  BudgetViewProxyModel *m_budgetProxyModel;
};

#endif
