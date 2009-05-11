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

// ----------------------------------------------------------------------------
// QT Includes

#include <qlabel.h>
#include <qtabwidget.h>
#include <qpixmap.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3VBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <k3iconview.h>
#include <kguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <kmymoney/mymoneyfile.h>
#include "kaccountsview.h"
#include "kmymoneyview.h"
#include "../widgets/klistviewsearchline.h"
#include "../kmymoneyglobalsettings.h"
#include "../kmymoney2.h"


KMyMoneyAccountIconItem::KMyMoneyAccountIconItem(Q3IconView *parent, const MyMoneyAccount& account) :
  K3IconViewItem(parent, account.name()),
  m_account(account),
  m_reconcileFlag(false)
{
  updateAccount(account);
}

KMyMoneyAccountIconItem::~KMyMoneyAccountIconItem()
{
}

void KMyMoneyAccountIconItem::setReconciliation(bool on)
{
  if(m_reconcileFlag == on)
    return;
  m_reconcileFlag = on;
  updateAccount(m_account);
}

void KMyMoneyAccountIconItem::updateAccount(const MyMoneyAccount& account)
{
  setPixmap(account.accountPixmap(m_reconcileFlag));
}

KAccountsView::KAccountsView(QWidget *parent, const char *name) :
  KAccountsViewDecl(parent,name),
  m_assetItem(0),
  m_liabilityItem(0)
{
  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new K3ListViewSearchLineWidget(m_accountTree, m_accountTree->parentWidget());
  Q3VBoxLayout* layout = dynamic_cast<Q3VBoxLayout*>(m_accountTree->parentWidget()->layout());
  if(layout) {
    layout->insertWidget(0, m_searchWidget);
  }

  // setup icons for collapse and expand button
  KIconLoader *ic = KIconLoader::global();
  KGuiItem collapseGuiItem("",
                          QIcon(ic->loadIcon("viewmag-", KIcon::Small, KIconLoader::SizeSmall)),
                          QString(),
                          QString());
  KGuiItem expandGuiItem("",
                          QIcon(ic->loadIcon("viewmag+", KIcon::Small, KIconLoader::SizeSmall)),
                          QString(),
                          QString());
  m_collapseButton->setGuiItem(collapseGuiItem);
  m_expandButton->setGuiItem(expandGuiItem);

  for(int i=0; i < MaxViewTabs; ++i)
    m_needReload[i] = false;

  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  m_tab->setCurrentPage(config->readNumEntry("KAccountsView_LastType", 0));

  connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabChanged(QWidget*)));

  connect(m_accountTree, SIGNAL(selectObject(const MyMoneyObject&)), this, SIGNAL(selectObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(openContextMenu(const MyMoneyObject&)), this, SIGNAL(openContextMenu(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(valueChanged(void)), this, SLOT(slotUpdateNetWorth(void)));
  connect(m_accountTree, SIGNAL(openObject(const MyMoneyObject&)), this, SIGNAL(openObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)), this, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)));

  connect(m_accountIcons, SIGNAL(selectionChanged(Q3IconViewItem*)), this, SLOT(slotSelectIcon(Q3IconViewItem*)));
  connect(m_accountIcons, SIGNAL(rightButtonClicked(Q3IconViewItem*, const QPoint&)), this, SLOT(slotOpenContext(Q3IconViewItem*)));
  connect(m_accountIcons, SIGNAL(executed(Q3IconViewItem*)), this, SLOT(slotOpenObject(Q3IconViewItem*)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));

}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::slotLoadAccounts(void)
{
  m_needReload[ListView] = true;
  m_needReload[IconView] = true;
  if(isVisible())
    slotTabChanged(m_tab->currentPage());
}

void KAccountsView::slotTabChanged(QWidget* _tab)
{
  AccountsViewTab tab = static_cast<AccountsViewTab>(m_tab->indexOf(_tab));

  // remember this setting for startup
  KConfig *config = KGlobal::config();
  config->setGroup("Last Use Settings");
  config->writeEntry("KAccountsView_LastType", tab);

  loadAccounts(tab);

  switch(tab) {
    case ListView:
      // update the hint if categories are hidden
      m_hiddenCategories->setShown(m_haveUnusedCategories);
      break;

    case IconView:
      m_hiddenCategories->hide();
      break;

    default:
      break;
  }

  KMyMoneyAccountTreeBaseItem* treeItem = m_accountTree->selectedItem();
  KMyMoneyAccountIconItem* iconItem = selectedIcon();

  emit selectObject(MyMoneyAccount());
  switch(static_cast<AccountsViewTab>(m_tab->indexOf(m_tab->currentPage()))) {
    case ListView:
      // if we have a selected account, let the application know about it
      if(treeItem) {
        emit selectObject(treeItem->itemObject());
      }
      break;

    case IconView:
      if(iconItem) {
        emit selectObject(iconItem->itemObject());
      }
      break;

    default:
      break;
  }
}

void KAccountsView::show(void)
{
  // don't forget base class implementation
  KAccountsViewDecl::show();
  slotTabChanged(m_tab->currentPage());
}

void KAccountsView::polish(void)
{
  // don't forget base class implementation
  KAccountsViewDecl::polish();
  m_accountTree->setResizeMode(Q3ListView::LastColumn);
  m_accountTree->restoreLayout("Account View Settings");
}

void KAccountsView::loadAccounts(AccountsViewTab tab)
{
  if(m_needReload[tab]) {
    switch(tab) {
      case ListView:
        loadListView();
        break;
      case IconView:
        loadIconView();
        break;
      default:
        break;
    }
    m_needReload[tab] = false;
  }
}

void KAccountsView::loadIconView(void)
{
  ::timetrace("start load accounts icon view");

  // remember the positions of the icons
  QMap<QString, QPoint> posMap;
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
  for(;p; p = dynamic_cast<KMyMoneyAccountIconItem*>(p->nextItem()))
    posMap[p->itemObject().id()] = p->pos();

  // turn off updates to avoid flickering during reload
  m_accountIcons->setUpdatesEnabled(false);
  m_accountIcons->setAutoArrange(true);

  // clear the current contents and recreate it
  m_accountIcons->clear();
  QMap<QString, MyMoneyAccount> accountMap;

  MyMoneyFile* file = MyMoneyFile::instance();

  // get account list and sort by name
  Q3ValueList<MyMoneyAccount> alist;
  file->accountList(alist);
  Q3ValueList<MyMoneyAccount>::const_iterator it_a;
  for(it_a = alist.begin(); it_a != alist.end(); ++it_a) {
    accountMap[QString("%1-%2").arg((*it_a).name()).arg((*it_a).id())] = *it_a;
  }

  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked()
      || !KMyMoneyGlobalSettings::hideClosedAccounts();

  // parse list and add all asset and liability accounts
  QMap<QString, MyMoneyAccount>::const_iterator it;
  QPoint loc;
  for(it = accountMap.begin(); it != accountMap.end(); ++it) {
    if((*it).isClosed() && !showClosedAccounts)
      continue;
    const QString& pos = (*it).value("kmm-iconpos");
    KMyMoneyAccountIconItem* item;
    switch((*it).accountGroup()) {
      case MyMoneyAccount::Equity:
        if(!KMyMoneyGlobalSettings::expertMode())
          continue;
        // tricky fall through here

      case MyMoneyAccount::Asset:
      case MyMoneyAccount::Liability:
        // don't show stock accounts
        if((*it).isInvest())
          continue;

        // if we have a position stored with the object and no other
        // idea of it's current position, then take the one
        // stored inside the object. Also, turn off auto arrangement
        if(!pos.isEmpty() && posMap[(*it).id()] == QPoint()) {
          posMap[(*it).id()] = point(pos);
        }

        loc = posMap[(*it).id()];
        if(loc != QPoint()) {
          m_accountIcons->setAutoArrange(false);
        }

        item = new KMyMoneyAccountIconItem(m_accountIcons, *it);
        if((*it).id() == m_reconciliationAccount.id())
          item->setReconciliation(true);

        if(loc != QPoint()) {
          item->move(loc);
        }
        break;

      default:
        break;
    }
  }

  // clear the current contents
  m_securityMap.clear();
  m_transactionCountMap.clear();

  m_accountIcons->setAutoArrange(false);
  // turn updates back on
  m_accountIcons->setUpdatesEnabled(true);
  m_accountIcons->repaintContents();

  ::timetrace("done load accounts icon view");
}

void KAccountsView::loadListView(void)
{
  QMap<QString, bool> isOpen;

  ::timetrace("start load accounts list view");
  // remember the id of the current selected item
  KMyMoneyAccountTreeBaseItem *item = m_accountTree->selectedItem();
  QString selectedItemId = (item) ? item->id() : QString();

  // keep a map of all 'expanded' accounts
  Q3ListViewItemIterator it_lvi(m_accountTree);
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
    if(item && item->isOpen()) {
      isOpen[item->id()] = true;
    }
    ++it_lvi;
  }

  // remember the upper left corner of the viewport
  QPoint startPoint = m_accountTree->viewportToContents(QPoint(0, 0));

  // turn off updates to avoid flickering during reload
  m_accountTree->setUpdatesEnabled(false);

  // clear the current contents and recreate it
  m_accountTree->clear();
  m_securityMap.clear();
  m_transactionCountMap.clear();

  // make sure, the pointers are not pointing to some deleted object
  m_assetItem = m_liabilityItem = 0;

  MyMoneyFile* file = MyMoneyFile::instance();

  Q3ValueList<MyMoneySecurity> slist = file->currencyList();
  slist += file->securityList();
  Q3ValueList<MyMoneySecurity>::const_iterator it_s;
  for(it_s = slist.begin(); it_s != slist.end(); ++it_s) {
    m_securityMap[(*it_s).id()] = *it_s;
  }
  m_transactionCountMap = file->transactionCountMap();

  m_haveUnusedCategories = false;

  // create the items
  try {
    const MyMoneySecurity security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    const MyMoneyAccount& asset = file->asset();
    m_assetItem = new KMyMoneyAccountTreeItem(m_accountTree, asset, security, i18n("Asset"));
    loadSubAccounts(m_assetItem, asset.accountList());

    const MyMoneyAccount& liability = file->liability();
    m_liabilityItem = new KMyMoneyAccountTreeItem(m_accountTree, liability, security, i18n("Liability"));
    loadSubAccounts(m_liabilityItem, liability.accountList());

    const MyMoneyAccount& income = file->income();
    KMyMoneyAccountTreeItem *incomeItem = new KMyMoneyAccountTreeItem(m_accountTree, income, security, i18n("Income"));
    m_haveUnusedCategories |= loadSubAccounts(incomeItem, income.accountList());

    const MyMoneyAccount& expense = file->expense();
    KMyMoneyAccountTreeItem *expenseItem = new KMyMoneyAccountTreeItem(m_accountTree, expense, security, i18n("Expense"));
    m_haveUnusedCategories |= loadSubAccounts(expenseItem, expense.accountList());

    if(KMyMoneyGlobalSettings::expertMode()) {
      const MyMoneyAccount equity = file->equity();
      KMyMoneyAccountTreeItem *equityItem = new KMyMoneyAccountTreeItem(m_accountTree, equity, security, i18n("Equity"));
      loadSubAccounts(equityItem, equity.accountList());
    }

  } catch(MyMoneyException *e) {
    kDebug(2) << "Problem in accounts list view: " << e->what();
    delete e;
  }

  // scan through the list of accounts and re-expand those that were
  // expanded and re-select the one that was probably selected before
  it_lvi = Q3ListViewItemIterator(m_accountTree);
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
    if(item) {
      if(item->id() == selectedItemId)
        m_accountTree->setSelected(item, true);
      if(isOpen.find(item->id()) != isOpen.end())
        item->setOpen(true);
    }
    ++it_lvi;
  }

  // reposition viewport
  m_accountTree->setContentsPos(startPoint.x(), startPoint.y());

  m_searchWidget->searchLine()->updateSearch(QString::null);

  // turn updates back on
  m_accountTree->setUpdatesEnabled(true);
  m_accountTree->repaintContents();

  // clear the current contents
  m_securityMap.clear();
  m_transactionCountMap.clear();
  ::timetrace("done load accounts list view");
}

bool KAccountsView::loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QStringList& accountList)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  bool unused = false;
  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked()
                         || !KMyMoneyGlobalSettings::hideClosedAccounts();

  QStringList::const_iterator it_a;
  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    const MyMoneyAccount& acc = file->account(*it_a);
    Q3ValueList<MyMoneyPrice> prices;
    MyMoneySecurity security = file->baseCurrency();
    try {
      if(acc.isInvest()) {
        security = m_securityMap[acc.currencyId()];
        prices += file->price(acc.currencyId(), security.tradingCurrency());
        if(security.tradingCurrency() != file->baseCurrency().id()) {
          MyMoneySecurity sec = m_securityMap[security.tradingCurrency()];
          prices += file->price(sec.id(), file->baseCurrency().id());
        }
      } else if(acc.currencyId() != file->baseCurrency().id()) {
        if(acc.currencyId() != file->baseCurrency().id()) {
          security = m_securityMap[acc.currencyId()];
          prices += file->price(acc.currencyId(), file->baseCurrency().id());
        }
      }

    } catch(MyMoneyException *e) {
      kDebug(2) << __PRETTY_FUNCTION__ << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
      delete e;
    }

    KMyMoneyAccountTreeItem* item = new KMyMoneyAccountTreeItem(parent, acc, prices, security);
    if(acc.id() == m_reconciliationAccount.id())
      item->setReconciliation(true);

    unused |= loadSubAccounts(item, acc.accountList());

    // no child accounts and no transactions in this account means 'unused'
    bool thisUnused = (!item->firstChild()) && (m_transactionCountMap[acc.id()] == 0);

    // In case of a category which is unused and we are requested to suppress
    // the display of those,
    if(acc.isIncomeExpense()) {
      if(KMyMoneyGlobalSettings::hideUnusedCategory() && thisUnused) {
        unused = true;
        delete item;
      }
    }

    // if the account is closed and we should not show it, we delete the item
    if(acc.isClosed() && !showClosedAccounts) {
      delete item;
    }
  }
  return unused;
}

void KAccountsView::slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
{
  Q_UNUSED(reconciliationDate);
  Q_UNUSED(endingBalance);

  // scan through the list of accounts and mark all non
  // expanded and re-select the one that was probably selected before
  Q3ListViewItemIterator it_lvi(m_accountTree);
  KMyMoneyAccountTreeItem* item;
  while(it_lvi.current()) {
    item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
    if(item) {
      item->setReconciliation(false);
    }
    ++it_lvi;
  }

  // scan trough the icon list and do the same thing
  KMyMoneyAccountIconItem* icon = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
  for(;icon; icon = dynamic_cast<KMyMoneyAccountIconItem*>(icon->nextItem())) {
    icon->setReconciliation(false);
  }

  m_reconciliationAccount = acc;

  if(!acc.id().isEmpty()) {
    // scan through the list of accounts and mark
    // the one that is currently reconciled
    it_lvi = Q3ListViewItemIterator(m_accountTree);
    while(it_lvi.current()) {
      item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
      if(item && item->itemObject().id() == acc.id()) {
        item->setReconciliation(true);
        break;
      }
      ++it_lvi;
    }

    // scan trough the icon list and do the same thing
    icon = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
    for(;icon; icon = dynamic_cast<KMyMoneyAccountIconItem*>(icon->nextItem())) {
      if(icon->itemObject().id() == acc.id()) {
        icon->setReconciliation(true);
        break;
      }
    }
  }
}

void KAccountsView::slotUpdateNetWorth(void)
{
  if(!m_assetItem || !m_liabilityItem)
    return;

  MyMoneyMoney netWorth = m_assetItem->totalValue() - m_liabilityItem->totalValue();

  QString s(i18n("Net Worth: "));

  // FIXME figure out how to deal with the approximate
  // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
  //  s += "~ ";

  s.replace(QString(" "), QString("&nbsp;"));
  if(netWorth.isNegative()) {
    s += "<b><font color=\"red\">";
  }
  const MyMoneySecurity& sec = MyMoneyFile::instance()->baseCurrency();
  QString v(netWorth.formatMoney(sec));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if(netWorth.isNegative()) {
    s += "</font></b>";
  }

  m_totalProfitsLabel->setFont(KMyMoneyGlobalSettings::listCellFont());
  m_totalProfitsLabel->setText(s);
}

KMyMoneyAccountIconItem* KAccountsView::selectedIcon(void) const
{
  return dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->currentItem());
}

void KAccountsView::slotSelectIcon(Q3IconViewItem* item)
{
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(item);
  if(p)
    emit selectObject(p->itemObject());
}

void KAccountsView::slotOpenContext(Q3IconViewItem* item)
{
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(item);
  if(p)
    emit openContextMenu(p->itemObject());
}

void KAccountsView::slotOpenObject(Q3IconViewItem* item)
{
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(item);
  if(p)
    emit openObject(p->itemObject());
}

QString KAccountsView::point(const QPoint& val) const
{
  return QString("%1;%2").arg(val.x()).arg(val.y());
}

QPoint KAccountsView::point(const QString& val) const
{
  QRegExp exp("(\\d+);(\\d+)");
  int x = 0;
  int y = 0;
  if(exp.search(val) != -1) {
    x = exp.cap(1).toInt();
    y = exp.cap(2).toInt();
  }
  return QPoint(x, y);
}

void KAccountsView::slotUpdateIconPos(unsigned int action)
{
  if(action != KMyMoneyView::preSave)
    return;

  MyMoneyFileTransaction ft;
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
  for(;p; p = dynamic_cast<KMyMoneyAccountIconItem*>(p->nextItem())) {
    const MyMoneyAccount& acc = dynamic_cast<const MyMoneyAccount&>(p->itemObject());
    if(acc.value("kmm-iconpos") != point(p->pos())) {
      MyMoneyAccount a(acc);
      a.setValue("kmm-iconpos", point(p->pos()));
      try {
        MyMoneyFile::instance()->modifyAccount(a);
      } catch(MyMoneyException* e) {
        kDebug(2) << "Unable to update icon pos: " << e->what();
        delete e;
      }
    }
  }
  ft.commit();
}


#include "kaccountsview.moc"
