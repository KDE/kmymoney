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

#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <KToggleAction>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "models.h"

KCategoriesView::KCategoriesView(QWidget *parent) :
    QWidget(parent),
    m_needReload(false),
    m_haveUnusedCategories(false)
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

  connect(Models::instance()->accountsModel(), SIGNAL(profitChanged(MyMoneyMoney)), this, SLOT(slotProfitChanged(MyMoneyMoney)));

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Income);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Expense);
  m_filterProxyModel->setSourceModel(Models::instance()->accountsModel());
  m_filterProxyModel->setFilterKeyColumn(-1);

  connect(m_filterProxyModel, SIGNAL(unusedIncomeExpenseAccountHidden()), this, SLOT(slotUnusedIncomeExpenseAccountHidden()));

  m_accountTree->setModel(m_filterProxyModel);
  m_accountTree->setConfigGroupName("KCategoriesView");
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
  connect(m_collapseButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_expandButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));

  // connect the two buttons to all required slots
  connect(m_collapseButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_collapseButton, SIGNAL(clicked()), m_accountTree, SLOT(collapseAll()));
  connect(m_accountTree, SIGNAL(collapsedAll()), m_filterProxyModel, SLOT(collapseAll()));
  connect(m_expandButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_expandButton, SIGNAL(clicked()), m_accountTree, SLOT(expandAll()));
  connect(m_accountTree, SIGNAL(expandedAll()), m_filterProxyModel, SLOT(expandAll()));
}

KCategoriesView::~KCategoriesView()
{
}

void KCategoriesView::slotExpandCollapse(void)
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}

void KCategoriesView::showEvent(QShowEvent * event)
{
  emit aboutToShow();

  if (m_needReload) {
    loadAccounts();
    m_needReload = false;
  }

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KCategoriesView::slotLoadAccounts(void)
{
  if (isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}

void KCategoriesView::loadAccounts(void)
{
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->toggleAction("view_show_all_accounts")->isChecked());

  // reinitialize the default state of the hidden categories label
  m_haveUnusedCategories = false;
  m_hiddenCategories->hide();
  m_filterProxyModel->setHideUnusedIncomeExpenseAccounts(kmymoney->toggleAction("view_hide_unused_categories")->isChecked());

  // and in case we need to show things expanded, we'll do so
  if (KMyMoneyGlobalSettings::showAccountsExpanded()) {
    m_accountTree->expandAll();
  }
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KCategoriesView::slotUnusedIncomeExpenseAccountHidden(void)
{
  m_haveUnusedCategories = true;
  m_hiddenCategories->setVisible(m_haveUnusedCategories);
}

void KCategoriesView::slotProfitChanged(const MyMoneyMoney &profit)
{
  QString s(i18n("Profit: "));
  if (profit.isNegative())
    s = i18n("Loss: ");

  // FIXME figure out how to deal with the approximate
  // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
  //  s += "~ ";

  s.replace(QString(" "), QString("&nbsp;"));
  if (profit.isNegative()) {
    s += "<b><font color=\"red\">";
  }
  const MyMoneySecurity& sec = MyMoneyFile::instance()->baseCurrency();
  QString v(MyMoneyUtils::formatMoney(profit.abs(), sec));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if (profit.isNegative()) {
    s += "</font></b>";
  }

  m_totalProfitsLabel->setFont(KMyMoneyGlobalSettings::listCellFont());
  m_totalProfitsLabel->setText(s);
}
