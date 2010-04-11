/***************************************************************************
 *   Copyright 2009  Cristian Onet onet.cristian@gmail.com                 *
 *   Copyright 2004  Martin Preuss aquamaniac@users.sourceforge.net        *
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
#ifndef KBANKING_KBJOBVIEW_H
#define KBANKING_KBJOBVIEW_H

#include "ui_kbjobview.h"

#include <QWidget>


class KBJobView;


#include "kbjoblist.h"
#include "kbanking.h"


class KBJobView: public QWidget, public Ui::KBJobViewUi
{
  Q_OBJECT
public:
  explicit KBJobView(KBanking *kb,
                     QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
  ~KBJobView();

  bool init();
  bool fini();

protected:
  //void resizeEvent(QResizeEvent *e);

private:
  KBanking *_app;
  KBJobListView *_jobList;

protected slots:
  void slotQueueUpdated();
  void slotExecute();
  void slotDequeue();
  void slotSelectionChanged();
};







#endif /* KBANKING_KBJOBVIEW_H */



