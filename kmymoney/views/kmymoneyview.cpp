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

#ifdef KF5Activities_FOUND
#include <KActivities/ResourceInstance>
#endif

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneyglobalsettings.h"
#include "kmymoneytitlelabel.h"
#include <libkgpgfile/kgpgfile.h>
#include "kcurrencyeditdlg.h"
#include "mymoneyseqaccessmgr.h"
#include "mymoneydatabasemgr.h"
#include "imymoneystorageformat.h"
#include "mymoneystoragebin.h"
#include "mymoneyexception.h"
#include "mymoneystoragexml.h"
#include "mymoneystoragesql.h"
#include "mymoneygncreader.h"
#include "mymoneystorageanon.h"
#include "khomeview.h"
#include "kaccountsview.h"
#include "kcategoriesview.h"
#include "kinstitutionsview.h"
#include "kpayeesview.h"
#include "ktagsview.h"
#include "kscheduledview.h"
#include "kgloballedgerview.h"
#include "simpleledgerview.h"
#include "kinvestmentview.h"
#include "kreportsview.h"
#include "kbudgetview.h"
#include "kforecastview.h"
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
#include "mymoneybudget.h"
#include "mymoneyprice.h"
#include "mymoneyschedule.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "kmymoneyedit.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyreport.h"
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
    m_lastViewSelected(0)
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
      m_header->setVisible(KMyMoneyGlobalSettings::showTitleBar());
      gridLayout->addWidget(m_header, 1, 1);
    }
  }

  newStorage();
  m_model = new KPageWidgetModel(this); // cannot be parentless, otherwise segfaults at exit

  connect(kmymoney, SIGNAL(fileLoaded(QUrl)), this, SLOT(slotRefreshViews()));

  // let the accounts model know which account is being currently reconciled
  connect(this, SIGNAL(reconciliationStarts(MyMoneyAccount,QDate,MyMoneyMoney)), Models::instance()->accountsModel(), SLOT(slotReconcileAccount(MyMoneyAccount,QDate,MyMoneyMoney)));

  // Page 0
  m_homeView = new KHomeView();
  viewFrames[View::Home] = m_model->addPage(m_homeView, i18n("Home"));
  viewFrames[View::Home]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewHome]));
  connect(m_homeView, SIGNAL(ledgerSelected(QString,QString)),
          this, SLOT(slotLedgerSelected(QString,QString)));
  connect(m_homeView, SIGNAL(scheduleSelected(QString)),
          this, SLOT(slotScheduleSelected(QString)));
  connect(m_homeView, SIGNAL(reportSelected(QString)),
          this, SLOT(slotShowReport(QString)));
  connect(m_homeView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 1
  m_institutionsView = new KInstitutionsView;
  viewFrames[View::Institutions] = m_model->addPage(m_institutionsView, i18n("Institutions"));
  viewFrames[View::Institutions]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewInstitutions]));
  connect(m_institutionsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_institutionsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 2
  m_accountsView = new KAccountsView;
  viewFrames[View::Accounts] = m_model->addPage(m_accountsView, i18n("Accounts"));
  viewFrames[View::Accounts]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewAccounts]));
  connect(m_accountsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_accountsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 3
  m_scheduledView = new KScheduledView();
//this is to solve the way long strings are handled differently among versions of KPageWidget
  viewFrames[View::Schedules] = m_model->addPage(m_scheduledView, i18n("Scheduled transactions"));
  viewFrames[View::Schedules]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewSchedules]));

  connect(m_scheduledView, SIGNAL(scheduleSelected(MyMoneySchedule)), kmymoney, SLOT(slotSelectSchedule(MyMoneySchedule)));
  connect(m_scheduledView, SIGNAL(openContextMenu()), kmymoney, SLOT(slotShowScheduleContextMenu()));
  connect(m_scheduledView, SIGNAL(enterSchedule()), kmymoney, SLOT(slotScheduleEnter()));
  connect(m_scheduledView, SIGNAL(skipSchedule()), kmymoney, SLOT(slotScheduleSkip()));
  connect(m_scheduledView, SIGNAL(editSchedule()), kmymoney, SLOT(slotScheduleEdit()));
  connect(m_scheduledView, SIGNAL(aboutToShow()), this, SIGNAL(aboutToChangeView()));

  // Page 4
  m_categoriesView = new KCategoriesView;
  viewFrames[View::Categories] = m_model->addPage(m_categoriesView, i18n("Categories"));
  viewFrames[View::Categories]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewCategories]));
  connect(m_categoriesView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_categoriesView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 5
  m_tagsView = new KTagsView;
  viewFrames[View::Tags] = m_model->addPage(m_tagsView, i18n("Tags"));
  viewFrames[View::Tags]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewTags]));
  connect(m_tagsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_tagsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 6
  m_payeesView = new KPayeesView();
  viewFrames[View::Payees] = m_model->addPage(m_payeesView, i18n("Payees"));
  viewFrames[View::Payees]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewPayees]));

  connect(kmymoney, SIGNAL(payeeCreated(QString)), m_payeesView, SLOT(slotSelectPayeeAndTransaction(QString)));
  connect(kmymoney, SIGNAL(payeeRename()), m_payeesView, SLOT(slotRenameButtonCliked()));
  connect(m_payeesView, SIGNAL(openContextMenu(MyMoneyObject)), kmymoney, SLOT(slotShowPayeeContextMenu()));
  connect(m_payeesView, SIGNAL(selectObjects(QList<MyMoneyPayee>)), kmymoney, SLOT(slotSelectPayees(QList<MyMoneyPayee>)));
  connect(m_payeesView, SIGNAL(transactionSelected(QString,QString)),
          this, SLOT(slotLedgerSelected(QString,QString)));
  connect(m_payeesView, SIGNAL(aboutToShow()), this, SIGNAL(aboutToChangeView()));

  // Page 7
  m_ledgerView = new KGlobalLedgerView();
  viewFrames[View::Ledgers] = m_model->addPage(m_ledgerView, i18n("Ledgers"));
  viewFrames[View::Ledgers]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewLedgers]));

  connect(m_ledgerView, SIGNAL(accountSelected(MyMoneyObject)), kmymoney, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(m_ledgerView, SIGNAL(openContextMenu()), kmymoney, SLOT(slotShowTransactionContextMenu()));
  connect(m_ledgerView, SIGNAL(transactionsSelected(KMyMoneyRegister::SelectedTransactions)), kmymoney, SLOT(slotSelectTransactions(KMyMoneyRegister::SelectedTransactions)));
  connect(m_ledgerView, SIGNAL(newTransaction()), kmymoney, SLOT(slotTransactionsNew()));
  connect(m_ledgerView, SIGNAL(cancelOrEndEdit(bool&)), kmymoney, SLOT(slotTransactionsCancelOrEnter(bool&)));
  connect(m_ledgerView, SIGNAL(startEdit()), kmymoney, SLOT(slotTransactionsEdit()));
  connect(m_ledgerView, SIGNAL(endEdit()), kmymoney, SLOT(slotTransactionsEnter()));
  connect(m_ledgerView, SIGNAL(toggleReconciliationFlag()), kmymoney, SLOT(slotToggleReconciliationFlag()));
  connect(this, SIGNAL(reconciliationStarts(MyMoneyAccount,QDate,MyMoneyMoney)), m_ledgerView, SLOT(slotSetReconcileAccount(MyMoneyAccount,QDate,MyMoneyMoney)));
  connect(kmymoney, SIGNAL(selectAllTransactions()), m_ledgerView, SLOT(slotSelectAllTransactions()));
  connect(m_ledgerView, &KAccountsView::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 8
  m_investmentView = new KInvestmentView(kmymoney, this);
  viewFrames[View::Investments] = m_model->addPage(m_investmentView, i18n("Investments"));
  viewFrames[View::Investments]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewInvestment]));

  // Page 9
  m_reportsView = new KReportsView();
  viewFrames[View::Reports] = m_model->addPage(m_reportsView, i18n("Reports"));
  viewFrames[View::Reports]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewReports]));
  connect(m_reportsView, SIGNAL(ledgerSelected(QString,QString)),
          this, SLOT(slotLedgerSelected(QString,QString)));
  connect(m_reportsView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 10
  m_budgetView = new KBudgetView(kmymoney);
  viewFrames[View::Budget] = m_model->addPage(m_budgetView, i18n("Budgets"));
  viewFrames[View::Budget]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewBudgets]));
  connect(m_budgetView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::connectView);
  connect(m_budgetView, &KMyMoneyViewBase::aboutToShow, this, &KMyMoneyView::resetViewSelection);

  // Page 11
  m_forecastView = new KForecastView();
  viewFrames[View::Forecast] = m_model->addPage(m_forecastView, i18n("Forecast"));
  viewFrames[View::Forecast]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewForecast]));

  // Page 12
  m_onlineJobOutboxView = new KOnlineJobOutbox();
  viewFrames[View::OnlineJobOutbox] = m_model->addPage(m_onlineJobOutboxView, i18n("Outbox"));
  viewFrames[View::OnlineJobOutbox]->setIcon(QIcon::fromTheme(g_Icons[Icon::ViewOutbox]));
  connect(m_onlineJobOutboxView, SIGNAL(sendJobs(QList<onlineJob>)), kmymoney, SLOT(slotOnlineJobSend(QList<onlineJob>)));
  connect(m_onlineJobOutboxView, SIGNAL(editJob(QString)), kmymoney, SLOT(slotEditOnlineJob(QString)));
  connect(m_onlineJobOutboxView, SIGNAL(newCreditTransfer()), kmymoney, SLOT(slotNewOnlineTransfer()));
  connect(m_onlineJobOutboxView, SIGNAL(aboutToShow()), this, SIGNAL(aboutToChangeView()));
  connect(m_onlineJobOutboxView, SIGNAL(showContextMenu(onlineJob)), kmymoney, SLOT(slotShowOnlineJobContextMenu()));

  SimpleLedgerView* view = new SimpleLedgerView(kmymoney, this);
  KPageWidgetItem* frame = m_model->addPage(view, i18n("New ledger"));
  frame->setIcon(QIcon::fromTheme(g_Icons[Icon::DocumentProperties]));


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
    auto a = new QAction(0);
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
  KMyMoneyGlobalSettings::setLastViewSelected(m_lastViewSelected);
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
  showPage(viewFrames[View::Forecast]);
  m_forecastView->setDefaultFocus();
}

void KMyMoneyView::slotShowOutboxPage()
{
  showPage(viewFrames[View::OnlineJobOutbox]);
  m_onlineJobOutboxView->setDefaultFocus();
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
  }
}

bool KMyMoneyView::showPageHeader() const
{
  return false;
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

bool KMyMoneyView::canCreateTransactions(const KMyMoneyRegister::SelectedTransactions& /* list */, QString& tooltip) const
{
  // we can only create transactions in the ledger view so
  // we check that this is the active page
  bool rc = (viewFrames[View::Ledgers] == currentPage());
  if (rc)
    rc = m_ledgerView->canCreateTransactions(tooltip);
  else
    tooltip = i18n("Creating transactions can only be performed in the ledger view");
  return rc;
}

bool KMyMoneyView::canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  // we can only modify transactions in the ledger view so
  // we check that this is the active page

  bool rc = (viewFrames[View::Ledgers] == currentPage());

  if (rc) {
    rc = m_ledgerView->canModifyTransactions(list, tooltip);
  } else {
    tooltip = i18n("Modifying transactions can only be performed in the ledger view");
  }
  return rc;
}

bool KMyMoneyView::canDuplicateTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  // we can only duplicate transactions in the ledger view so
  // we check that this is the active page
  bool rc = (viewFrames[View::Ledgers] == currentPage());

  if (rc) {
    rc = m_ledgerView->canDuplicateTransactions(list, tooltip);
  } else {
    tooltip = i18n("Duplicating transactions can only be performed in the ledger view");
  }
  return rc;
}

bool KMyMoneyView::canEditTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  bool rc;
  // we can only edit transactions in the ledger view so
  // we check that this is the active page

  if ((rc = canModifyTransactions(list, tooltip)) == true) {
    tooltip = i18n("Edit the current selected transactions");
    rc = m_ledgerView->canEditTransactions(list, tooltip);
  }
  return rc;
}

bool KMyMoneyView::createNewTransaction()
{
  bool rc = false;
  KMyMoneyRegister::SelectedTransactions list;
  QString txt;
  if (canCreateTransactions(list, txt)) {
    rc = m_ledgerView->selectEmptyTransaction();
  }
  return rc;
}

TransactionEditor* KMyMoneyView::startEdit(const KMyMoneyRegister::SelectedTransactions& list)
{
  TransactionEditor* editor = 0;
  QString txt;
  if (canEditTransactions(list, txt) || canCreateTransactions(list, txt)) {
    editor = m_ledgerView->startEdit(list);
  }
  return editor;
}

void KMyMoneyView::newStorage(storageTypeE t)
{
  removeStorage();
  auto file = MyMoneyFile::instance();
  if (t == Memory)
    file->attachStorage(new MyMoneySeqAccessMgr);
  else
    file->attachStorage(new MyMoneyDatabaseMgr);

}

void KMyMoneyView::removeStorage()
{
  auto file = MyMoneyFile::instance();
  IMyMoneyStorage* p = file->storage();
  if (p != 0) {
    file->detachStorage(p);
    delete p;
  }
}

void KMyMoneyView::enableViewsIfFileOpen()
{
  // call set enabled only if the state differs to avoid widgets 'bouncing on the screen' while doing this
  if (viewFrames[View::Accounts]->isEnabled() != m_fileOpen)
    viewFrames[View::Accounts]->setEnabled(m_fileOpen);
  if (viewFrames[View::Institutions]->isEnabled() != m_fileOpen)
    viewFrames[View::Institutions]->setEnabled(m_fileOpen);
  if (viewFrames[View::Schedules]->isEnabled() != m_fileOpen)
    viewFrames[View::Schedules]->setEnabled(m_fileOpen);
  if (viewFrames[View::Categories]->isEnabled() != m_fileOpen)
    viewFrames[View::Categories]->setEnabled(m_fileOpen);
  if (viewFrames[View::Payees]->isEnabled() != m_fileOpen)
    viewFrames[View::Payees]->setEnabled(m_fileOpen);
  if (viewFrames[View::Tags]->isEnabled() != m_fileOpen)
    viewFrames[View::Tags]->setEnabled(m_fileOpen);
  if (viewFrames[View::Budget]->isEnabled() != m_fileOpen)
    viewFrames[View::Budget]->setEnabled(m_fileOpen);
  if (viewFrames[View::Ledgers]->isEnabled() != m_fileOpen)
    viewFrames[View::Ledgers]->setEnabled(m_fileOpen);
  if (viewFrames[View::Investments]->isEnabled() != m_fileOpen)
    viewFrames[View::Investments]->setEnabled(m_fileOpen);
  if (viewFrames[View::Reports]->isEnabled() != m_fileOpen)
    viewFrames[View::Reports]->setEnabled(m_fileOpen);
  if (viewFrames[View::Forecast]->isEnabled() != m_fileOpen)
    viewFrames[View::Forecast]->setEnabled(m_fileOpen);
  if (viewFrames[View::OnlineJobOutbox]->isEnabled() != m_fileOpen)
    viewFrames[View::OnlineJobOutbox]->setEnabled(m_fileOpen);
  emit viewStateChanged(m_fileOpen);
}

void KMyMoneyView::slotLedgerSelected(const QString& _accId, const QString& transaction)
{
  auto acc = MyMoneyFile::instance()->account(_accId);
  QString accId(_accId);

  switch (acc.accountType()) {
    case Account::Type::Stock:
      // if a stock account is selected, we show the
      // the corresponding parent (investment) account
      acc = MyMoneyFile::instance()->account(acc.parentAccountId());
      accId = acc.id();
      // intentional fall through

    case Account::Type::Checkings:
    case Account::Type::Savings:
    case Account::Type::Cash:
    case Account::Type::CreditCard:
    case Account::Type::Loan:
    case Account::Type::Asset:
    case Account::Type::Liability:
    case Account::Type::AssetLoan:
    case Account::Type::Income:
    case Account::Type::Expense:
    case Account::Type::Investment:
    case Account::Type::Equity:
      setCurrentPage(viewFrames[View::Ledgers]);
      m_ledgerView->slotSelectAccount(accId, transaction);
      break;

    case Account::Type::CertificateDep:
    case Account::Type::MoneyMarket:
    case Account::Type::Currency:
      qDebug("No ledger view available for account type %d", (int)acc.accountType());
      break;

    default:
      qDebug("Unknown account type %d in KMyMoneyView::slotLedgerSelected", (int)acc.accountType());
      break;
  }
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

void KMyMoneyView::slotScheduleSelected(const QString& scheduleId)
{
  MyMoneySchedule sched = MyMoneyFile::instance()->schedule(scheduleId);
  kmymoney->slotSelectSchedule(sched);
}

void KMyMoneyView::slotShowReport(const QString& reportid)
{
  showPage(viewFrames[View::Reports]);
  m_reportsView->slotOpenReport(reportid);
}

void KMyMoneyView::slotShowReport(const MyMoneyReport& report)
{
  showPage(viewFrames[View::Reports]);
  m_reportsView->slotOpenReport(report);
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

  disconnect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_homeView, &KHomeView::slotLoadView);

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

bool KMyMoneyView::readFile(const QUrl &url)
{
  QString filename;

  m_fileOpen = false;
  bool isEncrypted = false;

  IMyMoneyStorageFormat* pReader = 0;

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
    return (openDatabase(newUrl)); // on error, any message will have been displayed
  }

  IMyMoneyStorage *storage = new MyMoneySeqAccessMgr;

  if (url.isLocalFile()) {
    filename = url.toLocalFile();
  } else {
    // TODO: port to kf5 (NetAccess)
#if 0
    if (!KIO::NetAccess::download(url, filename, 0)) {
      KMessageBox::detailedError(this,
                                 i18n("Error while loading file '%1'.", url.url()),
                                 KIO::NetAccess::lastErrorString(),
                                 i18n("File access error"));
      return false;
    }
#endif
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
                  MyMoneyFile::instance()->attachStorage(storage);
                  loadAllCurrencies(); // currency list required for gnc
                  MyMoneyFile::instance()->detachStorage(storage);

                  pReader = new MyMoneyGncReader;
                  m_fileType = GncXML;
                }
              }
            }
            if (pReader) {
              pReader->setProgressCallback(&KMyMoneyView::progressCallback);
              pReader->readFile(qfile, dynamic_cast<IMyMoneySerialize*>(storage));
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
    MyMoneyFile::instance()->setValue("kmm-encryption-key", KMyMoneyGlobalSettings::gpgRecipientList().join(","));
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

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  // TODO: port to KF5 (NetAccess)
  //KIO::NetAccess::removeTempFile(filename);
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
  IMyMoneySerialize* pStorage = dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage());
  MyMoneyDatabaseMgr* pDBMgr = 0;
  if (! pStorage) {
    pDBMgr = new MyMoneyDatabaseMgr;
    pStorage = dynamic_cast<IMyMoneySerialize*>(pDBMgr);
  }
  QExplicitlySharedDataPointer <MyMoneyStorageSql> reader = pStorage->connectToDatabase(url);
  QUrl dbURL(url);
  bool retry = true;
  while (retry) {
    switch (reader->open(dbURL, QIODevice::ReadWrite)) {
      case 0: // opened okay
        retry = false;
        break;
      case 1: // permanent error
        KMessageBox::detailedError(this, i18n("Cannot open database %1\n", dbURL.toDisplayString()), reader->lastError());
        if (pDBMgr) {
          removeStorage();
          delete pDBMgr;
        }
        return false;
      case -1: // retryable error
        if (KMessageBox::warningYesNo(this, reader->lastError(), PACKAGE) == KMessageBox::No) {
          if (pDBMgr) {
            removeStorage();
            delete pDBMgr;
          }
          return false;
        } else {
          QUrlQuery query(dbURL);
          const QString optionKey = QLatin1String("options");
          QString options = query.queryItemValue(optionKey);
          if(!options.isEmpty()) {
            options += QLatin1Char(',');
          }
          options += QLatin1String("override");
          query.removeQueryItem(QLatin1String("mode"));
          query.removeQueryItem(optionKey);
          query.addQueryItem(optionKey, options);
          dbURL.setQuery(query);
        }
    }
  }
  if (pDBMgr) {
    removeStorage();
    MyMoneyFile::instance()->attachStorage(pDBMgr);
  }
  // single user mode; read some of the data into memory
  // FIXME - readFile no longer relevant?
  // tried removing it but then got no indication that loading was complete
  // also, didn't show home page
  reader->setProgressCallback(&KMyMoneyView::progressCallback);
  if (!reader->readFile()) {
    KMessageBox::detailedError(0,
                               i18n("An unrecoverable error occurred while reading the database"),
                               reader->lastError().toLatin1(),
                               i18n("Database malfunction"));
    return false;
  }
  m_fileOpen = true;
  reader->setProgressCallback(0);
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

  if (KMyMoneyGlobalSettings::startLastViewSelected() != 0)
    page = viewFrames.value(static_cast<View>(KMyMoneyGlobalSettings::lastViewSelected()));
  else
    page = viewFrames[View::Home];

  // For debugging purposes, we can turn off the automatic fix manually
  // by setting the entry in kmymoneyrc to true
  grp = config->group("General Options");
  if (grp.readEntry("SkipFix", false) != true) {
    MyMoneyFileTransaction ft;
    try {
      // Check if we have to modify the file before we allow to work with it
      IMyMoneyStorage* s = MyMoneyFile::instance()->storage();
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
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_homeView, &KHomeView::slotLoadView);

  // if we currently see a different page, then select the right one
  if (page != currentPage()) {
    showPage(page);
  }

  emit fileOpened();
  return true;
}

void KMyMoneyView::saveToLocalFile(const QString& localFile, IMyMoneyStorageFormat* pWriter, bool plaintext, const QString& keyList)
{
  // Check GPG encryption
  bool encryptFile = true;
  bool encryptRecover = false;
  if (!keyList.isEmpty()) {
    if (!KGPGFile::GPGAvailable()) {
      KMessageBox::sorry(this, i18n("GPG does not seem to be installed on your system. Please make sure that GPG can be found using the standard search path. This time, encryption is disabled."), i18n("GPG not found"));
      encryptFile = false;
    } else {
      if (KMyMoneyGlobalSettings::encryptRecover()) {
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
  pWriter->writeFile(device.get(), dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage()));
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
  std::unique_ptr<IMyMoneyStorageFormat> storageWriter;

  // If this file ends in ".ANON.XML" then this should be written using the
  // anonymous writer.
  bool plaintext = filename.right(4).toLower() == ".xml";
  if (filename.right(9).toLower() == ".anon.xml") {
    //! @todo C++14: use std::make_unique, also some lines below
    storageWriter = std::unique_ptr<IMyMoneyStorageFormat>(new MyMoneyStorageANON);
  } else {
    storageWriter = std::unique_ptr<IMyMoneyStorageFormat>(new MyMoneyStorageXML);
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
        const unsigned int nbak = KMyMoneyGlobalSettings::autoBackupCopies();
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
      // TODO: port to kf5
      //if (!KIO::NetAccess::upload(tmpfile.fileName(), url, 0))
      //  throw MYMONEYEXCEPTION(i18n("Unable to upload to '%1'", url.toDisplayString()));
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

bool KMyMoneyView::saveAsDatabase(const QUrl &url)
{
  bool rc = false;
  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it has not been opened"));
    return (rc);
  }
  MyMoneyStorageSql *writer = new MyMoneyStorageSql(dynamic_cast<IMyMoneySerialize*>(MyMoneyFile::instance()->storage()), url);
  bool canWrite = false;
  switch (writer->open(url, QIODevice::WriteOnly)) {
    case 0:
      canWrite = true;
      break;
    case -1: // dbase already has data, see if he wants to clear it out
      if (KMessageBox::warningContinueCancel(0,
                                             i18n("Database contains data which must be removed before using Save As.\n"
                                                  "Do you wish to continue?"), "Database not empty") == KMessageBox::Continue) {
        if (writer->open(url, QIODevice::WriteOnly, true) == 0)
          canWrite = true;
      } else {
        delete writer;
        return false;
      }
      break;
  }
  if (canWrite) {
    writer->setProgressCallback(&KMyMoneyView::progressCallback);
    if (!writer->writeFile()) {
      KMessageBox::detailedError(0,
                                 i18n("An unrecoverable error occurred while writing to the database.\n"
                                      "It may well be corrupt."),
                                 writer->lastError().toLatin1(),
                                 i18n("Database malfunction"));
      rc =  false;
    }
    writer->setProgressCallback(0);
    rc = true;
  } else {
    KMessageBox::detailedError(this,
                               i18n("Cannot open or create database %1.\n"
                                    "Retry Save As Database and click Help"
                                    " for further info.", url.toDisplayString()), writer->lastError());
  }
  delete writer;
  return (rc);
}

bool KMyMoneyView::dirty()
{
  if (!fileOpen())
    return false;

  return MyMoneyFile::instance()->dirty();
}

bool KMyMoneyView::startReconciliation(const MyMoneyAccount& account, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
{
  bool  ok = true;

  // we cannot reconcile standard accounts
  if (MyMoneyFile::instance()->isStandardAccount(account.id()))
    ok = false;

  // check if we can reconcile this account
  // it makes sense for asset and liability accounts only
  if (ok == true) {
    if (account.isAssetLiability()) {
      showPage(viewFrames[View::Ledgers]);
      // prepare reconciliation mode
      emit reconciliationStarts(account, reconciliationDate, endingBalance);
    } else {
      ok = false;
    }
  }

  return ok;
}

void KMyMoneyView::finishReconciliation(const MyMoneyAccount& /* account */)
{
  emit reconciliationStarts(MyMoneyAccount(), QDate(), MyMoneyMoney());
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
  disconnect(m_investmentView, SIGNAL(accountSelected(MyMoneyObject)), m_ledgerView, SLOT(slotSelectAccount(MyMoneyObject)));
  disconnect(m_ledgerView, SIGNAL(accountSelected(MyMoneyObject)), m_investmentView, SLOT(slotSelectAccount(MyMoneyObject)));


  // TODO turn sync between ledger and investment view if selected by user
  if (KMyMoneyGlobalSettings::syncLedgerInvestment()) {
    connect(m_investmentView, SIGNAL(accountSelected(MyMoneyObject)), m_ledgerView, SLOT(slotSelectAccount(MyMoneyObject)));
    connect(m_ledgerView, SIGNAL(accountSelected(MyMoneyObject)), m_investmentView, SLOT(slotSelectAccount(MyMoneyObject)));
  }

  showTitleBar(KMyMoneyGlobalSettings::showTitleBar());

  m_accountsView->refresh();
  m_institutionsView->refresh();
  m_categoriesView->refresh();
  m_payeesView->slotLoadPayees();
  m_tagsView->refresh();
  m_ledgerView->slotLoadView();
  m_budgetView->refresh();
  m_homeView->slotLoadView();
  m_investmentView->slotLoadView();
  m_reportsView->slotLoadView();
  m_forecastView->slotLoadForecast();
  m_scheduledView->slotReloadView();
}

void KMyMoneyView::slotShowTransactionDetail(bool detailed)
{
  KMyMoneyGlobalSettings::setShowRegisterDetailed(detailed);
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
  if (!KMyMoneyGlobalSettings::expertMode()) {
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
  if (!KMyMoneyGlobalSettings::expertMode()) {
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

      if (split.action() == MyMoneySplit::ActionInterest) {
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
    const char *defaultAction = 0;
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
          defaultAction = MyMoneySplit::ActionAmortization;
        else
          defaultAction = MyMoneySplit::ActionTransfer;
      } else {
        if (val.isNegative())
          defaultAction = MyMoneySplit::ActionWithdrawal;
        else
          defaultAction = MyMoneySplit::ActionDeposit;
      }
    }

    isLoan = false;
    foreach (const auto split, splits) {
      auto acc = file->account(split.accountId());
      MyMoneyMoney val = split.value();
      if (acc.accountGroup() == Account::Type::Asset
          || acc.accountGroup() == Account::Type::Liability) {
        if (!val.isPositive()) {
          defaultAction = MyMoneySplit::ActionWithdrawal;
          break;
        } else {
          defaultAction = MyMoneySplit::ActionDeposit;
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
        if (val < 0 && (*it_s).action() != MyMoneySplit::ActionWithdrawal && (*it_s).action() != MyMoneySplit::ActionTransfer)
          needModify = true;
        if (val >= 0 && (*it_s).action() != MyMoneySplit::ActionDeposit && (*it_s).action() != MyMoneySplit::ActionTransfer)
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
        if (split.action() != MyMoneySplit::ActionInterest) {
          qDebug() << Q_FUNC_INFO << " " << transaction.id() << " contains an interest account (" << split.accountId() << ") but does not have ActionInterest";
          split.setAction(MyMoneySplit::ActionInterest);
          transaction.modifySplit(split);
          file->modifyTransaction(transaction);
          qDebug("Fixed interest action in %s", qPrintable(transaction.id()));
        }
        // if it does not reference an interest account, it must not be
        // of type ActionInterest
      } else {
        if (split.action() == MyMoneySplit::ActionInterest) {
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
    case View::Accounts:
      disconnect(m_accountsView, &KAccountsView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_accountsView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectAccount);
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectInstitution);
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectInvestment);
      connect(treeView, &KMyMoneyAccountTreeView::openObject, kmymoney, &KMyMoneyApp::slotAccountOpen);
      connect(treeView, &KMyMoneyAccountTreeView::openContextMenu, kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled , this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(Models::instance()->accountsModel(), &AccountsModel::netWorthChanged, m_accountsView, &KAccountsView::slotNetWorthChanged);
      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_accountsView, &KAccountsView::refresh);
      break;
    case View::Institutions:
      disconnect(m_institutionsView, &KInstitutionsView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_institutionsView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectAccount);
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectInstitution);
      connect(treeView, &KMyMoneyAccountTreeView::openObject, kmymoney, &KMyMoneyApp::slotAccountOpen);
      connect(treeView, &KMyMoneyAccountTreeView::openObject, kmymoney, &KMyMoneyApp::slotInstitutionEdit);
      connect(treeView, &KMyMoneyAccountTreeView::openContextMenu, kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);
      connect(treeView, &KMyMoneyAccountTreeView::openContextMenu, kmymoney, &KMyMoneyApp::slotShowInstitutionContextMenu);
      connect(treeView, &KMyMoneyAccountTreeView::columnToggled , this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(Models::instance()->institutionsModel(), &AccountsModel::netWorthChanged, m_institutionsView, &KInstitutionsView::slotNetWorthChanged);
      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_institutionsView, &KInstitutionsView::refresh);
      break;
    case View::Categories:
      disconnect(m_categoriesView, &KCategoriesView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_categoriesView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectAccount);
      connect(treeView, &KMyMoneyAccountTreeView::openObject, kmymoney, &KMyMoneyApp::slotAccountOpen);
      connect(treeView, &KMyMoneyAccountTreeView::openContextMenu, kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);

      connect(treeView, &KMyMoneyAccountTreeView::columnToggled , this, &KMyMoneyView::slotAccountTreeViewChanged);

      connect(Models::instance()->institutionsModel(), &AccountsModel::profitChanged, m_categoriesView, &KCategoriesView::slotProfitChanged);
      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_categoriesView, &KCategoriesView::refresh);

      break;
    case View::Tags:
      disconnect(m_tagsView, &KTagsView::aboutToShow, this, &KMyMoneyView::connectView);
      connect(kmymoney,   &KMyMoneyApp::tagCreated,         m_tagsView, static_cast<void (KTagsView::*)(const QString&)>(&KTagsView::slotSelectTagAndTransaction));
      connect(kmymoney,   &KMyMoneyApp::slotTagRename,      m_tagsView, &KTagsView::slotRenameButtonCliked);
      connect(m_tagsView, &KTagsView::tagNewClicked,        kmymoney,   static_cast<void (KMyMoneyApp::*)()>(&KMyMoneyApp::slotTagNew));
      connect(m_tagsView, &KTagsView::tagDeleteClicked,     kmymoney,   static_cast<void (KMyMoneyApp::*)()>(&KMyMoneyApp::slotTagDelete));
      connect(m_tagsView, &KTagsView::openContextMenu,      kmymoney,   &KMyMoneyApp::slotShowTagContextMenu);
      connect(m_tagsView, &KTagsView::selectObjects,        kmymoney,   &KMyMoneyApp::slotSelectTags);
      connect(m_tagsView, &KTagsView::transactionSelected,  this,       &KMyMoneyView::slotLedgerSelected);
      connect(m_tagsView, &KMyMoneyViewBase::aboutToShow,   this,       &KMyMoneyView::aboutToChangeView);
      break;
    case View::Budget:
      disconnect(m_budgetView, &KBudgetView::aboutToShow, this, &KMyMoneyView::connectView);
      treeView = m_budgetView->getTreeView();
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectAccount);
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectInstitution);
      connect(treeView, &KMyMoneyAccountTreeView::selectObject, kmymoney, &KMyMoneyApp::slotSelectInvestment);
      connect(treeView, &KMyMoneyAccountTreeView::openObject, kmymoney, &KMyMoneyApp::slotAccountOpen);
      connect(treeView, &KMyMoneyAccountTreeView::openContextMenu, kmymoney, &KMyMoneyApp::slotShowAccountContextMenu);

      connect(m_budgetView, &KBudgetView::openContextMenu, kmymoney, &KMyMoneyApp::slotShowBudgetContextMenu);
      connect(m_budgetView, &KBudgetView::selectObjects, kmymoney, &KMyMoneyApp::slotSelectBudget);
      connect(kmymoney, &KMyMoneyApp::budgetRename, m_budgetView, &KBudgetView::slotStartRename);
      connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, m_budgetView, &KBudgetView::refresh);

      connect(treeView, &KMyMoneyAccountTreeView::columnToggled , this, &KMyMoneyView::slotAccountTreeViewChanged);
      break;
    default:
      break;
  }
}
