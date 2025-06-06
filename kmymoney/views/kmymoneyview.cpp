/*
    SPDX-FileCopyrightText: 2000-2001 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017, 2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <config-kmymoney.h>
#include "kmymoneyview.h"

// ----------------------------------------------------------------------------
// Std Includes

#include <memory>

// ----------------------------------------------------------------------------
// QT Includes

#include <QFile>
#include <QLayout>
#include <QList>
#include <QByteArray>
#include <QUrl>
#include <QIcon>
#include <QTemporaryFile>
#include <QUrlQuery>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KActionCollection>
#include <KBackup>
#include <KDualAction>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KTitleWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "accountsmodel.h"
#include "equitiesmodel.h"
#include "icons.h"
#include "journalmodel.h"
#include "kaccountsview.h"
#include "kcategoriesview.h"
#include "kcurrencyeditdlg.h"
#include "khomeview.h"
#include "kinstitutionsview.h"
#include "kinvestmentview.h"
#include "kmymoneyaccounttreeview.h"
#include "kmymoneyplugin.h"
#include "kmymoneysettings.h"
#include "kpayeesview.h"
#include "kscheduledview.h"
#include "ktagsview.h"
#include "menuenums.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "mymoneypayee.h"
#include "mymoneyprice.h"
#include "mymoneyreport.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytag.h"
#include "mymoneyutils.h"
#include "onlinejobadministration.h"
#include "securitiesmodel.h"
#include "selectedobjects.h"
#include "simpleledgerview.h"

using namespace Icons;
using namespace eMyMoney;

class KMyMoneyViewPrivate
{
    Q_DECLARE_PUBLIC(KMyMoneyView)

public:
    KMyMoneyViewPrivate(KMyMoneyView* qq)
        : q_ptr(qq)
        , m_model(nullptr)
    {
    }

    /**
     * Returns the id of the current selected view
     */
    View currentViewId() const
    {
        Q_Q(const KMyMoneyView);
        return viewFrames.key(q->currentPage());
    }

    /**
     *Returns the id of the view identified by the pointer
     */
    View viewIdByWidget(KMyMoneyViewBase* widget)
    {
        return viewBases.key(widget, View::Home);
    }

    void selectSharedActions(View viewId)
    {
        Q_Q(KMyMoneyView);
        const auto view = viewBases.value(viewId, nullptr);
        if (view) {
            const auto sharedActions = view->sharedToolbarActions();
            if (sharedActions.isEmpty()) {
                // reset to default
                Q_EMIT q->selectSharedActionButton(eMenu::Action::None, nullptr);
            } else {
                for (auto it = sharedActions.cbegin(); it != sharedActions.cend(); ++it) {
                    Q_EMIT q->selectSharedActionButton(it.key(), it.value());
                }
            }
        }
    }

    void addSharedActions(KMyMoneyViewBase* view)
    {
        Q_Q(KMyMoneyView);
        const auto sharedActions = view->sharedToolbarActions();
        for (auto it = sharedActions.cbegin(); it != sharedActions.cend(); ++it) {
            Q_EMIT q->addSharedActionButton(it.key(), it.value());
        }
        selectSharedActions(currentViewId());
    }

    void switchView(eMenu::Action action)
    {
        Q_Q(KMyMoneyView);
        const static QHash<eMenu::Action, View> actionRoutes = {
            {eMenu::Action::GoToPayee, View::Payees},
            {eMenu::Action::OpenAccount, View::NewLedgers},
            {eMenu::Action::GoToAccount, View::NewLedgers},
            {eMenu::Action::StartReconciliation, View::NewLedgers},
            {eMenu::Action::ReportOpen, View::Reports},
            {eMenu::Action::ReportAccountTransactions, View::Reports},
            {eMenu::Action::FileClose, View::Home},
        };

        const auto viewId = actionRoutes.value(action, View::None);

        if (viewId != View::None) {
            q->showPage(viewId);
        }
    }

    void _q_selectionChangeRequested(const SelectedObjects& selection)
    {
        Q_Q(KMyMoneyView);
        // Only emit for current view and if selection has really changed
        if (q->sender() == viewBases[currentViewId()]) {
            if (selection != m_lastSelection) {
                m_lastSelection = selection;
                Q_EMIT q->requestSelectionChange(selection);
            }
        }
    }

    KMyMoneyView* q_ptr;
    KPageWidgetModel* m_model;
    SelectedObjects m_lastSelection;

    QHash<View, KPageWidgetItem*> viewFrames;
    QHash<View, KMyMoneyViewBase*> viewBases;

    struct viewInfo {
        View id;
        QString name;
        Icon icon;
    };

    // clang-format off
    const QVector<viewInfo> viewsInfo
    {
        {View::Home,            i18n("Home"),                         Icon::Home},
        {View::Institutions,    i18n("Institutions"),                 Icon::Institutions},
        {View::Accounts,        i18n("Accounts"),                     Icon::Accounts},
        {View::Schedules,       i18n("Scheduled\ntransactions"),      Icon::Schedule},
        {View::Categories,      i18n("Categories"),                   Icon::FinancialCategories},
        {View::Tags,            i18n("Tags"),                         Icon::Tags},
        {View::Payees,          i18n("Payees"),                       Icon::Payees},
        {View::NewLedgers,      i18n("Ledgers"),                      Icon::Ledgers},
        {View::Investments,     i18n("Investments"),                  Icon::Investments},
    };
    // clang-format on
};




KMyMoneyView::KMyMoneyView()
    : KPageWidget(nullptr)
    , d_ptr(new KMyMoneyViewPrivate(this))
{
    Q_D(KMyMoneyView);
    // this is a workaround for the bug in KPageWidget that causes the header to be shown
    // for a short while during page switch which causes a kind of bouncing of the page's
    // content and if the page's content is at it's minimum size then during a page switch
    // the main window's size is also increased to fit the header that is shown for a sort
    // period - reading the code in kpagewidget.cpp we know that the header should be at (1,1)
    // in a grid layout so if we find it there remove it for good to avoid the described issues
    QGridLayout* gridLayout =  qobject_cast<QGridLayout*>(layout());
    if (gridLayout) {
        QLayoutItem* headerItem = gridLayout->itemAtPosition(1, 1);
        // make sure that we remove only the header - we avoid surprises if the header is not at (1,1) in the layout
        if (headerItem && qobject_cast<KTitleWidget*>(headerItem->widget()) != nullptr) {
            gridLayout->removeItem(headerItem);
        }
    }

    d->m_model = new KPageWidgetModel(this); // cannot be parentless, otherwise segfaults at exit

    const auto homeView = new KHomeView;
    const auto accountsView = new KAccountsView;
    d->viewBases[View::Home] = homeView;
    d->viewBases[View::Institutions] = new KInstitutionsView;
    d->viewBases[View::Accounts] = accountsView;
    d->viewBases[View::Schedules] = new KScheduledView;
    d->viewBases[View::Categories] = new KCategoriesView;
    d->viewBases[View::Tags] = new KTagsView;
    d->viewBases[View::Payees] = new KPayeesView;
    d->viewBases[View::NewLedgers] = new SimpleLedgerView;
    d->viewBases[View::Investments] = new KInvestmentView;

    for (const auto& view : d->viewsInfo) {
        addView(d->viewBases[view.id], view.name, view.id, view.icon);
    }

    // set the model
    setModel(d->m_model);
    setCurrentPage(d->viewFrames[View::Home]);
    connect(this, &KMyMoneyView::currentPageChanged, this, &KMyMoneyView::slotSwitchView);

    // suppress update of homepage during statement import
    connect(accountsView, &KAccountsView::beginImportingStatements, homeView, &KHomeView::slotDisableRefresh);
    connect(accountsView, &KAccountsView::endImportingStatements, homeView, &KHomeView::slotEnableRefresh);

    updateViewType();
}

KMyMoneyView::~KMyMoneyView()
{
}

void KMyMoneyView::updateViewType()
{
    Q_D(KMyMoneyView);

    // set the face type
    KPageView::FaceType faceType = KPageView::List;
    switch (KMyMoneySettings::viewType()) {
    case 0:
        faceType = KPageView::List;
        break;
    case 1:
        faceType = KPageView::Tree;
        break;
    case 2:
        faceType = KPageView::Tabbed;
        break;
    }
    if (faceType != KMyMoneyView::faceType()) {
        // swap line break with non-breakable space and vice versa
        for (auto view : d->viewFrames) {
            const auto linebreak = QString::fromUtf8("\xe2\x80\xa8");
            const auto space = QString::fromUtf8("\xe2\x80\x87");
            auto name = view->name();
            if (faceType == Tabbed) {
                name.replace(linebreak, space);
            } else {
                name.replace(space, linebreak);
            }
            view->setName(name);
        }

        setFaceType(faceType);
        if (faceType == KPageView::Tree) {
            const QList<QTreeView*> views = findChildren<QTreeView*>();
            for (QTreeView* view : views) {
                if (view && (view->parent() == this)) {
                    view->setRootIsDecorated(false);
                    break;
                }
            }
        }
    }
}

void KMyMoneyView::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>* plugins)
{
    // propagate to all views
    Q_EMIT onlinePluginsChanged(plugins);
}

eDialogs::ScheduleResultCode KMyMoneyView::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{
    Q_D(KMyMoneyView);
    return static_cast<KScheduledView*>(d->viewBases[View::Schedules])->enterSchedule(schedule, autoEnter, extendedKeys);
}

void KMyMoneyView::addView(KMyMoneyViewBase* view, const QString& name, View idView, Icons::Icon icon)
{
    Q_D(KMyMoneyView);
    /* There is a bug in
     *    static int layoutText(QTextLayout *layout, int maxWidth)
     *    from kpageview_p.cpp from kwidgetsaddons.
     *    The method doesn't break strings that are too long. Following line
     *    workarounds this by using LINE SEPARATOR character which is accepted by
     *    QTextLayout::createLine().*/
    auto adjustedName(name);
    adjustedName.replace(QLatin1Char('\n'), QString::fromUtf8("\xe2\x80\xa8"));

    auto isViewInserted = false;
    for (auto i = (int)idView; i < (int)View::None; ++i) {
        if (d->viewFrames.contains((View)i)) {
            d->viewFrames[idView] = d->m_model->insertPage(d->viewFrames[(View)i],view, adjustedName);
            isViewInserted = true;
            break;
        }
    }

    if (!isViewInserted)
        d->viewFrames[idView] = d->m_model->addPage(view, adjustedName);

    d->viewFrames[idView]->setIcon(Icons::get(icon));
    d->viewBases[idView] = view;
    connect(view, &KMyMoneyViewBase::requestCustomContextMenu, this, &KMyMoneyView::requestCustomContextMenu);
    connect(view, &KMyMoneyViewBase::requestActionTrigger, this, &KMyMoneyView::requestActionTrigger);
    connect(this, &KMyMoneyView::settingsChanged, view, &KMyMoneyViewBase::slotSettingsChanged);
    connect(this, &KMyMoneyView::onlinePluginsChanged, view, &KMyMoneyViewBase::setOnlinePlugins);

    connect(view, &KMyMoneyViewBase::viewStateChanged, d->viewFrames[idView], &KPageWidgetItem::setEnabled);
    connect(view, SIGNAL(requestSelectionChange(SelectedObjects)), this, SLOT(_q_selectionChangeRequested(SelectedObjects)));
    connect(view, &KMyMoneyViewBase::requestView, this, &KMyMoneyView::switchView);

    d->addSharedActions(view);
}

void KMyMoneyView::setupSharedActions()
{
    Q_D(KMyMoneyView);
    for (const auto& view : d->viewsInfo) {
        d->addSharedActions(d->viewBases[view.id]);
    }
}

void KMyMoneyView::removeView(View idView)
{
    Q_D(KMyMoneyView);
    auto view = d->viewBases.value(idView, nullptr);
    if (!view)
        return;

    disconnect(view, &KMyMoneyViewBase::viewStateChanged, d->viewFrames[idView], &KPageWidgetItem::setEnabled);
    disconnect(view, SIGNAL(requestSelectionChange(SelectedObjects)), this, SLOT(_q_selectionChangeRequested(SelectedObjects)));

    disconnect(view, &KMyMoneyViewBase::requestCustomContextMenu, this, &KMyMoneyView::requestCustomContextMenu);
    disconnect(view, &KMyMoneyViewBase::requestActionTrigger, this, &KMyMoneyView::requestActionTrigger);
    disconnect(this, &KMyMoneyView::settingsChanged, view, &KMyMoneyViewBase::slotSettingsChanged);
    disconnect(this, &KMyMoneyView::onlinePluginsChanged, view, &KMyMoneyViewBase::setOnlinePlugins);

    d->m_model->removePage(d->viewFrames[idView]);
    d->viewFrames.remove(idView);
    d->viewBases.remove(idView);
}

void KMyMoneyView::updateActions(const SelectedObjects& selections)
{
    Q_D(KMyMoneyView);

    const auto currentView = d->currentViewId();
    const auto file = MyMoneyFile::instance();

    pActions[eMenu::Action::NewTransaction]->setDisabled(true);
    pActions[eMenu::Action::EditTransaction]->setDisabled(true);
    pActions[eMenu::Action::EditSplits]->setDisabled(true);
    pActions[eMenu::Action::DeleteTransaction]->setDisabled(true);
    pActions[eMenu::Action::DuplicateTransaction]->setDisabled(true);
    pActions[eMenu::Action::AddReversingTransaction]->setDisabled(true);
    pActions[eMenu::Action::DisplayTransactionDetails]->setDisabled(true);
    pActions[eMenu::Action::CopySplits]->setDisabled(true);
    pActions[eMenu::Action::MarkNotReconciled]->setDisabled(true);
    pActions[eMenu::Action::MarkCleared]->setDisabled(true);
    pActions[eMenu::Action::MarkReconciled]->setDisabled(true);
    pActions[eMenu::Action::SelectAllTransactions]->setEnabled(false);
    pActions[eMenu::Action::MatchTransaction]->setEnabled(false);
    pActions[eMenu::Action::AcceptTransaction]->setEnabled(false);
    pActions[eMenu::Action::NewScheduledTransaction]->setEnabled(false);
    pActions[eMenu::Action::StartReconciliation]->setEnabled(false);
    pActions[eMenu::Action::PostponeReconciliation]->setEnabled(false);
    pActions[eMenu::Action::FinishReconciliation]->setEnabled(false);
    pActions[eMenu::Action::CancelReconciliation]->setEnabled(false);
    pActions[eMenu::Action::MoveToToday]->setEnabled(false);
    pActions[eMenu::Action::TransactionOpenURL]->setEnabled(false);

    // update actions in all views. process the current last
    const auto viewBasesKeys = d->viewBases.keys();
    for (const auto& view : qAsConst(viewBasesKeys)) {
        if (view == currentView)
            continue;
        d->viewBases[view]->updateActions(selections);
    }
    d->viewBases[currentView]->updateActions(selections);

    // global actions
    // --------------
    if (!selections.selection(SelectedObjects::JournalEntry).isEmpty()) {
        const auto enabled = MyMoneyUtils::transactionWarnLevel(selections.selection(SelectedObjects::JournalEntry)) < OneSplitFrozen;
        pActions[eMenu::Action::MarkNotReconciled]->setEnabled(enabled);
        pActions[eMenu::Action::MarkCleared]->setEnabled(enabled);
        pActions[eMenu::Action::MarkReconciled]->setEnabled(enabled);
    }

    if (selections.selection(SelectedObjects::ReconciliationAccount).isEmpty() && (selections.selection(SelectedObjects::Account).count() == 1)) {
        const auto idx = MyMoneyFile::instance()->accountsModel()->indexById(selections.firstSelection(SelectedObjects::Account));
        const auto disabled = idx.data(eMyMoney::Model::AccountIsClosedRole).toBool();
        pActions[eMenu::Action::StartReconciliation]->setDisabled(disabled);
    }

    switch (d->currentViewId()) {
    case View::NewLedgers:
        pActions[eMenu::Action::NewTransaction]->setEnabled(true);
        // intentional fall through

    case View::Payees:
        pActions[eMenu::Action::SelectAllTransactions]->setEnabled(true);
        if (selections.selection(SelectedObjects::JournalEntry).isEmpty()) {
            pActions[eMenu::Action::EditTransaction]->setDisabled(true);
            pActions[eMenu::Action::EditSplits]->setDisabled(true);
            pActions[eMenu::Action::DeleteTransaction]->setDisabled(true);
            pActions[eMenu::Action::DuplicateTransaction]->setDisabled(true);
            pActions[eMenu::Action::AddReversingTransaction]->setDisabled(true);
            pActions[eMenu::Action::CopySplits]->setDisabled(true);
            pActions[eMenu::Action::MoveToToday]->setDisabled(true);
            pActions[eMenu::Action::TransactionOpenURL]->setDisabled(true);
        } else {
            const auto warnLevel = MyMoneyUtils::transactionWarnLevel(selections.selection(SelectedObjects::JournalEntry));
            pActions[eMenu::Action::EditTransaction]->setEnabled(true);
            pActions[eMenu::Action::EditSplits]->setEnabled(selections.selection(SelectedObjects::JournalEntry).count() == 1);
            pActions[eMenu::Action::DeleteTransaction]->setEnabled(warnLevel <= OneSplitReconciled);
            pActions[eMenu::Action::DuplicateTransaction]->setEnabled(warnLevel <= OneSplitReconciled);
            pActions[eMenu::Action::AddReversingTransaction]->setEnabled(warnLevel <= OneSplitReconciled);
            pActions[eMenu::Action::DisplayTransactionDetails]->setEnabled(true);
            pActions[eMenu::Action::CopySplits]->setDisabled(true);
            pActions[eMenu::Action::MoveToToday]->setEnabled(true);
            // TODO: limit to transactions with a related payee
            pActions[eMenu::Action::TransactionOpenURL]->setEnabled(true);

            int singleSplitTransactions(0);
            int multipleSplitTransactions(0);
            int matchedTransactions(0);
            int importedTransactions(0);

            const auto journalEntryIds = selections.selection(SelectedObjects::JournalEntry);
            for (const auto& journalEntryId : qAsConst(journalEntryIds)) {
                const auto idx = file->journalModel()->indexById(journalEntryId);
                if ((singleSplitTransactions < 1) || (multipleSplitTransactions < 2)) {
                    const auto indeces = file->journalModel()->indexesByTransactionId(idx.data(eMyMoney::Model::JournalTransactionIdRole).toString());
                    switch (indeces.count()) {
                    case 0:
                        break;
                    case 1:
                        singleSplitTransactions++;
                        break;
                    default:
                        multipleSplitTransactions++;
                        break;
                    }
                }

                if (idx.data(eMyMoney::Model::JournalSplitIsMatchedRole).toBool()) {
                    ++matchedTransactions;
                }
                if (idx.data(eMyMoney::Model::TransactionIsImportedRole).toBool()) {
                    ++importedTransactions;
                }
                if ((singleSplitTransactions > 0) && (multipleSplitTransactions > 1) && (matchedTransactions > 0) && (importedTransactions > 0)) {
                    break;
                }
            }

            if (singleSplitTransactions > 0 && multipleSplitTransactions == 1) {
                pActions[eMenu::Action::CopySplits]->setEnabled(true);
            }

            // Matching is enabled as soon as one regular and one imported transaction is selected
            if (selections.selection(SelectedObjects::JournalEntry).count() == 2 /* && pActions[Action::TransactionEdit]->isEnabled() */) {
                qobject_cast<KDualAction*>(pActions[eMenu::Action::MatchTransaction])->setActive(true);
                pActions[eMenu::Action::MatchTransaction]->setEnabled(true);
            }
            if ((importedTransactions > 0) || (matchedTransactions > 0)) {
                pActions[eMenu::Action::AcceptTransaction]->setEnabled(true);
            }
            if (matchedTransactions > 0) {
                pActions[eMenu::Action::MatchTransaction]->setEnabled(true);
                qobject_cast<KDualAction*>(pActions[eMenu::Action::MatchTransaction])->setActive(false);
            }

            const auto accountId = selections.firstSelection(SelectedObjects::Account);
            if (!accountId.isEmpty()) {
                const auto idx = file->accountsModel()->indexById(accountId);
                if ((idx.data(eMyMoney::Model::AccountIsAssetLiabilityRole).toBool() == true)
                    && (idx.data(eMyMoney::Model::AccountTypeRole).value<eMyMoney::Account::Type>() != eMyMoney::Account::Type::Investment)) {
                    pActions[eMenu::Action::NewScheduledTransaction]->setEnabled(selections.selection(SelectedObjects::JournalEntry).count() == 1);
                }
                pActions[eMenu::Action::NewTransaction]->setEnabled(!idx.data(eMyMoney::Model::AccountIsClosedRole).toBool());
            }
        }
        break;

    case View::Home:
    case View::Reports:
        pActions[eMenu::Action::Print]->setEnabled(true);
        pActions[eMenu::Action::PrintPreview]->setEnabled(true);
        break;

    default:
        break;
    }

    // the open ledger function only makes sense if we have an account selection
    pActions[eMenu::Action::OpenAccount]->setEnabled(!selections.isEmpty(SelectedObjects::Account));
}

void KMyMoneyView::slotSettingsChanged()
{
    updateViewType();

    Q_EMIT settingsChanged();
}

QHash<eMenu::Action, QAction *> KMyMoneyView::actionsToBeConnected()
{
    using namespace eMenu;
    // add fast switching of main views through Ctrl + NUM_X
    struct pageInfo {
        Action           action;
        View             view;
        QString          text;
        QKeySequence     shortcut = QKeySequence();
    };
    const QVector<pageInfo> pageInfos {
        {Action::ShowHomeView,            View::Home,               i18n("Show home page"),                   Qt::CTRL | Qt::Key_1},
        {Action::ShowInstitutionsView,    View::Institutions,       i18n("Show institutions page"),           Qt::CTRL | Qt::Key_2},
        {Action::ShowAccountsView,        View::Accounts,           i18n("Show accounts page"),               Qt::CTRL | Qt::Key_3},
        {Action::ShowSchedulesView,       View::Schedules,          i18n("Show scheduled transactions page"), Qt::CTRL | Qt::Key_4},
        {Action::ShowCategoriesView,      View::Categories,         i18n("Show categories page"),             Qt::CTRL | Qt::Key_5},
        {Action::ShowTagsView,            View::Tags,               i18n("Show tags page"),                   },
        {Action::ShowPayeesView,          View::Payees,             i18n("Show payees page"),                 Qt::CTRL | Qt::Key_6},
        {Action::ShowLedgersView,         View::NewLedgers,         i18n("Show ledgers page"),                Qt::CTRL | Qt::Key_7},
        {Action::ShowInvestmentsView,     View::Investments,        i18n("Show investments page"),            Qt::CTRL | Qt::Key_8},
        {Action::ShowReportsView,         View::Reports,            i18n("Show reports page"),                Qt::CTRL | Qt::Key_9},
        {Action::ShowBudgetView,          View::Budget,             i18n("Show budget page"),                 },
        {Action::ShowForecastView,        View::Forecast,           i18n("Show forecast page"),               },
        {Action::ShowOnlineJobOutboxView, View::OnlineJobOutbox,    i18n("Show outbox page")                  },
    };

    QHash<Action, QAction *> lutActions;
    auto pageCount = 0;
    for (const pageInfo& info : pageInfos) {
        auto a = new QAction(this);
        // KActionCollection::addAction by name sets object name anyways,
        // so, as better alternative, set it here right from the start
        a->setObjectName(QString::fromLatin1("ShowPage%1").arg(QString::number(pageCount++)));
        a->setText(info.text);
        a->setData(static_cast<int>(info.view));
        connect(a, &QAction::triggered, this, [this, a] {
            showPageAndFocus(static_cast<View>(a->data().toUInt()));
        });
        lutActions.insert(info.action, a);  // store QAction's pointer for later processing
        if (!info.shortcut.isEmpty())
            a->setShortcut(info.shortcut);
    }
    return lutActions;
}

bool KMyMoneyView::showPageHeader() const
{
    return false;
}

void KMyMoneyView::showPageAndFocus(View idView)
{
    Q_D(KMyMoneyView);
    if (d->viewFrames.contains(idView)) {
        showPage(idView);
        d->viewBases[idView]->setDefaultFocus();
    }
}

void KMyMoneyView::showPage(View idView, bool forceShow)
{
    Q_D(KMyMoneyView);
    if ((!d->viewFrames.contains(idView) || (currentPage() == d->viewFrames[idView])) && !forceShow)
        return;

    setCurrentPage(d->viewFrames[idView]);
}

void KMyMoneyView::enableViewsIfFileOpen(bool fileOpen)
{
    Q_D(KMyMoneyView);
    // call set enabled only if the state differs to avoid widgets 'bouncing on the screen' while doing this
    Q_ASSERT_X((int)(View::Home) == 0, "viewenums.h", "View::Home must be the first entry");
    Q_ASSERT_X(((int)(View::Home)+1) == (int)View::Institutions, "viewenums.h", "View::Institutions must be the second entry");

    // the home view is always enabled
    d->viewFrames[View::Home]->setEnabled(true);
    for (auto i = (int)View::Institutions; i < (int)View::None; ++i)
        if (d->viewFrames.contains(View(i)))
            if (d->viewFrames[View(i)]->isEnabled() != fileOpen)
                d->viewFrames[View(i)]->setEnabled(fileOpen);

    Q_EMIT viewStateChanged(fileOpen);
}

void KMyMoneyView::switchToDefaultView()
{
    const auto idView = KMyMoneySettings::startLastViewSelected() ?
                        static_cast<View>(KMyMoneySettings::lastViewSelected()) :
                        View::Home;
    showPage(idView, true);

    executeCustomAction(eView::Action::UnblockViewAfterFileOpen);
}

void KMyMoneyView::slotSwitchView(KPageWidgetItem* current, KPageWidgetItem* previous)
{
    Q_D(KMyMoneyView);
    if (previous != nullptr) {
        const auto view = qobject_cast<KMyMoneyViewBase*>(previous->widget());
        if (view) {
            view->aboutToHide();
        }
    }

    if (current != nullptr) {
        const auto view = qobject_cast<KMyMoneyViewBase*>(current->widget());
        if (view) {
            view->aboutToShow();
            // remember last selected view
            // omit the initial page selection
            if (previous != nullptr) {
                for (auto it = d->viewFrames.cbegin(); it != d->viewFrames.cend(); ++it) {
                    if (it.value() == current) {
                        Q_EMIT viewActivated(it.key());
                        break;
                    }
                }
            }
            d->selectSharedActions(d->currentViewId());
        }
    }
}

void KMyMoneyView::slotRememberLastView(View view)
{
    KMyMoneySettings::setLastViewSelected(static_cast<int>(view));
}

void KMyMoneyView::executeCustomAction(eView::Action action)
{
    Q_D(KMyMoneyView);
    // execute the action, at last on the current view
    const auto currentView = d->viewBases[d->currentViewId()];
    for (const auto& view : qAsConst(d->viewBases)) {
        if (view != currentView) {
            view->executeCustomAction(action);
        }
    }
    currentView->executeCustomAction(action);
}

void KMyMoneyView::executeAction(eMenu::Action action, const SelectedObjects& selections)
{
    Q_D(KMyMoneyView);

    // when closing, we don't remember the switch to the home view anymore
    switch (action) {
    case eMenu::Action::FileNew: // opened a file or database
        // make sure to catch view activations
        connect(this, &KMyMoneyView::viewActivated, this, &KMyMoneyView::slotRememberLastView);

        // delay the switchToDefaultView call until the event loop is running
        QMetaObject::invokeMethod(this, "switchToDefaultView", Qt::QueuedConnection);
        break;

    case eMenu::Action::FileClose:
        disconnect(this, &KMyMoneyView::viewActivated, this, &KMyMoneyView::slotRememberLastView);
        // block home view from unnecessary updates
        d->viewBases[View::Home]->executeAction(action, selections);
        break;

    default:
        break;
    }

    d->switchView(action);

    // execute the action, at last on the current view
    const auto currentView = d->viewBases[d->currentViewId()];
    for (const auto& view : qAsConst(d->viewBases)) {
        if (view != currentView) {
            view->executeAction(action, selections);
        }
    }
    currentView->executeAction(action, selections);
}

void KMyMoneyView::switchView(QWidget* viewWidget, const QString& accountId, const QString& journalEntryId)
{
    Q_D(KMyMoneyView);
    auto baseView = qobject_cast<KMyMoneyViewBase*>(viewWidget);
    const auto viewId = d->viewIdByWidget(baseView);
    showPage(viewId);

    SelectedObjects selections;
    selections.addSelection(SelectedObjects::Account, accountId);
    selections.addSelection(SelectedObjects::JournalEntry, journalEntryId);
    executeAction(eMenu::Action::ShowTransaction, selections);
}

bool KMyMoneyView::hasClosableView() const
{
    Q_D(const KMyMoneyView);
    const auto currentView = d->viewBases[d->currentViewId()];
    if (currentView) {
        return currentView->hasClosableView();
    }
    return false;
}

void KMyMoneyView::closeCurrentView()
{
    Q_D(const KMyMoneyView);
    const auto currentView = d->viewBases[d->currentViewId()];
    if (currentView) {
        currentView->closeCurrentView();
    }
}

#include "moc_kmymoneyview.cpp"
