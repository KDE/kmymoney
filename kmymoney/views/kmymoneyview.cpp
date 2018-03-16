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

  // Page 0
  m_homeView = new KHomeView;
  viewFrames[View::Home] = m_model->addPage(m_homeView, i18n("Home"));
  viewFrames[View::Home]->setIcon(Icons::get(Icon::ViewHome));
  connect(m_homeView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_homeView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 1
  m_institutionsView = new KInstitutionsView;
  viewFrames[View::Institutions] = m_model->addPage(m_institutionsView, i18n("Institutions"));
  viewFrames[View::Institutions]->setIcon(Icons::get(Icon::ViewInstitutions));
  connect(m_institutionsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_institutionsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 2
  m_accountsView = new KAccountsView;
  viewFrames[View::Accounts] = m_model->addPage(m_accountsView, i18n("Accounts"));
  viewFrames[View::Accounts]->setIcon(Icons::get(Icon::ViewAccounts));
  connect(m_accountsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_accountsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 3
  m_scheduledView = new KScheduledView;
//this is to solve the way long strings are handled differently among versions of KPageWidget
  viewFrames[View::Schedules] = m_model->addPage(m_scheduledView, i18nc("use \u2028 as line break", "Scheduled\u2028transactions"));
  viewFrames[View::Schedules]->setIcon(Icons::get(Icon::ViewSchedules));
  connect(m_scheduledView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_scheduledView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 4
  m_categoriesView = new KCategoriesView;
  viewFrames[View::Categories] = m_model->addPage(m_categoriesView, i18n("Categories"));
  viewFrames[View::Categories]->setIcon(Icons::get(Icon::ViewCategories));
  connect(m_categoriesView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_categoriesView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 5
  m_tagsView = new KTagsView;
  viewFrames[View::Tags] = m_model->addPage(m_tagsView, i18n("Tags"));
  viewFrames[View::Tags]->setIcon(Icons::get(Icon::ViewTags));
  connect(m_tagsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_tagsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 6
  m_payeesView = new KPayeesView;
  viewFrames[View::Payees] = m_model->addPage(m_payeesView, i18n("Payees"));
  viewFrames[View::Payees]->setIcon(Icons::get(Icon::ViewPayees));
  connect(m_payeesView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_payeesView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 7
  m_ledgerView = new KGlobalLedgerView;
  viewFrames[View::Ledgers] = m_model->addPage(m_ledgerView, i18n("Ledgers"));
  viewFrames[View::Ledgers]->setIcon(Icons::get(Icon::ViewLedgers));
  connect(m_ledgerView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_ledgerView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 8
  m_investmentView = new KInvestmentView;
  viewFrames[View::Investments] = m_model->addPage(m_investmentView, i18n("Investments"));
  viewFrames[View::Investments]->setIcon(Icons::get(Icon::ViewInvestment));
  connect(m_investmentView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_investmentView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 9
  m_reportsView = new KReportsView;
  viewFrames[View::Reports] = m_model->addPage(m_reportsView, i18n("Reports"));
  viewFrames[View::Reports]->setIcon(Icons::get(Icon::ViewReports));
  connect(m_reportsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_reportsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 10
  m_budgetView = new KBudgetView;
  viewFrames[View::Budget] = m_model->addPage(m_budgetView, i18n("Budgets"));
  viewFrames[View::Budget]->setIcon(Icons::get(Icon::ViewBudgets));
  connect(m_budgetView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_budgetView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 11
  // KForecastView

  // Page 12
  m_onlineJobOutboxView = new KOnlineJobOutbox;
  addView(m_onlineJobOutboxView, i18n("Outbox"), View::OnlineJobOutbox);
  connect(m_onlineJobOutboxView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_onlineJobOutboxView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  connect(m_reportsView, &KReportsView::switchViewRequested, this, &KMyMoneyView::slotSwitchView);
  connect(m_ledgerView, &KGlobalLedgerView::switchViewRequested, this, &KMyMoneyView::slotSwitchView);
  connect(m_homeView, &KHomeView::ledgerSelected, m_ledgerView, &KGlobalLedgerView::slotLedgerSelected);

#ifdef ENABLE_UNFINISHEDFEATURES
  m_simpleLedgerView = new SimpleLedgerView(kmymoney, this);
  KPageWidgetItem* frame = m_model->addPage(m_simpleLedgerView, i18n("New ledger"));
  frame->setIcon(Icons::get(Icon::DocumentProperties));
#endif


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
  m_simpleLedgerView->openFavoriteLedgers();
  #endif
  switchToDefaultView();
}

void KMyMoneyView::slotFileClosed()
{
  if (m_reportsView)
    m_reportsView->slotCloseAll();
  #ifdef ENABLE_UNFINISHEDFEATURES
  m_simpleLedgerView->closeLedgers();
  #endif
  slotShowHomePage();
}

void KMyMoneyView::slotShowHomePage()
{
  showPage(viewFrames[View::Home]);
}

void KMyMoneyView::slotShowInstitutionsPage()
{
  showPage(viewFrames[View::Institutions]);
  m_institutionsView->setDefaultFocus();
}

void KMyMoneyView::slotShowAccountsPage()
{
  showPage(viewFrames[View::Accounts]);
  m_accountsView->setDefaultFocus();
}

void KMyMoneyView::slotShowSchedulesPage()
{
  showPage(viewFrames[View::Schedules]);
  m_scheduledView->setDefaultFocus();
}

void KMyMoneyView::slotShowCategoriesPage()
{
  showPage(viewFrames[View::Categories]);
  m_categoriesView->setDefaultFocus();
}

void KMyMoneyView::slotShowTagsPage()
{
  showPage(viewFrames[View::Tags]);
  m_tagsView->setDefaultFocus();
}

void KMyMoneyView::slotShowPayeesPage()
{
  showPage(viewFrames[View::Payees]);
  m_payeesView->setDefaultFocus();
}

void KMyMoneyView::slotShowLedgersPage()
{
  showPage(viewFrames[View::Ledgers]);
  m_ledgerView->setDefaultFocus();
}

void KMyMoneyView::slotShowInvestmentsPage()
{
  showPage(viewFrames[View::Investments]);
  m_investmentView->setDefaultFocus();
}

void KMyMoneyView::slotShowReportsPage()
{
  showPage(viewFrames[View::Reports]);
  m_reportsView->setDefaultFocus();
}

void KMyMoneyView::slotShowBudgetPage()
{
  showPage(viewFrames[View::Budget]);
  m_budgetView->setDefaultFocus();
}

void KMyMoneyView::slotShowForecastPage()
{
  if (viewFrames.contains(View::Forecast)) {
    showPage(viewFrames[View::Forecast]);
    viewBases[View::Forecast]->setDefaultFocus();
  }
}

void KMyMoneyView::slotShowOutboxPage()
{
  if (viewFrames[View::OnlineJobOutbox]) {
    showPage(viewFrames[View::OnlineJobOutbox]);
    viewBases[View::OnlineJobOutbox]->setDefaultFocus();
  }
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
  QVector<AccountsViewProxyModel *> proxyModels {m_institutionsView->getProxyModel(), m_accountsView->getProxyModel(),
                                                 m_categoriesView->getProxyModel(), m_budgetView->getProxyModel()};

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
  m_accountsView->setOnlinePlugins(plugins);
  if (m_onlineJobOutboxView) {
    m_onlineJobOutboxView->setOnlinePlugins(plugins);
  }
  if (plugins.isEmpty()) {
    removeView(View::OnlineJobOutbox);
    m_onlineJobOutboxView = nullptr;
  }
}

void KMyMoneyView::setStoragePlugins(QMap<QString, KMyMoneyPlugin::StoragePlugin*>& plugins)
{
  m_storagePlugins = &plugins;
}

eDialogs::ScheduleResultCode KMyMoneyView::enterSchedule(MyMoneySchedule& schedule, bool autoEnter, bool extendedKeys)
{
  return m_scheduledView->enterSchedule(schedule, autoEnter, extendedKeys);
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

void KMyMoneyView::slotSwitchView(View view)
{
  showPage(viewFrames[view]);
}

void KMyMoneyView::showPage(KPageWidgetItem* pageItem)
{
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
  KPageWidgetItem* page;
  if (KMyMoneySettings::startLastViewSelected() != 0)
    page = viewFrames.value(static_cast<View>(KMyMoneySettings::lastViewSelected()));
  else
    page = viewFrames[View::Home];
  // if we currently see a different page, then select the right one
  if (page != currentPage())
    showPage(page);
}

void KMyMoneyView::slotPayeeSelected(const QString& payee, const QString& account, const QString& transaction)
{
  showPage(viewFrames[View::Payees]);
  m_payeesView->slotSelectPayeeAndTransaction(payee, account, transaction);
}

void KMyMoneyView::slotTagSelected(const QString& tag, const QString& account, const QString& transaction)
{
  showPage(viewFrames[View::Tags]);
  m_tagsView->slotSelectTagAndTransaction(tag, account, transaction);
}

void KMyMoneyView::finishReconciliation(const MyMoneyAccount& /* account */)
{
  Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
  m_ledgerView->slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
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
    showPage(viewFrames[View::Accounts]);
  m_accountsView->show();
}

void KMyMoneyView::slotRefreshViews()
{
  // turn off sync between ledger and investment view
  disconnect(m_investmentView, &KInvestmentView::accountSelected, m_ledgerView, static_cast<void (KGlobalLedgerView::*)(const MyMoneyObject&)>(&KGlobalLedgerView::slotSelectAccount));
  disconnect(m_ledgerView, &KGlobalLedgerView::objectSelected, m_investmentView, static_cast<void (KInvestmentView::*)(const MyMoneyObject&)>(&KInvestmentView::slotSelectAccount));


  // TODO turn sync between ledger and investment view if selected by user
  if (KMyMoneySettings::syncLedgerInvestment()) {
    connect(m_investmentView, &KInvestmentView::accountSelected, m_ledgerView, static_cast<void (KGlobalLedgerView::*)(const MyMoneyObject&)>(&KGlobalLedgerView::slotSelectAccount));
    connect(m_ledgerView, &KGlobalLedgerView::objectSelected, m_investmentView, static_cast<void (KInvestmentView::*)(const MyMoneyObject&)>(&KInvestmentView::slotSelectAccount));
  }

  showTitleBar(KMyMoneySettings::showTitleBar());

  m_accountsView->refresh();
  m_institutionsView->refresh();
  m_categoriesView->refresh();
  m_payeesView->refresh();
  m_tagsView->refresh();
  m_ledgerView->refresh();
  m_budgetView->refresh();
  m_homeView->refresh();
  m_investmentView->refresh();
  m_reportsView->refresh();
  m_scheduledView->refresh();
  for (auto i = (int)View::Home; i < (int)View::None; ++i)
    if (viewBases.contains(View(i)))
      viewBases[View(i)]->refresh();

  m_payeesView->slotClosePayeeIdentifierSource();
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
    m_reportsView->slotPrintView();
  else if (viewFrames[View::Home] == currentPage())
    m_homeView->slotPrintView();
}

void KMyMoneyView::resetViewSelection(const View)
{
  emit aboutToChangeView();
}

void KMyMoneyView::connectView(const View view)
{
  KMyMoneyAccountTreeView *treeView;
  switch (view) {
    case View::Home:
      disconnect(m_homeView, &KHomeView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(m_homeView, &KHomeView::objectSelected,      this, &KMyMoneyView::slotObjectSelected);
      connect(m_homeView, &KHomeView::openObjectRequested, this, &KMyMoneyView::slotOpenObjectRequested);
      // views can wait since they are going to be refresed in slotRefreshViews
      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_homeView, &KHomeView::refresh);
      break;

    case View::Accounts:
      disconnect(m_accountsView, &KAccountsView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_accountsView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(m_accountsView, &KAccountsView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      connect(Models::instance()->accountsModel(), &AccountsModel::netWorthChanged, m_accountsView, &KAccountsView::slotNetWorthChanged);
      break;

    case View::Schedules:
      disconnect(m_scheduledView, &KScheduledView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(m_scheduledView, &KScheduledView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(m_scheduledView, &KScheduledView::objectSelected,         this, &KMyMoneyView::slotObjectSelected);
      connect(m_scheduledView, &KScheduledView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      break;

    case View::Institutions:
      disconnect(m_institutionsView, &KInstitutionsView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_institutionsView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(m_institutionsView, &KInstitutionsView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      connect(Models::instance()->institutionsModel(), &AccountsModel::netWorthChanged, m_institutionsView, &KInstitutionsView::slotNetWorthChanged);
      break;

    case View::Categories:
      disconnect(m_categoriesView, &KCategoriesView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_categoriesView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::objectSelected,         this, &KMyMoneyView::slotObjectSelected);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(m_categoriesView, &KCategoriesView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      connect(Models::instance()->institutionsModel(), &AccountsModel::profitChanged, m_categoriesView, &KCategoriesView::slotProfitChanged);
      break;

    case View::Tags:
      disconnect(m_tagsView, &KTagsView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(m_tagsView, &KTagsView::transactionSelected,  m_ledgerView, &KGlobalLedgerView::slotLedgerSelected);
      break;

    case View::Payees:
      disconnect(m_payeesView, &KTagsView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(m_payeesView, &KPayeesView::transactionSelected, m_ledgerView, &KGlobalLedgerView::slotLedgerSelected);
      break;

    case View::Ledgers:
      disconnect(m_ledgerView, &KGlobalLedgerView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(m_ledgerView, &KGlobalLedgerView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(m_ledgerView, &KGlobalLedgerView::openPayeeRequested,     this, &KMyMoneyView::slotPayeeSelected);
      connect(m_ledgerView, &KGlobalLedgerView::objectSelected,         this, &KMyMoneyView::slotObjectSelected);
      connect(m_ledgerView, &KGlobalLedgerView::transactionsSelected,   this, &KMyMoneyView::slotTransactionsSelected);
      connect(m_ledgerView, &KGlobalLedgerView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(m_ledgerView, &KGlobalLedgerView::transactionsContextMenuRequested,   this, &KMyMoneyView::slotTransactionsMenuRequested);
      connect(m_ledgerView, &KGlobalLedgerView::statusProgress,   this, &KMyMoneyView::statusProgress);
      connect(m_ledgerView, &KGlobalLedgerView::statusMsg,   this, &KMyMoneyView::statusMsg);

      connect(m_ledgerView, &KGlobalLedgerView::accountReconciled, this, &KMyMoneyView::accountReconciled);
      connect(m_ledgerView, &KGlobalLedgerView::enterOverdueSchedulesRequested, m_scheduledView, &KScheduledView::slotEnterOverdueSchedules);
      connect(m_scheduledView, &KScheduledView::enterOverdueSchedulesFinished,   m_ledgerView, &KGlobalLedgerView::slotContinueReconciliation);
      break;

    case View::Budget:
      disconnect(m_budgetView, &KBudgetView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_budgetView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::openObjectRequested,    this, &KMyMoneyView::slotOpenObjectRequested);
      connect(treeView, &KMyMoneyAccountTreeView::contextMenuRequested,   this, &KMyMoneyView::slotContextMenuRequested);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled,          this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(m_budgetView, &KBudgetView::objectSelected, this, &KMyMoneyView::slotObjectSelected);
      break;

    case View::Investments:
      disconnect(m_investmentView, &KInvestmentView::aboutToShow, this, &KMyMoneyView::connectView);

      connect(m_investmentView, &KInvestmentView::objectSelected,       this, &KMyMoneyView::slotObjectSelected);
      connect(m_investmentView, &KInvestmentView::contextMenuRequested, this, &KMyMoneyView::slotContextMenuRequested);
      break;

    case View::Reports:
      disconnect(m_reportsView, &KReportsView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(m_reportsView, &KReportsView::transactionSelected, m_ledgerView, &KGlobalLedgerView::slotLedgerSelected);
      break;

    case View::OnlineJobOutbox:
      disconnect(m_onlineJobOutboxView, &KOnlineJobOutbox::aboutToShow, this, &KMyMoneyView::connectView);
      break;

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
        m_ledgerView->slotLedgerSelected(acc.id(), QString());

  } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
//    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);
    m_institutionsView->slotEditInstitution();
  } else if (typeid(obj) == typeid(MyMoneySchedule)) {
    m_scheduledView->slotEditSchedule();
  } else if (typeid(obj) == typeid(MyMoneyReport)) {
    const auto& rep = static_cast<const MyMoneyReport&>(obj);
    m_reportsView->slotOpenReport(rep);
  }
}

void KMyMoneyView::slotObjectSelected(const MyMoneyObject& obj)
{
  // carrying some slots over to views isn't easy for all slots...
  // ...so calls to kmymoney still must be here
  if (typeid(obj) == typeid(MyMoneyAccount)) {
    m_investmentView->updateActions(obj);
    m_categoriesView->updateActions(obj);
    m_accountsView->updateActions(obj);
    m_ledgerView->updateActions(obj);
    m_reportsView->updateActions(obj);
    if (m_onlineJobOutboxView) {
      m_onlineJobOutboxView->updateActions(obj);
    }

    // for plugin only
    const auto& acc = static_cast<const MyMoneyAccount&>(obj);
    if (!acc.isIncomeExpense() &&
        !MyMoneyFile::instance()->isStandardAccount(acc.id()))
      emit accountSelected(acc);
  } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
    m_institutionsView->updateActions(obj);
  } else if (typeid(obj) == typeid(MyMoneySchedule)) {
    m_scheduledView->updateActions(obj);
  }
}

void KMyMoneyView::slotContextMenuRequested(const MyMoneyObject& obj)
{
  if (typeid(obj) == typeid(MyMoneyAccount)) {
    const auto& acc = static_cast<const MyMoneyAccount&>(obj);
    if (acc.isInvest()) {
      m_investmentView->slotShowInvestmentMenu(acc);
      return;
    } else if (acc.isIncomeExpense()) {
      m_categoriesView->slotShowCategoriesMenu(acc);
    } else {
      m_accountsView->slotShowAccountMenu(acc);
    }
  } else if (typeid(obj) == typeid(MyMoneyInstitution)) {
    const auto& inst = static_cast<const MyMoneyInstitution&>(obj);
    m_institutionsView->slotShowInstitutionsMenu(inst);
  } else if (typeid(obj) == typeid(MyMoneySchedule)) {
    const auto& sch = static_cast<const MyMoneySchedule&>(obj);
    m_scheduledView->slotShowScheduleMenu(sch);
  }
}

void KMyMoneyView::slotTransactionsMenuRequested(const KMyMoneyRegister::SelectedTransactions& list)
{
  Q_UNUSED(list)
  m_ledgerView->slotShowTransactionMenu(MyMoneySplit());
}

void KMyMoneyView::slotTransactionsSelected(const KMyMoneyRegister::SelectedTransactions& list)
{
  m_ledgerView->updateLedgerActions(list);
  emit transactionsSelected(list); // for plugins
}
