/***************************************************************************
                          kinstitutionsview.cpp
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

#include "kinstitutionsview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QTabWidget>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <KToggleAction>
// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "models.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"

KInstitutionsView::KInstitutionsView(QWidget *parent) :
    QWidget(parent),
    m_needReload(false)
{
  setupUi(this);

  // setup icons for collapse and expand button
  KGuiItem collapseGuiItem("",
                           KIcon("zoom-out"),
                           QString(),
                           QString());
  KGuiItem expandGuiItem("",
                         KIcon("zoom-in"),
                         QString(),
                         QString());
  m_collapseButton->setGuiItem(collapseGuiItem);
  m_expandButton->setGuiItem(expandGuiItem);

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Asset);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Liability);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Equity);
  m_filterProxyModel->setSourceModel(Models::instance()->institutionsModel());
  m_filterProxyModel->setFilterKeyColumn(-1);

  m_accountTree->setConfigGroupName("KInstitutionsView");
  m_accountTree->setAlternatingRowColors(true);
  m_accountTree->setIconSize(QSize(22, 22));
  m_accountTree->setSortingEnabled(true);
  m_accountTree->setModel(m_filterProxyModel);

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, SIGNAL(collapsed(QModelIndex)), m_filterProxyModel, SLOT(collapsed(QModelIndex)));
  connect(m_accountTree, SIGNAL(expanded(QModelIndex)), m_filterProxyModel, SLOT(expanded(QModelIndex)));
  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), this, SIGNAL(selectObject(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openContextMenu(MyMoneyObject)), this, SIGNAL(openContextMenu(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), this, SIGNAL(openObject(MyMoneyObject)));

  // connect the two buttons to all required slots
  connect(m_collapseButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_collapseButton, SIGNAL(clicked()), m_accountTree, SLOT(collapseAll()));
  connect(m_collapseButton, SIGNAL(clicked()), m_filterProxyModel, SLOT(collapseAll()));
  connect(m_expandButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_expandButton, SIGNAL(clicked()), m_accountTree, SLOT(expandAll()));
  connect(m_expandButton, SIGNAL(clicked()), m_filterProxyModel, SLOT(expandAll()));

  connect(m_searchWidget, SIGNAL(textChanged(QString)), m_filterProxyModel, SLOT(setFilterFixedString(QString)));

  connect(Models::instance()->accountsModel(), SIGNAL(netWorthChanged(MyMoneyMoney)), this, SLOT(slotNetWorthChanged(MyMoneyMoney)));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));
}

KInstitutionsView::~KInstitutionsView()
{
}

void KInstitutionsView::showEvent(QShowEvent * event)
{
  emit aboutToShow();

  if (m_needReload) {
    loadAccounts();
    m_needReload = false;
  }

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KInstitutionsView::slotLoadAccounts(void)
{
  if (isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}

void KInstitutionsView::loadAccounts(void)
{
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->toggleAction("view_show_all_accounts")->isChecked());
}

void KInstitutionsView::slotNetWorthChanged(const MyMoneyMoney &netWorth)
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

void KInstitutionsView::slotExpandCollapse(void)
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}

#include "kinstitutionsview.moc"
