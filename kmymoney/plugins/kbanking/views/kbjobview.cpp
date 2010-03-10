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

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include "kbjobview.h"

#include <qevent.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <QHBoxLayout>

#include <kpushbutton.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "kbanking.h"
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>

#include <gwenhywfar/debug.h>


#define BUTTON_WIDTH 110


KBJobView::KBJobView(KBanking *kb,
                     QWidget* parent,
                     const char* name,
                     Qt::WFlags fl)
    : QWidget(parent, fl), _app(kb)
{
  assert(kb);
  setObjectName(name);
  setupUi(this);

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  QBoxLayout *jobBoxLayout = new QHBoxLayout(jobBox);
  jobBoxLayout->setAlignment(Qt::AlignTop);

  _jobList = new KBJobListView(jobBox);
  jobBoxLayout->addWidget(_jobList);

  QObject::connect(_app->flagStaff(), SIGNAL(signalQueueUpdated()),
                   this, SLOT(slotQueueUpdated()));
  QObject::connect(executeButton, SIGNAL(clicked()),
                   this, SLOT(slotExecute()));
  QObject::connect(dequeueButton, SIGNAL(clicked()),
                   this, SLOT(slotDequeue()));
  connect(_jobList, SIGNAL(selectionChanged()),
          this, SLOT(slotSelectionChanged()));

  // add some icons to the buttons
  KIconLoader* il = KIconLoader::global();
  KGuiItem dequeueItem(i18n("Dequeue"),
                       KIcon(il->loadIcon("edit-delete-shred", KIconLoader::Small, KIconLoader::SizeSmall)),
                       i18n("Dequeue selected job"),
                       i18n("Remove the selected job from the list"));
  KGuiItem executeItem(i18n("Execute"),
                       KIcon(il->loadIcon("system-run", KIconLoader::Small, KIconLoader::SizeSmall)),
                       i18n("Execute all jobs in the queue"),
                       i18n("Execute all jobs in the queue"));

  dequeueButton->setGuiItem(dequeueItem);
  executeButton->setGuiItem(executeItem);
  dequeueButton->setToolTip(dequeueItem.toolTip());
  executeButton->setToolTip(executeItem.toolTip());
}



KBJobView::~KBJobView()
{
}

void KBJobView::slotSelectionChanged(void)
{
  dequeueButton->setEnabled(_jobList->selectedItem() != 0);
}

bool KBJobView::init()
{
#if !AQB_IS_VERSION(3,9,0,0)
  GWEN_DB_NODE *db;
  db = _app->getAppData();
  assert(db);
  db = GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                        "gui/views/jobview");
  if (db) {
    int i, j;

    /* found settings */
    for (i = 0; i < _jobList->columns(); i++) {
      _jobList->setColumnWidthMode(i, Q3ListView::Manual);
      j = GWEN_DB_GetIntValue(db, "columns", i, -1);
      if (j != -1)
        _jobList->setColumnWidth(i, j);
    } /* for */
  } /* if settings */
#endif
  _jobList->addJobs(_app->getEnqueuedJobs());

  return true;
}



bool KBJobView::fini()
{
#if !AQB_IS_VERSION(3,9,0,0)
  GWEN_DB_NODE *db;
  int i, j;

  db = _app->getAppData();
  assert(db);
  assert(db);
  GWEN_DB_ClearGroup(db, "gui/views/jobview");
  for (i = 0; i < _jobList->columns(); ++i) {
    j = _jobList->columnWidth(i);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                        "gui/views/jobview/columns", j);
  } /* for */
#endif
  return true;
}


void KBJobView::slotQueueUpdated()
{
  DBG_NOTICE(0, "Job queue updated");
  _jobList->clear();
  std::list<AB_JOB*> jl;
  jl = _app->getEnqueuedJobs();
  _jobList->addJobs(jl);
  executeButton->setEnabled(jl.size() > 0);
  if (jl.size() == 0)
    dequeueButton->setDisabled(true);
}



void KBJobView::slotExecute()
{
  std::list<AB_JOB*> jl;
  int rv;
  bool updated;
  AB_IMEXPORTER_CONTEXT *ctx;

  updated = false;
  jl = _app->getEnqueuedJobs();
  if (jl.size() == 0) {
    KMessageBox::warningContinueCancel(this,
                                       tr("No Jobs"),
                                       tr("There are no jobs in the queue."));
    return;
  }

  DBG_NOTICE(0, "Executing queue");
  ctx = AB_ImExporterContext_new();
  rv = _app->executeQueue(ctx);
  if (!rv)
    _app->importContext(ctx, 0);
  else {
    DBG_ERROR(0, "Error: %d", rv);
  }
  AB_ImExporterContext_free(ctx);

  // let App emit signals to inform account views
  _app->accountsUpdated();
}



void KBJobView::slotDequeue()
{
  KBJobListViewItem* p = dynamic_cast<KBJobListViewItem*>(_jobList->selectedItem());
  if (p) {
    AB_JOB* job = p->getJob();
    if (job) {
      _app->dequeueJob(job);
    }
  }
}









