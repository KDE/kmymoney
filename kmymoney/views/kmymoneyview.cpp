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

#ifdef ENABLE_UNFINISHEDFEATURES
#include "simpleledgerview.h"
#endif

#include "kmymoneysettings.h"
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
#include "models.h"
#include "accountsmodel.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"
#include "icons.h"
#include "onlinejobadministration.h"
#include "kmymoneyaccounttreeview.h"
#include "accountsviewproxymodel.h"
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

using namespace Icons;
using namespace eMyMoney;

typedef void(KMyMoneyView::*KMyMoneyViewFunc)();

KMyMoneyView::KMyMoneyView()
    : KPageWidget(nullptr)
{
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
        }
    }

//  newStorage();
    m_model = new KPageWidgetModel(this); // cannot be parentless, otherwise segfaults at exit

    viewBases[View::Home] = new KHomeView;
    viewBases[View::Institutions] = new KInstitutionsView;
    viewBases[View::Accounts] = new KAccountsView;
    viewBases[View::Schedules] = new KScheduledView;
    viewBases[View::Categories] = new KCategoriesView;
    viewBases[View::Tags] = new KTagsView;
    viewBases[View::Payees] = new KPayeesView;
    viewBases[View::Ledgers] = new KGlobalLedgerView;
    viewBases[View::Investments] = new KInvestmentView;
#ifdef ENABLE_UNFINISHEDFEATURES
    viewBases[View::NewLedgers] = new SimpleLedgerView;
#endif

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
        {View::Ledgers,         i18n("Ledgers"),                      Icon::Ledger},
        {View::Investments,     i18n("Investments"),                  Icon::Investments},
#ifdef ENABLE_UNFINISHEDFEATURES
        {View::NewLedgers,      i18n("New ledger"),                   Icon::DocumentProperties},
#endif
    };

    for (const viewInfo& view : viewsInfo) {
        addView(viewBases[view.id], view.name, view.id, view.icon);
    }

    connect(Models::instance()->accountsModel(), &AccountsModel::netWorthChanged, this, &KMyMoneyView::slotSelectByVariant);
    connect(Models::instance()->accountsModel(), &AccountsModel::profitChanged, this, &KMyMoneyView::slotSelectByVariant);
    connect(Models::instance()->institutionsModel(), &AccountsModel::netWorthChanged, this, &KMyMoneyView::slotSelectByVariant);
    connect(Models::instance()->institutionsModel(), &AccountsModel::profitChanged, this, &KMyMoneyView::slotSelectByVariant);

    // set the model
    setModel(m_model);
    setCurrentPage(viewFrames[View::Home]);
    connect(this, SIGNAL(currentPageChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentPageChanged(QModelIndex,QModelIndex)));

    updateViewType();
}

KMyMoneyView::~KMyMoneyView()
{
}

void KMyMoneyView::slotFileOpened()
{
    if (viewBases.contains(View::OnlineJobOutbox))
        viewBases[View::OnlineJobOutbox]->executeCustomAction(eView::Action::InitializeAfterFileOpen);

    if (viewBases.contains(View::Ledgers))
        viewBases[View::Ledgers]->executeCustomAction(eView::Action::InitializeAfterFileOpen);

#ifdef ENABLE_UNFINISHEDFEATURES
    static_cast<SimpleLedgerView*>(viewBases[View::NewLedgers])->openFavoriteLedgers();
#endif
    // delay the switchToDefaultView call until the event loop is running
    QMetaObject::invokeMethod(this, "switchToDefaultView", Qt::QueuedConnection);
    slotObjectSelected(MyMoneyAccount()); // in order to enable update all accounts on file reload
}

void KMyMoneyView::slotFileClosed()
{
    slotShowHomePage();
    if (viewBases.contains(View::Home))
        viewBases[View::Home]->executeCustomAction(eView::Action::CleanupBeforeFileClose);

    if (viewBases.contains(View::Reports))
        viewBases[View::Reports]->executeCustomAction(eView::Action::CleanupBeforeFileClose);

    if (viewBases.contains(View::OnlineJobOutbox))
        viewBases[View::OnlineJobOutbox]->executeCustomAction(eView::Action::CleanupBeforeFileClose);

    if (viewBases.contains(View::Ledgers))
        viewBases[View::Ledgers]->executeCustomAction(eView::Action::CleanupBeforeFileClose);

#ifdef ENABLE_UNFINISHEDFEATURES
    static_cast<SimpleLedgerView*>(viewBases[View::NewLedgers])->closeLedgers();
#endif

    pActions[eMenu::Action::Print]->setEnabled(false);
    pActions[eMenu::Action::AccountCreditTransfer]->setEnabled(false);
    pActions[eMenu::Action::UpdateAllAccounts]->setEnabled(false);
}

void KMyMoneyView::slotShowHomePage()
{
    showPageAndFocus(View::Home);
}

void KMyMoneyView::slotShowInstitutionsPage()
{
    showPageAndFocus(View::Institutions);
}

void KMyMoneyView::slotShowAccountsPage()
{
    showPageAndFocus(View::Accounts);
}

void KMyMoneyView::slotShowSchedulesPage()
{
    showPageAndFocus(View::Schedules);
}

void KMyMoneyView::slotShowCategoriesPage()
{
    showPageAndFocus(View::Categories);
}

void KMyMoneyView::slotShowTagsPage()
{
    showPageAndFocus(View::Tags);
}

void KMyMoneyView::slotShowPayeesPage()
{
    showPageAndFocus(View::Payees);
}

void KMyMoneyView::slotShowLedgersPage()
{
    showPageAndFocus(View::Ledgers);
}

void KMyMoneyView::slotShowInvestmentsPage()
{
    showPageAndFocus(View::Investments);
}

void KMyMoneyView::slotShowReportsPage()
{
    showPageAndFocus(View::Reports);
}

void KMyMoneyView::slotShowBudgetPage()
{
    showPageAndFocus(View::Budget);
}

void KMyMoneyView::slotShowForecastPage()
{
    showPageAndFocus(View::Forecast);
}

void KMyMoneyView::slotShowOutboxPage()
{
    showPageAndFocus(View::OnlineJobOutbox);
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
        Models::instance()->accountsModel()->setColumnVisibility(column, show);
        Models::instance()->institutionsModel()->setColumnVisibility(column, show);
        foreach(AccountsViewProxyModel *proxyModel, proxyModels) {
            if (!proxyModel)
                continue;
            proxyModel->setColumnVisibility(column, show);
            proxyModel->invalidate();
        }
    } else if(show) {
        // in case we need to show it, we have to make sure to set the visibility
        // in the base model as well. Otherwise, we don't see the column through the proxy model
        Models::instance()->accountsModel()->setColumnVisibility(column, show);
        Models::instance()->institutionsModel()->setColumnVisibility(column, show);
    }
}

void KMyMoneyView::setOnlinePlugins(QMap<QString, KMyMoneyPlugin::OnlinePlugin*>& plugins)
{
    if (viewBases.contains(View::Accounts))
        viewBases[View::Accounts]->slotSelectByVariant(QVariantList {QVariant::fromValue(static_cast<void*>(&plugins))}, eView::Intent::SetOnlinePlugins);

    if (viewBases.contains(View::OnlineJobOutbox))
        viewBases[View::OnlineJobOutbox]->slotSelectByVariant(QVariantList {QVariant::fromValue(static_cast<void*>(&plugins))}, eView::Intent::SetOnlinePlugins);
}

eDialogs::ScheduleResultCode KMyMoneyView::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{
    return static_cast<KScheduledView*>(viewBases[View::Schedules])->enterSchedule(schedule, autoEnter, extendedKeys);
}

void KMyMoneyView::addView(KMyMoneyViewBase* view, const QString& name, View idView, Icons::Icon icon)
{
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
        if (viewFrames.contains((View)i)) {
            viewFrames[idView] = m_model->insertPage(viewFrames[(View)i],view, adjustedName);
            isViewInserted = true;
            break;
        }
    }

    if (!isViewInserted)
        viewFrames[idView] = m_model->addPage(view, adjustedName);

    viewFrames[idView]->setIcon(Icons::get(icon));
    viewBases[idView] = view;
    connect(viewBases[idView], &KMyMoneyViewBase::selectByObject, this, &KMyMoneyView::slotSelectByObject);
    connect(viewBases[idView], &KMyMoneyViewBase::selectByVariant, this, &KMyMoneyView::slotSelectByVariant);
    connect(viewBases[idView], &KMyMoneyViewBase::customActionRequested, this, &KMyMoneyView::slotCustomActionRequested);
}

void KMyMoneyView::removeView(View idView)
{
    if (!viewBases.contains(idView))
        return;

    disconnect(viewBases[idView], &KMyMoneyViewBase::selectByObject, this, &KMyMoneyView::slotSelectByObject);
    disconnect(viewBases[idView], &KMyMoneyViewBase::selectByVariant, this, &KMyMoneyView::slotSelectByVariant);
    disconnect(viewBases[idView], &KMyMoneyViewBase::customActionRequested, this, &KMyMoneyView::slotCustomActionRequested);

    m_model->removePage(viewFrames[idView]);
    viewFrames.remove(idView);
    viewBases.remove(idView);
}

QHash<eMenu::Action, QAction *> KMyMoneyView::actionsToBeConnected()
{
    using namespace eMenu;
    // add fast switching of main views through Ctrl + NUM_X
    struct pageInfo {
        Action           view;
        KMyMoneyViewFunc callback;
        QString          text;
        QKeySequence     shortcut = QKeySequence();
    };

    const QVector<pageInfo> pageInfos {
        {Action::ShowHomeView,            &KMyMoneyView::slotShowHomePage,          i18n("Show home page"),                   Qt::CTRL + Qt::Key_1},
        {Action::ShowInstitutionsView,    &KMyMoneyView::slotShowInstitutionsPage,  i18n("Show institutions page"),           Qt::CTRL + Qt::Key_2},
        {Action::ShowAccountsView,        &KMyMoneyView::slotShowAccountsPage,      i18n("Show accounts page"),               Qt::CTRL + Qt::Key_3},
        {Action::ShowSchedulesView,       &KMyMoneyView::slotShowSchedulesPage,     i18n("Show scheduled transactions page"), Qt::CTRL + Qt::Key_4},
        {Action::ShowCategoriesView,      &KMyMoneyView::slotShowCategoriesPage,    i18n("Show categories page"),             Qt::CTRL + Qt::Key_5},
        {Action::ShowTagsView,            &KMyMoneyView::slotShowTagsPage,          i18n("Show tags page"),                   },
        {Action::ShowPayeesView,          &KMyMoneyView::slotShowPayeesPage,        i18n("Show payees page"),                 Qt::CTRL + Qt::Key_6},
        {Action::ShowLedgersView,         &KMyMoneyView::slotShowLedgersPage,       i18n("Show ledgers page"),                Qt::CTRL + Qt::Key_7},
        {Action::ShowInvestmentsView,     &KMyMoneyView::slotShowInvestmentsPage,   i18n("Show investments page"),            Qt::CTRL + Qt::Key_8},
        {Action::ShowReportsView,         &KMyMoneyView::slotShowReportsPage,       i18n("Show reports page"),                Qt::CTRL + Qt::Key_9},
        {Action::ShowBudgetView,          &KMyMoneyView::slotShowBudgetPage,        i18n("Show budget page"),                },
        {Action::ShowForecastView,        &KMyMoneyView::slotShowForecastPage,      i18n("Show forecast page"),              },
        {Action::ShowOnlineJobOutboxView, &KMyMoneyView::slotShowOutboxPage,        i18n("Show outbox page")                 },
    };

    QHash<Action, QAction *> lutActions;
    auto pageCount = 0;
    for (const pageInfo& info : pageInfos) {
        auto a = new QAction(this);
        // KActionCollection::addAction by name sets object name anyways,
        // so, as better alternative, set it here right from the start
        a->setObjectName(QString::fromLatin1("ShowPage%1").arg(QString::number(pageCount++)));
        a->setText(info.text);
        connect(a, &QAction::triggered, this, info.callback);
        lutActions.insert(info.view, a);  // store QAction's pointer for later processing
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
    if (viewFrames.contains(idView)) {
        showPage(idView);
        viewBases[idView]->executeCustomAction(eView::Action::SetDefaultFocus);
    }
}

void KMyMoneyView::showPage(View idView)
{
    if (!viewFrames.contains(idView) ||
            currentPage() == viewFrames[idView])
        return;

    resetViewSelection();
    setCurrentPage(viewFrames[idView]);
}

bool KMyMoneyView::canPrint()
{
    return (MyMoneyFile::instance()->storageAttached() &&
            ((viewFrames.contains(View::Reports) && viewFrames[View::Reports] == currentPage()) ||
             (viewFrames.contains(View::Home) && viewFrames[View::Home] == currentPage()))
           );
}

void KMyMoneyView::enableViewsIfFileOpen(bool fileOpen)
{
    // call set enabled only if the state differs to avoid widgets 'bouncing on the screen' while doing this
    Q_ASSERT_X(((int)(View::Home)+1) == (int)View::Institutions, "viewenums.h", "View::Home must be first and View::Institutions second entry");

    for (auto i = (int)View::Institutions; i < (int)View::None; ++i)
        if (viewFrames.contains(View(i)))
            if (viewFrames[View(i)]->isEnabled() != fileOpen)
                viewFrames[View(i)]->setEnabled(fileOpen);

    emit viewStateChanged(fileOpen);
}

void KMyMoneyView::switchToDefaultView()
{
    const auto idView = KMyMoneySettings::startLastViewSelected() ?
                        static_cast<View>(KMyMoneySettings::lastViewSelected()) :
                        View::Home;
    // if we currently see a different page, then select the right one
    if (viewFrames.contains(idView) && viewFrames[idView] != currentPage())
        showPage(idView);
}

void KMyMoneyView::slotPayeeSelected(const QString& payee, const QString& account, const QString& transaction)
{
    showPage(View::Payees);
    static_cast<KPayeesView*>(viewBases[View::Payees])->slotSelectPayeeAndTransaction(payee, account, transaction);
}

void KMyMoneyView::slotTagSelected(const QString& tag, const QString& account, const QString& transaction)
{
    showPage(View::Tags);
    static_cast<KTagsView*>(viewBases[View::Tags])->slotSelectTagAndTransaction(tag, account, transaction);
}

void KMyMoneyView::finishReconciliation(const MyMoneyAccount& /* account */)
{
    Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
    static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers])->slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
}

void KMyMoneyView::viewAccountList(const QString& /*selectAccount*/)
{
    if (viewFrames[View::Accounts] != currentPage())
        showPage(View::Accounts);
    viewBases[View::Accounts]->show();
}

void KMyMoneyView::slotRefreshViews()
{
    for (auto i = (int)View::Home; i < (int)View::None; ++i)
        if (viewBases.contains(View(i)))
            viewBases[View(i)]->executeCustomAction(eView::Action::Refresh);

    viewBases[View::Payees]->executeCustomAction(eView::Action::ClosePayeeIdentifierSource);
}

void KMyMoneyView::slotShowTransactionDetail(bool detailed)
{
    KMyMoneySettings::setShowRegisterDetailed(detailed);
    slotRefreshViews();
}

void KMyMoneyView::slotCurrentPageChanged(const QModelIndex current, const QModelIndex previous)
{
    const auto view = currentPage();
    // remember the selected view if there is a real change
    if (previous.isValid()) {
        QHash<View, KPageWidgetItem*>::const_iterator it;
        for(it = viewFrames.cbegin(); it != viewFrames.cend(); ++it) {
            if ((*it) == view) {
                emit viewActivated(it.key());
                break;
            }
        }
    }

    if (viewBases.contains(View::Ledgers) && view != viewFrames.value(View::Ledgers))
        viewBases[View::Ledgers]->executeCustomAction(eView::Action::DisableViewDepenedendActions);

    pActions[eMenu::Action::Print]->setEnabled(canPrint());
    pActions[eMenu::Action::AccountCreditTransfer]->setEnabled(onlineJobAdministration::instance()->canSendCreditTransfer());
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
    if (viewFrames.contains(View::Reports) && viewFrames[View::Reports] == currentPage())
        viewBases[View::Reports]->executeCustomAction(eView::Action::Print);
    else if (viewFrames.contains(View::Home) && viewFrames[View::Home] == currentPage())
        viewBases[View::Home]->executeCustomAction(eView::Action::Print);
}

void KMyMoneyView::resetViewSelection()
{
    if (!MyMoneyFile::instance()->storageAttached())
        return;
    slotObjectSelected(MyMoneyAccount());
    slotObjectSelected(MyMoneyInstitution());
    slotObjectSelected(MyMoneySchedule());
    slotObjectSelected(MyMoneyTag());
    slotSelectByVariant(QVariantList {QVariant::fromValue(KMyMoneyRegister::SelectedTransactions())}, eView::Intent::SelectRegisterTransactions);
}

void KMyMoneyView::slotOpenObjectRequested(const MyMoneyObject& obj)
{
    if (typeid(obj) == typeid(MyMoneyAccount)) {
        const auto& acc = static_cast<const MyMoneyAccount&>(obj);
        // check if we can open this account
        // currently it make's sense for asset and liability accounts
        if (!MyMoneyFile::instance()->isStandardAccount(acc.id()))
            if (viewBases.contains(View::Ledgers))
                viewBases[View::Ledgers]->slotSelectByVariant(QVariantList {QVariant(acc.id()), QVariant(QString()) }, eView::Intent::ShowTransaction );

    } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
//    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);
        if (viewBases.contains(View::Institutions))
            viewBases[View::Institutions]->executeCustomAction(eView::Action::EditInstitution);
    } else if (typeid(obj) == typeid(MyMoneySchedule)) {
        if (viewBases.contains(View::Schedules))
            viewBases[View::Schedules]->executeCustomAction(eView::Action::EditSchedule);
    } else if (typeid(obj) == typeid(MyMoneyReport)) {
//    const auto& rep = static_cast<const MyMoneyReport&>(obj);
        showPage(View::Reports);
        if (viewBases.contains(View::Reports))
            viewBases[View::Reports]->slotSelectByObject(obj, eView::Intent::OpenObject);
    }
}

void KMyMoneyView::slotSelectByObject(const MyMoneyObject& obj, eView::Intent intent)
{
    switch (intent) {
    case eView::Intent::None:
        slotObjectSelected(obj);
        break;

    case eView::Intent::SynchronizeAccountInInvestmentView:
        if (viewBases.contains(View::Investments))
            viewBases[View::Investments]->slotSelectByObject(obj, intent);
        break;

    case eView::Intent::SynchronizeAccountInLedgersView:
        if (viewBases.contains(View::Ledgers))
            viewBases[View::Ledgers]->slotSelectByObject(obj, intent);
        break;

    case eView::Intent::OpenObject:
        slotOpenObjectRequested(obj);
        break;

    case eView::Intent::OpenContextMenu:
        slotContextMenuRequested(obj);
        break;

    case eView::Intent::StartEnteringOverdueScheduledTransactions:
        if (viewBases.contains(View::Schedules))
            viewBases[View::Schedules]->slotSelectByObject(obj, intent);
        break;

    case eView::Intent::FinishEnteringOverdueScheduledTransactions:
        if (viewBases.contains(View::Ledgers)) {
            showPage(View::Ledgers);
            viewBases[View::Ledgers]->slotSelectByObject(obj, intent);
        }
        break;

    default:
        break;
    }
}

void KMyMoneyView::slotSelectByVariant(const QVariantList& variant, eView::Intent intent)
{
    switch(intent) {
    case eView::Intent::ReportProgress:
        if (variant.count() == 2)
            emit statusProgress(variant.at(0).toInt(), variant.at(1).toInt());
        break;

    case eView::Intent::ReportProgressMessage:
        if (variant.count() == 1)
            emit statusMsg(variant.first().toString());
        break;

    case eView::Intent::UpdateNetWorth:
        if (viewBases.contains(View::Accounts))
            viewBases[View::Accounts]->slotSelectByVariant(variant, intent);

        if (viewBases.contains(View::Institutions))
            viewBases[View::Institutions]->slotSelectByVariant(variant, intent);
        break;

    case eView::Intent::UpdateProfit:
        if (viewBases.contains(View::Categories))
            viewBases[View::Categories]->slotSelectByVariant(variant, intent);
        break;

    case eView::Intent::ShowTransaction:
        if (viewBases.contains(View::Ledgers)) {
            showPage(View::Ledgers);
            viewBases[View::Ledgers]->slotSelectByVariant(variant, intent);
        }
        break;

    case eView::Intent::ToggleColumn:
        if (variant.count() == 2)
            slotAccountTreeViewChanged(variant.at(0).value<eAccountsModel::Column>(), variant.at(1).value<bool>());
        break;

    case eView::Intent::ShowPayee:
        if (viewBases.contains(View::Payees)) {
            showPage(View::Payees);
            viewBases[View::Payees]->slotSelectByVariant(variant, intent);
        }
        break;

    case eView::Intent::SelectRegisterTransactions:
        if (variant.count() == 1) {
            emit transactionsSelected(variant.at(0).value<KMyMoneyRegister::SelectedTransactions>()); // for plugins
            if (viewBases.contains(View::Ledgers))
                viewBases[View::Ledgers]->slotSelectByVariant(variant, intent);
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
    switch (action) {
    case eView::Action::AboutToShow:
        resetViewSelection();
        break;
    case eView::Action::SwitchView:
        showPage(view);
        break;
    case eView::Action::ShowBalanceChart:
        if (viewBases.contains(View::Reports))
            viewBases[View::Reports]->executeCustomAction(action);
        break;
    default:
        break;
    }
}

void KMyMoneyView::slotObjectSelected(const MyMoneyObject& obj)
{
    // carrying some slots over to views isn't easy for all slots...
    // ...so calls to kmymoney still must be here
    if (typeid(obj) == typeid(MyMoneyAccount)) {
        QVector<View> views {View::Investments, View::Categories, View::Accounts,
                             View::Ledgers, View::Reports, View::OnlineJobOutbox};
        for (const auto view : views)
            if (viewBases.contains(view))
                viewBases[view]->slotSelectByObject(obj, eView::Intent::UpdateActions);

        // for plugin only
        const auto& acc = static_cast<const MyMoneyAccount&>(obj);
        if (!acc.isIncomeExpense() &&
                !MyMoneyFile::instance()->isStandardAccount(acc.id()))
            emit accountSelected(acc);
    } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
        viewBases[View::Institutions]->slotSelectByObject(obj, eView::Intent::UpdateActions);
    } else if (typeid(obj) == typeid(MyMoneySchedule)) {
        viewBases[View::Schedules]->slotSelectByObject(obj, eView::Intent::UpdateActions);
    }
}

void KMyMoneyView::slotContextMenuRequested(const MyMoneyObject& obj)
{
    if (typeid(obj) == typeid(MyMoneyAccount)) {
        const auto& acc = static_cast<const MyMoneyAccount&>(obj);
        if (acc.isInvest())
            viewBases[View::Investments]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
        else if (acc.isIncomeExpense())
            viewBases[View::Categories]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
        else
            viewBases[View::Accounts]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);

    } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
        viewBases[View::Institutions]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
    } else if (typeid(obj) == typeid(MyMoneySchedule)) {
        viewBases[View::Schedules]->slotSelectByObject(obj, eView::Intent::OpenContextMenu);
    }
}
