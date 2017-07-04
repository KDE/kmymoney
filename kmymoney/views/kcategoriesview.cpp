/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
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

#include "kcategoriesview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QLayout>
#include <QList>
#include <QVBoxLayout>
#include <QPushButton>
#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <kguiitem.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "models.h"
#include <icons.h>

using namespace Icons;

KCategoriesView::KCategoriesView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview) :
    QWidget(nullptr),
    m_kmymoney(kmymoney),
    m_kmymoneyview(kmymoneyview),
    m_needReload(false),
    m_needLoad(true),
    m_haveUnusedCategories(false)
{
}

KCategoriesView::~KCategoriesView()
{
}

KRecursiveFilterProxyModel *KCategoriesView::getProxyModel()
{
  return m_filterProxyModel;
}

QList<AccountsModel::Columns> *KCategoriesView::getProxyColumns()
{
  return m_accountTree->getColumns(KMyMoneyView::View::Categories);
}

bool KCategoriesView::isLoaded()
{
  return !m_needLoad;
}

void KCategoriesView::init()
{
  m_needLoad = false;
  setupUi(this);

  // setup icons for collapse and expand button
  m_collapseButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListCollapse]));
  m_expandButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListExpand]));

  auto const model = Models::instance()->accountsModel();
  connect(model, &AccountsModel::profitChanged, this, &KCategoriesView::slotProfitChanged);

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);

//  const QVector<MyMoneyAccount::_accountTypeE> accGroups {MyMoneyAccount::Income, MyMoneyAccount::Expense};
  m_filterProxyModel->addAccountGroup(QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Income, MyMoneyAccount::Expense});

  m_filterProxyModel->init(model, getProxyColumns());
  m_filterProxyModel->setFilterKeyColumn(-1);

  m_accountTree->init(m_filterProxyModel, model->getColumns());

  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectInstitution(MyMoneyObject)));
  connect(m_accountTree, &KMyMoneyAccountTreeView::openContextMenu, m_kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), m_kmymoney, SLOT(slotAccountOpen(MyMoneyObject)));
  connect(this, SIGNAL(reparent(MyMoneyAccount,MyMoneyAccount)), m_kmymoney, SLOT(slotReparentAccount(MyMoneyAccount,MyMoneyAccount))); //TODO : nothing emits this signal

  connect(m_filterProxyModel, &AccountsFilterProxyModel::unusedIncomeExpenseAccountHidden, this, &KCategoriesView::slotUnusedIncomeExpenseAccountHidden);
  connect(m_searchWidget, &QLineEdit::textChanged, m_filterProxyModel, &QSortFilterProxyModel::setFilterFixedString);

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, &QTreeView::collapsed, m_filterProxyModel, &AccountsViewFilterProxyModel::collapsed);
  connect(m_accountTree, &QTreeView::expanded, m_filterProxyModel, &AccountsViewFilterProxyModel::expanded);
  connect(m_accountTree, &KMyMoneyAccountTreeView::columnToggled , m_kmymoneyview, &KMyMoneyView::slotAccountTreeViewChanged);

  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KCategoriesView::slotLoadAccounts);
  connect(m_collapseButton, &QAbstractButton::clicked, this, &KCategoriesView::slotExpandCollapse);
  connect(m_expandButton, &QAbstractButton::clicked, this, &KCategoriesView::slotExpandCollapse);

  // connect the two buttons to all required slots
  connect(m_collapseButton, &QAbstractButton::clicked, this, &KCategoriesView::slotExpandCollapse);
  connect(m_collapseButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::collapseAll);
  connect(m_accountTree, &KMyMoneyAccountTreeView::collapsedAll, m_filterProxyModel, &AccountsViewFilterProxyModel::collapseAll);
  connect(m_expandButton, &QAbstractButton::clicked, this, &KCategoriesView::slotExpandCollapse);
  connect(m_expandButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::expandAll);
  connect(m_accountTree, &KMyMoneyAccountTreeView::expandedAll, m_filterProxyModel, &AccountsViewFilterProxyModel::expandAll);
}

void KCategoriesView::slotExpandCollapse()
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}

void KCategoriesView::showEvent(QShowEvent * event)
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

void KCategoriesView::slotLoadAccounts()
{
  if (isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}

void KCategoriesView::loadAccounts()
{
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->isActionToggled(Action::ViewShowAll));

  // reinitialize the default state of the hidden categories label
  m_haveUnusedCategories = false;
  m_hiddenCategories->hide();
  m_filterProxyModel->setHideUnusedIncomeExpenseAccounts(kmymoney->isActionToggled(Action::ViewHideCategories));

  // and in case we need to show things expanded, we'll do so
  if (KMyMoneyGlobalSettings::showAccountsExpanded()) {
    m_accountTree->expandAll();
  }
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KCategoriesView::slotUnusedIncomeExpenseAccountHidden()
{
  m_haveUnusedCategories = true;
  m_hiddenCategories->setVisible(m_haveUnusedCategories);
}

void KCategoriesView::slotProfitChanged(const MyMoneyMoney &profit)
{
  m_kmymoneyview->slotNetBalProChanged(profit, m_totalProfitsLabel, KMyMoneyView::View::Categories);
}
