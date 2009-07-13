/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes
    email                : mte@users.sourceforge.net
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

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3VBoxLayout>

// ----------------------------------------------------------------------------
// KDE Includes

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kpushbutton.h>

// ----------------------------------------------------------------------------
// Project Includes

#include <mymoneyfile.h>
#include <kmymoneyaccounttree.h>
#include "kcategoriesview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney2.h"
#include <K3ListViewSearchLineWidget>

KCategoriesView::KCategoriesView(QWidget *parent) :
  KCategoriesViewDecl(parent),
  m_incomeItem(0),
  m_expenseItem(0),
  m_needReload(false)
{
  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new K3ListViewSearchLineWidget(m_accountTree, m_accountTree->parentWidget());
/*  Q3VBoxLayout* layout = dynamic_cast<Q3VBoxLayout*>(m_accountTree->parentWidget()->layout());
  if(layout) {
    layout->insertWidget(2, m_searchWidget);
  }*/
  hboxLayout->insertWidget(2, m_searchWidget);

  // setup icons for collapse and expand button
  KGuiItem collapseGuiItem("",
                          KIcon("viewmag"),
                          QString(),
                          QString());
  KGuiItem expandGuiItem("",
                          KIcon("viewmag+"),
                          QString(),
                          QString());
  m_collapseButton->setGuiItem(collapseGuiItem);
  m_expandButton->setGuiItem(expandGuiItem);

  m_accountTree->setSectionHeader(i18n("Category"));

  connect(m_accountTree, SIGNAL(selectObject(const MyMoneyObject&)), this, SIGNAL(selectObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(openContextMenu(const MyMoneyObject&)), this, SIGNAL(openContextMenu(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(valueChanged(void)), this, SLOT(slotUpdateProfit(void)));
  connect(m_accountTree, SIGNAL(openObject(const MyMoneyObject&)), this, SIGNAL(openObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)), this, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));

}

KCategoriesView::~KCategoriesView()
{
}

void KCategoriesView::show(void)
{
  if(m_needReload) {
    loadAccounts();
    m_needReload = false;
  }

  // don't forget base class implementation
  KCategoriesViewDecl::show();

  // if we have a selected account, let the application know about it
  KMyMoneyAccountTreeBaseItem *item = m_accountTree->selectedItem();
  if(item) {
    emit selectObject(item->itemObject());
  }
}

void KCategoriesView::polish(void)
{
  KCategoriesViewDecl::polish();
  m_accountTree->setResizeMode(Q3ListView::LastColumn);
  m_accountTree->restoreLayout("Category View Settings");

}

void KCategoriesView::slotLoadAccounts(void)
{
  if(isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}

void KCategoriesView::loadAccounts(void)
{
  QMap<QString, bool> isOpen;

  ::timetrace("start load categories view");
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
  m_incomeItem = m_expenseItem = 0;

  MyMoneyFile* file = MyMoneyFile::instance();

  Q3ValueList<MyMoneySecurity> slist = file->currencyList();
  slist += file->securityList();
  Q3ValueList<MyMoneySecurity>::const_iterator it_s;
  for(it_s = slist.begin(); it_s != slist.end(); ++it_s) {
    m_securityMap[(*it_s).id()] = *it_s;
  }

  m_transactionCountMap = file->transactionCountMap();

  bool haveUnusedCategories = false;

  // create the items
  try {
    const MyMoneySecurity& security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    const MyMoneyAccount& income = file->income();
    m_incomeItem = new KMyMoneyAccountTreeItem(m_accountTree, income, security, i18n("Income"));
    haveUnusedCategories |= loadSubAccounts(m_incomeItem, income.accountList());

    const MyMoneyAccount& expense = file->expense();
    m_expenseItem = new KMyMoneyAccountTreeItem(m_accountTree, expense, security, i18n("Expense"));
    haveUnusedCategories |= loadSubAccounts(m_expenseItem, expense.accountList());

  } catch(MyMoneyException *e) {
    kDebug(2) << "Problem in categoriesview: " << e->what();
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

  // update the hint if categories are hidden
  m_hiddenCategories->setVisible(haveUnusedCategories);

  ::timetrace("done load categories view");
}

bool KCategoriesView::loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QStringList& accountList)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  bool unused = false;

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
    unused |= loadSubAccounts(item, acc.accountList());

    // no child accounts and not transactions in this account means 'unused'
    bool thisUnused = (!item->firstChild()) && (m_transactionCountMap[acc.id()] == 0);

    // In case of a category which is unused and we are requested to suppress
    // the display of those,
    if(acc.accountGroup() == MyMoneyAccount::Income
    || acc.accountGroup() == MyMoneyAccount::Expense) {
      if(KMyMoneyGlobalSettings::hideUnusedCategory() && thisUnused) {
        unused = true;
        delete item;
      }
    }
  }
  return unused;
}

void KCategoriesView::slotUpdateProfit(void)
{
  if(!m_incomeItem || !m_expenseItem)
    return;

  MyMoneyMoney profit = m_incomeItem->totalValue() - m_expenseItem->totalValue();

  QString s(i18n("Profit: "));
  if(profit.isNegative())
    s = i18n("Loss: ");

  // FIXME figure out how to deal with the approximate
  // if(!(file->totalValueValid(assetAccount.id()) & file->totalValueValid(liabilityAccount.id())))
  //  s += "~ ";

  s.replace(QString(" "), QString("&nbsp;"));
  if(profit.isNegative()) {
    s += "<b><font color=\"red\">";
  }
  const MyMoneySecurity& sec = MyMoneyFile::instance()->baseCurrency();
  QString v(profit.abs().formatMoney(sec));
  s += v.replace(QString(" "), QString("&nbsp;"));
  if(profit.isNegative()) {
    s += "</font></b>";
  }

  m_totalProfitsLabel->setFont(KMyMoneyGlobalSettings::listCellFont());
  m_totalProfitsLabel->setText(s);
}

#include "kcategoriesview.moc"

