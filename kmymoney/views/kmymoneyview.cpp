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
#include <QRegExp>
#include <QLayout>
#include <QList>
#include <QByteArray>
#include <QUrl>
#include <QIcon>
#include <QTemporaryFile>
#include <QUrlQuery>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KTitleWidget>
#include <KSharedConfig>
#include <KBackup>
#include <KActionCollection>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "simpleledgerview.h"
#include "kmymoneysettings.h"
#include "kmymoneytitlelabel.h"
#include "kcurrencyeditdlg.h"
#include "mymoneyexception.h"
#include "khomeview.h"
#include "kaccountsview.h"
#include "kcategoriesview.h"
#include "kinstitutionsview.h"
#include "kpayeesview.h"
#include "ktagsview.h"
#include "kscheduledview.h"
#include "kgloballedgerview.h"
#include "kinvestmentview.h"
#include "modelenums.h"
#include "accountsmodel.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"
#include "specialdatesmodel.h"
#include "schedulesjournalmodel.h"
#include "icons.h"
#include "onlinejobadministration.h"
#include "kmymoneyaccounttreeview.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneytag.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyreport.h"
#include "kmymoneyplugin.h"
#include "mymoneyenums.h"
#include "menuenums.h"
#include "selectedobjects.h"

using namespace Icons;
using namespace eMyMoney;

class KMyMoneyViewPrivate
{
public:
    KMyMoneyViewPrivate(KMyMoneyView* qq)
        : q_ptr(qq)
        , m_model(nullptr)
        , m_header(nullptr)
    {
    }

    KMyMoneyView* q_ptr;
    KPageWidgetModel* m_model;
    KMyMoneyTitleLabel* m_header;

    QHash<View, KPageWidgetItem*> viewFrames;
    QHash<View, KMyMoneyViewBase*> viewBases;


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
        if (headerItem && qobject_cast<KTitleWidget*>(headerItem->widget()) != NULL) {
            gridLayout->removeItem(headerItem);
            // after we remove the KPageWidget standard header replace it with our own title label
            d->m_header = new KMyMoneyTitleLabel(this);
            d->m_header->setObjectName("titleLabel");
            d->m_header->setMinimumSize(QSize(100, 30));
            d->m_header->setRightImageFile("pics/titlelabel_background.png");
            d->m_header->setVisible(KMyMoneySettings::showTitleBar());
            gridLayout->addWidget(d->m_header, 1, 1);
        }
    }

    d->m_model = new KPageWidgetModel(this); // cannot be parentless, otherwise segfaults at exit

    d->viewBases[View::Home] = new KHomeView;
    d->viewBases[View::Institutions] = new KInstitutionsView;
    d->viewBases[View::Accounts] = new KAccountsView;
    d->viewBases[View::Schedules] = new KScheduledView;
    d->viewBases[View::Categories] = new KCategoriesView;
    d->viewBases[View::Tags] = new KTagsView;
    d->viewBases[View::Payees] = new KPayeesView;
    d->viewBases[View::NewLedgers] = new SimpleLedgerView;
    d->viewBases[View::Investments] = new KInvestmentView;
    d->viewBases[View::OldLedgers] = new KGlobalLedgerView;

    struct viewInfo
    {
        View id;
        QString name;
        Icon icon;
    };

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
        /// @todo remove when new ledger is fully functional
        {View::OldLedgers,      i18n("Old ledgers"),                  Icon::DocumentProperties},
    };

    for (const viewInfo& view : viewsInfo) {
        addView(d->viewBases[view.id], view.name, view.id, view.icon);
    }

    // set the model
    setModel(d->m_model);
    setCurrentPage(d->viewFrames[View::Home]);
    connect(this, &KMyMoneyView::currentPageChanged, this, &KMyMoneyView::slotSwitchView);

    updateViewType();
}

KMyMoneyView::~KMyMoneyView()
{
}

void KMyMoneyView::slotFileOpened()
{
    Q_D(KMyMoneyView);
    for( const auto& view : qAsConst(d->viewBases)) {
        view->executeCustomAction(eView::Action::InitializeAfterFileOpen);
    }

    // delay the switchToDefaultView call until the event loop is running
    QMetaObject::invokeMethod(this, "switchToDefaultView", Qt::QueuedConnection);
    slotObjectSelected(MyMoneyAccount()); // in order to enable update all accounts on file reload

    // make sure to catch view activations
    connect(this, &KMyMoneyView::viewActivated, this, &KMyMoneyView::slotRememberLastView);
}

void KMyMoneyView::slotFileClosed()
{
    Q_D(KMyMoneyView);
    disconnect(this, &KMyMoneyView::viewActivated, this, &KMyMoneyView::slotRememberLastView);

    showPageAndFocus(View::Home);

    for( const auto& view : qAsConst(d->viewBases)) {
        view->executeCustomAction(eView::Action::CleanupBeforeFileClose);
    }

    pActions[eMenu::Action::Print]->setEnabled(false);
    pActions[eMenu::Action::UpdateAllAccounts]->setEnabled(false);
}

void KMyMoneyView::showTitleBar(bool show)
{
    Q_D(KMyMoneyView);
    if (d->m_header)
        d->m_header->setVisible(show);
}

void KMyMoneyView::updateViewType()
{
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
        setFaceType(faceType);
        if (faceType == KPageView::Tree) {
            QList<QTreeView *> views = findChildren<QTreeView*>();
            foreach (QTreeView * view, views) {
                if (view && (view->parent() == this)) {
                    view->setRootIsDecorated(false);
                    break;
                }
            }
        }
    }
}

void KMyMoneyView::slotAccountTreeViewChanged(const eAccountsModel::Column column, const bool show)
{
/// @todo "Reimplement global acccount column visibility"
    /// @todo port to new model code
#if 0
    QVector<AccountsViewProxyModel *> proxyModels
    {
        static_cast<KMyMoneyAccountsViewBase*>(viewBases[View::Institutions])->getProxyModel(),
        static_cast<KMyMoneyAccountsViewBase*>(viewBases[View::Accounts])->getProxyModel(),
        static_cast<KMyMoneyAccountsViewBase*>(viewBases[View::Categories])->getProxyModel()
    };

    if (viewBases.contains(View::Budget))
        proxyModels.append(static_cast<KMyMoneyAccountsViewBase*>(viewBases[View::Budget])->getProxyModel());

    for (auto i = proxyModels.count() - 1; i >= 0; --i) { // weed out unloaded views
        if (!proxyModels.at(i))
            proxyModels.removeAt(i);
    }

    QString question;

    if (show)
        question = i18n("Do you want to show <b>%1</b> column on every loaded view?", AccountsModel::getHeaderName(column));
    else
        question = i18n("Do you want to hide <b>%1</b> column on every loaded view?", AccountsModel::getHeaderName(column));


    if (proxyModels.count() == 1 || // no need to ask what to do with other views because they aren't loaded
            KMessageBox::questionYesNo(this,
                                       question,
                                       QString(),
                                       KStandardGuiItem::yes(), KStandardGuiItem::no(),
                                       QStringLiteral("ShowColumnOnEveryView")) == KMessageBox::Yes) {
        MyMoneyFile::instance()->accountsModel()->setColumnVisibility(column, show);
        MyMoneyFile::instance()->institutionsModel()->setColumnVisibility(column, show);
        foreach(AccountsViewProxyModel *proxyModel, proxyModels) {
            if (!proxyModel)
                continue;
            proxyModel->setColumnVisibility(column, show);
            proxyModel->invalidate();
        }
    } else if(show) {
        // in case we need to show it, we have to make sure to set the visibility
        // in the base model as well. Otherwise, we don't see the column through the proxy model
        MyMoneyFile::instance()->accountsModel()->setColumnVisibility(column, show);
        MyMoneyFile::instance()->institutionsModel()->setColumnVisibility(column, show);
    }
#endif
}

void KMyMoneyView::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins)
{
    Q_D(KMyMoneyView);
    if (d->viewBases.contains(View::Accounts))
        d->viewBases[View::Accounts]->slotSelectByVariant(QVariantList {QVariant::fromValue(static_cast<void*>(&plugins))}, eView::Intent::SetOnlinePlugins);

    if (d->viewBases.contains(View::OnlineJobOutbox))
        d->viewBases[View::OnlineJobOutbox]->slotSelectByVariant(QVariantList {QVariant::fromValue(static_cast<void*>(&plugins))}, eView::Intent::SetOnlinePlugins);
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
    connect(d->viewBases[idView], &KMyMoneyViewBase::requestCustomContextMenu, this, &KMyMoneyView::requestCustomContextMenu);
    connect(d->viewBases[idView], &KMyMoneyViewBase::requestActionTrigger, this, &KMyMoneyView::requestActionTrigger);
    connect(d->viewBases[idView], &KMyMoneyViewBase::customActionRequested, this, &KMyMoneyView::slotCustomActionRequested);
    connect(this, &KMyMoneyView::settingsChanged, d->viewBases[idView], &KMyMoneyViewBase::slotSettingsChanged);

    connect(d->viewBases[idView], &KMyMoneyViewBase::viewStateChanged, d->viewFrames[idView], &KPageWidgetItem::setEnabled);
    connect(d->viewBases[idView], &KMyMoneyViewBase::requestSelectionChange, this, &KMyMoneyView::requestSelectionChange);
}

void KMyMoneyView::removeView(View idView)
{
    Q_D(KMyMoneyView);
    if (!d->viewBases.contains(idView))
        return;

    disconnect(d->viewBases[idView], &KMyMoneyViewBase::viewStateChanged, d->viewFrames[idView], &KPageWidgetItem::setEnabled);
    disconnect(d->viewBases[idView], &KMyMoneyViewBase::requestSelectionChange, this, &KMyMoneyView::requestSelectionChange);

    disconnect(d->viewBases[idView], &KMyMoneyViewBase::requestCustomContextMenu, this, &KMyMoneyView::requestCustomContextMenu);
    disconnect(d->viewBases[idView], &KMyMoneyViewBase::requestActionTrigger, this, &KMyMoneyView::requestActionTrigger);
    disconnect(d->viewBases[idView], &KMyMoneyViewBase::customActionRequested, this, &KMyMoneyView::slotCustomActionRequested);
    disconnect(this, &KMyMoneyView::settingsChanged, d->viewBases[idView], &KMyMoneyViewBase::slotSettingsChanged);

    d->m_model->removePage(d->viewFrames[idView]);
    d->viewFrames.remove(idView);
    d->viewBases.remove(idView);
}

void KMyMoneyView::updateActions(const SelectedObjects& selections)
{
    Q_D(KMyMoneyView);
    for (const auto& view : d->viewBases) {
        view->updateActions(selections);
    }

    // global actions
    pActions[eMenu::Action::OpenAccount]->setEnabled(!selections.isEmpty(SelectedObjects::Account));
}

void KMyMoneyView::slotSettingsChanged()
{
    const auto showHeaders = KMyMoneySettings::showFancyMarker();
    QDate firstFiscalDate;
    if (KMyMoneySettings::showFiscalMarker())
        firstFiscalDate = KMyMoneySettings::firstFiscalDate();

    MyMoneyFile::instance()->specialDatesModel()->setOptions(showHeaders, firstFiscalDate);
    MyMoneyFile::instance()->schedulesJournalModel()->setPreviewPeriod(KMyMoneySettings::schedulePreview());
    MyMoneyFile::instance()->schedulesJournalModel()->setShowPlannedDate(KMyMoneySettings::showPlannedScheduleDates());

    updateViewType();

    emit settingsChanged();
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
        {Action::ShowHomeView,            View::Home,               i18n("Show home page"),                   Qt::CTRL + Qt::Key_1},
        {Action::ShowInstitutionsView,    View::Institutions,       i18n("Show institutions page"),           Qt::CTRL + Qt::Key_2},
        {Action::ShowAccountsView,        View::Accounts,           i18n("Show accounts page"),               Qt::CTRL + Qt::Key_3},
        {Action::ShowSchedulesView,       View::Schedules,          i18n("Show scheduled transactions page"), Qt::CTRL + Qt::Key_4},
        {Action::ShowCategoriesView,      View::Categories,         i18n("Show categories page"),             Qt::CTRL + Qt::Key_5},
        {Action::ShowTagsView,            View::Tags,               i18n("Show tags page"),                   },
        {Action::ShowPayeesView,          View::Payees,             i18n("Show payees page"),                 Qt::CTRL + Qt::Key_6},
        {Action::ShowLedgersView,         View::NewLedgers,         i18n("Show ledgers page"),                Qt::CTRL + Qt::Key_7},
        {Action::ShowInvestmentsView,     View::Investments,        i18n("Show investments page"),            Qt::CTRL + Qt::Key_8},
        {Action::ShowReportsView,         View::Reports,            i18n("Show reports page"),                Qt::CTRL + Qt::Key_9},
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
        connect(a, &QAction::triggered, [this, a] { showPageAndFocus(static_cast<View>(a->data().toUInt())); } );
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
        d->viewBases[idView]->executeCustomAction(eView::Action::SetDefaultFocus);
    }
}

void KMyMoneyView::showPage(View idView)
{
    Q_D(KMyMoneyView);
    if (!d->viewFrames.contains(idView) || (currentPage() == d->viewFrames[idView]))
        return;

    resetViewSelection();
    setCurrentPage(d->viewFrames[idView]);
}

bool KMyMoneyView::canPrint()
{
    Q_D(KMyMoneyView);
    return ((d->viewFrames.contains(View::Reports) && d->viewFrames[View::Reports] == currentPage()) ||
    (d->viewFrames.contains(View::Home) && d->viewFrames[View::Home] == currentPage()));
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

    emit viewStateChanged(fileOpen);
}

void KMyMoneyView::switchToDefaultView()
{
    Q_D(KMyMoneyView);
    const auto idView = KMyMoneySettings::startLastViewSelected() ?
                        static_cast<View>(KMyMoneySettings::lastViewSelected()) :
                        View::Home;
    // if we currently see a different page, then select the right one
    if (d->viewFrames.contains(idView) && d->viewFrames[idView] != currentPage())
        showPage(idView);
}

void KMyMoneyView::slotTagSelected(const QString& tag, const QString& account, const QString& transaction)
{
    Q_D(KMyMoneyView);
    showPage(View::Tags);
    static_cast<KTagsView*>(d->viewBases[View::Tags])->slotSelectTagAndTransaction(tag, account, transaction);
}

void KMyMoneyView::finishReconciliation(const MyMoneyAccount& /* account */)
{
    Q_D(KMyMoneyView);
/// @todo port to new model code
#if 0
    MyMoneyFile::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
#endif
    static_cast<KGlobalLedgerView*>(d->viewBases[View::OldLedgers])->slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
}

void KMyMoneyView::viewAccountList(const QString& /*selectAccount*/)
{
    Q_D(KMyMoneyView);
    if (d->viewFrames[View::Accounts] != currentPage())
        showPage(View::Accounts);
    d->viewBases[View::Accounts]->show();
}

void KMyMoneyView::slotRefreshViews()
{
    Q_D(KMyMoneyView);
    showTitleBar(KMyMoneySettings::showTitleBar());

    for (auto i = (int)View::Home; i < (int)View::None; ++i) {
        if (d->viewBases.contains(View(i)))
            d->viewBases[View(i)]->executeCustomAction(eView::Action::Refresh);
    }

    d->viewBases[View::Payees]->executeCustomAction(eView::Action::ClosePayeeIdentifierSource);
}

void KMyMoneyView::slotShowTransactionDetail(bool detailed)
{
    KMyMoneySettings::setShowRegisterDetailed(detailed);
    slotRefreshViews();
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

    // set the current page's title in the header
    if (d->m_header)
        d->m_header->setText(current->header());

    if (current != nullptr) {
        const auto view = qobject_cast<KMyMoneyViewBase*>(current->widget());
        if (view) {
            view->aboutToShow();
            // remember last selected view
            // omit the initial page selection
            if (previous != nullptr) {
                for (auto it = d->viewFrames.constBegin(); it != d->viewFrames.constEnd(); ++it) {
                    if (it.value() == current) {
                        emit viewActivated(it.key());
                        break;
                    }
                }
            }
        }
    }

    pActions[eMenu::Action::Print]->setEnabled(canPrint());
}

void KMyMoneyView::slotRememberLastView(View view)
{
    KMyMoneySettings::setLastViewSelected(static_cast<int>(view));
}

void KMyMoneyView::createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount)
{
    // Add the schedule only if one exists
    //
    // Remember to modify the first split to reference the newly created account
    if (!newSchedule.name().isEmpty()) {
        MyMoneyFileTransaction ft;
        try {
            // We assume at least 2 splits in the transaction
            MyMoneyTransaction t = newSchedule.transaction();
            if (t.splitCount() < 2) {
                throw MYMONEYEXCEPTION_CSTRING("Transaction for schedule has less than 2 splits!");
            }
            // now search the split that does not have an account reference
            // and set it up to be the one of the account we just added
            // to the account pool. Note: the schedule code used to leave
            // this always the first split, but the loan code leaves it as
            // the second one. So I thought, searching is a good alternative ....
            foreach (const auto split, t.splits()) {
                if (split.accountId().isEmpty()) {
                    MyMoneySplit s = split;
                    s.setAccountId(newAccount.id());
                    t.modifySplit(s);
                    break;
                }
            }
            newSchedule.setTransaction(t);

            MyMoneyFile::instance()->addSchedule(newSchedule);

            // in case of a loan account, we keep a reference to this
            // schedule in the account
            if (newAccount.isLoan()) {
                newAccount.setValue("schedule", newSchedule.id());
                MyMoneyFile::instance()->modifyAccount(newAccount);
            }
            ft.commit();
        } catch (const MyMoneyException &e) {
            KMessageBox::information(this, i18n("Unable to add schedule: %1", QString::fromLatin1(e.what())));
        }
    }
}

void KMyMoneyView::slotPrintView()
{
    Q_D(KMyMoneyView);
    if (d->viewFrames.contains(View::Reports) && d->viewFrames[View::Reports] == currentPage())
        d->viewBases[View::Reports]->executeCustomAction(eView::Action::Print);
    else if (d->viewFrames.contains(View::Home) && d->viewFrames[View::Home] == currentPage())
        d->viewBases[View::Home]->executeCustomAction(eView::Action::Print);
}

void KMyMoneyView::resetViewSelection()
{
    slotObjectSelected(MyMoneyAccount());
    slotObjectSelected(MyMoneyInstitution());
    slotObjectSelected(MyMoneySchedule());
    slotObjectSelected(MyMoneyTag());
    slotSelectByVariant(QVariantList {QVariant::fromValue(KMyMoneyRegister::SelectedTransactions())}, eView::Intent::SelectRegisterTransactions);
}

void KMyMoneyView::slotOpenObjectRequested(const MyMoneyObject& obj)
{
    Q_D(KMyMoneyView);
    if (typeid(obj) == typeid(MyMoneyAccount)) {
        const auto& acc = static_cast<const MyMoneyAccount&>(obj);
        // check if we can open this account
        // currently it make's sense for asset and liability accounts
        if (!MyMoneyFile::instance()->isStandardAccount(acc.id()))
            if (d->viewBases.contains(View::OldLedgers))
                d->viewBases[View::OldLedgers]->slotSelectByVariant(QVariantList {QVariant(acc.id()), QVariant(QString()) }, eView::Intent::ShowTransactionInLedger );

    } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
//    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);
        if (d->viewBases.contains(View::Institutions))
            d->viewBases[View::Institutions]->executeCustomAction(eView::Action::EditInstitution);
    } else if (typeid(obj) == typeid(MyMoneySchedule)) {
        if (d->viewBases.contains(View::Schedules))
            d->viewBases[View::Schedules]->executeCustomAction(eView::Action::EditSchedule);
    } else if (typeid(obj) == typeid(MyMoneyReport)) {
//    const auto& rep = static_cast<const MyMoneyReport&>(obj);
        showPage(View::Reports);
        if (d->viewBases.contains(View::Reports))
            d->viewBases[View::Reports]->slotSelectByObject(obj, eView::Intent::OpenObject);
    }
}

void KMyMoneyView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
    Q_D(KMyMoneyView);
    switch (intent) {
    case eView::Intent::None:
        slotObjectSelected(obj);
        break;

    case eView::Intent::SynchronizeAccountInInvestmentView:
        if (d->viewBases.contains(View::Investments))
            d->viewBases[View::Investments]->slotSelectByObject(obj, intent);
        break;

    case eView::Intent::SynchronizeAccountInLedgersView:
        if (d->viewBases.contains(View::OldLedgers))
            d->viewBases[View::OldLedgers]->slotSelectByObject(obj, intent);
        break;

    case eView::Intent::OpenObject:
        slotOpenObjectRequested(obj);
        break;

    case eView::Intent::OpenContextMenu:
        slotContextMenuRequested(obj);
        break;

    case eView::Intent::StartEnteringOverdueScheduledTransactions:
        if (d->viewBases.contains(View::Schedules))
            d->viewBases[View::Schedules]->slotSelectByObject(obj, intent);
        break;

    case eView::Intent::FinishEnteringOverdueScheduledTransactions:
        if (d->viewBases.contains(View::OldLedgers)) {
            showPage(View::OldLedgers);
            d->viewBases[View::OldLedgers]->slotSelectByObject(obj, intent);
        }
        break;

    default:
        break;
    }
}

void KMyMoneyView::selectView(View idView, const QVariantList& args)
{
    Q_D(KMyMoneyView);
    if (d->viewBases.contains(idView)) {
        showPage(idView);
        d->viewBases[idView]->slotSelectByVariant(args, eView::Intent::None);
    }
}

void KMyMoneyView::executeAction(eMenu::Action action, const QVariantList& args)
{
    Q_D(KMyMoneyView);

    const static QHash<eMenu::Action, View> actionRoutes = {
        {eMenu::Action::GoToPayee, View::Payees},
        {eMenu::Action::GoToAccount, View::NewLedgers},
    };

    const auto viewId = actionRoutes.value(action, View::None);

    if (viewId != View::None) {
        showPage(viewId);
        d->viewBases[viewId]->executeAction(action, args);
    } else {
        // maybe, we need to pass it by all views
        // in case it is unknown to the routing table
    }
}

void KMyMoneyView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
    Q_D(KMyMoneyView);
    switch(intent) {
    case eView::Intent::ReportProgress:
        if (variant.count() == 2)
            emit statusProgress(variant.at(0).toInt(), variant.at(1).toInt());
        break;

    case eView::Intent::ReportProgressMessage:
        if (variant.count() == 1)
            emit statusMsg(variant.first().toString());
        break;

        /// @todo cleanup
#if 0
    case eView::Intent::UpdateNetWorth:
        if (viewBases.contains(View::Accounts))
            viewBases[View::Accounts]->slotSelectByVariant(variant, intent);

        if (viewBases.contains(View::Institutions))
            viewBases[View::Institutions]->slotSelectByVariant(variant, intent);
        break;
#endif

    case eView::Intent::UpdateProfit:
        if (d->viewBases.contains(View::Categories))
            d->viewBases[View::Categories]->slotSelectByVariant(variant, intent);
        break;

    case eView::Intent::ShowTransactionInLedger:
        if (d->viewBases.contains(View::NewLedgers)) {
            showPage(View::NewLedgers);
            d->viewBases[View::NewLedgers]->slotSelectByVariant(variant, intent);
        }
        break;

    case eView::Intent::ToggleColumn:
        if (variant.count() == 2) {
            /// @todo port to new model code
#if 0
            slotAccountTreeViewChanged(variant.at(0).value<eAccountsModel::Column>(), variant.at(1).value<bool>());
#endif
        }
        break;

    case eView::Intent::ShowPayee:
        if (d->viewBases.contains(View::Payees)) {
            showPage(View::Payees);
            d->viewBases[View::Payees]->slotSelectByVariant(variant, intent);
        }
        break;

    case eView::Intent::SelectRegisterTransactions:
        if (variant.count() == 1) {
            emit transactionsSelected(variant.at(0).value<KMyMoneyRegister::SelectedTransactions>()); // for plugins
            if (d->viewBases.contains(View::OldLedgers))
                d->viewBases[View::OldLedgers]->slotSelectByVariant(variant, intent);
        }
        break;

    case eView::Intent::AccountReconciled:
        if (variant.count() == 5)
            emit accountReconciled(variant.at(0).value<MyMoneyAccount>(),
                                   variant.at(1).value<QDate>(),
                                   variant.at(2).value<MyMoneyMoney>(),
                                   variant.at(3).value<MyMoneyMoney>(),
                                   variant.at(4).value<QList<QPair<MyMoneyTransaction, MyMoneySplit>>>()); // for plugins
        break;

    default:
        break;
    }
}

void KMyMoneyView::slotCustomActionRequested(View view, eView::Action action)
{
    Q_D(KMyMoneyView);
    switch (action) {
    case eView::Action::AboutToShow:
        resetViewSelection();
        break;
    case eView::Action::SwitchView:
        showPage(view);
        break;
    case eView::Action::ShowBalanceChart:
        if (d->viewBases.contains(View::Reports))
            d->viewBases[View::Reports]->executeCustomAction(action);
        break;
    default:
        break;
    }
}

void KMyMoneyView::slotObjectSelected(const MyMoneyObject& obj)
{
    Q_D(KMyMoneyView);
    // carrying some slots over to views isn't easy for all slots...
    // ...so calls to kmymoney still must be here
    if (typeid(obj) == typeid(MyMoneyAccount)) {
        QVector<View> views {View::Investments, View::Categories, View::Accounts,
                             View::OldLedgers, View::Reports, View::OnlineJobOutbox};
        for (const auto view : views)
            if (d->viewBases.contains(view))
                d->viewBases[view]->slotSelectByObject(obj, eView::Intent::UpdateActions);

        // for plugin only
        const auto& acc = static_cast<const MyMoneyAccount&>(obj);
        if (!acc.isIncomeExpense() &&
                !MyMoneyFile::instance()->isStandardAccount(acc.id()))
            emit accountSelected(acc);
    } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
        d->viewBases[View::Institutions]->slotSelectByObject(obj, eView::Intent::UpdateActions);
    } else if (typeid(obj) == typeid(MyMoneySchedule)) {
        d->viewBases[View::Schedules]->slotSelectByObject(obj, eView::Intent::UpdateActions);
    }
}

void KMyMoneyView::slotContextMenuRequested(const MyMoneyObject& obj)
{
    Q_D(KMyMoneyView);
    if (typeid(obj) == typeid(MyMoneyAccount)) {
        const auto& acc = static_cast<const MyMoneyAccount&>(obj);
        if (acc.isInvest())
            d->viewBases[View::Investments]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
        else if (acc.isIncomeExpense())
            d->viewBases[View::Categories]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
        else
            d->viewBases[View::Accounts]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);

    } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
        d->viewBases[View::Institutions]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
    } else if (typeid(obj) == typeid(MyMoneySchedule)) {
        d->viewBases[View::Schedules]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
    }
}
