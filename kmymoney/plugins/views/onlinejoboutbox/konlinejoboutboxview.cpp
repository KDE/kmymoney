/*
    SPDX-FileCopyrightText: 2013-2014 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konlinejoboutboxview.h"

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QModelIndex>
#include <QModelIndexList>
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KMessageBox>
#include <KActionCollection>
#include <KXMLGUIFactory>

// ----------------------------------------------------------------------------
// Project Includes

#include "columnselector.h"
#include "icons.h"
#include "kmymoneyplugin.h"
#include "kmymoneyviewbase_p.h"
#include "konlinetransferform.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "onlinejobadministration.h"
#include "onlinejobmessagesmodel.h"
#include "onlinejobmessagesview.h"
#include "onlinejobsmodel.h"
#include "onlinejobtyped.h"
#include "onlinepluginextended.h"

#include "ui_konlinejoboutboxview.h"

using namespace Icons;

class KOnlineJobOutboxViewPrivate : public KMyMoneyViewBasePrivate
{
    Q_DECLARE_PUBLIC(KOnlineJobOutboxView)

public:
    explicit KOnlineJobOutboxViewPrivate(KOnlineJobOutboxView* qq)
        : KMyMoneyViewBasePrivate(qq)
        , ui(new Ui::KOnlineJobOutboxView)
        , m_needLoad(true)
        , m_onlinePlugins(nullptr)
        , m_actionCollection(nullptr)
        , m_contextMenu(nullptr)
        , m_filterModel(nullptr)
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

        auto columnSelector = new ColumnSelector(ui->m_onlineJobView, q->metaObject()->className());
        columnSelector->setAlwaysVisible(QVector<int>({OnlineJobsModel::Columns::PostDate,
                                                       OnlineJobsModel::Columns::AccountName,
                                                       OnlineJobsModel::Columns::Destination,
                                                       OnlineJobsModel::Columns::Value}));

        m_filterModel = new QSortFilterProxyModel(q);
        m_filterModel->setSourceModel(MyMoneyFile::instance()->onlineJobsModel());
        ui->m_onlineJobView->setModel(m_filterModel);
        columnSelector->setModel(m_filterModel);

        ui->m_onlineJobView->setSortingEnabled(true);
        ui->m_onlineJobView->header()->restoreState(columns);
        ui->m_onlineJobView->header()->setSortIndicatorShown(true);

        ui->m_buttonSend->setDefaultAction(m_actions[eMenu::OnlineAction::SendOnlineJobs]);
        ui->m_buttonRemove->setDefaultAction(m_actions[eMenu::OnlineAction::DeleteOnlineJob]);
        ui->m_buttonEdit->setDefaultAction(m_actions[eMenu::OnlineAction::EditOnlineJob]);
        ui->m_buttonNewCreditTransfer->setDefaultAction(m_actions[eMenu::OnlineAction::AccountCreditTransfer]);

        q->connect(ui->m_onlineJobView, &QAbstractItemView::doubleClicked, q, static_cast<void (KOnlineJobOutboxView::*)(const QModelIndex &)>(&KOnlineJobOutboxView::slotEditJob));
        q->connect(ui->m_onlineJobView->selectionModel(), &QItemSelectionModel::selectionChanged, q, &KOnlineJobOutboxView::updateSelection);
        q->connect(onlineJobAdministration::instance(), &onlineJobAdministration::canSendCreditTransferChanged, m_actions[eMenu::OnlineAction::AccountCreditTransfer], &QAction::setEnabled);

        m_focusWidget = ui->m_onlineJobView;
    }

    void setSortRole(int column)
    {
        if (column == OnlineJobsModel::Columns::PostDate) {
            m_filterModel->setSortRole(eMyMoney::Model::OnlineJobPostDateRole);
        } else if (column == OnlineJobsModel::Columns::Value) {
            m_filterModel->setSortRole(eMyMoney::Model::OnlineJobValueAsDoubleRole);
        } else {
            m_filterModel->setSortRole(Qt::DisplayRole);
        }
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

    std::unique_ptr<Ui::KOnlineJobOutboxView> ui;

    /**
      * This member holds the load state of page
      */
    bool m_needLoad;
    QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* m_onlinePlugins;
    KActionCollection* m_actionCollection;
    QMenu* m_contextMenu;
    QSortFilterProxyModel* m_filterModel;
    MyMoneyAccount m_currentAccount;
    QHash<eMenu::OnlineAction, QAction*> m_actions;
};

KOnlineJobOutboxView::KOnlineJobOutboxView(QWidget *parent) :
    KMyMoneyViewBase(*new KOnlineJobOutboxViewPrivate(this), parent)
{
}

KOnlineJobOutboxView::~KOnlineJobOutboxView()
{
}

void KOnlineJobOutboxView::createActions(KXMLGUIFactory* guiFactory, KXMLGUIClient* guiClient)
{
    typedef void(KOnlineJobOutboxView::*ViewFunc)();
    struct actionInfo {
        QString             name;
        ViewFunc            callback;
        QString             text;
        Icon                icon;
        eMenu::OnlineAction id;
    };

    const QVector<actionInfo> actionInfos {
        {QStringLiteral("onlinejob_send"),    &KOnlineJobOutboxView::slotSendJobs,          i18n("Send transfer"),            Icon::MailSend,       eMenu::OnlineAction::SendOnlineJobs},
        {QStringLiteral("onlinejob_new"),     &KOnlineJobOutboxView::slotNewCreditTransfer, i18n("New credit transfer"),      Icon::OnlineTransfer, eMenu::OnlineAction::AccountCreditTransfer},
        {QStringLiteral("onlinejob_delete"),  &KOnlineJobOutboxView::slotRemoveJob,         i18n("Remove transfer"),          Icon::EditShred,      eMenu::OnlineAction::DeleteOnlineJob},
        {QStringLiteral("onlinejob_edit"),    &KOnlineJobOutboxView::slotEditJob,           i18n("Edit transfer"),            Icon::DocumentEdit,   eMenu::OnlineAction::EditOnlineJob},
        {QStringLiteral("onlinejob_log"),     &KOnlineJobOutboxView::slotOnlineJobLog,      i18n("Show log"),                 Icon::Empty,          eMenu::OnlineAction::LogOnlineJob},
    };

    Q_D(KOnlineJobOutboxView);
    d->m_actionCollection = guiClient->actionCollection();
    for (const auto& actionInfo : actionInfos) {
        QAction *action = d->m_actionCollection->addAction(actionInfo.name, this, actionInfo.callback);
        action->setText(actionInfo.text);
        action->setIcon(Icons::get(actionInfo.icon));
        d->m_actions.insert(actionInfo.id, action);
    }

    // create context menu
    d->m_contextMenu = qobject_cast<QMenu*>(guiFactory->container(QStringLiteral("onlinejob_context_menu"), guiClient));

    // For some unknown reason, the context menu does not get created this way from the .rc file.
    // I must be doing something wrong / don't understand something. This kxmlgui thingy
    // remains a mystery to me. Apparently, I am also too stupid to get the window tile showing up
    if (!d->m_contextMenu) {
        d->m_contextMenu = new QMenu(this);
        d->m_contextMenu->addSection(i18nc("@title:menu Online job context menu", "Credit transfer options"));
        for (const auto& actionInfo : actionInfos) {
            d->m_contextMenu->insertAction(nullptr, d->m_actions[actionInfo.id]);
        }
    }

    d->m_sharedToolbarActions.insert(eMenu::Action::FileNew, d->m_actions[eMenu::OnlineAction::AccountCreditTransfer]);
}

void KOnlineJobOutboxView::removeActions()
{
}

void KOnlineJobOutboxView::updateActions(const SelectedObjects& selections)
{
    Q_D(const KOnlineJobOutboxView);

    // Edit button/action
    bool editable = true;
    QString tooltip;

    // in case we're not initialized yet, we simply return
    if (d->m_needLoad) {
        return;
    }

    // no model available: bail out
    const auto model = d->ui->m_onlineJobView->model();
    if (model == nullptr) {
        return;
    }

    const auto rows = model->rowCount();
    bool sendableItems = false;

    for (auto row = 0; row < rows; ++row) {
        const auto idx = model->index(row, 0);
        if (idx.data(eMyMoney::Model::OnlineJobSendableRole).toBool()) {
            sendableItems = true;
            break;
        }
    }

    if (selections.count(SelectedObjects::OnlineJob) == 1) {
        const QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();
        const auto jobIdx = indexes.first();

        sendableItems = jobIdx.data(eMyMoney::Model::OnlineJobSendableRole).toBool();

        if (!jobIdx.data(eMyMoney::Model::OnlineJobEditableRole).toBool()) {
            editable = false;
            if (jobIdx.data(eMyMoney::Model::OnlineJobSendDateRole).toDate().isValid()) {
                /// @todo maybe add a word about unable to edit but able to copy here
                // I don't do it right away since we are in string freeze for 5.0.7
                tooltip = i18n("This job cannot be edited anymore because it was sent already.");
                editable = true;
            } else if (jobIdx.data(eMyMoney::Model::OnlineJobLockedRole).toBool())
                tooltip = i18n("Job is being processed at the moment.");
            else
                Q_ASSERT(false);
        } else if (!onlineJobAdministration::instance()->canEditOnlineJob(jobIdx.data(eMyMoney::Model::IdRole).toString())) {
            editable = false;
            tooltip = i18n("The plugin to edit this job is not available.");
        }
        d->m_actions[eMenu::OnlineAction::SendOnlineJobs]->setText(i18n("Send transfer"));
    } else {
        editable = false;
        tooltip = i18n("You need to select a single job for editing.");
        d->m_actions[eMenu::OnlineAction::SendOnlineJobs]->setText(i18n("Send transfers"));
    }

    d->m_actions[eMenu::OnlineAction::EditOnlineJob]->setEnabled(editable);
    d->m_actions[eMenu::OnlineAction::EditOnlineJob]->setToolTip(tooltip);

    d->m_actions[eMenu::OnlineAction::DeleteOnlineJob]->setEnabled(!selections.isEmpty(SelectedObjects::OnlineJob));

    d->m_actions[eMenu::OnlineAction::SendOnlineJobs]->setEnabled(sendableItems);
}

void KOnlineJobOutboxView::updateSelection()
{
    Q_D(KOnlineJobOutboxView);
    const QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();
    d->m_selections.clearSelections();
    for (const auto idx : indexes) {
        d->m_selections.addSelection(SelectedObjects::OnlineJob, idx.data(eMyMoney::Model::IdRole).toString());
    }

    Q_EMIT requestSelectionChange(d->m_selections);
}

void KOnlineJobOutboxView::slotRemoveJob()
{
    MyMoneyFileTransaction ft;
    try {
        MyMoneyFile::instance()->removeOnlineJob(selectedOnlineJobs());
        ft.commit();
    } catch (MyMoneyException& e) {
    }
    return;
}

QStringList KOnlineJobOutboxView::selectedOnlineJobs() const
{
    Q_D(const KOnlineJobOutboxView);
    QModelIndexList indexes = d->ui->m_onlineJobView->selectionModel()->selectedRows();

    if (indexes.isEmpty())
        return QStringList();

    QStringList jobIds;
    jobIds.reserve(indexes.count());
    for (const auto& idx : indexes) {
        jobIds << idx.data(eMyMoney::Model::IdRole).toString();
    }
    return jobIds;
}

void KOnlineJobOutboxView::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* plugins)
{
    Q_D(KOnlineJobOutboxView);
    d->m_onlinePlugins = plugins;
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
//    Q_EMIT sendJobs(validJobs);
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
    for (const auto& idx : indexes) {
        onlineJob job = idx.data(eMyMoney::Model::OnlineJobRole).value<onlineJob>();
        if (job.isValid() && job.isEditable())
            validJobs.append(job);
    }

    // Abort if not all jobs can be sent
    if (validJobs.count() != indexes.count()) {
        KMessageBox::information(this, i18nc("The user selected credit transfers to send. But they cannot be sent",
                                             "Not all selected credit transfers can be sent because some of them are invalid or were already sent."),
                                 i18nc("@title:window Online transfers", "Cannot send transfers"));
        return;
    }

    slotOnlineJobSend(validJobs);
//  Q_EMIT sendJobs(validJobs);
}

void KOnlineJobOutboxView::slotEditJob()
{
    Q_D(KOnlineJobOutboxView);
    const auto indexes = d->ui->m_onlineJobView->selectionModel()->selectedIndexes();
    if (!indexes.isEmpty()) {
        const auto jobId = indexes.first().data(eMyMoney::Model::IdRole).toString();
        Q_ASSERT(!jobId.isEmpty());
        d->editJob(jobId);
//    Q_EMIT editJob(jobId);
    }
}

void KOnlineJobOutboxView::slotEditJob(const QModelIndex &index)
{
    Q_D(KOnlineJobOutboxView);
    if (!d->m_actions[eMenu::OnlineAction::EditOnlineJob]->isEnabled())
        return;

    auto jobId = index.data(eMyMoney::Model::IdRole).toString();
    d->editJob(jobId);
//  Q_EMIT editJob(jobId);
}

void KOnlineJobOutboxView::contextMenuEvent(QContextMenuEvent*)
{
    Q_D(KOnlineJobOutboxView);
    if (d->m_contextMenu) {
        d->m_contextMenu->exec(QCursor::pos());
    } else {
        qDebug() << "No context menu assigned in KOnlineJobOutboxView";
    }
}

/**
 * Do not know why this is needed, but all other views in KMyMoney have it.
 */
void KOnlineJobOutboxView::showEvent(QShowEvent* event)
{
    Q_D(KOnlineJobOutboxView);

    if (d->m_needLoad) {
        d->init();
        connect(d->ui->m_onlineJobView->header(), &QHeaderView::sortIndicatorChanged, this, [&](int logicalIndex, Qt::SortOrder order) {
            Q_D(KOnlineJobOutboxView);
            Q_UNUSED(order)
            d->setSortRole(logicalIndex);
        });
        const auto header = d->ui->m_onlineJobView->header();
        d->setSortRole(header->sortIndicatorSection());
        d->ui->m_onlineJobView->sortByColumn(header->sortIndicatorSection(), header->sortIndicatorOrder());
    }
    // don't forget base class implementation
    QWidget::showEvent(event);
}

void KOnlineJobOutboxView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_UNUSED(action)
    Q_UNUSED(selections)
}

QString KOnlineJobOutboxView::slotOnlineJobSave(onlineJob job)
{
    MyMoneyFileTransaction fileTransaction;
    if (job.id().isEmpty())
        MyMoneyFile::instance()->addOnlineJob(job);
    else
        MyMoneyFile::instance()->modifyOnlineJob(job);
    fileTransaction.commit();

    return job.id();
}

void KOnlineJobOutboxView::slotOnlineJobSend(onlineJob job)
{
    // in case a complete new job is provided by the caller
    // there is no id assigned to the job (yet). This will
    // happen in the call to slotOnlineJobSave(). All we
    // need to do after the job is created, we need to
    // reload it from the engine to get it with the id
    // filled in.
    const auto jobId = slotOnlineJobSave(job);

    if (job.id().isEmpty() && !jobId.isEmpty()) {
        job = MyMoneyFile::instance()->getOnlineJob(jobId);
    }

    slotOnlineJobSend(QList<onlineJob>{job});
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
        job.addJobMessage(onlineJobMessage(eMyMoney::OnlineJob::MessageType::Debug, "KMyMoneyApp::slotOnlineJobSend", "Added to queue for plugin '" + originAcc.onlineBankingSettings().value("provider").toLower() + '\''));
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
    slotOnlineJobLog(selectedOnlineJobs());
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
