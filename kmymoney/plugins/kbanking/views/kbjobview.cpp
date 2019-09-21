/*
 *   Copyright 2004       Martin Preuss <aquamaniac@users.sourceforge.net>
 *   Copyright 2009       Cristian Onet <onet.cristian@gmail.com>
 *   Copyright 2010       Thomas Baumgart <tbaumgart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifdef HAVE_CONFIG_H
# include <config-kmymoney.h>
#endif

#include "kbjobview.h"

#include <QEvent>
#include <QLayout>
#include <QToolTip>
#include <QHBoxLayout>

#include <QPushButton>
#include <KGuiItem>
#include <KLocale>
#include <KGlobal>
#include <KIconLoader>
#include <KMessageBox>

#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>

#include <gwenhywfar/debug.h>

#include <ui_kbjobview.h>

struct KBJobView::Private {
  Ui::KBJobView ui;
};

KBJobView::KBJobView(KMyMoneyBanking *kb,
                     QWidget* parent,
                     const char* name,
                     Qt::WindowFlags fl) :
    QWidget(parent, fl),
    d(new Private),
    m_app(kb)
{
  assert(kb);
  setObjectName(name);
  d->ui.setupUi(this);

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  QBoxLayout *jobBoxLayout = new QHBoxLayout(d->ui.jobBox);
  jobBoxLayout->setAlignment(Qt::AlignTop);

  m_jobList = new KBJobListView(d->ui.jobBox);
  jobBoxLayout->addWidget(m_jobList);

  QObject::connect(d->ui.executeButton, SIGNAL(clicked()),
                   this, SLOT(slotExecute()));
  QObject::connect(d->ui.dequeueButton, SIGNAL(clicked()),
                   this, SLOT(slotDequeue()));
  connect(m_jobList, SIGNAL(itemSelectionChanged()),
          this, SLOT(slotSelectionChanged()));

  // add some icons to the buttons
  KIconLoader* il = KIconLoader::global();
  KGuiItem dequeueItem(i18n("Dequeue"),
                       il->loadIcon("edit-delete-shred", KIconLoader::Small, KIconLoader::SizeSmall),
                       i18n("Dequeue selected job"),
                       i18n("Remove the selected job from the list"));
  KGuiItem executeItem(i18n("Execute"),
                       il->loadIcon("system-run", KIconLoader::Small, KIconLoader::SizeSmall),
                       i18n("Execute all jobs in the queue"),
                       i18n("Execute all jobs in the queue"));

  KGuiItem::assign(d->ui.dequeueButton, dequeueItem);
  KGuiItem::assign(d->ui.executeButton, executeItem);
  d->ui.dequeueButton->setToolTip(dequeueItem.toolTip());
  d->ui.executeButton->setToolTip(executeItem.toolTip());

  d->ui.dequeueButton->setEnabled(false);
  d->ui.executeButton->setEnabled(false);
}



KBJobView::~KBJobView()
{
  delete d;
}

void KBJobView::slotSelectionChanged(void)
{
  d->ui.dequeueButton->setEnabled(false);
  if (m_jobList->currentItem())
    d->ui.dequeueButton->setEnabled(m_jobList->currentItem()->isSelected() != 0);
}

bool KBJobView::init()
{
  slotQueueUpdated();
  return true;
}



bool KBJobView::fini()
{
  return true;
}


void KBJobView::slotQueueUpdated()
{
  DBG_NOTICE(0, "Job queue updated");
  m_jobList->clear();
  std::list<AB_JOB*> jl;
  jl = m_app->getEnqueuedJobs();
  m_jobList->addJobs(jl);
  d->ui.executeButton->setEnabled(jl.size() > 0);
  slotSelectionChanged();
}



void KBJobView::slotExecute()
{
  if (m_app->getEnqueuedJobs().size() == 0) {
    KMessageBox::warningContinueCancel(this,
                                       i18nc("Warning message", "There are no jobs in the queue."),
                                       i18nc("Message title", "No Jobs"));
    return;
  }

  DBG_NOTICE(0, "Executing queue");

  AB_IMEXPORTER_CONTEXT *ctx;
  ctx = AB_ImExporterContext_new();

  int rv = m_app->executeQueue(ctx);
  if (rv == 0)
    m_app->importContext(ctx, 0);
  else {
    DBG_ERROR(0, "Error: %d", rv);
  }
  AB_ImExporterContext_free(ctx);
}



void KBJobView::slotDequeue()
{
  KBJobListViewItem* p = dynamic_cast<KBJobListViewItem*>(m_jobList->currentItem());
  if (p && p->isSelected()) {
    AB_JOB* job = p->getJob();
    if (job) {
      m_app->dequeueJob(job);
    }
  }
}









