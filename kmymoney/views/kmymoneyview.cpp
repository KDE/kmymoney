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

static constexpr KCompressionDevice::CompressionType const& COMPRESSION_TYPE = KCompressionDevice::GZip;
static constexpr char recoveryKeyId[] = "0xD2B08440";

typedef void(KMyMoneyView::*KMyMoneyViewFunc)();

KMyMoneyView::KMyMoneyView(KMyMoneyApp *kmymoney)
    : KPageWidget(nullptr),
    m_header(0),
    m_inConstructor(true),
    m_fileOpen(false),
    m_fmode(QFileDevice::ReadUser | QFileDevice::WriteUser),
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

  newStorage();
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
  SimpleLedgerView* view = new SimpleLedgerView(kmymoney, this);
  KPageWidgetItem* frame = m_model->addPage(view, i18n("New ledger"));
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
  removeStorage();
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

void KMyMoneyView::newStorage()
{
  removeStorage();
  auto file = MyMoneyFile::instance();
  file->attachStorage(new MyMoneyStorageMgr);
}

void KMyMoneyView::removeStorage()
{
  auto file = MyMoneyFile::instance();
  auto p = file->storage();
  if (p) {
    file->detachStorage(p);
    delete p;
  }
}

void KMyMoneyView::enableViewsIfFileOpen()
{
  // call set enabled only if the state differs to avoid widgets 'bouncing on the screen' while doing this
  for (auto i = (int)View::Home; i < (int)View::None; ++i)
    if (viewFrames.contains(View(i)))
      if (viewFrames[View(i)]->isEnabled() != m_fileOpen)
        viewFrames[View(i)]->setEnabled(m_fileOpen);

  emit viewStateChanged(m_fileOpen);
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

bool KMyMoneyView::fileOpen()
{
  return m_fileOpen;
}

void KMyMoneyView::closeFile()
{
  if (m_reportsView)
    m_reportsView->slotCloseAll();

  // disconnect the signals
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
             Models::instance()->accountsModel(), &AccountsModel::slotObjectAdded);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
             Models::instance()->accountsModel(), &AccountsModel::slotObjectModified);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
             Models::instance()->accountsModel(), &AccountsModel::slotObjectRemoved);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::balanceChanged,
             Models::instance()->accountsModel(), &AccountsModel::slotBalanceOrValueChanged);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::valueChanged,
             Models::instance()->accountsModel(), &AccountsModel::slotBalanceOrValueChanged);

  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
             Models::instance()->institutionsModel(), &InstitutionsModel::slotObjectAdded);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
             Models::instance()->institutionsModel(), &InstitutionsModel::slotObjectModified);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
             Models::instance()->institutionsModel(), &InstitutionsModel::slotObjectRemoved);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::balanceChanged,
             Models::instance()->institutionsModel(), &AccountsModel::slotBalanceOrValueChanged);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::valueChanged,
             Models::instance()->institutionsModel(), &AccountsModel::slotBalanceOrValueChanged);

  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
             Models::instance()->equitiesModel(), &EquitiesModel::slotObjectAdded);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
             Models::instance()->equitiesModel(), &EquitiesModel::slotObjectModified);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
             Models::instance()->equitiesModel(), &EquitiesModel::slotObjectRemoved);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::balanceChanged,
             Models::instance()->equitiesModel(), &EquitiesModel::slotBalanceOrValueChanged);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::valueChanged,
             Models::instance()->equitiesModel(), &EquitiesModel::slotBalanceOrValueChanged);

  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
             Models::instance()->securitiesModel(), &SecuritiesModel::slotObjectAdded);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
             Models::instance()->securitiesModel(), &SecuritiesModel::slotObjectModified);
  disconnect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
             Models::instance()->securitiesModel(), &SecuritiesModel::slotObjectRemoved);

  disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_homeView, &KHomeView::refresh);

  // notify the models that the file is going to be closed (we should have something like dataChanged that reaches the models first)
  Models::instance()->fileClosed();

  emit kmmFilePlugin(preClose);
  if (isDatabase())
    MyMoneyFile::instance()->storage()->close(); // to log off a database user
  newStorage();

  slotShowHomePage();

  emit kmmFilePlugin(postClose);
  m_fileOpen = false;

  emit fileClosed();
}

void KMyMoneyView::ungetString(QIODevice *qfile, char *buf, int len)
{
  buf = &buf[len-1];
  while (len--) {
    qfile->ungetChar(*buf--);
  }
}

bool KMyMoneyView::readFile(const QUrl &url, IMyMoneyOperationsFormat* pExtReader)
{
  QString filename;
  bool downloadedFile = false;
  m_fileOpen = false;
  bool isEncrypted = false;

  IMyMoneyOperationsFormat* pReader = 0;

  if (!url.isValid()) {
    qDebug("Invalid URL '%s'", qPrintable(url.url()));
    return false;
  }

  // disconnect the current storga manager from the engine
  MyMoneyFile::instance()->detachStorage();

  if (url.scheme() == QLatin1String("sql")) { // handle reading of database
    m_fileType = KmmDb;
    // get rid of the mode parameter which is now redundant
    QUrl newUrl(url);
    QUrlQuery query(url);
    query.removeQueryItem("mode");
    newUrl.setQuery(query);
    auto rc = openDatabase(newUrl); // on error, any message will have been displayed
    if (!rc)
      MyMoneyFile::instance()->attachStorage(new MyMoneyStorageMgr);
    return rc;
  }

  auto storage = new MyMoneyStorageMgr;

  if (url.isLocalFile()) {
    filename = url.toLocalFile();
  } else {
    downloadedFile = true;
    KIO::StoredTransferJob *transferjob = KIO::storedGet (url);
    KJobWidgets::setWindow(transferjob, this);
    if (! transferjob->exec()) {
        KMessageBox::detailedError(this,
                                 i18n("Error while loading file '%1'.", url.url()),
                                 transferjob->errorString(),
                                 i18n("File access error"));
        return false;
    }
    QTemporaryFile file;
    file.setAutoRemove(false);
    file.open();
    file.write(transferjob->data());
    filename = file.fileName();
    file.close();
  }

  // let's glimps into the file to figure out, if it's one
  // of the old (uncompressed) or new (compressed) files.
  QFile file(filename);
  QFileInfo info(file);
  if (!info.isFile()) {
    QString msg = i18n("<p><b>%1</b> is not a KMyMoney file.</p>", filename);
    KMessageBox::error(this, msg, i18n("Filetype Error"));
    return false;
  }
  m_fmode = QFileDevice::ReadUser | QFileDevice::WriteUser;
  m_fmode |= info.permissions();

  bool rc = true;

  // There's a problem with the KFilterDev and KGPGFile classes:
  // One supports the at(n) member but not ungetch() together with
  // read() and the other does not provide an at(n) method but
  // supports read() that considers the ungetch() buffer. QFile
  // supports everything so this is not a problem. We solve the problem
  // for now by keeping track of which method can be used.
  bool haveAt = true;

  emit kmmFilePlugin(preOpen);
  if (file.open(QIODevice::ReadOnly)) {
    QByteArray hdr(2, '\0');
    int cnt;
    cnt = file.read(hdr.data(), 2);
    file.close();

    if (cnt == 2) {
      QIODevice* qfile = nullptr;
      if (QString(hdr) == QString("\037\213")) {        // gzipped?
        qfile = new KCompressionDevice(filename, COMPRESSION_TYPE);
      } else if (QString(hdr) == QString("--")          // PGP ASCII armored?
                 || QString(hdr) == QString("\205\001")     // PGP binary?
                 || QString(hdr) == QString("\205\002")) {  // PGP binary?
        if (KGPGFile::GPGAvailable()) {
          qfile = new KGPGFile(filename);
          haveAt = false;
          isEncrypted = true;
        } else {
          KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("GPG is not available for decryption of file <b>%1</b>", filename)));
          qfile = new QFile(file.fileName());
        }
      } else {
        // we can't use file directly, as we delete qfile later on
        qfile = new QFile(file.fileName());
      }

      if (qfile->open(QIODevice::ReadOnly)) {
        try {
          hdr.resize(8);
          if (qfile->read(hdr.data(), 8) == 8) {
            if (haveAt)
              qfile->seek(0);
            else
              ungetString(qfile, hdr.data(), 8);

            // Ok, we got the first block of 8 bytes. Read in the two
            // unsigned long int's by preserving endianess. This is
            // achieved by reading them through a QDataStream object
            qint32 magic0, magic1;
            QDataStream s(&hdr, QIODevice::ReadOnly);
            s >> magic0;
            s >> magic1;

            // If both magic numbers match (we actually read in the
            // text 'KMyMoney' then we assume a binary file and
            // construct a reader for it. Otherwise, we construct
            // an XML reader object.
            //
            // The expression magic0 < 30 is only used to create
            // a binary reader if we assume an old binary file. This
            // should be removed at some point. An alternative is to
            // check the beginning of the file against an pattern
            // of the XML file (e.g. '?<xml' ).
            if ((magic0 == MAGIC_0_50 && magic1 == MAGIC_0_51)
                || magic0 < 30) {
              // we do not support this file format anymore
              pReader = 0;
              m_fileType = KmmBinary;
            } else {
              // Scan the first 70 bytes to see if we find something
              // we know. For now, we support our own XML format and
              // GNUCash XML format. If the file is smaller, then it
              // contains no valid data and we reject it anyway.
              hdr.resize(70);
              if (qfile->read(hdr.data(), 70) == 70) {
                if (haveAt)
                  qfile->seek(0);
                else
                  ungetString(qfile, hdr.data(), 70);
                QRegExp kmyexp("<!DOCTYPE KMYMONEY-FILE>");
                QRegExp gncexp("<gnc-v(\\d+)");
                QByteArray txt(hdr, 70);
                if (kmyexp.indexIn(txt) != -1) {
                  pReader = new MyMoneyStorageXML;
                  m_fileType = KmmXML;
                } else if (gncexp.indexIn(txt) != -1) {
                  if (pExtReader) {
                    MyMoneyFile::instance()->attachStorage(storage);
                    loadAllCurrencies(); // currency list required for gnc
                    MyMoneyFile::instance()->detachStorage(storage);

                    pReader = pExtReader;
                    m_fileType = GncXML;
                  }
                }
              }
            }
            if (pReader) {
              pReader->setProgressCallback(&KMyMoneyView::progressCallback);
              pReader->readFile(qfile, storage);
            } else {
              if (m_fileType == KmmBinary) {
                KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> contains the old binary format used by KMyMoney. Please use an older version of KMyMoney (0.8.x) that still supports this format to convert it to the new XML based format.", filename)));
              } else {
                KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> contains an unknown file format.", filename)));
              }
              rc = false;
            }
          } else {
            KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("Cannot read from file <b>%1</b>.", filename)));
            rc = false;
          }
        } catch (const MyMoneyException &e) {
          KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("Cannot load file <b>%1</b>. Reason: %2", filename, e.what())));
          rc = false;
        }
        if (pReader) {
          pReader->setProgressCallback(0);
          delete pReader;
        }
        qfile->close();
      } else {
        KGPGFile *gpgFile = qobject_cast<KGPGFile *>(qfile);
        if (gpgFile && !gpgFile->errorToString().isEmpty()) {
          KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("The following error was encountered while decrypting file <b>%1</b>: %2", filename, gpgFile->errorToString())));
        } else {
          KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> not found.", filename)));
        }
        rc = false;
      }
      delete qfile;
    }
  } else {
    KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> not found.", filename)));
    rc = false;
  }

  // things are finished, now we connect the storage to the engine
  // which forces a reload of the cache in the engine with those
  // objects that are cached
  MyMoneyFile::instance()->attachStorage(storage);

  if (rc == false)
    return rc;

  // encapsulate transactions to the engine to be able to commit/rollback
  MyMoneyFileTransaction ft;

  // make sure we setup the encryption key correctly
  if (isEncrypted && MyMoneyFile::instance()->value("kmm-encryption-key").isEmpty()) {
    MyMoneyFile::instance()->setValue("kmm-encryption-key", KMyMoneySettings::gpgRecipientList().join(","));
  }

  // make sure we setup the name of the base accounts in translated form
  try {
    MyMoneyFile *file = MyMoneyFile::instance();
    checkAccountName(file->asset(), i18n("Asset"));
    checkAccountName(file->liability(), i18n("Liability"));
    checkAccountName(file->income(), i18n("Income"));
    checkAccountName(file->expense(), i18n("Expense"));
    checkAccountName(file->equity(), i18n("Equity"));
    ft.commit();
  } catch (const MyMoneyException &) {
  }

  // if a temporary file was downloaded, then it will be removed
  // with the next call. Otherwise, it stays untouched on the local
  // filesystem.
  if (downloadedFile) {
    QFile::remove(filename);
  }

  return initializeStorage();
}

void KMyMoneyView::checkAccountName(const MyMoneyAccount& _acc, const QString& name) const
{
  auto file = MyMoneyFile::instance();
  if (_acc.name() != name) {
    MyMoneyAccount acc(_acc);
    acc.setName(name);
    file->modifyAccount(acc);
  }
}

bool KMyMoneyView::openDatabase(const QUrl &url)
{
  m_fileOpen = false;

  // open the database
  auto pStorage = MyMoneyFile::instance()->storage();
  if (!pStorage)
    pStorage = new MyMoneyStorageMgr;

  auto rc = false;
  auto pluginFound = false;
  if (m_storagePlugins) {
    for (const auto& plugin : *m_storagePlugins) {
      if (plugin->formatName().compare(QLatin1String("SQL")) == 0) {
        rc = plugin->open(pStorage, url);
        pluginFound = true;
        break;
      }
    }
  }

  if(!pluginFound)
    KMessageBox::error(this, i18n("Couldn't find suitable plugin to read your storage."));

  if(!rc) {
    removeStorage();
    delete pStorage;
    return false;
  }

  if (pStorage) {
    removeStorage();
    MyMoneyFile::instance()->attachStorage(pStorage);
  }

  m_fileOpen = true;
  return initializeStorage();
}

bool KMyMoneyView::initializeStorage()
{
  bool blocked = MyMoneyFile::instance()->signalsBlocked();
  MyMoneyFile::instance()->blockSignals(true);

  // we check, if we have any currency in the file. If not, we load
  // all the default currencies we know.
  MyMoneyFileTransaction ft;
  try {
    updateCurrencyNames();
    ft.commit();
  } catch (const MyMoneyException &) {
    MyMoneyFile::instance()->blockSignals(blocked);
    return false;
  }

  // make sure, we have a base currency and all accounts are
  // also assigned to a currency.
  QString baseId;
  try {
    baseId = MyMoneyFile::instance()->baseCurrency().id();
  } catch (const MyMoneyException &e) {
    qDebug() << e.what();
  }

  if (baseId.isEmpty()) {
    // Stay in this endless loop until we have a base currency,
    // as without it the application does not work anymore.
    while (baseId.isEmpty()) {
      selectBaseCurrency();
      try {
        baseId = MyMoneyFile::instance()->baseCurrency().id();
      } catch (const MyMoneyException &e) {
        qDebug() << e.what();
      }
    }
  } else {
    // in some odd intermediate cases there could be files out there
    // that have a base currency set, but still have accounts that
    // do not have a base currency assigned. This call will take
    // care of it. We can safely remove it later.
    //
    // Another work-around for this scenario is to remove the base
    // currency setting from the XML file by removing the line
    //
    //   <PAIR key="kmm-baseCurrency" value="xxx" />
    //
    // and restart the application with this file. This will force to
    // run the above loop.
    selectBaseCurrency();
  }

  // setup the standard precision
  AmountEdit::setStandardPrecision(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));
  KMyMoneyEdit::setStandardPrecision(MyMoneyMoney::denomToPrec(MyMoneyFile::instance()->baseCurrency().smallestAccountFraction()));

  KSharedConfigPtr config = KSharedConfig::openConfig();
  KPageWidgetItem* page;
  KConfigGroup grp = config->group("General Options");

  if (KMyMoneySettings::startLastViewSelected() != 0)
    page = viewFrames.value(static_cast<View>(KMyMoneySettings::lastViewSelected()));
  else
    page = viewFrames[View::Home];

  // For debugging purposes, we can turn off the automatic fix manually
  // by setting the entry in kmymoneyrc to true
  grp = config->group("General Options");
  if (grp.readEntry("SkipFix", false) != true) {
    MyMoneyFileTransaction ft;
    try {
      // Check if we have to modify the file before we allow to work with it
      auto s = MyMoneyFile::instance()->storage();
      while (s->fileFixVersion() < s->currentFixVersion()) {
        qDebug("%s", qPrintable((QString("testing fileFixVersion %1 < %2").arg(s->fileFixVersion()).arg(s->currentFixVersion()))));
        switch (s->fileFixVersion()) {
          case 0:
            fixFile_0();
            s->setFileFixVersion(1);
            break;

          case 1:
            fixFile_1();
            s->setFileFixVersion(2);
            break;

          case 2:
            fixFile_2();
            s->setFileFixVersion(3);
            break;

          case 3:
            fixFile_3();
            s->setFileFixVersion(4);
            break;

            // add new levels above. Don't forget to increase currentFixVersion() for all
            // the storage backends this fix applies to
          default:
            throw MYMONEYEXCEPTION(i18n("Unknown fix level in input file"));
        }
      }
      ft.commit();
    } catch (const MyMoneyException &) {
      MyMoneyFile::instance()->blockSignals(blocked);
      return false;
    }
  } else {
    qDebug("Skipping automatic transaction fix!");
  }
  MyMoneyFile::instance()->blockSignals(blocked);

  // FIXME: we need to check, if it's necessary to have this
  //        automatic funcitonality
  // if there's no asset account, then automatically start the
  // new account wizard
  // kmymoney->createInitialAccount();

  m_fileOpen = true;
  emit kmmFilePlugin(postOpen);

  Models::instance()->fileOpened();

  // connect the needed signals
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
          Models::instance()->accountsModel(), &AccountsModel::slotObjectAdded);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
          Models::instance()->accountsModel(), &AccountsModel::slotObjectModified);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
          Models::instance()->accountsModel(), &AccountsModel::slotObjectRemoved);
  connect(MyMoneyFile::instance(), &MyMoneyFile::balanceChanged,
          Models::instance()->accountsModel(), &AccountsModel::slotBalanceOrValueChanged);
  connect(MyMoneyFile::instance(), &MyMoneyFile::valueChanged,
          Models::instance()->accountsModel(), &AccountsModel::slotBalanceOrValueChanged);

  connect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
          Models::instance()->institutionsModel(), &InstitutionsModel::slotObjectAdded);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
          Models::instance()->institutionsModel(), &InstitutionsModel::slotObjectModified);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
          Models::instance()->institutionsModel(), &InstitutionsModel::slotObjectRemoved);
  connect(MyMoneyFile::instance(), &MyMoneyFile::balanceChanged,
          Models::instance()->institutionsModel(), &AccountsModel::slotBalanceOrValueChanged);
  connect(MyMoneyFile::instance(), &MyMoneyFile::valueChanged,
          Models::instance()->institutionsModel(), &AccountsModel::slotBalanceOrValueChanged);

  connect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
          Models::instance()->equitiesModel(), &EquitiesModel::slotObjectAdded);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
          Models::instance()->equitiesModel(), &EquitiesModel::slotObjectModified);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
          Models::instance()->equitiesModel(), &EquitiesModel::slotObjectRemoved);
  connect(MyMoneyFile::instance(), &MyMoneyFile::balanceChanged,
             Models::instance()->equitiesModel(), &EquitiesModel::slotBalanceOrValueChanged);
  connect(MyMoneyFile::instance(), &MyMoneyFile::valueChanged,
             Models::instance()->equitiesModel(), &EquitiesModel::slotBalanceOrValueChanged);

  connect(MyMoneyFile::instance(), &MyMoneyFile::objectAdded,
             Models::instance()->securitiesModel(), &SecuritiesModel::slotObjectAdded);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectModified,
             Models::instance()->securitiesModel(), &SecuritiesModel::slotObjectModified);
  connect(MyMoneyFile::instance(), &MyMoneyFile::objectRemoved,
             Models::instance()->securitiesModel(), &SecuritiesModel::slotObjectRemoved);

  // inform everyone about new data
  MyMoneyFile::instance()->preloadCache();
  MyMoneyFile::instance()->forceDataChanged();

  // views can wait since they are going to be refresed in slotRefreshViews
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_homeView, &KHomeView::refresh);

  // if we currently see a different page, then select the right one
  if (page != currentPage()) {
    showPage(page);
  }

  emit fileOpened();
  return true;
}

void KMyMoneyView::saveToLocalFile(const QString& localFile, IMyMoneyOperationsFormat* pWriter, bool plaintext, const QString& keyList)
{
  // Check GPG encryption
  bool encryptFile = true;
  bool encryptRecover = false;
  if (!keyList.isEmpty()) {
    if (!KGPGFile::GPGAvailable()) {
      KMessageBox::sorry(this, i18n("GPG does not seem to be installed on your system. Please make sure that GPG can be found using the standard search path. This time, encryption is disabled."), i18n("GPG not found"));
      encryptFile = false;
    } else {
      if (KMyMoneySettings::encryptRecover()) {
        encryptRecover = true;
        if (!KGPGFile::keyAvailable(QString(recoveryKeyId))) {
          KMessageBox::sorry(this, i18n("<p>You have selected to encrypt your data also with the KMyMoney recover key, but the key with id</p><p><center><b>%1</b></center></p><p>has not been found in your keyring at this time. Please make sure to import this key into your keyring. You can find it on the <a href=\"https://kmymoney.org/\">KMyMoney web-site</a>. This time your data will not be encrypted with the KMyMoney recover key.</p>", QString(recoveryKeyId)), i18n("GPG Key not found"));
          encryptRecover = false;
        }
      }

      for(const QString& key: keyList.split(',', QString::SkipEmptyParts)) {
        if (!KGPGFile::keyAvailable(key)) {
          KMessageBox::sorry(this, i18n("<p>You have specified to encrypt your data for the user-id</p><p><center><b>%1</b>.</center></p><p>Unfortunately, a valid key for this user-id was not found in your keyring. Please make sure to import a valid key for this user-id. This time, encryption is disabled.</p>", key), i18n("GPG Key not found"));
          encryptFile = false;
          break;
        }
      }

      if (encryptFile == true) {
        QString msg = i18n("<p>You have configured to save your data in encrypted form using GPG. Make sure you understand that you might lose all your data if you encrypt it, but cannot decrypt it later on. If unsure, answer <b>No</b>.</p>");
        if (KMessageBox::questionYesNo(this, msg, i18n("Store GPG encrypted"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "StoreEncrypted") == KMessageBox::No) {
          encryptFile = false;
        }
      }
    }
  }


  // Create a temporary file if needed
  QString writeFile = localFile;
  QTemporaryFile tmpFile;
  if (QFile::exists(localFile)) {
    tmpFile.open();
    writeFile = tmpFile.fileName();
    tmpFile.close();
  }

  /**
   * @brief Automatically restore settings when scope is left
   */
  struct restorePreviousSettingsHelper {
    restorePreviousSettingsHelper()
      : m_signalsWereBlocked{MyMoneyFile::instance()->signalsBlocked()}
    {
      MyMoneyFile::instance()->blockSignals(true);
    }

    ~restorePreviousSettingsHelper()
    {
      MyMoneyFile::instance()->blockSignals(m_signalsWereBlocked);
    }
    const bool m_signalsWereBlocked;
  } restoreHelper;

  MyMoneyFileTransaction ft;
  MyMoneyFile::instance()->deletePair("kmm-encryption-key");
  std::unique_ptr<QIODevice> device;

  if (!keyList.isEmpty() && encryptFile && !plaintext) {
    std::unique_ptr<KGPGFile> kgpg = std::unique_ptr<KGPGFile>(new KGPGFile{writeFile});
    if (kgpg) {
      for(const QString& key: keyList.split(',', QString::SkipEmptyParts)) {
        kgpg->addRecipient(key.toLatin1());
      }

      if (encryptRecover) {
        kgpg->addRecipient(recoveryKeyId);
      }
      MyMoneyFile::instance()->setValue("kmm-encryption-key", keyList);
      device = std::unique_ptr<decltype(device)::element_type>(kgpg.release());
    }
  } else {
    QFile *file = new QFile(writeFile);
    // The second parameter of KCompressionDevice means that KCompressionDevice will delete the QFile object
    device = std::unique_ptr<decltype(device)::element_type>(new KCompressionDevice{file, true, (plaintext) ? KCompressionDevice::None : COMPRESSION_TYPE});
  }

  ft.commit();

  if (!device || !device->open(QIODevice::WriteOnly)) {
    throw MYMONEYEXCEPTION(i18n("Unable to open file '%1' for writing.", localFile));
  }

  pWriter->setProgressCallback(&KMyMoneyView::progressCallback);
  pWriter->writeFile(device.get(), MyMoneyFile::instance()->storage());
  device->close();

  // Check for errors if possible, only possible for KGPGFile
  QFileDevice *fileDevice = qobject_cast<QFileDevice*>(device.get());
  if (fileDevice && fileDevice->error() != QFileDevice::NoError) {
    throw MYMONEYEXCEPTION(i18n("Failure while writing to '%1'", localFile));
  }

  if (writeFile != localFile) {
    // This simple comparison is possible because the strings are equal if no temporary file was created.
    // If a temporary file was created, it is made in a way that the name is definitely different. So no
    // symlinks etc. have to be evaluated.
    if (!QFile::remove(localFile) || !QFile::rename(writeFile, localFile))
      throw MYMONEYEXCEPTION(i18n("Failure while writing to '%1'", localFile));
  }
  QFile::setPermissions(localFile, m_fmode);
  pWriter->setProgressCallback(0);
}

bool KMyMoneyView::saveFile(const QUrl &url, const QString& keyList)
{
  QString filename = url.path();

  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it has not been opened"));
    return false;
  }

  emit kmmFilePlugin(preSave);
  std::unique_ptr<IMyMoneyOperationsFormat> storageWriter;

  // If this file ends in ".ANON.XML" then this should be written using the
  // anonymous writer.
  bool plaintext = filename.right(4).toLower() == ".xml";
  if (filename.right(9).toLower() == ".anon.xml") {
    //! @todo C++14: use std::make_unique, also some lines below
    storageWriter = std::unique_ptr<IMyMoneyOperationsFormat>(new MyMoneyStorageANON);
  } else {
    storageWriter = std::unique_ptr<IMyMoneyOperationsFormat>(new MyMoneyStorageXML);
  }

  // actually, url should be the parameter to this function
  // but for now, this would involve too many changes
  bool rc = true;
  try {
    if (! url.isValid()) {
      throw MYMONEYEXCEPTION(i18n("Malformed URL '%1'", url.url()));
    }

    if (url.isLocalFile()) {
      filename = url.toLocalFile();
      try {
        const unsigned int nbak = KMyMoneySettings::autoBackupCopies();
        if (nbak) {
          KBackup::numberedBackupFile(filename, QString(), QString::fromLatin1("~"), nbak);
        }
        saveToLocalFile(filename, storageWriter.get(), plaintext, keyList);
      } catch (const MyMoneyException &) {
        throw MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'", filename));
      }
    } else {
      QTemporaryFile tmpfile;
      tmpfile.open(); // to obtain the name
      tmpfile.close();
      saveToLocalFile(tmpfile.fileName(), storageWriter.get(), plaintext, keyList);

      Q_CONSTEXPR int permission = -1;
      QFile file(tmpfile.fileName());
      file.open(QIODevice::ReadOnly);
      KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
      if (!putjob->exec()) {
        throw MYMONEYEXCEPTION(i18n("Unable to upload to '%1'.<br />%2", url.toDisplayString(), putjob->errorString()));
      }
      file.close();
    }
    m_fileType = KmmXML;
  } catch (const MyMoneyException &e) {
    KMessageBox::error(this, e.what());
    MyMoneyFile::instance()->setDirty();
    rc = false;
  }
  emit kmmFilePlugin(postSave);
  return rc;
}

bool KMyMoneyView::dirty()
{
  if (!fileOpen())
    return false;

  return MyMoneyFile::instance()->dirty();
}

void KMyMoneyView::finishReconciliation(const MyMoneyAccount& /* account */)
{
  Models::instance()->accountsModel()->slotReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
  m_ledgerView->slotSetReconcileAccount(MyMoneyAccount(), QDate(), MyMoneyMoney());
}

void KMyMoneyView::newFile()
{
  closeFile();
  m_fileType = KmmXML; // assume native type until saved
  m_fileOpen = true;
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

void KMyMoneyView::selectBaseCurrency()
{
  auto file = MyMoneyFile::instance();

  // check if we have a base currency. If not, we need to select one
  QString baseId;
  try {
    baseId = MyMoneyFile::instance()->baseCurrency().id();
  } catch (const MyMoneyException &e) {
    qDebug("%s", qPrintable(e.what()));
  }

  if (baseId.isEmpty()) {
    QPointer<KCurrencyEditDlg> dlg = new KCurrencyEditDlg(this);
    connect(dlg, SIGNAL(selectBaseCurrency(MyMoneySecurity)), this, SLOT(slotSetBaseCurrency(MyMoneySecurity)));
    dlg->exec();
    delete dlg;
  }

  try {
    baseId = MyMoneyFile::instance()->baseCurrency().id();
  } catch (const MyMoneyException &e) {
    qDebug("%s", qPrintable(e.what()));
  }

  if (!baseId.isEmpty()) {
    // check that all accounts have a currency
    QList<MyMoneyAccount> list;
    file->accountList(list);
    QList<MyMoneyAccount>::Iterator it;

    // don't forget those standard accounts
    list << file->asset();
    list << file->liability();
    list << file->income();
    list << file->expense();
    list << file->equity();


    for (it = list.begin(); it != list.end(); ++it) {
      QString cid;
      try {
        if (!(*it).currencyId().isEmpty() || (*it).currencyId().length() != 0)
          cid = MyMoneyFile::instance()->currency((*it).currencyId()).id();
      } catch (const MyMoneyException &) {
      }

      if (cid.isEmpty()) {
        (*it).setCurrencyId(baseId);
        MyMoneyFileTransaction ft;
        try {
          file->modifyAccount(*it);
          ft.commit();
        } catch (const MyMoneyException &e) {
          qDebug("Unable to setup base currency in account %s (%s): %s", qPrintable((*it).name()), qPrintable((*it).id()), qPrintable(e.what()));
        }
      }
    }
  }
}

void KMyMoneyView::updateCurrencyNames()
{
  auto file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;

  QList<MyMoneySecurity> storedCurrencies = MyMoneyFile::instance()->currencyList();
  QList<MyMoneySecurity> availableCurrencies = MyMoneyFile::instance()->availableCurrencyList();
  QStringList currencyIDs;

  foreach (auto currency, availableCurrencies)
    currencyIDs.append(currency.id());

  try {
    foreach (auto currency, storedCurrencies) {
      int i = currencyIDs.indexOf(currency.id());
      if (i != -1 && availableCurrencies.at(i).name() != currency.name()) {
        currency.setName(availableCurrencies.at(i).name());
        file->modifyCurrency(currency);
      }
    }
    ft.commit();
  } catch (const MyMoneyException &e) {
    qDebug("Error %s updating currency names", qPrintable(e.what()));
  }
}

void KMyMoneyView::loadAllCurrencies()
{
  auto file = MyMoneyFile::instance();
  MyMoneyFileTransaction ft;
  if (!file->currencyList().isEmpty())
    return;
  QMap<MyMoneySecurity, MyMoneyPrice> ancientCurrencies = file->ancientCurrencies();
  try {
  foreach (auto currency, file->availableCurrencyList()) {
    file->addCurrency(currency);
    MyMoneyPrice price = ancientCurrencies.value(currency, MyMoneyPrice());
    if (price != MyMoneyPrice())
      file->addPrice(price);
  }
  ft.commit();
  } catch (const MyMoneyException &e) {
    qDebug("Error %s loading currency", qPrintable(e.what()));
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


void KMyMoneyView::progressCallback(int current, int total, const QString& msg)
{
  kmymoney->progressCallback(current, total, msg);
}

void KMyMoneyView::slotCurrentPageChanged(const QModelIndex current, const QModelIndex)
{
  // remember the current page
  m_lastViewSelected = current.row();
  // set the current page's title in the header
  if (m_header)
    m_header->setText(m_model->data(current, KPageModel::HeaderRole).toString());
}

/* DO NOT ADD code to this function or any of it's called ones.
   Instead, create a new function, fixFile_n, and modify the initializeStorage()
   logic above to call it */

void KMyMoneyView::fixFile_3()
{
  // make sure each storage object contains a (unique) id
  MyMoneyFile::instance()->storageId();
}

void KMyMoneyView::fixFile_2()
{
  auto file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  QList<MyMoneyTransaction> transactionList;
  file->transactionList(transactionList, filter);

  // scan the transactions and modify transactions with two splits
  // which reference an account and a category to have the memo text
  // of the account.
  auto count = 0;
  foreach (const auto transaction, transactionList) {
    if (transaction.splitCount() == 2) {
      QString accountId;
      QString categoryId;
      QString accountMemo;
      QString categoryMemo;
      foreach (const auto split, transaction.splits()) {
        auto acc = file->account(split.accountId());
        if (acc.isIncomeExpense()) {
          categoryId = split.id();
          categoryMemo = split.memo();
        } else {
          accountId = split.id();
          accountMemo = split.memo();
        }
      }

      if (!accountId.isEmpty() && !categoryId.isEmpty()
          && accountMemo != categoryMemo) {
        MyMoneyTransaction t(transaction);
        MyMoneySplit s(t.splitById(categoryId));
        s.setMemo(accountMemo);
        t.modifySplit(s);
        file->modifyTransaction(t);
        ++count;
      }
    }
  }
  qDebug("%d transactions fixed in fixFile_2", count);
}

void KMyMoneyView::fixFile_1()
{
  // we need to fix reports. If the account filter list contains
  // investment accounts, we need to add the stock accounts to the list
  // as well if we don't have the expert mode enabled
  if (!KMyMoneySettings::expertMode()) {
    try {
      QList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();
      QList<MyMoneyReport>::iterator it_r;
      for (it_r = reports.begin(); it_r != reports.end(); ++it_r) {
        QStringList list;
        (*it_r).accounts(list);
        QStringList missing;
        QStringList::const_iterator it_a, it_b;
        for (it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
          auto acc = MyMoneyFile::instance()->account(*it_a);
          if (acc.accountType() == Account::Type::Investment) {
            foreach (const auto accountID, acc.accountList()) {
              if (!list.contains(accountID)) {
                missing.append(accountID);
              }
            }
          }
        }
        if (!missing.isEmpty()) {
          (*it_r).addAccount(missing);
          MyMoneyFile::instance()->modifyReport(*it_r);
        }
      }
    } catch (const MyMoneyException &) {
    }
  }
}

#if 0
if (!m_accountsView->allItemsSelected())
{
  // retrieve a list of selected accounts
  QStringList list;
  m_accountsView->selectedItems(list);

  // if we're not in expert mode, we need to make sure
  // that all stock accounts for the selected investment
  // account are also selected
  if (!KMyMoneySettings::expertMode()) {
    QStringList missing;
    QStringList::const_iterator it_a, it_b;
    for (it_a = list.begin(); it_a != list.end(); ++it_a) {
      auto acc = MyMoneyFile::instance()->account(*it_a);
      if (acc.accountType() == Account::Type::Investment) {
        foreach (const auto accountID, acc.accountList()) {
          if (!list.contains(accountID)) {
            missing.append(accountID);
          }
        }
      }
    }
    list += missing;
  }

  m_filter.addAccount(list);
}

#endif





void KMyMoneyView::fixFile_0()
{
  /* (Ace) I am on a crusade against file fixups.  Whenever we have to fix the
   * file, it is really a warning.  So I'm going to print a debug warning, and
   * then go track them down when I see them to figure out how they got saved
   * out needing fixing anyway.
   */

  auto file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accountList;
  file->accountList(accountList);
  QList<MyMoneyAccount>::Iterator it_a;
  QList<MyMoneySchedule> scheduleList = file->scheduleList();
  QList<MyMoneySchedule>::Iterator it_s;

  MyMoneyAccount equity = file->equity();
  MyMoneyAccount asset = file->asset();
  bool equityListEmpty = equity.accountList().count() == 0;

  for (it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    if ((*it_a).accountType() == Account::Type::Loan
        || (*it_a).accountType() == Account::Type::AssetLoan) {
      fixLoanAccount_0(*it_a);
    }
    // until early before 0.8 release, the equity account was not saved to
    // the file. If we have an equity account with no sub-accounts but
    // find and equity account that has equity() as it's parent, we reparent
    // this account. Need to move it to asset() first, because otherwise
    // MyMoneyFile::reparent would act as NOP.
    if (equityListEmpty && (*it_a).accountType() == Account::Type::Equity) {
      if ((*it_a).parentAccountId() == equity.id()) {
        auto acc = *it_a;
        // tricky, force parent account to be empty so that we really
        // can re-parent it
        acc.setParentAccountId(QString());
        file->reparentAccount(acc, equity);
        qDebug() << Q_FUNC_INFO << " fixed account " << acc.id() << " reparented to " << equity.id();
      }
    }
  }

  for (it_s = scheduleList.begin(); it_s != scheduleList.end(); ++it_s) {
    fixSchedule_0(*it_s);
  }

  fixTransactions_0();
}

void KMyMoneyView::fixSchedule_0(MyMoneySchedule sched)
{
  MyMoneyTransaction t = sched.transaction();
  QList<MyMoneySplit> splitList = t.splits();
  QList<MyMoneySplit>::ConstIterator it_s;
  bool updated = false;

  try {
    // Check if the splits contain valid data and set it to
    // be valid.
    for (it_s = splitList.constBegin(); it_s != splitList.constEnd(); ++it_s) {
      // the first split is always the account on which this transaction operates
      // and if the transaction commodity is not set, we take this
      if (it_s == splitList.constBegin() && t.commodity().isEmpty()) {
        qDebug() << Q_FUNC_INFO << " " << t.id() << " has no commodity";
        try {
          auto acc = MyMoneyFile::instance()->account((*it_s).accountId());
          t.setCommodity(acc.currencyId());
          updated = true;
        } catch (const MyMoneyException &) {
        }
      }
      // make sure the account exists. If not, remove the split
      try {
        MyMoneyFile::instance()->account((*it_s).accountId());
      } catch (const MyMoneyException &) {
        qDebug() << Q_FUNC_INFO << " " << sched.id() << " " << (*it_s).id() << " removed, because account '" << (*it_s).accountId() << "' does not exist.";
        t.removeSplit(*it_s);
        updated = true;
      }
      if ((*it_s).reconcileFlag() != eMyMoney::Split::State::NotReconciled) {
        qDebug() << Q_FUNC_INFO << " " << sched.id() << " " << (*it_s).id() << " should be 'not reconciled'";
        MyMoneySplit split = *it_s;
        split.setReconcileDate(QDate());
        split.setReconcileFlag(eMyMoney::Split::State::NotReconciled);
        t.modifySplit(split);
        updated = true;
      }
      // the schedule logic used to operate only on the value field.
      // This is now obsolete.
      if ((*it_s).shares().isZero() && !(*it_s).value().isZero()) {
        MyMoneySplit split = *it_s;
        split.setShares(split.value());
        t.modifySplit(split);
        updated = true;
      }
    }

    // If there have been changes, update the schedule and
    // the engine data.
    if (updated) {
      sched.setTransaction(t);
      MyMoneyFile::instance()->modifySchedule(sched);
    }
  } catch (const MyMoneyException &e) {
    qWarning("Unable to update broken schedule: %s", qPrintable(e.what()));
  }
}

void KMyMoneyView::fixLoanAccount_0(MyMoneyAccount acc)
{
  if (acc.value("final-payment").isEmpty()
      || acc.value("term").isEmpty()
      || acc.value("periodic-payment").isEmpty()
      || acc.value("loan-amount").isEmpty()
      || acc.value("interest-calculation").isEmpty()
      || acc.value("schedule").isEmpty()
      || acc.value("fixed-interest").isEmpty()) {
    KMessageBox::information(this,
                             i18n("<p>The account \"%1\" was previously created as loan account but some information is missing.</p><p>The new loan wizard will be started to collect all relevant information.</p><p>Please use KMyMoney version 0.8.7 or later and earlier than version 0.9 to correct the problem.</p>"
                                  , acc.name()),
                             i18n("Account problem"));

    throw MYMONEYEXCEPTION("Fix LoanAccount0 not supported anymore");
  }
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

void KMyMoneyView::fixTransactions_0()
{
  auto file = MyMoneyFile::instance();

  QList<MyMoneySchedule> scheduleList = file->scheduleList();
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits(false);
  QList<MyMoneyTransaction> transactionList;
  file->transactionList(transactionList, filter);

  QList<MyMoneySchedule>::Iterator it_x;
  QStringList interestAccounts;

  KMSTATUS(i18n("Fix transactions"));
  kmymoney->slotStatusProgressBar(0, scheduleList.count() + transactionList.count());

  int cnt = 0;
  // scan the schedules to find interest accounts
  for (it_x = scheduleList.begin(); it_x != scheduleList.end(); ++it_x) {
    MyMoneyTransaction t = (*it_x).transaction();
    QList<MyMoneySplit>::ConstIterator it_s;
    QStringList accounts;
    bool hasDuplicateAccounts = false;

    foreach (const auto split, t.splits()) {
      if (accounts.contains(split.accountId())) {
        hasDuplicateAccounts = true;
        qDebug() << Q_FUNC_INFO << " " << t.id() << " has multiple splits with account " << split.accountId();
      } else {
        accounts << split.accountId();
      }

      if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
        if (interestAccounts.contains(split.accountId()) == 0) {
          interestAccounts << split.accountId();
        }
      }
    }
    if (hasDuplicateAccounts) {
      fixDuplicateAccounts_0(t);
    }
    ++cnt;
    if (!(cnt % 10))
      kmymoney->slotStatusProgressBar(cnt);
  }

  // scan the transactions and modify loan transactions
  for (auto& transaction : transactionList) {
    QString defaultAction;
    QList<MyMoneySplit> splits = transaction.splits();
    QStringList accounts;

    // check if base commodity is set. if not, set baseCurrency
    if (transaction.commodity().isEmpty()) {
      qDebug() << Q_FUNC_INFO << " " << transaction.id() << " has no base currency";
      transaction.setCommodity(file->baseCurrency().id());
      file->modifyTransaction(transaction);
    }

    bool isLoan = false;
    // Determine default action
    if (transaction.splitCount() == 2) {
      // check for transfer
      int accountCount = 0;
      MyMoneyMoney val;
      foreach (const auto split, splits) {
        auto acc = file->account(split.accountId());
        if (acc.accountGroup() == Account::Type::Asset
            || acc.accountGroup() == Account::Type::Liability) {
          val = split.value();
          accountCount++;
          if (acc.accountType() == Account::Type::Loan
              || acc.accountType() == Account::Type::AssetLoan)
            isLoan = true;
        } else
          break;
      }
      if (accountCount == 2) {
        if (isLoan)
          defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Amortization);
        else
          defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer);
      } else {
        if (val.isNegative())
          defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal);
        else
          defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit);
      }
    }

    isLoan = false;
    foreach (const auto split, splits) {
      auto acc = file->account(split.accountId());
      MyMoneyMoney val = split.value();
      if (acc.accountGroup() == Account::Type::Asset
          || acc.accountGroup() == Account::Type::Liability) {
        if (!val.isPositive()) {
          defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal);
          break;
        } else {
          defaultAction = MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit);
          break;
        }
      }
    }

#if 0
    // Check for correct actions in transactions referencing credit cards
    bool needModify = false;
    // The action fields are actually not used anymore in the ledger view logic
    // so we might as well skip this whole thing here!
    for (it_s = splits.begin(); needModify == false && it_s != splits.end(); ++it_s) {
      auto acc = file->account((*it_s).accountId());
      MyMoneyMoney val = (*it_s).value();
      if (acc.accountType() == Account::Type::CreditCard) {
        if (val < 0 && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Withdrawal) && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer))
          needModify = true;
        if (val >= 0 && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Deposit) && (*it_s).action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Transfer))
          needModify = true;
      }
    }

    // (Ace) Extended the #endif down to cover this conditional, because as-written
    // it will ALWAYS be skipped.

    if (needModify == true) {
      for (it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        (*it_s).setAction(defaultAction);
        transaction.modifySplit(*it_s);
        file->modifyTransaction(transaction);
      }
      splits = transaction.splits();    // update local copy
      qDebug("Fixed credit card assignment in %s", transaction.id().data());
    }
#endif

    // Check for correct assignment of ActionInterest in all splits
    // and check if there are any duplicates in this transactions
    for (auto& split : splits) {
      MyMoneyAccount splitAccount = file->account(split.accountId());
      if (!accounts.contains(split.accountId())) {
        accounts << split.accountId();
      }
      // if this split references an interest account, the action
      // must be of type ActionInterest
      if (interestAccounts.contains(split.accountId())) {
        if (split.action() != MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
          qDebug() << Q_FUNC_INFO << " " << transaction.id() << " contains an interest account (" << split.accountId() << ") but does not have ActionInterest";
          split.setAction(MyMoneySplit::actionName(eMyMoney::Split::Action::Interest));
          transaction.modifySplit(split);
          file->modifyTransaction(transaction);
          qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
        }
        // if it does not reference an interest account, it must not be
        // of type ActionInterest
      } else {
        if (split.action() == MyMoneySplit::actionName(eMyMoney::Split::Action::Interest)) {
          qDebug() << Q_FUNC_INFO << " " << transaction.id() << " does not contain an interest account so it should not have ActionInterest";
          split.setAction(defaultAction);
          transaction.modifySplit(split);
          file->modifyTransaction(transaction);
          qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
        }
      }

      // check that for splits referencing an account that has
      // the same currency as the transactions commodity the value
      // and shares field are the same.
      if (transaction.commodity() == splitAccount.currencyId()
          && split.value() != split.shares()) {
        qDebug() << Q_FUNC_INFO << " " << transaction.id() << " " << split.id() << " uses the transaction currency, but shares != value";
        split.setShares(split.value());
        transaction.modifySplit(split);
        file->modifyTransaction(transaction);
      }

      // fix the shares and values to have the correct fraction
      if (!splitAccount.isInvest()) {
        try {
          int fract = splitAccount.fraction();
          if (split.shares() != split.shares().convert(fract)) {
            qDebug("adjusting fraction in %s,%s", qPrintable(transaction.id()), qPrintable(split.id()));
            split.setShares(split.shares().convert(fract));
            split.setValue(split.value().convert(fract));
            transaction.modifySplit(split);
            file->modifyTransaction(transaction);
          }
        } catch (const MyMoneyException &) {
          qDebug("Missing security '%s', split not altered", qPrintable(splitAccount.currencyId()));
        }
      }
    }

    ++cnt;
    if (!(cnt % 10))
      kmymoney->slotStatusProgressBar(cnt);
  }

  kmymoney->slotStatusProgressBar(-1, -1);
}

void KMyMoneyView::fixDuplicateAccounts_0(MyMoneyTransaction& t)
{
  qDebug("Duplicate account in transaction %s", qPrintable(t.id()));
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
      connect(m_investmentView, &KInvestmentView::accountSelected, kmymoney, &KMyMoneyApp::slotSelectAccount);

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
    kmymoney->slotSelectAccount(obj);
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
    kmymoney->slotUpdateActions();
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
