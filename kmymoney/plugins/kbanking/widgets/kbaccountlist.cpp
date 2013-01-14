/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif


#include "kbaccountlist.h"
#include <assert.h>
#include <QtCore/qstring.h>
#include <klocale.h>


KBAccountListViewItem::KBAccountListViewItem(KBAccountListView *parent,
    AB_ACCOUNT *acc)
    : QTreeWidgetItem(parent)
    , _account(acc)
{
  assert(acc);
  _populate();
}

KBAccountListViewItem::KBAccountListViewItem(const KBAccountListViewItem &item)
    : QTreeWidgetItem(item)
    , _account(0)
{
  if (item._account) {
    _account = item._account;
  }
}

KBAccountListViewItem::KBAccountListViewItem(KBAccountListView *parent,
    QTreeWidgetItem *after,
    AB_ACCOUNT *acc)
    : QTreeWidgetItem(parent, after)
    , _account(acc)
{
  assert(acc);
  _populate();
}

KBAccountListViewItem::~KBAccountListViewItem()
{
}

AB_ACCOUNT *KBAccountListViewItem::getAccount()
{
  return _account;
}

void KBAccountListViewItem::_populate()
{
  QString tmp;
  int i;

  assert(_account);

  i = 0;

  // unique id
  setText(i++, QString::number(AB_Account_GetUniqueId(_account)));

  // bank code
  setText(i++, QString::fromUtf8(AB_Account_GetBankCode(_account)));

  // bank name
  tmp = AB_Account_GetBankName(_account);
  if (tmp.isEmpty())
    tmp = i18nc("replacement for institution or account w/o name", "(unnamed)");
  setText(i++, tmp);

  // account id
  setText(i++, QString::fromUtf8(AB_Account_GetAccountNumber(_account)));

  // account name
  tmp = QString::fromUtf8(AB_Account_GetAccountName(_account));
  if (tmp.isEmpty())
    tmp = i18nc("replacement for institution or account w/o name", "(unnamed)");
  setText(i++, tmp);

  tmp = QString::fromUtf8(AB_Account_GetOwnerName(_account));
  if (tmp.isEmpty())
    tmp = "";
  setText(i++, tmp);

  tmp = QString::fromUtf8(AB_Provider_GetName(AB_Account_GetProvider(_account)));
  if (tmp.isEmpty())
    tmp = i18nc("replacement for institution or account w/o name", "(unnamed)");
  setText(i++, tmp);
}

bool KBAccountListViewItem::operator< (const QTreeWidgetItem & other) const
{
  bool ok1, ok2;
  int column = treeWidget() ? treeWidget()->sortColumn() : 0;
  int a = text(column).toInt(&ok1);
  int b = other.text(column).toInt(&ok2);
  if (ok1 && ok2)
    return a < b;
  return QTreeWidgetItem::operator<(other);
}

KBAccountListView::KBAccountListView(QWidget *parent)
    : QTreeWidget(parent)
{
  setAllColumnsShowFocus(true);
  setColumnCount(7);
  QStringList header;
  header << i18nc("Header for AqBanking account list", "Id");
  header << i18nc("Header for AqBanking account list", "Institution Code");
  header << i18nc("Header for AqBanking account list", "Institution Name");
  header << i18nc("Header for AqBanking account list", "Account Number");
  header << i18nc("Header for AqBanking account list", "Account Name");
  header << i18nc("Header for AqBanking account list", "Owner");
  header << i18nc("Header for AqBanking account list", "Backend");
  setHeaderLabels(header);

  setSortingEnabled(true);
  sortItems(0, Qt::AscendingOrder);
}

KBAccountListView::~KBAccountListView()
{
}

void KBAccountListView::addAccount(AB_ACCOUNT *acc)
{
  new KBAccountListViewItem(this, acc);
}

void KBAccountListView::addAccounts(const std::list<AB_ACCOUNT*> &accs)
{
  std::list<AB_ACCOUNT*>::const_iterator it;

  for (it = accs.begin(); it != accs.end(); ++it) {
    new KBAccountListViewItem(this, *it);
  } /* for */
}

AB_ACCOUNT *KBAccountListView::getCurrentAccount()
{
  KBAccountListViewItem *entry;

  entry = dynamic_cast<KBAccountListViewItem*>(currentItem());
  if (!entry) {
    return 0;
  }
  return entry->getAccount();
}

std::list<AB_ACCOUNT*> KBAccountListView::getSelectedAccounts()
{
  std::list<AB_ACCOUNT*> accs;
  KBAccountListViewItem *entry;

  // Create an iterator and give the listview as argument
  QTreeWidgetItemIterator it(this);
  // iterate through all items of the listview
  for (; *it; ++it) {
    if ((*it)->isSelected()) {
      entry = dynamic_cast<KBAccountListViewItem*>(*it);
      if (entry)
        accs.push_back(entry->getAccount());
    }
  } // for

  return accs;
}

std::list<AB_ACCOUNT*> KBAccountListView::getSortedAccounts()
{
  std::list<AB_ACCOUNT*> accs;
  KBAccountListViewItem *entry;

  // Create an iterator and give the listview as argument
  QTreeWidgetItemIterator it(this);
  // iterate through all items of the listview
  for (; *it; ++it) {
    entry = dynamic_cast<KBAccountListViewItem*>(*it);
    if (entry)
      accs.push_back(entry->getAccount());
  } // for

  return accs;
}
