/*
    SPDX-FileCopyrightText: 2004 Martin Preuss <martin@libchipcard.de>

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif


#include "kbaccountlist.h"
#include <assert.h>
#include <QString>
#include <KLocalizedString>


KBAccountListViewItem::KBAccountListViewItem(KBAccountListView *parent,
    AB_ACCOUNT_SPEC *acc)
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
    AB_ACCOUNT_SPEC *acc)
    : QTreeWidgetItem(parent, after)
    , _account(acc)
{
  assert(acc);
  _populate();
}

KBAccountListViewItem::~KBAccountListViewItem()
{
}

AB_ACCOUNT_SPEC *KBAccountListViewItem::getAccount()
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
  setText(i++, QString::number(AB_AccountSpec_GetUniqueId(_account)));

  // bank code
  setText(i++, QString::fromUtf8(AB_AccountSpec_GetBankCode(_account)));

  // bank name
//  tmp = AB_Account_GetBankName(_account);
//  if (tmp.isEmpty())
    tmp = i18nc("replacement for institution or account w/o name", "(unnamed)");
  setText(i++, tmp);

  // account id
  setText(i++, QString::fromUtf8(AB_AccountSpec_GetAccountNumber(_account)));

  // account name
  tmp = QString::fromUtf8(AB_AccountSpec_GetAccountName(_account));
  if (tmp.isEmpty())
    tmp = i18nc("replacement for institution or account w/o name", "(unnamed)");
  setText(i++, tmp);

  tmp = QString::fromUtf8(AB_AccountSpec_GetOwnerName(_account));
  if (tmp.isEmpty())
    tmp = "";
  setText(i++, tmp);

  tmp = QString::fromUtf8(AB_AccountSpec_GetBackendName(_account));
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

void KBAccountListView::addAccount(AB_ACCOUNT_SPEC *acc)
{
  new KBAccountListViewItem(this, acc);
}

void KBAccountListView::addAccounts(const std::list<AB_ACCOUNT_SPEC*> &accs)
{
  std::list<AB_ACCOUNT_SPEC*>::const_iterator it;

  for (it = accs.begin(); it != accs.end(); ++it) {
    new KBAccountListViewItem(this, *it);
  } /* for */
}

AB_ACCOUNT_SPEC *KBAccountListView::getCurrentAccount()
{
  KBAccountListViewItem *entry;

  entry = dynamic_cast<KBAccountListViewItem*>(currentItem());
  if (!entry) {
    return 0;
  }
  return entry->getAccount();
}

std::list<AB_ACCOUNT_SPEC*> KBAccountListView::getSelectedAccounts()
{
  std::list<AB_ACCOUNT_SPEC*> accs;
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

std::list<AB_ACCOUNT_SPEC*> KBAccountListView::getSortedAccounts()
{
  std::list<AB_ACCOUNT_SPEC*> accs;
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
