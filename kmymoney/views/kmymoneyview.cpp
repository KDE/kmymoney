/***************************************************************************
                          kmymoneyview.cpp
                             -------------------
    copyright            : (C) 2000-2001 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>
                               2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
#include <KCompressionDevice>
#include <KSharedConfig>
#include <KBackup>
#include <KActionCollection>
#include <KIO/StoredTransferJob>
#include <KJobWidgets>
#include <KLocalizedString>

#ifdef KF5Activities_FOUND
#include <KActivities/ResourceInstance>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#ifdef ENABLE_UNFINISHEDFEATURES
#include "simpleledgerview.h"
#endif

#include "kmymoneysettings.h"
#include "kmymoneytitlelabel.h"
#include <libkgpgfile/kgpgfile.h>
#include "kcurrencyeditdlg.h"
#include "mymoneystoragemgr.h"
#include "mymoneystoragebin.h"
#include "mymoneyexception.h"
#include "mymoneystoragexml.h"
#include "mymoneystorageanon.h"
#include "khomeview.h"
#include "kaccountsview.h"
#include "kcategoriesview.h"
#include "kinstitutionsview.h"
#include "kpayeesview.h"
#include "ktagsview.h"
#include "kscheduledview.h"
#include "kgloballedgerview.h"
#include "kinvestmentview.h"
#include "kreportsview.h"
#include "kbudgetview.h"
#include "konlinejoboutbox.h"
#include "kmymoney.h"
#include "models.h"
#include "accountsmodel.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"
#include "icons.h"
#include "amountedit.h"
#include "kmymoneyaccounttreeview.h"
#include "accountsviewproxymodel.h"
#include "mymoneyprice.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "kmymoneyedit.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyreport.h"
#include "kmymoneyplugin.h"
#include "mymoneyenums.h"

using namespace Icons;
using namespace eMyMoney;

typedef void(KMyMoneyView::*KMyMoneyViewFunc)();

KMyMoneyView::KMyMoneyView(KMyMoneyApp *kmymoney)
    : KPageWidget(nullptr),
    m_header(0),
    m_inConstructor(true),
    m_lastViewSelected(0),
    m_storagePlugins(nullptr)
#ifdef KF5Activities_FOUND
    , m_activityResourceInstance(0)
#endif
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
      // after we remove the KPageWidget standard header replace it with our own title label
      m_header = new KMyMoneyTitleLabel(this);
      m_header->setObjectName("titleLabel");
      m_header->setMinimumSize(QSize(100, 30));
      m_header->setRightImageFile("pics/titlelabel_background.png");
      m_header->setVisible(KMyMoneySettings::showTitleBar());
      gridLayout->addWidget(m_header, 1, 1);
    }
  }

//  newStorage();
  m_model = new KPageWidgetModel(this); // cannot be parentless, otherwise segfaults at exit

  connect(kmymoney, &KMyMoneyApp::fileLoaded, this, &KMyMoneyView::slotRefreshViews);

  viewBases[View::Home] = new KHomeView;
  viewBases[View::Institutions] = new KInstitutionsView;
  viewBases[View::Accounts] = new KAccountsView;
  viewBases[View::Schedules] = new KScheduledView;
  viewBases[View::Categories] = new KCategoriesView;
  viewBases[View::Tags] = new KTagsView;
  viewBases[View::Payees] = new KPayeesView;
  viewBases[View::Ledgers] = new KGlobalLedgerView;
  viewBases[View::Investments] = new KInvestmentView;
  viewBases[View::Reports] = new KReportsView;
  viewBases[View::Budget] = new KBudgetView;
  viewBases[View::OnlineJobOutbox] = new KOnlineJobOutbox;
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
    {View::Home,            i18n("Home"),                         Icon::ViewHome},
    {View::Institutions,    i18n("Institutions"),                 Icon::ViewInstitutions},
    {View::Accounts,        i18n("Accounts"),                     Icon::ViewAccounts},
    {View::Schedules,       i18n("Scheduled\u2028transactions"),  Icon::ViewSchedules},
    {View::Categories,      i18n("Categories"),                   Icon::ViewCategories},
    {View::Tags,            i18n("Tags"),                         Icon::ViewTags},
    {View::Payees,          i18n("Payees"),                       Icon::ViewPayees},
    {View::Ledgers,         i18n("Ledgers"),                      Icon::ViewLedgers},
    {View::Investments,     i18n("Investments"),                  Icon::ViewInvestment},
    {View::Reports,         i18n("Reports"),                      Icon::ViewReports},
    {View::Budget,          i18n("Budgets"),                      Icon::ViewBudgets},
    {View::OnlineJobOutbox, i18n("Outbox"),                       Icon::ViewOutbox},
    #ifdef ENABLE_UNFINISHEDFEATURES
    {View::NewLedgers,      i18n("New ledger"),                   Icon::DocumentProperties},
    #endif
  };

  for (const viewInfo& view : viewsInfo) {
    viewFrames[view.id] = m_model->addPage(viewBases[view.id], view.name);
    viewFrames[view.id]->setIcon(Icons::get(view.icon));
    connect(viewBases[view.id], &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
    connect(viewBases[view.id], &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);
  }

  const auto& homeView = static_cast<KHomeView*>(viewBases[View::Home]);
  const auto& ledgersView = static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers]);
  const auto& reportsView = static_cast<KReportsView*>(viewBases[View::Reports]);

  connect(reportsView, &KReportsView::switchViewRequested, this, &KMyMoneyView::showPage);
  connect(ledgersView, &KGlobalLedgerView::switchViewRequested, this, &KMyMoneyView::showPage);
  connect(homeView, &KHomeView::ledgerSelected, ledgersView, &KGlobalLedgerView::slotLedgerSelected);
  connect(kmymoney, &KMyMoneyApp::transactionSelected, ledgersView, &KGlobalLedgerView::slotLedgerSelected);

  //set the model
  setModel(m_model);
  setCurrentPage(viewFrames[View::Home]);
  connect(this, SIGNAL(currentPageChanged(QModelIndex,QModelIndex)), this, SLOT(slotCurrentPageChanged(QModelIndex,QModelIndex)));

  updateViewType();

  m_inConstructor = false;

  // add fast switching of main views through Ctrl + NUM_X
  struct pageInfo {
    View             view;
    KMyMoneyViewFunc callback;
    QString          text;
    QKeySequence     shortcut = QKeySequence();
  };

  const QVector<pageInfo> pageInfos {
    {View::Home,            &KMyMoneyView::slotShowHomePage,          i18n("Show home page"),                   Qt::CTRL + Qt::Key_1},
    {View::Institutions,    &KMyMoneyView::slotShowInstitutionsPage,  i18n("Show institutions page"),           Qt::CTRL + Qt::Key_2},
    {View::Accounts,        &KMyMoneyView::slotShowAccountsPage,      i18n("Show accounts page"),               Qt::CTRL + Qt::Key_3},
    {View::Schedules,       &KMyMoneyView::slotShowSchedulesPage,     i18n("Show scheduled transactions page"), Qt::CTRL + Qt::Key_4},
    {View::Categories,      &KMyMoneyView::slotShowCategoriesPage,    i18n("Show categories page"),             Qt::CTRL + Qt::Key_5},
    {View::Tags,            &KMyMoneyView::slotShowTagsPage,          i18n("Show tags page"),                   },
    {View::Payees,          &KMyMoneyView::slotShowPayeesPage,        i18n("Show payees page"),                 Qt::CTRL + Qt::Key_6},
    {View::Ledgers,         &KMyMoneyView::slotShowLedgersPage,       i18n("Show ledgers page"),                Qt::CTRL + Qt::Key_7},
    {View::Investments,     &KMyMoneyView::slotShowInvestmentsPage,   i18n("Show investments page"),            Qt::CTRL + Qt::Key_8},
    {View::Reports,         &KMyMoneyView::slotShowReportsPage,       i18n("Show reports page"),                Qt::CTRL + Qt::Key_9},
    {View::Budget,          &KMyMoneyView::slotShowBudgetPage,        i18n("Show budget page"),                },
    {View::Forecast,        &KMyMoneyView::slotShowForecastPage,      i18n("Show forecast page"),              },
    {View::OnlineJobOutbox, &KMyMoneyView::slotShowOutboxPage,        i18n("Show outbox page")                 }
  };

  QHash<View, QAction *> lutActions;
  auto aC = kmymoney->actionCollection();
  auto pageCount = 0;
  foreach (const pageInfo info, pageInfos) {
    auto a = new QAction(this);
    // KActionCollection::addAction by name sets object name anyways,
    // so, as better alternative, set it here right from the start
    a->setObjectName(QLatin1String("ShowPage") + QString::number(pageCount++));
    a->setText(info.text);
    connect(a, &QAction::triggered, this, info.callback);
    lutActions.insert(info.view, a);  // store QAction's pointer for later processing
    if (!info.shortcut.isEmpty())
      aC->setDefaultShortcut(a, info.shortcut);
  }
  aC->addActions(lutActions.values());

  // Initialize kactivities resource instance

#ifdef KF5Activities_FOUND
  m_activityResourceInstance = new KActivities::ResourceInstance(window()->winId(), this);
  connect(kmymoney, SIGNAL(fileLoaded(QUrl)), m_activityResourceInstance, SLOT(setUri(QUrl)));
#endif
}

KMyMoneyView::~KMyMoneyView()
{
  KMyMoneySettings::setLastViewSelected(m_lastViewSelected);
#ifdef KF5Activities_FOUND
delete m_activityResourceInstance;
#endif
//  removeStorage();
}

void KMyMoneyView::slotFileOpened()
{
  #ifdef ENABLE_UNFINISHEDFEATURES
  static_cast<SimpleLedgerView*>(viewBases[View::NewLedgers])->openFavoriteLedgers();
  #endif
  switchToDefaultView();
}

void KMyMoneyView::slotFileClosed()
{

  if (viewBases.contains(View::Reports))
    static_cast<KReportsView*>(viewBases[View::Reports])->slotCloseAll();
  #ifdef ENABLE_UNFINISHEDFEATURES
  static_cast<SimpleLedgerView*>(viewBases[View::NewLedgers])->closeLedgers();
  #endif
  slotShowHomePage();
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

void KMyMoneyView::showTitleBar(bool show)
{
  if (m_header)
    m_header->setVisible(show);
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
    static_cast<KMyMoneyAccountsViewBase*>(viewBases[View::Categories])->getProxyModel(),
    static_cast<KMyMoneyAccountsViewBase*>(viewBases[View::Budget])->getProxyModel(),
  };

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
  static_cast<KAccountsView*>(viewBases[View::Accounts])->setOnlinePlugins(plugins);
  if (viewBases.contains(View::OnlineJobOutbox)) {
    if (plugins.isEmpty())
      removeView(View::OnlineJobOutbox);
    else
      static_cast<KOnlineJobOutbox*>(viewBases[View::OnlineJobOutbox])->setOnlinePlugins(plugins);
  }
}

void KMyMoneyView::setStoragePlugins(QMap<QString, KMyMoneyPlugin::StoragePlugin*>& plugins)
{
  m_storagePlugins = &plugins;
}

eDialogs::ScheduleResultCode KMyMoneyView::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{

  return static_cast<KScheduledView*>(viewBases[View::Schedules])->enterSchedule(schedule, autoEnter, extendedKeys);
}

void KMyMoneyView::addView(KMyMoneyViewBase* view, const QString& name, View idView)
{
  auto isViewInserted = false;
  for (auto i = (int)idView; i < (int)View::None; ++i) {
    if (viewFrames.contains((View)i)) {
      viewFrames[idView] = m_model->insertPage(viewFrames[(View)i],view, name);
      viewBases[idView] = view;
      isViewInserted = true;
      break;
    }
  }

  if (!isViewInserted)
    viewFrames[idView] = m_model->addPage(view, name);

  auto icon = Icon::ViewForecast;
  switch (idView) {
    case View::Forecast:
      icon = Icon::ViewForecast;
      break;
    case View::OnlineJobOutbox:
      icon = Icon::ViewOutbox;
      break;
    default:
      break;
  }
  viewFrames[idView]->setIcon(Icons::get(icon));
}

void KMyMoneyView::removeView(View idView)
{
  m_model->removePage(viewFrames[idView]);
  viewFrames.remove(idView);
  viewBases.remove(idView);
}

bool KMyMoneyView::showPageHeader() const
{
  return false;
}

void KMyMoneyView::showPageAndFocus(View idView)
{
  if (viewFrames.contains(idView)) {
    showPage(idView);
    viewBases[idView]->setDefaultFocus();
  }
}

void KMyMoneyView::showPage(View idView)
{
  const auto pageItem = viewFrames[idView];
  // reset all selected items before showing the selected view
  // but not while we're in our own constructor
  if (!m_inConstructor && pageItem != currentPage()) {
    kmymoney->slotResetSelections();
  }

  // pretend we're in the constructor to avoid calling the
  // above resets. For some reason which I don't know the details
  // of, KJanusWidget::showPage() calls itself recursively. This
  // screws up the action handling, as items could have been selected
  // in the meantime. We prevent this by setting the m_inConstructor
  // to true and reset it to the previos value when we leave this method.
  bool prevConstructor = m_inConstructor;
  m_inConstructor = true;

  setCurrentPage(pageItem);

  m_inConstructor = prevConstructor;

  if (!m_inConstructor) {
    // fixup some actions that are dependant on the view
    // this does not work during construction
    kmymoney->slotUpdateActions();
  }
}

bool KMyMoneyView::canPrint()
{
  bool rc = (
              viewFrames[View::Reports] == currentPage()
              || viewFrames[View::Home] == currentPage()
            );
  return rc;
}

void KMyMoneyView::enableViewsIfFileOpen(bool fileOpen)
{
  // call set enabled only if the state differs to avoid widgets 'bouncing on the screen' while doing this
  for (auto i = (int)View::Home; i < (int)View::None; ++i)
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

void KMyMoneyView::slotSetBaseCurrency(const MyMoneySecurity& baseCurrency)
{
  if (!baseCurrency.id().isEmpty()) {
    QString baseId;
    try {
      baseId = MyMoneyFile::instance()->baseCurrency().id();
    } catch (const MyMoneyException &e) {
      qDebug("%s", qPrintable(e.what()));
    }

    if (baseCurrency.id() != baseId) {
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->setBaseCurrency(baseCurrency);
        ft.commit();
      } catch (const MyMoneyException &e) {
        KMessageBox::sorry(this, i18n("Cannot set %1 as base currency: %2", baseCurrency.name(), e.what()), i18n("Set base currency"));
      }
    }
    AmountEdit::setStandardPrecision(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));
    KMyMoneyEdit::setStandardPrecision(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));
  }
}

void KMyMoneyView::viewAccountList(const QString& /*selectAccount*/)
{
  if (viewFrames[View::Accounts] != currentPage())
    showPage(View::Accounts);
  viewBases[View::Accounts]->show();
}

void KMyMoneyView::slotRefreshViews()
{
  const auto& investmentsView = static_cast<KInvestmentView*>(viewBases[View::Investments]);
  const auto& ledgersView = static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers]);
  // turn off sync between ledger and investment view
  disconnect(investmentsView, &KInvestmentView::accountSelected, ledgersView, static_cast<void (KGlobalLedgerView::*)(const MyMoneyObject&)>(&KGlobalLedgerView::slotSelectAccount));
  disconnect(ledgersView, &KGlobalLedgerView::objectSelected, investmentsView, static_cast<void (KInvestmentView::*)(const MyMoneyObject&)>(&KInvestmentView::slotSelectAccount));

  // TODO turn sync between ledger and investment view if selected by user
  if (KMyMoneySettings::syncLedgerInvestment()) {
    connect(investmentsView, &KInvestmentView::accountSelected, ledgersView, static_cast<void (KGlobalLedgerView::*)(const MyMoneyObject&)>(&KGlobalLedgerView::slotSelectAccount));
    connect(ledgersView, &KGlobalLedgerView::objectSelected, investmentsView, static_cast<void (KInvestmentView::*)(const MyMoneyObject&)>(&KInvestmentView::slotSelectAccount));
  }

  showTitleBar(KMyMoneySettings::showTitleBar());

  for (auto i = (int)View::Home; i < (int)View::None; ++i)
    if (viewBases.contains(View(i)))
      viewBases[View(i)]->refresh();

  static_cast<KPayeesView*>(viewBases[View::Payees])->slotClosePayeeIdentifierSource();
}

void KMyMoneyView::slotShowTransactionDetail(bool detailed)
{
  KMyMoneySettings::setShowRegisterDetailed(detailed);
  slotRefreshViews();
}

void KMyMoneyView::slotCurrentPageChanged(const QModelIndex current, const QModelIndex)
{
  // remember the current page
  m_lastViewSelected = current.row();
  // set the current page's title in the header
  if (m_header)
    m_header->setText(m_model->data(current, KPageModel::HeaderRole).toString());
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
        throw MYMONEYEXCEPTION("Transaction for schedule has less than 2 splits!");
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
      KMessageBox::information(this, i18n("Unable to add schedule: %1", e.what()));
    }
  }
}

void KMyMoneyView::slotPrintView()
{
  if (viewFrames[View::Reports] == currentPage())
    static_cast<KReportsView*>(viewBases[View::Reports])->slotPrintView();
  else if (viewFrames[View::Home] == currentPage())
    static_cast<KHomeView*>(viewBases[View::Home])->slotPrintView();
}

void KMyMoneyView::resetViewSelection(const View)
{
  emit aboutToChangeView();
}

void KMyMoneyView::connectView(const View view)
{
  KMyMoneyAccountTreeView *treeView;

  if (!viewBases.contains(view))
    return;

  disconnect(viewBases[view], &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);

  switch (view) {
    case View::Home:
      {
      const auto homeView = static_cast<KHomeView*>(viewBases[view]);
      connect(homeView, &KHomeView::objectSelected,      this, &KMyMoneyView::slotObjectSelected);
      connect(homeView, &KHomeView::openObjectRequested, this, &KMyMoneyView::slotOpenObjectRequested);
      // views can wait since they are going to be refresed in slotRefreshViews
      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, homeView, &KHomeView::refresh);
      break;
      }
    case View::Accounts:
      {
      const auto accountsView = static_cast<KAccountsView*>(viewBases[view]);
      treeView = static_cast<KMyMoneyAccountsViewBase*>(viewBases[view])->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(accountsView, &KAccountsView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      connect(Models::instance()->accountsModel(), &AccountsModel::netWorthChanged, accountsView, &KAccountsView::slotNetWorthChanged);
      break;
      }

    case View::Schedules:
      {
      const auto schedulesView = static_cast<KScheduledView*>(viewBases[view]);
      connect(schedulesView, &KScheduledView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(schedulesView, &KScheduledView::objectSelected,         this, &KMyMoneyView::slotObjectSelected);
      connect(schedulesView, &KScheduledView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      break;
      }

    case View::Institutions:
      {
      const auto institutionsView = static_cast<KInstitutionsView*>(viewBases[view]);
      treeView = static_cast<KMyMoneyAccountsViewBase*>(viewBases[view])->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(institutionsView, &KInstitutionsView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      connect(Models::instance()->institutionsModel(), &AccountsModel::netWorthChanged, institutionsView, &KInstitutionsView::slotNetWorthChanged);
      break;
      }

    case View::Categories:
      {
      const auto categoriesView = static_cast<KCategoriesView*>(viewBases[view]);
      treeView = static_cast<KMyMoneyAccountsViewBase*>(viewBases[view])->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::objectSelected,         this, &KMyMoneyView::slotObjectSelected);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(categoriesView, &KCategoriesView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      connect(Models::instance()->institutionsModel(), &AccountsModel::profitChanged, categoriesView, &KCategoriesView::slotProfitChanged);
      break;
      }

    case View::Tags:
      {
      const auto tagsView = static_cast<KTagsView*>(viewBases[view]);
      const auto ledgersView = static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers]);
      connect(tagsView, &KTagsView::transactionSelected,  ledgersView, &KGlobalLedgerView::slotLedgerSelected);
      break;
      }

    case View::Payees:
      {
      const auto payeesView = static_cast<KPayeesView*>(viewBases[view]);
      const auto ledgersView = static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers]);
      connect(payeesView, &KPayeesView::transactionSelected, ledgersView, &KGlobalLedgerView::slotLedgerSelected);
      break;
      }

    case View::Ledgers:
      {
      const auto ledgersView = static_cast<KGlobalLedgerView*>(viewBases[view]);
      const auto schedulesView = static_cast<KScheduledView*>(viewBases[View::Schedules]);
      connect(ledgersView, &KGlobalLedgerView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(ledgersView, &KGlobalLedgerView::openPayeeRequested,     this, &KMyMoneyView::slotPayeeSelected);
      connect(ledgersView, &KGlobalLedgerView::objectSelected,         this, &KMyMoneyView::slotObjectSelected);
      connect(ledgersView, &KGlobalLedgerView::transactionsSelected,   this, &KMyMoneyView::slotTransactionsSelected);
      connect(ledgersView, &KGlobalLedgerView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(ledgersView, &KGlobalLedgerView::transactionsContextMenuRequested,   this, &KMyMoneyView::slotTransactionsMenuRequested);
      connect(ledgersView, &KGlobalLedgerView::statusProgress,   this, &KMyMoneyView::statusProgress);
      connect(ledgersView, &KGlobalLedgerView::statusMsg,   this, &KMyMoneyView::statusMsg);

      connect(ledgersView, &KGlobalLedgerView::accountReconciled, this, &KMyMoneyView::accountReconciled);
      connect(ledgersView, &KGlobalLedgerView::enterOverdueSchedulesRequested, schedulesView, &KScheduledView::slotEnterOverdueSchedules);
      connect(schedulesView, &KScheduledView::enterOverdueSchedulesFinished, ledgersView, &KGlobalLedgerView::slotContinueReconciliation);
      break;
      }

    case View::Budget:
      {
      const auto budgetView = static_cast<KBudgetView*>(viewBases[view]);
      treeView = static_cast<KMyMoneyAccountsViewBase*>(viewBases[view])->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(budgetView, &KBudgetView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      break;
      }

    case View::Investments:
      {
      const auto investmentsView = static_cast<KInvestmentView*>(viewBases[view]);
      connect(investmentsView, &KInvestmentView::objectSelected,       this, &KMyMoneyView::slotObjectSelected);
      connect(investmentsView, &KInvestmentView::contextMenuRequested, this, &KMyMoneyView::slotContextMenuRequested);
      break;
      }

    case View::Reports:
      {
      const auto reportsView = static_cast<KReportsView*>(viewBases[view]);
      const auto ledgersView = static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers]);
      connect(reportsView, &KReportsView::transactionSelected, ledgersView, &KGlobalLedgerView::slotLedgerSelected);
      break;
      }

    default:
      break;
  }
}

void KMyMoneyView::slotOpenObjectRequested(const MyMoneyObject& obj)
{
  if (typeid(obj) == typeid(MyMoneyAccount)) {
    const auto& acc = static_cast<const MyMoneyAccount&>(obj);
    // check if we can open this account
    // currently it make's sense for asset and liability accounts
    if (!MyMoneyFile::instance()->isStandardAccount(acc.id()))
      static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers])->slotLedgerSelected(acc.id(), QString());

  } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
//    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);
    static_cast<KInstitutionsView*>(viewBases[View::Institutions])->slotEditInstitution();
  } else if (typeid(obj) == typeid(MyMoneySchedule)) {
    static_cast<KScheduledView*>(viewBases[View::Schedules])->slotEditSchedule();
  } else if (typeid(obj) == typeid(MyMoneyReport)) {
    const auto& rep = static_cast<const MyMoneyReport&>(obj);
    static_cast<KReportsView*>(viewBases[View::Reports])->slotOpenReport(rep);
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
        viewBases[view]->updateActions(obj);

    // for plugin only
    const auto& acc = static_cast<const MyMoneyAccount&>(obj);
    if (!acc.isIncomeExpense() &&
        !MyMoneyFile::instance()->isStandardAccount(acc.id()))
      emit accountSelected(acc);
  } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
    viewBases[View::Institutions]->updateActions(obj);
  } else if (typeid(obj) == typeid(MyMoneySchedule)) {
    viewBases[View::Schedules]->updateActions(obj);
  }
}

void KMyMoneyView::slotContextMenuRequested(const MyMoneyObject& obj)
{
  if (typeid(obj) == typeid(MyMoneyAccount)) {
    const auto& acc = static_cast<const MyMoneyAccount&>(obj);
    if (acc.isInvest()) {
      static_cast<KInvestmentView*>(viewBases[View::Investments])->slotShowInvestmentMenu(acc);
      return;
    } else if (acc.isIncomeExpense()) {
      static_cast<KCategoriesView*>(viewBases[View::Categories])->slotShowCategoriesMenu(acc);
    } else {
      static_cast<KAccountsView*>(viewBases[View::Accounts])->slotShowAccountMenu(acc);
    }
  } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);
    static_cast<KInstitutionsView*>(viewBases[View::Institutions])->slotShowInstitutionsMenu(inst);
  } else if (typeid(obj) == typeid(MyMoneySchedule)) {
    const auto& sch = static_cast<const MyMoneySchedule&>(obj);
    static_cast<KScheduledView*>(viewBases[View::Schedules])->slotShowScheduleMenu(sch);
  }
}

void KMyMoneyView::slotTransactionsMenuRequested(const KMyMoneyRegister::SelectedTransactions& list)
{
  Q_UNUSED(list)
  static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers])->slotShowTransactionMenu(MyMoneySplit());
}

void KMyMoneyView::slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& list)
{
  static_cast<KGlobalLedgerView*>(viewBases[View::Ledgers])->updateLedgerActions(list);
  emit transactionsSelected(list); // for plugins
}
