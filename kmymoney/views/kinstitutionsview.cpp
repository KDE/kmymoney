/***************************************************************************
                          kinstitutionsview.cpp
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

#include "kinstitutionsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QTabWidget>
#include <QList>
#include <QIcon>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "models.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include <icons.h>

using namespace Icons;

KInstitutionsView::KInstitutionsView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview) :
    QWidget(nullptr),
    m_kmymoney(kmymoney),
    m_kmymoneyview(kmymoneyview),
    m_needReload(false),
    m_needLoad(true)
{
}

KInstitutionsView::~KInstitutionsView()
{
}

KRecursiveFilterProxyModel *KInstitutionsView::getProxyModel()
{
  return m_filterProxyModel;
}

QList<AccountsModel::Columns> *KInstitutionsView::getProxyColumns()
{
  return m_accountTree->getColumns(KMyMoneyView::View::Institutions);
}

void KInstitutionsView::setDefaultFocus()
{
  QTimer::singleShot(0, m_accountTree, SLOT(setFocus()));
}

bool KInstitutionsView::isLoaded()
{
  return !m_needLoad;
}

void KInstitutionsView::init()
{
  m_needLoad = false;
  setupUi(this);

  // setup icons for collapse and expand button
  m_collapseButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListCollapse]));
  m_expandButton->setIcon(QIcon::fromTheme(g_Icons[Icon::ListExpand]));

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(QVector<MyMoneyAccount::_accountTypeE> {MyMoneyAccount::Asset, MyMoneyAccount::Liability, MyMoneyAccount::Equity});
  auto const model = Models::instance()->institutionsModel();
  m_filterProxyModel->init(model, getProxyColumns());
  m_filterProxyModel->setFilterKeyColumn(-1);
  m_accountTree->init(m_filterProxyModel, model->getColumns());

  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), m_kmymoney, SLOT(slotSelectInstitution(MyMoneyObject)));
  connect(m_accountTree, &KMyMoneyAccountTreeView::openContextMenu, m_kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);
  connect(m_accountTree, SIGNAL(openContextMenu(MyMoneyObject)), m_kmymoney, SLOT(slotShowInstitutionContextMenu(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), m_kmymoney, SLOT(slotInstitutionEdit(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), m_kmymoney, SLOT(slotAccountOpen(MyMoneyObject)));

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, &QTreeView::collapsed, m_filterProxyModel, &AccountsViewFilterProxyModel::collapsed);
  connect(m_accountTree, &QTreeView::expanded, m_filterProxyModel, &AccountsViewFilterProxyModel::expanded);
  connect(m_accountTree, &KMyMoneyAccountTreeView::columnToggled , m_kmymoneyview, &KMyMoneyView::slotAccountTreeViewChanged);

  // connect the two buttons to all required slots
  connect(m_collapseButton, &QAbstractButton::clicked, this, &KInstitutionsView::slotExpandCollapse);
  connect(m_collapseButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::collapseAll);
  connect(m_collapseButton, &QAbstractButton::clicked, m_filterProxyModel, &AccountsViewFilterProxyModel::collapseAll);
  connect(m_expandButton, &QAbstractButton::clicked, this, &KInstitutionsView::slotExpandCollapse);
  connect(m_expandButton, &QAbstractButton::clicked, m_accountTree, &KMyMoneyAccountTreeView::expandAll);
  connect(m_expandButton, &QAbstractButton::clicked, m_filterProxyModel, &AccountsViewFilterProxyModel::expandAll);

  connect(m_searchWidget, &QLineEdit::textChanged, m_filterProxyModel, &QSortFilterProxyModel::setFilterFixedString);

  connect(model, &AccountsModel::netWorthChanged, this, &KInstitutionsView::slotNetWorthChanged);
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KInstitutionsView::slotLoadAccounts);
}

void KInstitutionsView::showEvent(QShowEvent * event)
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

void KInstitutionsView::slotLoadAccounts()
{
  if (isVisible())
    loadAccounts();
  else
    m_needReload = true;
}

void KInstitutionsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
  m_kmymoneyview->slotNetBalProChanged(netWorth, m_totalProfitsLabel, KMyMoneyView::View::Institutions);
}

void KInstitutionsView::loadAccounts()
{
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->isActionToggled(Action::ViewShowAll));
}

void KInstitutionsView::slotExpandCollapse()
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}
