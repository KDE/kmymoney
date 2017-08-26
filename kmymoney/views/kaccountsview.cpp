/***************************************************************************
                          kaccountsview.cpp
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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

#include "kaccountsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QTabWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QPixmap>
#include <QLayout>
#include <QList>
#include <QPushButton>
#include <QIcon>
#include <QListWidget>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <kguiitem.h>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "kmymoneyaccounttreeview.h"
#include "models.h"
#include <icons.h>

using namespace Icons;

KAccountsView::KAccountsView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview) :
    QWidget(nullptr),
    m_kmymoney(kmymoney),
    m_kmymoneyview(kmymoneyview),
    m_needReload(false),
    m_needLoad(true)
{
}

KAccountsView::~KAccountsView()
{
}

KRecursiveFilterProxyModel *KAccountsView::getProxyModel()
{
  return m_filterProxyModel;
}

QList<AccountsModel::Columns> *KAccountsView::getProxyColumns()
{
  return m_accountTree->getColumns(KMyMoneyView::View::Accounts);
}

bool KAccountsView::isLoaded()
{
  return !m_needLoad;
}

void KAccountsView::init()
{
  m_needLoad = false;
  setupUi(this);

  // setup icons for collapse and expand button
  m_collapseButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListCollapse]));
  m_expandButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListExpand]));

  auto const model = Models::instance()->accountsModel();

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);

  QVector<MyMoneyAccount::_accountTypeE> accGroups {MyMoneyAccount::Asset, MyMoneyAccount::Liability, MyMoneyAccount::Equity};
  if (KMyMoneyGlobalSettings::showCategoriesInAccountsView())
    accGroups << MyMoneyAccount::Income << MyMoneyAccount::Expense;

  m_filterProxyModel->addAccountGroup(accGroups);

  m_filterProxyModel->init(model, getProxyColumns());
  m_filterProxyModel->setFilterKeyColumn(-1);
  m_accountTree->init(m_filterProxyModel, model->getColumns());

  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectInstitution(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectInvestment(MyMoneyObject)));
  connect(m_accountTree, &KMyMoneyAccountTreeView::openContextMenu, m_kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), m_kmymoney, SLOT(slotAccountOpen(MyMoneyObject)));

  connect(m_filterProxyModel, &AccountsFilterProxyModel::unusedIncomeExpenseAccountHidden, this, &KAccountsView::slotUnusedIncomeExpenseAccountHidden);
  connect(m_searchWidget, &QLineEdit::textChanged, m_filterProxyModel, &QSortFilterProxyModel::setFilterFixedString);

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, &QTreeView::collapsed, m_filterProxyModel, &AccountsViewFilterProxyModel::collapsed);
  connect(m_accountTree, &QTreeView::expanded, m_filterProxyModel, &AccountsViewFilterProxyModel::expanded);

  connect(m_accountTree, &KMyMoneyAccountTreeView::columnToggled , m_kmymoneyview, &KMyMoneyView::slotAccountTreeViewChanged);
  // connect the two buttons to all required slots
  connect(m_collapseButton, &QAbstractButton::clicked, this, &KAccountsView::slotExpandCollapse);
  connect(m_collapseButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::collapseAll);
  connect(m_accountTree, &KMyMoneyAccountTreeView::collapsedAll, m_filterProxyModel, &AccountsViewFilterProxyModel::collapseAll);
  connect(m_expandButton, &QAbstractButton::clicked, this, &KAccountsView::slotExpandCollapse);
  connect(m_expandButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::expandAll);
  connect(m_accountTree, &KMyMoneyAccountTreeView::expandedAll, m_filterProxyModel, &AccountsViewFilterProxyModel::expandAll);

  connect(model, &AccountsModel::netWorthChanged, this, &KAccountsView::slotNetWorthChanged);
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KAccountsView::slotLoadAccounts);
}

void KAccountsView::showEvent(QShowEvent * event)
{
  if (m_needLoad)
    init();

  m_kmymoney->slotResetSelections();

  if (m_needReload) {
    loadAccounts();
    m_needReload = false;
  }

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KAccountsView::slotExpandCollapse()
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KAccountsView::slotUnusedIncomeExpenseAccountHidden()
{
  m_haveUnusedCategories = true;
  m_hiddenCategories->setVisible(m_haveUnusedCategories);
}

void KAccountsView::slotLoadAccounts()
{
  if (isVisible())
    loadAccounts();
  else
    m_needReload = true;
}

void KAccountsView::loadAccounts()
{
  // TODO: check why the invalidate is needed here
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->isActionToggled(Action::ViewShowAll));
  m_filterProxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  if (KMyMoneyGlobalSettings::showCategoriesInAccountsView()) {
    m_filterProxyModel->addAccountGroup(QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Income, MyMoneyAccount::Expense});
  } else {
    m_filterProxyModel->removeAccountType(MyMoneyAccount::Income);
    m_filterProxyModel->removeAccountType(MyMoneyAccount::Expense);
  }

  // reinitialize the default state of the hidden categories label
  m_haveUnusedCategories = false;
  m_hiddenCategories->hide();
  m_filterProxyModel->setHideUnusedIncomeExpenseAccounts(kmymoney->isActionToggled(Action::ViewShowAll));

  // and in case we need to show things expanded, we'll do so
//  if (KMyMoneyGlobalSettings::showAccountsExpanded()) {
//    m_filterProxyModel->expandAll();
//    m_accountTree->expandAll();
//  }
}

void KAccountsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
  m_kmymoneyview->slotNetBalProChanged(netWorth, m_totalProfitsLabel, KMyMoneyView::View::Accounts);
}
