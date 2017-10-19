/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2013-2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "konlinejoboutbox.h"
#include "ui_konlinejoboutbox.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QAction>
#include <QTimer>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMessageBox>
#include <KActionCollection>

#include "models/models.h"
#include "models/onlinejobmodel.h"
#include "onlinejobadministration.h"

#include "mymoney/mymoneyfile.h"
#include "kmymoney.h"

#include <QDebug>

KOnlineJobOutbox::KOnlineJobOutbox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KOnlineJobOutbox),
    m_needLoad(true)
{
}

KOnlineJobOutbox::~KOnlineJobOutbox()
{
  if (!m_needLoad) {
    // Save column state
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("KOnlineJobOutbox");
    configGroup.writeEntry("HeaderState", ui->m_onlineJobView->header()->saveState());
  }
}

void KOnlineJobOutbox::setDefaultFocus()
{
  QTimer::singleShot(0, ui->m_onlineJobView, SLOT(setFocus()));
}

void KOnlineJobOutbox::init()
{
  m_needLoad = false;
  ui->setupUi(this);

  // Restore column state
  KConfigGroup configGroup = KSharedConfig::openConfig()->group("KOnlineJobOutbox");
  QByteArray columns;
  columns = configGroup.readEntry("HeaderState", columns);
  ui->m_onlineJobView->header()->restoreState(columns);

  ui->m_onlineJobView->setModel(Models::instance()->onlineJobsModel());
  connect(ui->m_buttonSend, SIGNAL(clicked()), this, SLOT(slotSendJobs()));
  connect(ui->m_buttonRemove, SIGNAL(clicked()), this, SLOT(slotRemoveJob()));
  connect(ui->m_buttonEdit, SIGNAL(clicked()), this, SLOT(slotEditJob()));
  connect(ui->m_onlineJobView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotEditJob(QModelIndex)));
  connect(ui->m_onlineJobView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtonState()));

  // Set new credit transfer button
  connect(kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::AccountCreditTransfer]), SIGNAL(changed()), SLOT(updateNewCreditTransferButton()));
  connect(ui->m_buttonNewCreditTransfer, SIGNAL(clicked()), this, SIGNAL(newCreditTransfer()));
  updateNewCreditTransferButton();
}

void KOnlineJobOutbox::updateButtonState() const
{
  const QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedRows();
  const int selectedItems = indexes.count();

  // Send button
  //! @todo Enable button if it is useful
  //ui->m_buttonSend->setEnabled(selectedItems > 0);

  // Edit button/action
  bool editable = true;
  QString tooltip;

  if (selectedItems == 1) {
    const onlineJob job = ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobRole).value<onlineJob>();

    if (!job.isEditable()) {
      editable = false;
      if (job.sendDate().isValid())
        tooltip = i18n("This job cannot be edited anymore because is was sent already.");
      else if (job.isLocked())
        tooltip = i18n("Job is being processed at the moment.");
      else
        Q_ASSERT(false);
    } else if (!onlineJobAdministration::instance()->canEditOnlineJob(job)) {
      editable = false;
      tooltip = i18n("The plugin to edit this job is not available.");
    }
  } else {
    editable = false;
    tooltip = i18n("You must select a single job for editing.");
  }

  QAction *const onlinejob_edit = kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::OnlineJobEdit]);
  Q_CHECK_PTR(onlinejob_edit);
  onlinejob_edit->setEnabled(editable);
  onlinejob_edit->setToolTip(tooltip);

  ui->m_buttonEdit->setEnabled(editable);
  ui->m_buttonEdit->setToolTip(tooltip);

  // Delete button/action
  QAction *const onlinejob_delete = kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::OnlineJobDelete]);
  Q_CHECK_PTR(onlinejob_delete);
  onlinejob_delete->setEnabled(selectedItems > 0);
  ui->m_buttonRemove->setEnabled(onlinejob_delete->isEnabled());
}

void KOnlineJobOutbox::updateNewCreditTransferButton()
{
  QAction* action = kmymoney->actionCollection()->action(kmymoney->s_Actions[Action::AccountCreditTransfer]);
  Q_CHECK_PTR(action);
  ui->m_buttonNewCreditTransfer->setEnabled(action->isEnabled());
}

void KOnlineJobOutbox::slotRemoveJob()
{
  QAbstractItemModel* model = ui->m_onlineJobView->model();
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedRows();

  while (!indexes.isEmpty()) {
    model->removeRow(indexes.at(0).row());
    indexes = ui->m_onlineJobView->selectionModel()->selectedRows();
  }
}

QStringList KOnlineJobOutbox::selectedOnlineJobs() const
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedRows();

  if (indexes.isEmpty())
    return QStringList();

  QStringList list;
  list.reserve(indexes.count());

  const QAbstractItemModel *const model = ui->m_onlineJobView->model();
  Q_FOREACH(const QModelIndex& index, indexes) {
    list.append(model->data(index, onlineJobModel::OnlineJobId).toString());
  }
  return list;
}

void KOnlineJobOutbox::slotSendJobs()
{
  if (ui->m_onlineJobView->selectionModel()->hasSelection())
    slotSendSelectedJobs();
  else
    slotSendAllSendableJobs();
}

void KOnlineJobOutbox::slotSendAllSendableJobs()
{
  QList<onlineJob> validJobs;
  foreach (const onlineJob& job, MyMoneyFile::instance()->onlineJobList()) {
    if (job.isValid() && job.isEditable())
      validJobs.append(job);
  }
  qDebug() << "I shall send " << validJobs.count() << "/" << MyMoneyFile::instance()->onlineJobList().count() << " onlineJobs";
  if (!validJobs.isEmpty())
    emit sendJobs(validJobs);
}

void KOnlineJobOutbox::slotSendSelectedJobs()
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedRows();
  if (indexes.isEmpty())
    return;

  // Valid jobs to send
  QList<onlineJob> validJobs;
  validJobs.reserve(indexes.count());

  // Get valid jobs
  const QAbstractItemModel *const model = ui->m_onlineJobView->model();
  foreach (const QModelIndex& index, indexes) {
    onlineJob job = model->data(index, onlineJobModel::OnlineJobRole).value<onlineJob>();
    if (job.isValid() && job.isEditable())
      validJobs.append(job);
  }

  // Abort if not all jobs can be sent
  if (validJobs.count() != indexes.count()) {
    QMessageBox::information(this, i18nc("The user selected credit transfers to send. But they cannot be sent.",
                                         "Cannot send selection"),
                             i18n("Not all selected credit transfers can be sent because some of them are invalid or were already sent."));
    return;
  }

  emit sendJobs(validJobs);
}

void KOnlineJobOutbox::slotEditJob()
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
    QString jobId = ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobId).toString();
    Q_ASSERT(!jobId.isEmpty());
    emit editJob(jobId);
  }
}

void KOnlineJobOutbox::slotEditJob(const QModelIndex &index)
{
  QString jobId = ui->m_onlineJobView->model()->data(index, onlineJobModel::OnlineJobId).toString();
  emit editJob(jobId);
}

void KOnlineJobOutbox::contextMenuEvent(QContextMenuEvent*)
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
    onlineJob job = ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobRole).value<onlineJob>();
    emit showContextMenu(job);
  }
}

/**
 * Do not know why this is needed, but all other views in KMyMoney have it.
 */
void KOnlineJobOutbox::showEvent(QShowEvent* event)
{
  if (m_needLoad)
    init();

  emit aboutToShow();
  // don't forget base class implementation
  QWidget::showEvent(event);
}
