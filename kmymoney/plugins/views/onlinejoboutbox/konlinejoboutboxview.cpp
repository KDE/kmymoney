/*
 * Copyright 2013-2014  Christian Dávid <christian-david@web.de>
 * Copyright 2019       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "konlinejoboutboxview.h"

#include <memory>

#include "ui_konlinejoboutboxview.h"
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
#include "mymoneyexception.h"

#include <QDebug>

class KOnlineJobOutboxViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KOnlineJobOutboxView)

public:
  explicit KOnlineJobOutboxViewPrivate(KOnlineJobOutboxView *qq) :
    KMyMoneyViewBasePrivate(),
    q_ptr(qq),
    ui(new Ui::KOnlineJobOutboxView),
    m_needLoad(true),
    m_onlinePlugins(nullptr),
    m_onlineJobModel(nullptr)
  {
  }

  ~KOnlineJobOutboxViewPrivate()
  {
    if (!m_needLoad) {
      // Save column state
      KConfigGroup configGroup = KSharedConfig::openConfig()->group("KOnlineJobOutboxView");
      configGroup.writeEntry("HeaderState", ui->m_onlineJobView->header()->saveState());
    }
  }

  void init()
  {
    Q_Q(KOnlineJobOutboxView);
    m_needLoad = false;
    ui->setupUi(q);

    // Restore column state
    KConfigGroup configGroup = KSharedConfig::openConfig()->group("KOnlineJobOutboxView");
    QByteArray columns;
    columns = configGroup.readEntry("HeaderState", columns);
    ui->m_onlineJobView->header()->restoreState(columns);

    ui->m_onlineJobView->setModel(this->onlineJobsModel());
    q->connect(ui->m_buttonSend, &QAbstractButton::clicked, q, &KOnlineJobOutboxView::slotSendJobs);
    q->connect(ui->m_buttonRemove, &QAbstractButton::clicked, q, &KOnlineJobOutboxView::slotRemoveJob);
    q->connect(ui->m_buttonEdit, &QAbstractButton::clicked, q, static_cast<void (KOnlineJobOutboxView::*)()>(&KOnlineJobOutboxView::slotEditJob));
    q->connect(ui->m_onlineJobView, &QAbstractItemView::doubleClicked, q, static_cast<void (KOnlineJobOutboxView::*)(const QModelIndex &)>(&KOnlineJobOutboxView::slotEditJob));
    q->connect(ui->m_onlineJobView->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KOnlineJobOutboxView::updateButtonState);

    // Set new credit transfer button
    q->connect(pActions[eMenu::Action::AccountCreditTransfer], &QAction::changed, q, &KOnlineJobOutboxView::updateNewCreditTransferButton);
    q->connect(ui->m_buttonNewCreditTransfer, &QAbstractButton::clicked, q, &KOnlineJobOutboxView::slotNewCreditTransfer);
    q->updateNewCreditTransferButton();
  }

  onlineJobModel* onlineJobsModel()
  {
    Q_Q(KOnlineJobOutboxView);
    if (!m_onlineJobModel) {
      m_onlineJobModel = new onlineJobModel(q);
  #ifdef KMM_MODELTEST
      /// @todo using the ModelTest feature on the onlineJobModel crashes. Need to fix.
      // new ModelTest(m_onlineJobModel, MyMoneyFile::instance());
  #endif
    }
    return m_onlineJobModel;
  }


  void editJob(const QString jobId)
  {
    try {
      const onlineJob constJob = MyMoneyFile::instance()->getOnlineJob(jobId);
      editJob(constJob);
    } catch (const MyMoneyException &) {
      // Prevent a crash in very rare cases
    }
  }

  void editJob(onlineJob job)
  {
    try {
      editJob(onlineJobTyped<creditTransfer>(job));
    } catch (const MyMoneyException &) {
    }
  }

  void editJob(const onlineJobTyped<creditTransfer> job)
  {
    Q_Q(KOnlineJobOutboxView);
    auto transferForm = new kOnlineTransferForm(q);
    transferForm->setOnlineJob(job);
    q->connect(transferForm, &QDialog::rejected, transferForm, &QObject::deleteLater);
    q->connect(transferForm, &kOnlineTransferForm::acceptedForSave, q, &KOnlineJobOutboxView::slotOnlineJobSave);
    q->connect(transferForm, &kOnlineTransferForm::acceptedForSend, q, static_cast<void (KOnlineJobOutboxView::*)(onlineJob)>(&KOnlineJobOutboxView::slotOnlineJobSend));
    q->connect(transferForm, &QDialog::accepted, transferForm, &QObject::deleteLater);
    transferForm->show();
  }

  KOnlineJobOutboxView     *q_ptr;
  std::unique_ptr<Ui::KOnlineJobOutboxView> ui;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;
  QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* m_onlinePlugins;
  onlineJobModel *m_onlineJobModel;
  MyMoneyAccount m_currentAccount;
};

KOnlineJobOutboxView::KOnlineJobOutboxView(QWidget *parent) :
  KMyMoneyViewBase(*new KOnlineJobOutboxViewPrivate(this), parent)
{
  connect(pActions[eMenu::Action::LogOnlineJob], &QAction::triggered, this, static_cast<void (KOnlineJobOutboxView::*)()>(&KOnlineJobOutboxView::slotOnlineJobLog));
  connect(pActions[eMenu::Action::AccountCreditTransfer], &QAction::triggered, this, &KOnlineJobOutboxView::slotNewCreditTransfer);
}

KOnlineJobOutboxView::~KOnlineJobOutboxView()
{
}

void KOnlineJobOutboxView::updateButtonState() const
{
  Q_D(const KOnlineJobOutboxView);
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
      if (job.sendDate().isValid()) {
        /// @todo maybe add a word about unable to edit but able to copy here
        // I don't do it right away since we are in string freeze for 5.0.7
        tooltip = i18n("This job cannot be edited anymore because it was sent already.");
        editable = true;
      } else if (job.isLocked())
        tooltip = i18n("Job is being processed at the moment.");
      else
        Q_ASSERT(false);
    } else if (!onlineJobAdministration::instance()->canEditOnlineJob(job)) {
      editable = false;
      tooltip = i18n("The plugin to edit this job is not available.");
    }
  } else {
    editable = false;
    tooltip = i18n("You need to select a single job for editing.");
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

void KOnlineJobOutboxView::updateNewCreditTransferButton()
{
  Q_D(KOnlineJobOutboxView);
  auto action = pActions[eMenu::Action::AccountCreditTransfer];
  Q_CHECK_PTR(action);
  d->ui->m_buttonNewCreditTransfer->setEnabled(action->isEnabled());
}

void KOnlineJobOutboxView::slotRemoveJob()
{
  Q_D(KOnlineJobOutboxView);
  QAbstractItemModel* model = d->ui->m_onlineJobView->model();
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();

  while (!indexes.isEmpty()) {
    model->removeRow(indexes.at(0).row());
    indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();
  }
}

QStringList KOnlineJobOutboxView::selectedOnlineJobs() const
{
  Q_D(const KOnlineJobOutboxView);
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

void KOnlineJobOutboxView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
  switch(intent) {
    case eView::Intent::UpdateActions:
      updateActions(obj);
      break;

    default:
      break;
  }
}

void KOnlineJobOutboxView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
  Q_D(KOnlineJobOutboxView);
  switch(intent) {
    case eView::Intent::SetOnlinePlugins:
      if (variant.count() == 1)
        d->m_onlinePlugins = static_cast<QMap<QString, KMyMoneyPlugin::OnlinePlugin*>*>(variant.first().value<void*>());
      break;
    default:
      break;
  }
}

void KOnlineJobOutboxView::slotSendJobs()
{
  Q_D(KOnlineJobOutboxView);
  if (d->ui->m_onlineJobView->selectionModel()->hasSelection())
    slotSendSelectedJobs();
  else
    slotSendAllSendableJobs();
}

void KOnlineJobOutboxView::slotSendAllSendableJobs()
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

void KOnlineJobOutboxView::slotSendSelectedJobs()
{
  Q_D(KOnlineJobOutboxView);
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

void KOnlineJobOutboxView::slotEditJob()
{
  Q_D(KOnlineJobOutboxView);
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
    QString jobId = d->ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobId).toString();
    Q_ASSERT(!jobId.isEmpty());
    d->editJob(jobId);
//    emit editJob(jobId);
  }
}

void KOnlineJobOutboxView::slotEditJob(const QModelIndex &index)
{
  if (!pActions[eMenu::Action::EditOnlineJob]->isEnabled())
    return;

  Q_D(KOnlineJobOutboxView);
  auto jobId = d->ui->m_onlineJobView->model()->data(index, onlineJobModel::OnlineJobId).toString();
  d->editJob(jobId);
//  emit editJob(jobId);
}

void KOnlineJobOutboxView::contextMenuEvent(QContextMenuEvent*)
{
  if (!pActions[eMenu::Action::EditOnlineJob]->isEnabled())
    return;

  Q_D(KOnlineJobOutboxView);
  QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedIndexes();
  if (!indexes.isEmpty()) {
//    onlineJob job = d->ui->m_onlineJobView->model()->data(indexes.first(), onlineJobModel::OnlineJobRole).value<onlineJob>();
    pMenus[eMenu::Menu::OnlineJob]->exec(QCursor::pos());
  }
}

/**
 * Do not know why this is needed, but all other views in KMyMoney have it.
 */
void KOnlineJobOutboxView::showEvent(QShowEvent* event)
{
  Q_D(KOnlineJobOutboxView);
  if (d->m_needLoad)
    d->init();

  emit customActionRequested(View::OnlineJobOutbox, eView::Action::AboutToShow);
  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KOnlineJobOutboxView::executeCustomAction(eView::Action action)
{
  Q_D(KOnlineJobOutboxView);
  switch(action) {
    case eView::Action::SetDefaultFocus:
      {
        Q_D(KOnlineJobOutboxView);
        QTimer::singleShot(0, d->ui->m_onlineJobView, SLOT(setFocus()));
      }
      break;

    case eView::Action::InitializeAfterFileOpen:
      d->onlineJobsModel()->load();
      break;

    case eView::Action::CleanupBeforeFileClose:
      d->onlineJobsModel()->unload();
      break;

    default:
      break;
  }
}

void KOnlineJobOutboxView::updateActions(const MyMoneyObject& obj)
{
  Q_D(KOnlineJobOutboxView);
  if (typeid(obj) != typeid(MyMoneyAccount) &&
      (obj.id().isEmpty() && d->m_currentAccount.id().isEmpty())) // do not disable actions that were already disabled)))
    return;

  const auto& acc = static_cast<const MyMoneyAccount&>(obj);
  d->m_currentAccount = acc;
}

void KOnlineJobOutboxView::slotOnlineJobSave(onlineJob job)
{
  MyMoneyFileTransaction fileTransaction;
  if (job.id().isEmpty())
    MyMoneyFile::instance()->addOnlineJob(job);
  else
    MyMoneyFile::instance()->modifyOnlineJob(job);
  fileTransaction.commit();
}

/** @todo when onlineJob queue is used, continue here */
void KOnlineJobOutboxView::slotOnlineJobSend(onlineJob job)
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

void KOnlineJobOutboxView::slotOnlineJobSend(QList<onlineJob> jobs)
{
  Q_D(KOnlineJobOutboxView);
  MyMoneyFile *const kmmFile = MyMoneyFile::instance();
  QMultiMap<QString, onlineJob> jobsByPlugin;

  // Sort jobs by online plugin & lock them
  foreach (onlineJob job, jobs) {
    Q_ASSERT(!job.id().isEmpty());
    // find the provider
    const MyMoneyAccount originAcc = job.responsibleMyMoneyAccount();
    job.setLock();
    job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Debug, "KMyMoney::slotOnlineJobSend", "Added to queue for plugin '" + originAcc.onlineBankingSettings().value("provider").toLower() + '\''));
    MyMoneyFileTransaction fileTransaction;
    kmmFile->modifyOnlineJob(job);
    fileTransaction.commit();
    jobsByPlugin.insert(originAcc.onlineBankingSettings().value("provider").toLower(), job);
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

void KOnlineJobOutboxView::slotOnlineJobLog()
{
  QStringList jobIds = this->selectedOnlineJobs();
  slotOnlineJobLog(jobIds);
}

void KOnlineJobOutboxView::slotOnlineJobLog(const QStringList& onlineJobIds)
{
  onlineJobMessagesView *const dialog = new onlineJobMessagesView();
  onlineJobMessagesModel *const model = new onlineJobMessagesModel(dialog);
  model->setOnlineJob(MyMoneyFile::instance()->getOnlineJob(onlineJobIds.first()));
  dialog->setModel(model);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->show();
  // Note: Objects are not deleted here, Qt's parent-child system has to do that.
}

void KOnlineJobOutboxView::slotNewCreditTransfer()
{
  Q_D(KOnlineJobOutboxView);
  auto *transferForm = new kOnlineTransferForm(this);
  if (!d->m_currentAccount.id().isEmpty()) {
    transferForm->setCurrentAccount(d->m_currentAccount.id());
  }
  connect(transferForm, &QDialog::rejected, transferForm, &QObject::deleteLater);
  connect(transferForm, &kOnlineTransferForm::acceptedForSave, this, &KOnlineJobOutboxView::slotOnlineJobSave);
  connect(transferForm, &kOnlineTransferForm::acceptedForSend, this, static_cast<void (KOnlineJobOutboxView::*)(onlineJob)>(&KOnlineJobOutboxView::slotOnlineJobSend));
  connect(transferForm, &QDialog::accepted, transferForm, &QObject::deleteLater);
  transferForm->show();
}
