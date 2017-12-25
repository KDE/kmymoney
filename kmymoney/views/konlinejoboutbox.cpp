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

#include <memory>

#include "konlinejoboutbox.h"

#include "ui_konlinejoboutbox.h"
#include "kmymoneyviewbase_p.h"
#include "konlinetransferform.h"
#include "kmymoneyplugin.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <QAction>
#include <QMenu>
#include <QTimer>
#include <QModelIndex>
#include <QModelIndexList>
#include <KMessageBox>
#include <KActionCollection>

#include "models.h"
#include "onlinejobmodel.h"
#include "onlinejobadministration.h"
#include "onlinejobtyped.h"
#include "onlinejobmessagesview.h"
#include "onlinejobmessagesmodel.h"
#include "onlinepluginextended.h"

#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "menuenums.h"
#include "mymoneyenums.h"

#include <QDebug>

class KOnlineJobOutboxPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KOnlineJobOutbox)

public:
  explicit KOnlineJobOutboxPrivate(KOnlineJobOutbox *qq) :
    KMyMoneyViewBasePrivate(),
    q_ptr(qq),
    ui(new Ui::KOnlineJobOutbox),
    m_needLoad(true)
  {
  }

  ~KOnlineJobOutboxPrivate()
  {
    if (!m_needLoad) {
      // Save column state
      KConfigGroup configGroup = KSharedConfig::openConfig()->group("KOnlineJobOutbox");
      configGroup.writeEntry("HeaderState", ui->m_onlineJobView->header()->saveState());
    }
  }

  void init()
  {
    Q_Q(KOnlineJobOutbox);
    m_needLoad = false;
    ui->setupUi(q);

    // Restore column state
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("KOnlineJobOutbox");
    QByteArray columns;
    columns = configGroup.readEntry("HeaderState", columns);
    ui->m_onlineJobView->header()->restoreState(columns);

    ui->m_onlineJobView->setModel(Models::instance()->onlineJobsModel());
    q->connect(ui->m_buttonSend, &QAbstractButton::clicked, q, &KOnlineJobOutbox::slotSendJobs);
    q->connect(ui->m_buttonRemove, &QAbstractButton::clicked, q, &KOnlineJobOutbox::slotRemoveJob);
    q->connect(ui->m_buttonEdit, &QAbstractButton::clicked, q, static_cast<void (KOnlineJobOutbox::*)()>(&KOnlineJobOutbox::slotEditJob));
    q->connect(ui->m_onlineJobView, &QAbstractItemView::doubleClicked, q, static_cast<void (KOnlineJobOutbox::*)(const QModelIndex &)>(&KOnlineJobOutbox::slotEditJob));
    q->connect(ui->m_onlineJobView->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KOnlineJobOutbox::updateButtonState);

    // Set new credit transfer button
    q->connect(pActions[eMenu::Action::AccountCreditTransfer], &QAction::changed, q, &KOnlineJobOutbox::updateNewCreditTransferButton);
    q->connect(ui->m_buttonNewCreditTransfer, &QAbstractButton::clicked, q, &KOnlineJobOutbox::slotNewCreditTransfer);
    q->updateNewCreditTransferButton();
  }


  void editJob(const QString jobId)
  {
    try {
      const onlineJob constJob = MyMoneyFile::instance()->getOnlineJob(jobId);
      editJob(constJob);
    } catch (MyMoneyException&) {
      // Prevent a crash in very rare cases
    }
  }

  void editJob(onlineJob job)
  {
    try {
      editJob(onlineJobTyped<creditTransfer>(job));
    } catch (MyMoneyException&) {
    }
  }

  void editJob(const onlineJobTyped<creditTransfer> job)
  {
    Q_Q(KOnlineJobOutbox);
    auto transferForm = new kOnlineTransferForm(q);
    transferForm->setOnlineJob(job);
    q->connect(transferForm, &QDialog::rejected, transferForm, &QObject::deleteLater);
    q->connect(transferForm, &kOnlineTransferForm::acceptedForSave, q, &KOnlineJobOutbox::slotOnlineJobSave);
    q->connect(transferForm, &kOnlineTransferForm::acceptedForSend, q, static_cast<void (KOnlineJobOutbox::*)(onlineJob)>(&KOnlineJobOutbox::slotOnlineJobSend));
    q->connect(transferForm, &QDialog::accepted, transferForm, &QObject::deleteLater);
    transferForm->show();
  }

  KOnlineJobOutbox     *q_ptr;
  std::unique_ptr<Ui::KOnlineJobOutbox> ui;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* m_onlinePlugins;
  MyMoneyAccount m_currentAccount;
};

KOnlineJobOutbox::KOnlineJobOutbox(QWidget *parent) :
  KMyMoneyViewBase(*new KOnlineJobOutboxPrivate(this), parent)
{
  connect(pActions[eMenu::Action::LogOnlineJob], &QAction::triggered, this, static_cast<void (KOnlineJobOutbox::*)()>(&KOnlineJobOutbox::slotOnlineJobLog));
  connect(pActions[eMenu::Action::AccountCreditTransfer], &QAction::triggered, this, &KOnlineJobOutbox::slotNewCreditTransfer);
}

KOnlineJobOutbox::~KOnlineJobOutbox()
{
}

void KOnlineJobOutbox::setDefaultFocus()
{
  Q_D(KOnlineJobOutbox);
  QTimer::singleShot(0, d->ui->m_onlineJobView, SLOT(setFocus()));
}

void KOnlineJobOutbox::updateButtonState() const
{
  Q_D(const KOnlineJobOutbox);
  const QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();
  const int selectedItems = indexes.count();

  // Send button
  //! @todo Enable button if it is useful
  //ui->m_buttonSend->setEnabled(selectedItems > 0);

  // Edit button/action
  bool editable = true;
  QString tooltip;

  if (selectedItems == 1) {
    const onlineJob job = d->ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobRole).value<onlineJob>();

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

  QAction *const onlinejob_edit = pActions[eMenu::Action::EditOnlineJob];
  Q_CHECK_PTR(onlinejob_edit);
  onlinejob_edit->setEnabled(editable);
  onlinejob_edit->setToolTip(tooltip);

  d->ui->m_buttonEdit->setEnabled(editable);
  d->ui->m_buttonEdit->setToolTip(tooltip);

  // Delete button/action
  QAction *const onlinejob_delete = pActions[eMenu::Action::DeleteOnlineJob];
  Q_CHECK_PTR(onlinejob_delete);
  onlinejob_delete->setEnabled(selectedItems > 0);
  d->ui->m_buttonRemove->setEnabled(onlinejob_delete->isEnabled());
}

void KOnlineJobOutbox::updateNewCreditTransferButton()
{
  Q_D(KOnlineJobOutbox);
  auto action = pActions[eMenu::Action::AccountCreditTransfer];
  Q_CHECK_PTR(action);
  d->ui->m_buttonNewCreditTransfer->setEnabled(action->isEnabled());
}

void KOnlineJobOutbox::slotRemoveJob()
{
  Q_D(KOnlineJobOutbox);
  QAbstractItemModel* model = d->ui->m_onlineJobView->model();
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();

  while (!indexes.isEmpty()) {
    model->removeRow(indexes.at(0).row());
    indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();
  }
}

QStringList KOnlineJobOutbox::selectedOnlineJobs() const
{
  Q_D(const KOnlineJobOutbox);
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();

  if (indexes.isEmpty())
    return QStringList();

  QStringList list;
  list.reserve(indexes.count());

  const QAbstractItemModel *const model = d->ui->m_onlineJobView->model();
  Q_FOREACH(const QModelIndex& index, indexes) {
    list.append(model->data(index, onlineJobModel::OnlineJobId).toString());
  }
  return list;
}

void KOnlineJobOutbox::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins)
{
  Q_D(KOnlineJobOutbox);
  d->m_onlinePlugins = &plugins;
}

void KOnlineJobOutbox::slotSendJobs()
{
  Q_D(KOnlineJobOutbox);
  if (d->ui->m_onlineJobView->selectionModel()->hasSelection())
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
    slotOnlineJobSend(validJobs);
//    emit sendJobs(validJobs);
}

void KOnlineJobOutbox::slotSendSelectedJobs()
{
  Q_D(KOnlineJobOutbox);
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();
  if (indexes.isEmpty())
    return;

  // Valid jobs to send
  QList<onlineJob> validJobs;
  validJobs.reserve(indexes.count());

  // Get valid jobs
  const QAbstractItemModel *const model = d->ui->m_onlineJobView->model();
  foreach (const QModelIndex& index, indexes) {
    onlineJob job = model->data(index, onlineJobModel::OnlineJobRole).value<onlineJob>();
    if (job.isValid() && job.isEditable())
      validJobs.append(job);
  }

  // Abort if not all jobs can be sent
  if (validJobs.count() != indexes.count()) {
    KMessageBox::information(this, i18nc("The user selected credit transfers to send. But they cannot be sent.",
                                         "Cannot send selection"),
                             i18n("Not all selected credit transfers can be sent because some of them are invalid or were already sent."));
    return;
  }

  slotOnlineJobSend(validJobs);
//  emit sendJobs(validJobs);
}

void KOnlineJobOutbox::slotEditJob()
{
  Q_D(KOnlineJobOutbox);
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
    QString jobId = d->ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobId).toString();
    Q_ASSERT(!jobId.isEmpty());
    d->editJob(jobId);
//    emit editJob(jobId);
  }
}

void KOnlineJobOutbox::slotEditJob(const QModelIndex &index)
{
  Q_D(KOnlineJobOutbox);
  auto jobId = d->ui->m_onlineJobView->model()->data(index, onlineJobModel::OnlineJobId).toString();
  d->editJob(jobId);
//  emit editJob(jobId);
}

void KOnlineJobOutbox::contextMenuEvent(QContextMenuEvent*)
{
  Q_D(KOnlineJobOutbox);
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
//    onlineJob job = d->ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobRole).value<onlineJob>();
    pMenus[eMenu::Menu::OnlineJob]->exec();
  }
}

/**
 * Do not know why this is needed, but all other views in KMyMoney have it.
 */
void KOnlineJobOutbox::showEvent(QShowEvent* event)
{
  Q_D(KOnlineJobOutbox);
  if (d->m_needLoad)
    d->init();

  emit aboutToShow(View::OnlineJobOutbox);
  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KOnlineJobOutbox::updateActions(const MyMoneyObject& obj)
{
  Q_D(KOnlineJobOutbox);
  if (typeid(obj) != typeid(MyMoneyAccount) &&
      (obj.id().isEmpty() && d->m_currentAccount.id().isEmpty())) // do not disable actions that were already disabled)))
    return;

  const auto& acc = static_cast<const MyMoneyAccount&>(obj);
  d->m_currentAccount = acc;
}

void KOnlineJobOutbox::slotOnlineJobSave(onlineJob job)
{
  MyMoneyFileTransaction fileTransaction;
  if (job.id().isEmpty())
    MyMoneyFile::instance()->addOnlineJob(job);
  else
    MyMoneyFile::instance()->modifyOnlineJob(job);
  fileTransaction.commit();
}

/** @todo when onlineJob queue is used, continue here */
void KOnlineJobOutbox::slotOnlineJobSend(onlineJob job)
{
  MyMoneyFileTransaction fileTransaction;
  if (job.id().isEmpty())
    MyMoneyFile::instance()->addOnlineJob(job);
  else
    MyMoneyFile::instance()->modifyOnlineJob(job);
  fileTransaction.commit();

  QList<onlineJob> jobList;
  jobList.append(job);
  slotOnlineJobSend(jobList);
}

void KOnlineJobOutbox::slotOnlineJobSend(QList<onlineJob> jobs)
{
  Q_D(KOnlineJobOutbox);
  MyMoneyFile *const kmmFile = MyMoneyFile::instance();
  QMultiMap<QString, onlineJob> jobsByPlugin;

  // Sort jobs by online plugin & lock them
  foreach (onlineJob job, jobs) {
    Q_ASSERT(!job.id().isEmpty());
    // find the provider
    const MyMoneyAccount originAcc = job.responsibleMyMoneyAccount();
    job.setLock();
    job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Debug, "KMyMoneyApp::slotOnlineJobSend", "Added to queue for plugin '" + originAcc.onlineBankingSettings().value("provider") + '\''));
    MyMoneyFileTransaction fileTransaction;
    kmmFile->modifyOnlineJob(job);
    fileTransaction.commit();
    jobsByPlugin.insert(originAcc.onlineBankingSettings().value("provider"), job);
  }

  // Send onlineJobs to plugins
  QList<QString> usedPlugins = jobsByPlugin.keys();
  std::sort(usedPlugins.begin(), usedPlugins.end());
  const QList<QString>::iterator newEnd = std::unique(usedPlugins.begin(), usedPlugins.end());
  usedPlugins.erase(newEnd, usedPlugins.end());

  foreach (const QString& pluginKey, usedPlugins) {
    QMap<QString, KMyMoneyPlugin::OnlinePlugin*>::const_iterator it_p = d->m_onlinePlugins->constFind(pluginKey);

    if (it_p != d->m_onlinePlugins->constEnd()) {
      // plugin found, call it
      KMyMoneyPlugin::OnlinePluginExtended *pluginExt = dynamic_cast< KMyMoneyPlugin::OnlinePluginExtended* >(*it_p);
      if (pluginExt == 0) {
        qWarning("Job given for plugin which is not an extended plugin");
        continue;
      }
      //! @fixme remove debug message
      qDebug() << "Sending " << jobsByPlugin.count(pluginKey) << " job(s) to online plugin " << pluginKey;
      QList<onlineJob> jobsToExecute = jobsByPlugin.values(pluginKey);
      QList<onlineJob> executedJobs = jobsToExecute;
      pluginExt->sendOnlineJob(executedJobs);

      // Save possible changes of the online job and remove lock
      MyMoneyFileTransaction fileTransaction;
      foreach (onlineJob job, executedJobs) {
        fileTransaction.restart();
        job.setLock(false);
        kmmFile->modifyOnlineJob(job);
        fileTransaction.commit();
      }

      if (Q_UNLIKELY(executedJobs.size() != jobsToExecute.size())) {
        // OnlinePlugin did not return all jobs
        qWarning() << "Error saving send online tasks. After restart you should see at minimum all successfully executed jobs marked send. Imperfect plugin: " << pluginExt->objectName();
      }

    } else {
      qWarning() << "Error, got onlineJob for an account without online plugin.";
      /** @FIXME can this actually happen? */
    }
  }
}

void KOnlineJobOutbox::slotOnlineJobLog()
{
  QStringList jobIds = this->selectedOnlineJobs();
  slotOnlineJobLog(jobIds);
}

void KOnlineJobOutbox::slotOnlineJobLog(const QStringList& onlineJobIds)
{
  onlineJobMessagesView *const dialog = new onlineJobMessagesView();
  onlineJobMessagesModel *const model = new onlineJobMessagesModel(dialog);
  model->setOnlineJob(MyMoneyFile::instance()->getOnlineJob(onlineJobIds.first()));
  dialog->setModel(model);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
  // Note: Objects are not deleted here, Qt's parent-child system has to do that.
}

void KOnlineJobOutbox::slotNewCreditTransfer()
{
  Q_D(KOnlineJobOutbox);
  auto *transferForm = new kOnlineTransferForm(this);
  if (!d->m_currentAccount.id().isEmpty()) {
    transferForm->setCurrentAccount(d->m_currentAccount.id());
  }
  connect(transferForm, &QDialog::rejected, transferForm, &QObject::deleteLater);
  connect(transferForm, &kOnlineTransferForm::acceptedForSave, this, &KOnlineJobOutbox::slotOnlineJobSave);
  connect(transferForm, &kOnlineTransferForm::acceptedForSend, this, static_cast<void (KOnlineJobOutbox::*)(onlineJob)>(&KOnlineJobOutbox::slotOnlineJobSend));
  connect(transferForm, &QDialog::accepted, transferForm, &QObject::deleteLater);
  transferForm->show();
}
