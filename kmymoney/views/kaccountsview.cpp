/***************************************************************************
                          kaccountsview.cpp
                             -------------------
    copyright            : (C) 2005 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
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
#include <kiconloader.h>
#include <kguiitem.h>
#include <KToggleAction>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "kmymoneyaccounttreeview.h"
#include "models.h"

KAccountsView::KAccountsView(QWidget *parent) :
    QWidget(parent)
{
  setupUi(this);

  // setup icons for collapse and expand button
  KGuiItem collapseGuiItem("",
                           QIcon::fromTheme("zoom-out"),
                           QString(),
                           QString());
  KGuiItem expandGuiItem("",
                         QIcon::fromTheme("zoom-in"),
                         QString(),
                         QString());
  KGuiItem::assign(m_collapseButton, collapseGuiItem);
  KGuiItem::assign(m_expandButton, expandGuiItem);

  connect(Models::instance()->accountsModel(), SIGNAL(netWorthChanged(MyMoneyMoney)), this, SLOT(slotNetWorthChanged(MyMoneyMoney)));

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Asset);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Liability);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Equity);
  if (KMyMoneyGlobalSettings::showCategoriesInAccountsView()) {
    m_filterProxyModel->addAccountGroup(MyMoneyAccount::Income);
    m_filterProxyModel->addAccountGroup(MyMoneyAccount::Expense);
  }
  m_filterProxyModel->setSourceModel(Models::instance()->accountsModel());
  m_filterProxyModel->setFilterKeyColumn(-1);

  connect(m_filterProxyModel, SIGNAL(unusedIncomeExpenseAccountHidden()), this, SLOT(slotUnusedIncomeExpenseAccountHidden()));

  m_accountTree->setModel(m_filterProxyModel);
  m_accountTree->setConfigGroupName("KAccountsView");
  m_accountTree->setAlternatingRowColors(true);
  m_accountTree->setIconSize(QSize(22, 22));
  m_accountTree->setSortingEnabled(true);


  connect(m_searchWidget, SIGNAL(textChanged(QString)), m_filterProxyModel, SLOT(setFilterFixedString(QString)));

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, SIGNAL(collapsed(QModelIndex)), m_filterProxyModel, SLOT(collapsed(QModelIndex)));
  connect(m_accountTree, SIGNAL(expanded(QModelIndex)), m_filterProxyModel, SLOT(expanded(QModelIndex)));

  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), this, SIGNAL(selectObject(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openContextMenu(MyMoneyObject)), this, SIGNAL(openContextMenu(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), this, SIGNAL(openObject(MyMoneyObject)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));

  // connect the two buttons to all required slots
  connect(m_collapseButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_collapseButton, SIGNAL(clicked()), m_accountTree, SLOT(collapseAll()));
  connect(m_accountTree, SIGNAL(collapsedAll()), m_filterProxyModel, SLOT(collapseAll()));
  connect(m_expandButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_expandButton, SIGNAL(clicked()), m_accountTree, SLOT(expandAll()));
  connect(m_accountTree, SIGNAL(expandedAll()), m_filterProxyModel, SLOT(expandAll()));
}

KAccountsView::~KAccountsView()
{
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
  // TODO: check why the invalidate is needed here
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->toggleAction("view_show_all_accounts")->isChecked());
  m_filterProxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  if (KMyMoneyGlobalSettings::showCategoriesInAccountsView()) {
    m_filterProxyModel->addAccountGroup(MyMoneyAccount::Income);
    m_filterProxyModel->addAccountGroup(MyMoneyAccount::Expense);
  } else {
    m_filterProxyModel->removeAccountType(MyMoneyAccount::Income);
    m_filterProxyModel->removeAccountType(MyMoneyAccount::Expense);
  }

  // reinitialize the default state of the hidden categories label
  m_haveUnusedCategories = false;
  m_hiddenCategories->hide();
  m_filterProxyModel->setHideUnusedIncomeExpenseAccounts(kmymoney->toggleAction("view_hide_unused_categories")->isChecked());

  // and in case we need to show things expanded, we'll do so
  if (KMyMoneyGlobalSettings::showAccountsExpanded()) {
    m_filterProxyModel->expandAll();
    m_accountTree->expandAll();
  }
}

void KAccountsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
{
  QString s(i18n("Net Worth: "));

  // FIXME figure out how to deal with the approximate
  // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
  //  s += "~ ";

  s.replace(QString(" "), QString("&nbsp;"));
  if (netWorth.isNegative()) {
    s += "<b><font color=\"red\">";
  }
  const MyMoneySecurity& sec = MyMoneyFile::instance()->baseCurrency();
  QString v(MyMoneyUtils::formatMoney(netWorth, sec));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if (netWorth.isNegative()) {
    s += "</font></b>";
  }

  m_totalProfitsLabel->setFont(KMyMoneyGlobalSettings::listCellFont());
  m_totalProfitsLabel->setText(s);
}

void KAccountsView::slotOpenContextMenu(MyMoneyAccount account)
{
  emit openContextMenu(account);
}

void KAccountsView::slotOpenObject(QListWidgetItem* item)
{
  if (item)
    emit openObject((item->data(Qt::UserRole)).value<MyMoneyAccount>());
}
