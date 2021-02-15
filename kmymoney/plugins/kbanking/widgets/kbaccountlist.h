/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef KBACCOUNTLIST_H
#define KBACCOUNTLIST_H


#include <QTreeWidget>

#include <aqbanking/types/account_spec.h>

#include <list>

class KBAccountListView;
class KBAccountListViewItem;


class KBAccountListViewItem: public QTreeWidgetItem
{
private:
  AB_ACCOUNT_SPEC *_account;

  void _populate();
  bool operator< (const QTreeWidgetItem & other) const final override;   //!< correctly sort text columns, which contain numbers

public:
  KBAccountListViewItem(KBAccountListView *parent, AB_ACCOUNT_SPEC *acc);
  KBAccountListViewItem(KBAccountListView *parent,
                        QTreeWidgetItem *after,
                        AB_ACCOUNT_SPEC *acc);
  KBAccountListViewItem(const KBAccountListViewItem &item);

  virtual ~KBAccountListViewItem();

  AB_ACCOUNT_SPEC *getAccount();
};



class KBAccountListView: public QTreeWidget
{
private:
public:
  explicit KBAccountListView(QWidget *parent = 0);
  virtual ~KBAccountListView();

  void addAccount(AB_ACCOUNT_SPEC *acc);
  void addAccounts(const std::list<AB_ACCOUNT_SPEC*> &accs);

  AB_ACCOUNT_SPEC *getCurrentAccount();
  std::list<AB_ACCOUNT_SPEC*> getSelectedAccounts();

  std::list<AB_ACCOUNT_SPEC*> getSortedAccounts();

};




#endif /* QBANKING_ACCOUNTLIST_H */



