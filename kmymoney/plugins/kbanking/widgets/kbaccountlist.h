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

#ifndef KBACCOUNTLIST_H
#define KBACCOUNTLIST_H


#include <QTreeWidget>

#include <aqbanking/account.h>

#include <list>

class KBAccountListView;
class KBAccountListViewItem;


class KBAccountListViewItem: public QTreeWidgetItem
{
private:
  AB_ACCOUNT *_account;

  void _populate();
  bool operator< (const QTreeWidgetItem & other) const;   //!< correctly sort text columns, which contain numbers

public:
  KBAccountListViewItem(KBAccountListView *parent, AB_ACCOUNT *acc);
  KBAccountListViewItem(KBAccountListView *parent,
                        QTreeWidgetItem *after,
                        AB_ACCOUNT *acc);
  KBAccountListViewItem(const KBAccountListViewItem &item);

  virtual ~KBAccountListViewItem();

  AB_ACCOUNT *getAccount();
};



class KBAccountListView: public QTreeWidget
{
private:
public:
  explicit KBAccountListView(QWidget *parent = 0);
  virtual ~KBAccountListView();

  void addAccount(AB_ACCOUNT *acc);
  void addAccounts(const std::list<AB_ACCOUNT*> &accs);

  AB_ACCOUNT *getCurrentAccount();
  std::list<AB_ACCOUNT*> getSelectedAccounts();

  std::list<AB_ACCOUNT*> getSortedAccounts();

};




#endif /* QBANKING_ACCOUNTLIST_H */



