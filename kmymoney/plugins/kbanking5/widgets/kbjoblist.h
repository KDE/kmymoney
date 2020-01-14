/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
 *   Copyright 2010  Thomas Baumgart ipwizard@users.sourceforge.net        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>  *
 ***************************************************************************/
#ifndef AQHBCI_KBJOBLIST_H
#define AQHBCI_KBJOBLIST_H


#include <QTreeWidget>
#include <aqbanking/job.h>

#include <list>


class KBJobListView;
class KBJobListViewItem;


class KBJobListViewItem: public QTreeWidgetItem
{
private:
  AB_JOB *_job;

  void _populate();

public:
  KBJobListViewItem(KBJobListView *parent, AB_JOB *j);
  KBJobListViewItem(const KBJobListViewItem &item);

  virtual ~KBJobListViewItem();

  AB_JOB *getJob();
};



class KBJobListView: public QTreeWidget
{
private:
public:
  KBJobListView(QWidget *parent = 0);
  virtual ~KBJobListView();

  void addJob(AB_JOB *j);
  void addJobs(const std::list<AB_JOB*> &js);

  AB_JOB *getCurrentJob();
  std::list<AB_JOB*> getSelectedJobs();
};




#endif /* AQHBCI_KBJOBLIST_H */
