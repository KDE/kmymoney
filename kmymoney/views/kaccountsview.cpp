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

// ----------------------------------------------------------------------------
// KDE Includes

#include <KListWidget>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <KToggleAction>

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
                           KIcon("zoom-out"),
                           QString(),
                           QString());
  KGuiItem expandGuiItem("",
                         KIcon("zoom-in"),
                         QString(),
                         QString());
  m_collapseButton->setGuiItem(collapseGuiItem);
  m_expandButton->setGuiItem(expandGuiItem);

  for (int i = 0; i < MaxViewTabs; ++i)
    m_needReload[i] = false;

  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  m_tab->setCurrentIndex(grp.readEntry("KAccountsView_LastType", 0));

  connect(m_tab, SIGNAL(currentChanged(int)), this, SLOT(slotTabCurrentChanged(int)));

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

  m_assetsGroup->setLayout(m_assetsListLayout);
  m_liabilitiesGroup->setLayout(m_liabilitiesListLayout);
  m_equitiesGroup->setLayout(m_equitiesListLayout);

  m_assetsList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_assetsList->setWordWrap(true);
  m_liabilitiesList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_liabilitiesList->setWordWrap(true);
  m_equitiesList->setContextMenuPolicy(Qt::CustomContextMenu);
  m_equitiesList->setWordWrap(true);

  connect(m_searchWidget, SIGNAL(textChanged(QString)), m_filterProxyModel, SLOT(setFilterFixedString(QString)));

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, SIGNAL(collapsed(QModelIndex)), m_filterProxyModel, SLOT(collapsed(QModelIndex)));
  connect(m_accountTree, SIGNAL(expanded(QModelIndex)), m_filterProxyModel, SLOT(expanded(QModelIndex)));

  connect(m_accountTree, SIGNAL(selectObject(MyMoneyObject)), this, SIGNAL(selectObject(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openContextMenu(MyMoneyObject)), this, SIGNAL(openContextMenu(MyMoneyObject)));
  connect(m_accountTree, SIGNAL(openObject(MyMoneyObject)), this, SIGNAL(openObject(MyMoneyObject)));

  connect(m_assetsList, SIGNAL(itemSelectionChanged()), this, SLOT(slotAssetsSelectIcon()));
  connect(m_assetsList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotAssetsOpenContextMenu(QPoint)));
  connect(m_assetsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotOpenObject(QListWidgetItem*)));

  connect(m_liabilitiesList, SIGNAL(itemSelectionChanged()), this, SLOT(slotLiabilitiesSelectIcon()));
  connect(m_liabilitiesList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotLiabilitiesOpenContextMenu(QPoint)));
  connect(m_liabilitiesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotOpenObject(QListWidgetItem*)));

  connect(m_equitiesList, SIGNAL(itemSelectionChanged()), this, SLOT(slotEquitiesSelectIcon()));
  connect(m_equitiesList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotEquitiesOpenContextMenu(QPoint)));
  connect(m_equitiesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotOpenObject(QListWidgetItem*)));

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
  m_needReload[ListView] = true;
  m_needReload[IconView] = true;
  if (isVisible())
    slotTabCurrentChanged(m_tab->currentIndex());
}

void KAccountsView::slotTabCurrentChanged(int index)
{
  AccountsViewTab tab = static_cast<AccountsViewTab>(index);

  // remember this setting for startup
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KAccountsView_LastType", QVariant(tab).toString());

  loadAccounts(tab);

  switch (tab) {
    case ListView:
      // update the hint if categories are hidden
      m_hiddenCategories->setVisible(m_haveUnusedCategories);
      m_searchWidget->setEnabled(true);
      m_collapseButton->setEnabled(true);
      m_expandButton->setEnabled(true);
      break;

    case IconView:
      m_hiddenCategories->hide();
      m_searchWidget->setEnabled(false);
      m_searchWidget->setText(QString());
      m_collapseButton->setEnabled(false);
      m_expandButton->setEnabled(false);
      break;

    default:
      break;
  }

  QListWidgetItem* iconItem = selectedIcon();

  emit selectObject(MyMoneyAccount());
  switch (static_cast<AccountsViewTab>(m_tab->currentIndex())) {
    case ListView: {
        QModelIndexList selectedIndexes = m_accountTree->selectionModel()->selectedIndexes();
        if (!selectedIndexes.empty()) {
          QVariant data = m_accountTree->model()->data(selectedIndexes.front(), AccountsModel::AccountRole);
          if (data.isValid()) {
            emit selectObject(data.value<MyMoneyAccount>());
          }
        }
      }
      break;

    case IconView:
      if (iconItem) {
        emit selectObject((iconItem->data(Qt::UserRole)).value<MyMoneyAccount>());
      }
      break;

    default:
      break;
  }
}

void KAccountsView::showEvent(QShowEvent * event)
{
  emit aboutToShow();

  slotTabCurrentChanged(m_tab->currentIndex());

  QWidget::showEvent(event);
}

void KAccountsView::loadAccounts(AccountsViewTab tab)
{
  if (m_needReload[tab]) {
    switch (tab) {
      case ListView:
        loadListView();
        break;
      case IconView:
        loadIconGroups();
        break;
      default:
        break;
    }
    m_needReload[tab] = false;
  }
}

void KAccountsView::loadListView()
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

//FIXME: This will be deprecated once the lists use the accounts model
void KAccountsView::slotReconcileAccount(const MyMoneyAccount& acc, const QDate& /*reconciliationDate*/, const MyMoneyMoney& /*endingBalance*/)
{
  //call all lists
  slotReconcileAccount(m_assetsList, acc);
  slotReconcileAccount(m_liabilitiesList, acc);
  if (KMyMoneyGlobalSettings::expertMode()) {
    slotReconcileAccount(m_equitiesList, acc);
  }

}

void KAccountsView::slotReconcileAccount(KListWidget* list, const MyMoneyAccount& acc)
{
  //scan all the items in the list and set the flag
  for (int i = 0; i < list->count(); ++i) {
    //compare the id of the account to the items and set the reconcile flag accordingly
    QListWidgetItem* item = list->item(i);
    MyMoneyAccount itemAccount = (item->data(Qt::UserRole)).value<MyMoneyAccount>();
    item->setIcon(QIcon(itemAccount.accountPixmap(itemAccount.id() == acc.id())));
    item->setData(reconcileRole, QVariant(itemAccount.id() == acc.id()));
  }
  m_reconciliationAccount = acc;
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

QListWidgetItem* KAccountsView::selectedIcon() const
{
  if (m_assetsList->currentItem())
    return m_assetsList->currentItem();
  else if (m_liabilitiesList->currentItem())
    return m_liabilitiesList->currentItem();
  else
    return m_equitiesList->currentItem();
}

void KAccountsView::slotAssetsSelectIcon()
{
  QList<QListWidgetItem*> selectedItems = m_assetsList->selectedItems();
  if (selectedItems.count() > 0) {
    slotSelectIcon(selectedItems.at(0));
  }
}

void KAccountsView::slotLiabilitiesSelectIcon()
{
  QList<QListWidgetItem*> selectedItems = m_liabilitiesList->selectedItems();
  if (selectedItems.count() > 0) {
    slotSelectIcon(selectedItems.at(0));
  }
}

void KAccountsView::slotEquitiesSelectIcon()
{
  QList<QListWidgetItem*> selectedItems = m_equitiesList->selectedItems();
  if (selectedItems.count() > 0) {
    slotSelectIcon(selectedItems.at(0));
  }
}

void KAccountsView::slotSelectIcon(QListWidgetItem* item)
{
  emit selectObject((item->data(Qt::UserRole)).value<MyMoneyAccount>());
}

void KAccountsView::slotAssetsOpenContextMenu(const QPoint& point)
{
  QListWidgetItem* item = m_assetsList->itemAt(point);
  if (item)
    slotOpenContextMenu((item->data(Qt::UserRole)).value<MyMoneyAccount>());
}

void KAccountsView::slotLiabilitiesOpenContextMenu(const QPoint& point)
{
  QListWidgetItem* item = m_liabilitiesList->itemAt(point);
  if (item)
    slotOpenContextMenu((item->data(Qt::UserRole)).value<MyMoneyAccount>());
}

void KAccountsView::slotEquitiesOpenContextMenu(const QPoint& point)
{
  QListWidgetItem* item = m_equitiesList->itemAt(point);
  if (item)
    slotOpenContextMenu((item->data(Qt::UserRole)).value<MyMoneyAccount>());
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


/*void KAccountsView::slotUpdateIconPos(unsigned int action)
{
  if (action != KMyMoneyView::preSave)
    return;

  MyMoneyFileTransaction ft;
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
  for (; p; p = dynamic_cast<KMyMoneyAccountIconItem*>(p->nextItem())) {
    const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(p->itemObject());
    if (acc.value("kmm-iconpos") != point(p->pos())) {
      MyMoneyAccount a(acc);
      a.setValue("kmm-iconpos", point(p->pos()));
      try {
        MyMoneyFile::instance()->modifyAccount(a);
      } catch (const MyMoneyException &e) {
        kDebug(2) << "Unable to update icon pos: " << e.what();
      }
    }
  }
  ft.commit();
}*/

void KAccountsView::loadIconGroups()
{
  MyMoneyFile* file = MyMoneyFile::instance();

  //load list of asset accounts
  MyMoneyAccount assetAccount = file->asset();
  m_assetsList->clear();
  loadAccountIconsIntoList(assetAccount, m_assetsList);

  //load list of liability accounts
  MyMoneyAccount liabilityAccount = file->liability();
  m_liabilitiesList->clear();
  loadAccountIconsIntoList(liabilityAccount, m_liabilitiesList);

  //load list of equity accounts only if in expert mode
  if (KMyMoneyGlobalSettings::expertMode()) {
    MyMoneyAccount equityAccount = file->equity();
    m_equitiesList->clear();
    loadAccountIconsIntoList(equityAccount, m_equitiesList);
  } else {
    m_equitiesGroup->hide();
  }
}

void KAccountsView::loadAccountIconsIntoList(const MyMoneyAccount& parentAccount, KListWidget* listWidget)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  bool showClosedAccounts = kmymoney->toggleAction("view_show_all_accounts")->isChecked()
                            || !KMyMoneyGlobalSettings::hideClosedAccounts();

  //get the subaccounts
  QStringList subAccountsId = parentAccount.accountList();
  QList<MyMoneyAccount> subAccountsList;

  //go over each subaccount and add it and get subaccounts if it has any
  QStringList::const_iterator it_string;
  for (it_string = subAccountsId.constBegin(); it_string != subAccountsId.constEnd(); ++it_string) {
    MyMoneyAccount account = file->account((*it_string));

    //if it is already on the list continue with the next one
    if (subAccountsList.contains(account))
      continue;

    //add the account to the list and check if it has subaccounts
    subAccountsList.append(account);
    QStringList subAccounts = account.accountList();

    //if it has subaccounts, add them and then start from scratch
    if (subAccounts.size() > 0) {
      subAccountsId.append(subAccounts);
      it_string = subAccountsId.constBegin();
    }
  }

  //once all accounts are on the list, add them to listWidget
  QList<MyMoneyAccount>::const_iterator it_a;
  for (it_a = subAccountsList.constBegin(); it_a != subAccountsList.constEnd(); ++it_a) {
    //do not add investment accounts
    if ((*it_a).isInvest())
      continue;

    //check whether it is a closed account and it should be shown
    if ((*it_a).isClosed() && !showClosedAccounts)
      continue;

    QListWidgetItem* accountItem = new QListWidgetItem;
    accountItem->setText((*it_a).name());
    accountItem->setData(Qt::UserRole, QVariant::fromValue((*it_a)));
    accountItem->setIcon(QIcon((*it_a).accountPixmap()));
    accountItem->setData(reconcileRole, QVariant(false));
    listWidget->addItem(accountItem);
  }
}
