/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#include <KMessageBox>
#include <QAction>
#include <QModelIndexList>

#include "models/models.h"
#include "models/onlinejobmodel.h"

#include "mymoney/mymoneyfile.h"
#include "kmymoney.h"

#include <QDebug>

KOnlineJobOutbox::KOnlineJobOutbox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KOnlineJobOutbox)
{
    ui->setupUi(this);

    // Restore column state
    KConfigGroup configGroup = KGlobal::config()->group("KOnlineJobOutbox");
    QByteArray columns;
    columns = configGroup.readEntry("HeaderState", columns);
    ui->m_onlineJobView->header()->restoreState(columns);

    ui->m_onlineJobView->setModel( Models::instance()->onlineJobsModel() );
    connect(ui->m_buttonSend, SIGNAL(clicked()), this, SLOT(slotSendJobs()));
    connect(ui->m_buttonRemove, SIGNAL(clicked()), this, SLOT(slotRemoveJob()));
    connect(ui->m_buttonEdit, SIGNAL(clicked()), this, SLOT(slotEditJob()));
    connect(ui->m_onlineJobView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotEditJob(QModelIndex)));
    connect(ui->m_onlineJobView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtonState()));

    // Set new credit transfer button
    connect(kmymoney->action("account_online_new_credit_transfer"), SIGNAL(changed()), SLOT(updateNewCreditTransferButton()));
    connect(ui->m_buttonNewCreditTransfer, SIGNAL(clicked()), this, SIGNAL(newCreditTransfer()));
    updateNewCreditTransferButton();
}

KOnlineJobOutbox::~KOnlineJobOutbox()
{
    // Save column state
    KConfigGroup configGroup = KGlobal::config()->group("KOnlineJobOutbox");
    configGroup.writeEntry("HeaderState", ui->m_onlineJobView->header()->saveState());

    delete ui;
}

void KOnlineJobOutbox::updateButtonState() const
{
  const int selectedItems = ui->m_onlineJobView->selectionModel()->selectedRows().count();
  ui->m_buttonSend->setEnabled(selectedItems > 0);
  ui->m_buttonEdit->setEnabled(selectedItems == 1);
  ui->m_buttonRemove->setEnabled(selectedItems > 0);
}

void KOnlineJobOutbox::updateNewCreditTransferButton()
{
  QAction* action = kmymoney->action("account_online_new_credit_transfer");
  Q_CHECK_PTR( action );
  ui->m_buttonNewCreditTransfer->setEnabled( action->isEnabled() );
}

void KOnlineJobOutbox::slotRemoveJob()
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedRows();

  if (indexes.isEmpty())
    return;

  QAbstractItemModel* model = ui->m_onlineJobView->model();
  const int count = indexes.count();
  for ( int i = count-1; i >= 0; --i ) {
    model->removeRow( indexes.at(i).row() );
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
  Q_FOREACH( const QModelIndex& index, indexes ) {
    list.append(model->data( index, onlineJobModel::OnlineJobId ).toString());
  }
  return list;
}


void KOnlineJobOutbox::slotSendJobs()
{
  QList<onlineJob> validJobs;
  foreach( onlineJob job, MyMoneyFile::instance()->onlineJobList()) {
    if (job.isValid())
      validJobs.append(job);
  }
  qDebug() << "I shall send " << validJobs.count() << "/" << MyMoneyFile::instance()->onlineJobList().count() << " onlineJobs";
  if ( !validJobs.isEmpty() )
    emit sendJobs( validJobs );
}

void KOnlineJobOutbox::slotEditJob()
{
  QModelIndexList indexes = ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
    QString jobId = ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobId).toString();
    Q_ASSERT( !jobId.isEmpty() );
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
  emit aboutToShow();
  // don't forget base class implementation
  QWidget::showEvent(event);
}
