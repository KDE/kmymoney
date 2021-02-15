/*
    SPDX-FileCopyrightText: 2009 Cristian Onet onet.cristian @gmail.com
    SPDX-FileCopyrightText: 2018 Martin Preuss aquamaniac @users.sourceforge.net
    SPDX-FileCopyrightText: 2010 Thomas Baumgart ipwizard @users.sourceforge.net
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef AQHBCI_KBJOBLIST_H
#define AQHBCI_KBJOBLIST_H


#include <QTreeWidget>
#include <aqbanking/types/transaction.h>

#include <list>


class KBJobListView;
class KBJobListViewItem;


class KBJobListViewItem: public QTreeWidgetItem
{
private:
  AB_TRANSACTION *_job;

  void _populate();

public:
  KBJobListViewItem(KBJobListView *parent, AB_TRANSACTION *j);
  KBJobListViewItem(const KBJobListViewItem &item);

  virtual ~KBJobListViewItem();

  AB_TRANSACTION *getJob();
};



class KBJobListView: public QTreeWidget
{
private:
public:
  explicit KBJobListView(QWidget *parent = 0);
  virtual ~KBJobListView();

  void addJob(AB_TRANSACTION *j);
  void addJobs(const std::list<AB_TRANSACTION*> &js);

  AB_TRANSACTION *getCurrentJob();
  std::list<AB_TRANSACTION*> getSelectedJobs();
};




#endif /* AQHBCI_KBJOBLIST_H */
