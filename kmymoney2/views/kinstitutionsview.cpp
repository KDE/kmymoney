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

// ----------------------------------------------------------------------------
// QT Includes

#include <q3header.h>
#include <qlabel.h>
#include <qtabwidget.h>
//Added by qt3to4:
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
#include "kinstitutionsview.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoney2.h"

KInstitutionsView::KInstitutionsView(QWidget *parent) :
  KInstitutionsViewDecl(parent),
  m_needReload(false)
{
  m_accountTree->header()->setLabel(0, i18n("Institution/Account"));

  connect(m_accountTree, SIGNAL(selectObject(const MyMoneyObject&)), this, SIGNAL(selectObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(openContextMenu(const MyMoneyObject&)), this, SIGNAL(openContextMenu(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(valueChanged(void)), this, SLOT(slotUpdateNetWorth(void)));
  connect(m_accountTree, SIGNAL(openObject(const MyMoneyObject&)), this, SIGNAL(openObject(const MyMoneyObject&)));
  connect(m_accountTree, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&)), this, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&)));

  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadAccounts()));
}

KInstitutionsView::~KInstitutionsView()
{
}

void KInstitutionsView::show(void)
{
  if(m_needReload) {
    loadAccounts();
    m_needReload = false;
  }

  // don't forget base class implementation
  KInstitutionsViewDecl::show();

  // if we have a selected account, let the application know about it
  KMyMoneyAccountTreeBaseItem *item = m_accountTree->selectedItem();
  if(item) {
    emit selectObject(item->itemObject());
  }
}

void KInstitutionsView::polish(void)
{
  KInstitutionsViewDecl::polish();
  m_accountTree->setResizeMode(Q3ListView::LastColumn);
  m_accountTree->restoreLayout("Institution View Settings");
}

void KInstitutionsView::slotLoadAccounts(void)
{
  if(isVisible()) {
    loadAccounts();
  } else {
    m_needReload = true;
  }
}

void KInstitutionsView::loadAccounts(void)
{
  QMap<QString, bool> isOpen;

  ::timetrace("start load institutions view");
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
  m_accountMap.clear();
  m_securityMap.clear();
  m_transactionCountMap.clear();

  MyMoneyFile* file = MyMoneyFile::instance();

  QList<MyMoneyAccount> alist;
  file->accountList(alist);
  QList<MyMoneyAccount>::const_iterator it_a;
  for(it_a = alist.begin(); it_a != alist.end(); ++it_a) {
    m_accountMap[(*it_a).id()] = *it_a;
  }

  // we need to make sure we show stock accounts
  // under the right institution (the one of the parent account)
  QMap<QString, MyMoneyAccount>::iterator it_am;
  for(it_am = m_accountMap.begin(); it_am != m_accountMap.end(); ++it_am) {
    if((*it_am).isInvest()) {
      (*it_am).setInstitutionId(m_accountMap[(*it_am).parentAccountId()].institutionId());
    }
  }

  QList<MyMoneySecurity> slist = file->currencyList();
  slist += file->securityList();
  QList<MyMoneySecurity>::const_iterator it_s;
  for(it_s = slist.begin(); it_s != slist.end(); ++it_s) {
    m_securityMap[(*it_s).id()] = *it_s;
  }

  m_transactionCountMap = file->transactionCountMap();

  m_accountTree->setBaseCurrency(file->baseCurrency());

  // create the items
  try {
    const MyMoneySecurity& security = file->baseCurrency();
    m_accountTree->setBaseCurrency(security);

    MyMoneyInstitution none;
    none.setName(i18n("Accounts with no institution assigned"));
    KMyMoneyAccountTreeItem* noInstitutionItem = new KMyMoneyAccountTreeItem(m_accountTree, none);
    noInstitutionItem->setPixmap(0,none.pixmap());
    loadSubAccounts(noInstitutionItem, QString());

    // hide it, if unused
    noInstitutionItem->setVisible(noInstitutionItem->childCount() != 0);

    QList<MyMoneyInstitution> list = file->institutionList();
    QList<MyMoneyInstitution>::const_iterator it_i;
    for(it_i = list.begin(); it_i != list.end(); ++it_i) {
      KMyMoneyAccountTreeItem* item = new KMyMoneyAccountTreeItem(m_accountTree, *it_i);
      item->setPixmap(0, none.pixmap());
      loadSubAccounts(item, (*it_i).id());
    }

  } catch(MyMoneyException *e) {
    kDebug(2) << "Problem in institutions view: " << e->what();
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

  // turn updates back on
  m_accountTree->setUpdatesEnabled(true);
  m_accountTree->repaintContents();

  ::timetrace("done load institutions view");
}

void KInstitutionsView::loadSubAccounts(KMyMoneyAccountTreeItem* parent)
{
  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked();
  const MyMoneyAccount& account = dynamic_cast<const MyMoneyAccount&>(parent->itemObject());
  QList<QString>::const_iterator it_a;
  MyMoneyFile* file = MyMoneyFile::instance();
  for(it_a = account.accountList().constBegin(); it_a != account.accountList().constEnd(); ++it_a) {
    MyMoneyAccount acc = m_accountMap[(*it_a)];
    if(!acc.isInvest())
      continue;
    if(acc.isClosed() && !showClosedAccounts)
      continue;
    const MyMoneySecurity& security = m_securityMap[acc.currencyId()];
    QList<MyMoneyPrice> prices;
    prices += file->price(acc.currencyId(), security.tradingCurrency());
    if(security.tradingCurrency() != file->baseCurrency().id()) {
      MyMoneySecurity sec = m_securityMap[security.tradingCurrency()];
      prices += file->price(sec.id(), file->baseCurrency().id());
    }
    KMyMoneyAccountTreeItem* item = new KMyMoneyAccountTreeItem(parent, acc, prices, security);
    if(acc.id() == m_reconciliationAccount.id())
      item->setReconciliation(true);
  }
}

void KInstitutionsView::loadSubAccounts(KMyMoneyAccountTreeItem* parent, const QString& institutionId)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  QMap<QString, MyMoneyAccount>::const_iterator it_a;
  MyMoneyMoney  value;
  bool showClosedAccounts = kmymoney2->toggleAction("view_show_all_accounts")->isChecked();

  for(it_a = m_accountMap.begin(); it_a != m_accountMap.end(); ++it_a) {
    const MyMoneyAccount& acc = *it_a;
    MyMoneyMoney factor(1,1);
    switch(acc.accountGroup()) {
      case MyMoneyAccount::Liability:
        factor = MyMoneyMoney(-1,1);
        // tricky fall through here

      case MyMoneyAccount::Asset:
        if(acc.institutionId() == institutionId
        && !acc.isInvest()
        && (!acc.isClosed() || showClosedAccounts)) {
          QList<MyMoneyPrice> prices;
          MyMoneySecurity security = file->baseCurrency();
          try {
            if(acc.currencyId() != file->baseCurrency().id()) {
              security = m_securityMap[acc.currencyId()];
              prices += file->price(acc.currencyId(), file->baseCurrency().id());
            }

          } catch(MyMoneyException *e) {
            kDebug(2) << __PRETTY_FUNCTION__ << " caught exception while adding " << acc.name() << "[" << acc.id() << "]: " << e->what();
            delete e;
          }

          KMyMoneyAccountTreeItem* item = new KMyMoneyAccountTreeItem(parent, acc, prices, security);
          if(acc.id() == m_reconciliationAccount.id())
            item->setReconciliation(true);

          if(acc.accountType() == MyMoneyAccount::Investment)
            loadSubAccounts(item);
          value += (item->totalValue() * factor);
        }
        break;

      default:
        break;
    }
  }

  // the calulated value for the institution is not correct as
  // it does not take the negative sign for liability accounts
  // into account. So we correct this here with the value we
  // have calculated while filling the list
  parent->adjustTotalValue(-parent->totalValue());  // load a 0
  parent->adjustTotalValue(value);                  // now store the new value

  // we need to call slotUpdateNetWorth() here manually, because
  // KMyMoneyAccountTreeItem::adjustTotalValue() does not send out
  // the valueChanged() signal
  slotUpdateNetWorth();
}

void KInstitutionsView::slotUpdateNetWorth(void)
{
  MyMoneyMoney netWorth;

  // calculate by going through the account trees top items
  // and summing up the total value shown there
  KMyMoneyAccountTreeItem* item = dynamic_cast<KMyMoneyAccountTreeItem*>(m_accountTree->firstChild());
  while(item) {
    netWorth += item->totalValue();
    item = dynamic_cast<KMyMoneyAccountTreeItem*>(item->nextSibling());
  }

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

void KInstitutionsView::slotReconcileAccount(const MyMoneyAccount& acc, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
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

  m_reconciliationAccount = acc;
  if(!acc.id().isEmpty()) {
    it_lvi = Q3ListViewItemIterator(m_accountTree);
    while(it_lvi.current()) {
      item = dynamic_cast<KMyMoneyAccountTreeItem*>(it_lvi.current());
      if(item && item->itemObject().id() == acc.id()) {
        item->setReconciliation(true);
        break;
      }
      ++it_lvi;
    }
  }
}


#include "kinstitutionsview.moc"
