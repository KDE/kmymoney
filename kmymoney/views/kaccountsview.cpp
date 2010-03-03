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
#include <QPixmap>
#include <QLayout>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <k3iconview.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <KToggleAction>
#include <K3ListViewSearchLineWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include "kmymoneyview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney.h"
#include "kmymoneyaccounttreeview.h"
#include "models.h"

Q_DECLARE_METATYPE(MyMoneyAccount)
Q_DECLARE_METATYPE(MyMoneyMoney)

QPixmap accountPixmap(const MyMoneyAccount& account, bool reconcileFlag)
{
  QString pixmap;

  switch (account.accountType()) {
  default:
    if (account.accountGroup() == MyMoneyAccount::Asset)
      pixmap = "account-types-asset";
    else
      pixmap = "account-types-liability";
    break;

  case MyMoneyAccount::Investment:
    pixmap = "account-types-investments";
    break;

  case MyMoneyAccount::Checkings:
    pixmap = "account-types-checking";
    break;
  case MyMoneyAccount::Savings:
    pixmap = "account-types-savings";
    break;

  case MyMoneyAccount::AssetLoan:
  case MyMoneyAccount::Loan:
    pixmap = "account-types-loan";
    break;

  case MyMoneyAccount::CreditCard:
    pixmap = "account-types-credit-card";
    break;

  case MyMoneyAccount::Asset:
    pixmap = "account-types-asset";
    break;

  case MyMoneyAccount::Cash:
    pixmap = "account-types-cash";
    break;
  }
  QPixmap result = DesktopIcon(pixmap);

  if (account.isClosed()) {
    QPixmap overlay = DesktopIcon("account-types-closed");
    QPainter pixmapPainter(&result);
    pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());
  } else if (reconcileFlag) {
    QPixmap overlay = DesktopIcon("account-types-reconciled");
    QPainter pixmapPainter(&result);
    pixmapPainter.drawPixmap(0, 0, overlay, 0, 0, overlay.width(), overlay.height());
  }

  return result;
}

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
  if (m_reconcileFlag == on)
    return;
  m_reconcileFlag = on;
  updateAccount(m_account);
}

void KMyMoneyAccountIconItem::updateAccount(const MyMoneyAccount& account)
{
  setPixmap(accountPixmap(account, m_reconcileFlag));
}

KAccountsView::KAccountsView(QWidget *parent) :
    KAccountsViewDecl(parent),
    m_assetItem(0),
    m_liabilityItem(0)
{
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

  connect(m_tab, SIGNAL(currentChanged(QWidget*)), this, SLOT(slotTabCurrentChanged(QWidget*)));

  connect(Models::instance()->accountsModel(), SIGNAL(netWorthChanged(const MyMoneyMoney&)), this, SLOT(slotNetWorthChanged(const MyMoneyMoney&)));

  // the proxy filter model
  m_filterProxyModel = new AccountsViewFilterProxyModel(this);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Asset);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Liability);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Income);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Expense);
  m_filterProxyModel->addAccountGroup(MyMoneyAccount::Equity);
  m_filterProxyModel->setSourceModel(Models::instance()->accountsModel());
  m_filterProxyModel->setFilterKeyColumn(-1);

  connect(m_filterProxyModel, SIGNAL(unusedIncomeExpenseAccountHidden()), this, SLOT(slotUnusedIncomeExpenseAccountHidden()));

  m_accountTree->setAlternatingRowColors(true);
  m_accountTree->setIconSize(QSize(22, 22));
  m_accountTree->setSortingEnabled(true);
  m_accountTree->setModel(m_filterProxyModel);

  connect(m_searchWidget, SIGNAL(textChanged(const QString &)), m_filterProxyModel, SLOT(setFilterFixedString(const QString &)));

  // let the model know if the item is expanded or collapsed
  connect(m_accountTree, SIGNAL(collapsed(const QModelIndex &)), m_filterProxyModel, SLOT(collapsed(const QModelIndex &)));
  connect(m_accountTree, SIGNAL(expanded(const QModelIndex &)), m_filterProxyModel, SLOT(expanded(const QModelIndex &)));
  connect(m_accountTree, SIGNAL(selectObject(const MyMoneyObject&)), this, SIGNAL(selectObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(openContextMenu(const MyMoneyObject&)), this, SIGNAL(openContextMenu(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(openObject(const MyMoneyObject&)), this, SIGNAL(openObject(const MyMoneyObject&)));
  connect(m_collapseButton, SIGNAL(clicked()), m_accountTree, SLOT(collapseAll()));
  connect(m_expandButton, SIGNAL(clicked()), m_accountTree, SLOT(expandAll()));

  connect(m_accountIcons, SIGNAL(selectionChanged(Q3IconViewItem*)), this, SLOT(slotSelectIcon(Q3IconViewItem*)));
  connect(m_accountIcons, SIGNAL(rightButtonClicked(Q3IconViewItem*, const QPoint&)), this, SLOT(slotOpenContext(Q3IconViewItem*)));
  connect(m_accountIcons, SIGNAL(executed(Q3IconViewItem*)), this, SLOT(slotOpenObject(Q3IconViewItem*)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));
  connect(m_collapseButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
  connect(m_expandButton, SIGNAL(clicked()), this, SLOT(slotExpandCollapse()));
}

KAccountsView::~KAccountsView()
{
}

void KAccountsView::slotExpandCollapse(void)
{
  if (sender()) {
    KMyMoneyGlobalSettings::setShowAccountsExpanded(sender() == m_expandButton);
  }
}

/**
  * The view is notified that an unused income expense account has been hidden.
  */
void KAccountsView::slotUnusedIncomeExpenseAccountHidden(void)
{
  m_haveUnusedCategories = true;
  m_hiddenCategories->setVisible(m_haveUnusedCategories);
}

void KAccountsView::slotLoadAccounts(void)
{
  m_needReload[ListView] = true;
  m_needReload[IconView] = true;
  if (isVisible())
    slotTabCurrentChanged(m_tab->currentWidget());
}

void KAccountsView::slotTabCurrentChanged(QWidget* _tab)
{
  AccountsViewTab tab = static_cast<AccountsViewTab>(m_tab->indexOf(_tab));

  // remember this setting for startup
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("KAccountsView_LastType", QVariant(tab).toString());

  loadAccounts(tab);

  switch (tab) {
  case ListView:
    // update the hint if categories are hidden
    m_hiddenCategories->setVisible(m_haveUnusedCategories);
    break;

  case IconView:
    m_hiddenCategories->hide();
    break;

  default:
    break;
  }

  KMyMoneyAccountIconItem* iconItem = selectedIcon();

  emit selectObject(MyMoneyAccount());
  switch (static_cast<AccountsViewTab>(m_tab->currentIndex())) {
  case ListView:
    {
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
      emit selectObject(iconItem->itemObject());
    }
    break;

  default:
    break;
  }
}

void KAccountsView::showEvent(QShowEvent * event)
{
  KAccountsViewDecl::showEvent(event);
  slotTabCurrentChanged(m_tab->currentWidget());
}

void KAccountsView::loadAccounts(AccountsViewTab tab)
{
  if (m_needReload[tab]) {
    switch (tab) {
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
  for (; p; p = dynamic_cast<KMyMoneyAccountIconItem*>(p->nextItem()))
    posMap[p->itemObject().id()] = p->pos();

  // turn off updates to avoid flickering during reload
  m_accountIcons->setAutoArrange(true);

  // clear the current contents and recreate it
  m_accountIcons->clear();
  QMap<QString, MyMoneyAccount> accountMap;

  MyMoneyFile* file = MyMoneyFile::instance();

  // get account list and sort by name
  QList<MyMoneyAccount> alist;
  file->accountList(alist);
  QList<MyMoneyAccount>::const_iterator it_a;
  for (it_a = alist.constBegin(); it_a != alist.constEnd(); ++it_a) {
    accountMap[QString("%1-%2").arg((*it_a).name()).arg((*it_a).id())] = *it_a;
  }

  bool showClosedAccounts = kmymoney->toggleAction("view_show_all_accounts")->isChecked()
                            || !KMyMoneyGlobalSettings::hideClosedAccounts();
  bool existNewIcons = false;

  // parse list and add all asset and liability accounts
  QMap<QString, MyMoneyAccount>::const_iterator it;
  QPoint loc;
  for (it = accountMap.constBegin(); it != accountMap.constEnd(); ++it) {
    if ((*it).isClosed() && !showClosedAccounts)
      continue;
    const QString& pos = (*it).value("kmm-iconpos");
    KMyMoneyAccountIconItem* item;
    switch ((*it).accountGroup()) {
    case MyMoneyAccount::Equity:
      if (!KMyMoneyGlobalSettings::expertMode())
        continue;
      // tricky fall through here

    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
      // don't show stock accounts
      if ((*it).isInvest())
        continue;

      // if we have a position stored with the object and no other
      // idea of it's current position, then take the one
      // stored inside the object. Also, turn off auto arrangement
      if (!pos.isEmpty() && posMap[(*it).id()] == QPoint()) {
        posMap[(*it).id()] = point(pos);
      }

      loc = posMap[(*it).id()];
      if (loc == QPoint()) {
        existNewIcons = true;
      } else {
        m_accountIcons->setAutoArrange(false);
      }

      item = new KMyMoneyAccountIconItem(m_accountIcons, *it);
      if ((*it).id() == m_reconciliationAccount.id())
        item->setReconciliation(true);

      if (loc != QPoint()) {
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

  if (existNewIcons) {
    m_accountIcons->arrangeItemsInGrid(true);
  }

  m_accountIcons->setAutoArrange(false);
  ::timetrace("done load accounts icon view");
}

void KAccountsView::loadListView(void)
{
  // TODO: check why the invalidate is needed here
  m_filterProxyModel->invalidate();
  m_filterProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !kmymoney->toggleAction("view_show_all_accounts")->isChecked());
  m_filterProxyModel->setHideEquityAccounts(!KMyMoneyGlobalSettings::expertMode());

  // reinitialize the default state of the hidden categories label
  m_haveUnusedCategories = false;
  m_hiddenCategories->hide();
  m_filterProxyModel->setHideUnusedIncomeExpenseAccounts(kmymoney->toggleAction("view_hide_unused_categories")->isChecked());

  // and in case we need to show things expanded, we'll do so
  if (KMyMoneyGlobalSettings::showAccountsExpanded()) {
    m_accountTree->expandAll();
  }
}

bool KAccountsView::loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QStringList& accountList)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  bool unused = false;
  bool showClosedAccounts = kmymoney->toggleAction("view_show_all_accounts")->isChecked()
                            || !KMyMoneyGlobalSettings::hideClosedAccounts();

  QStringList::const_iterator it_a;
  for (it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    const MyMoneyAccount& acc = file->account(*it_a);
    QList<MyMoneyPrice> prices;
    MyMoneySecurity security = file->baseCurrency();
    try {
      if (acc.isInvest()) {
        security = m_securityMap[acc.currencyId()];
        prices += file->price(acc.currencyId(), security.tradingCurrency());
        if (security.tradingCurrency() != file->baseCurrency().id()) {
          MyMoneySecurity sec = m_securityMap[security.tradingCurrency()];
          prices += file->price(sec.id(), file->baseCurrency().id());
        }
      } else if (acc.currencyId() != file->baseCurrency().id()) {
        if (acc.currencyId() != file->baseCurrency().id()) {
          security = m_securityMap[acc.currencyId()];
          prices += file->price(acc.currencyId(), file->baseCurrency().id());
        }
      }

    } catch (MyMoneyException *e) {
      kDebug(2) << Q_FUNC_INFO << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
      delete e;
    }

    KMyMoneyAccountTreeItem* item = new KMyMoneyAccountTreeItem(parent, acc, prices, security);
    if (acc.id() == m_reconciliationAccount.id())
      item->setReconciliation(true);

    unused |= loadSubAccounts(item, acc.accountList());

    // no child accounts and no transactions in this account means 'unused'
    bool thisUnused = (!item->firstChild()) && (m_transactionCountMap[acc.id()] == 0);

    // In case of a category which is unused and we are requested to suppress
    // the display of those,
    if (acc.isIncomeExpense()) {
      if (KMyMoneyGlobalSettings::hideUnusedCategory() && thisUnused) {
        unused = true;
        delete item;
      }
    }

    // if the account is closed and we should not show it, we delete the item
    if (acc.isClosed() && !showClosedAccounts) {
      delete item;
    }
  }
  return unused;
}

void KAccountsView::slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
{
  Q_UNUSED(reconciliationDate);
  Q_UNUSED(endingBalance);

  // TODO: scan through the list of accounts and mark all non
  // expanded and re-select the one that was probably selected before

  // scan trough the icon list and do the same thing
  KMyMoneyAccountIconItem* icon = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
  for (; icon; icon = dynamic_cast<KMyMoneyAccountIconItem*>(icon->nextItem())) {
    icon->setReconciliation(false);
  }

  m_reconciliationAccount = acc;

  if (!acc.id().isEmpty()) {
    // TODO: scan through the list of accounts and mark
    // the one that is currently reconciled

    // scan trough the icon list and do the same thing
    icon = dynamic_cast<KMyMoneyAccountIconItem*>(m_accountIcons->firstItem());
    for (; icon; icon = dynamic_cast<KMyMoneyAccountIconItem*>(icon->nextItem())) {
      if (icon->itemObject().id() == acc.id()) {
        icon->setReconciliation(true);
        break;
      }
    }
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
  QString v(netWorth.formatMoney(sec));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if (netWorth.isNegative()) {
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
  if (p)
    emit selectObject(p->itemObject());
}

void KAccountsView::slotOpenContext(Q3IconViewItem* item)
{
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(item);
  if (p)
    emit openContextMenu(p->itemObject());
}

void KAccountsView::slotOpenObject(Q3IconViewItem* item)
{
  KMyMoneyAccountIconItem* p = dynamic_cast<KMyMoneyAccountIconItem*>(item);
  if (p)
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
  if (exp.indexIn(val) != -1) {
    x = exp.cap(1).toInt();
    y = exp.cap(2).toInt();
  }
  return QPoint(x, y);
}

void KAccountsView::slotUpdateIconPos(unsigned int action)
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
      } catch (MyMoneyException* e) {
        kDebug(2) << "Unable to update icon pos: " << e->what();
        delete e;
      }
    }
  }
  ft.commit();
}


#include "kaccountsview.moc"
