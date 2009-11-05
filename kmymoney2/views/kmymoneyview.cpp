/***************************************************************************
                          kmymoneyview.cpp
                             -------------------
    copyright            : (C) 2000 by Michael Edwardes <mte@users.sourceforge.net>
                               2004 by Thomas Baumgart <ipwizard@users.sourceforge.net>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/




#include <unistd.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QFile>
#include <q3textstream.h>
#include <q3progressdialog.h>
#include <QTextCodec>
#include <QStatusBar>

#include <QCursor>
#include <QRegExp>
#include <QLayout>
#include <QObject>
//Added by qt3to4:
#include <QList>
#include <QVBoxLayout>
#include <Q3Frame>
#include <QByteArray>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kfiledialog.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <ktemporaryfile.h>
#include <ksavefile.h>
#include <kfilterdev.h>
#include <kfilterbase.h>
#include <kfileitem.h>
#include <kpushbutton.h>
#include <kapplication.h>
#include <kdebug.h>

// ----------------------------------------------------------------------------
// Project Includes

// This is include is required here, because later it will produce
// compile errors on gcc 3.2 as we redefine new() in case of _CHECK_MEMORY
// being defined. To avoid these problems, we just include the header
// already here in this case
#ifdef _CHECK_MEMORY
#include <string>
#endif

#include <config-kmymoney.h>
#include <kmymoneyglobalsettings.h>
#include <kmymoneytitlelabel.h>
#include <libkgpgfile/kgpgfile.h>
#include "kendingbalancedlg.h"
#include "kchooseimportexportdlg.h"
#include "kcsvprogressdlg.h"
#include "kimportdlg.h"
#include "kexportdlg.h"
#include "knewloanwizard.h"
#include "kcurrencyeditdlg.h"
#include "kfindtransactiondlg.h"
#include "knewbankdlg.h"
#include "knewfiledlg.h"
#include "mymoneyseqaccessmgr.h"
#include "mymoneydatabasemgr.h"
#include "imymoneystorageformat.h"
#include "mymoneystoragebin.h"
#include "mymoneyexception.h"
#include "mymoneystoragexml.h"
#include "mymoneystoragesql.h"
#include "mymoneygncreader.h"
#include "mymoneystorageanon.h"
#include <transactioneditor.h>
#include "kmymoneyview.h"
#include "khomeview.h"
#include "kaccountsview.h"
#include "kcategoriesview.h"
#include "kinstitutionsview.h"
#include "kpayeesview.h"
#include "kscheduledview.h"
#include "kgloballedgerview.h"
#include "kinvestmentview.h"
#include "kreportsview.h"
#include "kbudgetview.h"
#include "kforecastview.h"
#include "kmymoney2.h"
#include "kmymoneyutils.h"

#define COMPRESSION_MIME_TYPE "application/x-gzip"
#define RECOVER_KEY_ID        "0xD2B08440"


KMyMoneyView::KMyMoneyView(QWidget *parent, const char *name)
    : KPageWidget(parent),
  // m_bankRightClick(false),
  m_inConstructor(true),
  m_fileOpen(false),
  m_fmode(0600)
{
  // the global variable kmymoney2 is not yet assigned. So we construct it here
  QObject* kmymoney2 = parent->parent();
  const int iconSize = (KMyMoneyGlobalSettings::iconSize()+1)*16;
  newStorage();

  m_model = new KPageWidgetModel();

  connect(this, SIGNAL(currentPageChanged (const QModelIndex, const QModelIndex)), this, SLOT(slotRefreshViews()));
  connect(this, SIGNAL(pageToggled(KPageWidgetItem*, bool)), this, SLOT(slotRefreshViews()));

  // Page 0
  m_homeView = new KHomeView();
  m_homeViewFrame = m_model->addPage( m_homeView, i18n("Home"));
  m_homeViewFrame->setIcon(KIcon("go-home"));
  m_homeViewFrame->setHeader(QString(""));
  connect(m_homeView, SIGNAL(ledgerSelected(const QString&, const QString&)),
          this, SLOT(slotLedgerSelected(const QString&, const QString&)));
  connect(m_homeView, SIGNAL(scheduleSelected(const QString&)),
    this, SLOT(slotScheduleSelected(const QString&)));
  connect(m_homeView, SIGNAL(reportSelected(const QString&)),
    this, SLOT(slotShowReport(const QString&)));

  // Page 1
  m_institutionsView = new KInstitutionsView();
  m_institutionsViewFrame = m_model->addPage( m_institutionsView, i18n("Institutions"));
  m_institutionsViewFrame->setIcon(KIcon("institutions"));
  m_institutionsViewFrame->setHeader(QString(""));
  addTitleBar(m_institutionsView, i18n("Institutions"));

  connect(m_institutionsView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectAccount(const MyMoneyObject&)));
  connect(m_institutionsView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectInstitution(const MyMoneyObject&)));
  connect(m_institutionsView, SIGNAL(openContextMenu(const MyMoneyObject&)), kmymoney2, SLOT(slotShowAccountContextMenu(const MyMoneyObject&)));
  connect(m_institutionsView, SIGNAL(openContextMenu(const MyMoneyObject&)), kmymoney2, SLOT(slotShowInstitutionContextMenu(const MyMoneyObject&)));
  connect(m_institutionsView, SIGNAL(openObject(const MyMoneyObject&)), kmymoney2, SLOT(slotInstitutionEdit(const MyMoneyObject&)));
  connect(m_institutionsView, SIGNAL(openObject(const MyMoneyObject&)), kmymoney2, SLOT(slotAccountOpen(const MyMoneyObject&)));
  connect(m_institutionsView, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyInstitution&)), kmymoney2, SLOT(slotReparentAccount(const MyMoneyAccount&, const MyMoneyInstitution&)));
  connect(this, SIGNAL(reconciliationStarts(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&)), m_institutionsView, SLOT(slotReconcileAccount(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&)));

  // Page 2
  m_accountsView = new KAccountsView();
  m_accountsViewFrame = m_model->addPage( m_accountsView, i18n("Accounts"));
  m_accountsViewFrame->setIcon(KIcon("accounts"));
  m_accountsViewFrame->setHeader(QString(""));
  addTitleBar(m_accountsView, i18n("Accounts"));

  connect(m_accountsView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectAccount(const MyMoneyObject&)));
  connect(m_accountsView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectInstitution(const MyMoneyObject&)));
  connect(m_accountsView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectInvestment(const MyMoneyObject&)));
  connect(m_accountsView, SIGNAL(openContextMenu(const MyMoneyObject&)), kmymoney2, SLOT(slotShowAccountContextMenu(const MyMoneyObject&)));
  connect(m_accountsView, SIGNAL(openObject(const MyMoneyObject&)), kmymoney2, SLOT(slotAccountOpen(const MyMoneyObject&)));
  connect(m_accountsView, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotReparentAccount(const MyMoneyAccount&, const MyMoneyAccount&)));
  connect(this, SIGNAL(kmmFilePlugin(unsigned int)), m_accountsView, SLOT(slotUpdateIconPos(unsigned int)));
  connect(this, SIGNAL(reconciliationStarts(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&)), m_accountsView, SLOT(slotReconcileAccount(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&)));

  // Page 3
  m_scheduledView = new KScheduledView();
  m_scheduleViewFrame = m_model->addPage( m_scheduledView, i18n("Scheduled\ntransactions"));
  m_scheduleViewFrame->setIcon(KIcon("view-pim-calendar"));
  m_scheduleViewFrame->setHeader(QString(""));
  addTitleBar(m_scheduledView, i18n("Scheduled transactions"));
  connect(kmymoney2, SIGNAL(fileLoaded(const KUrl&)), m_scheduledView, SLOT(slotReloadView()));
  connect(m_scheduledView, SIGNAL(scheduleSelected(const MyMoneySchedule&)), kmymoney2, SLOT(slotSelectSchedule(const MyMoneySchedule&)));
  connect(m_scheduledView, SIGNAL(openContextMenu()), kmymoney2, SLOT(slotShowScheduleContextMenu()));
  connect(m_scheduledView, SIGNAL(enterSchedule()), kmymoney2, SLOT(slotScheduleEnter()));
  connect(m_scheduledView, SIGNAL(skipSchedule()), kmymoney2, SLOT(slotScheduleSkip()));
  connect(m_scheduledView, SIGNAL(editSchedule()), kmymoney2, SLOT(slotScheduleEdit()));

  // Page 4
  m_categoriesView = new KCategoriesView();
  m_categoriesViewFrame = m_model->addPage( m_categoriesView, i18n("Categories"));
  m_categoriesViewFrame->setIcon(KIcon("categories"));
  m_categoriesViewFrame->setHeader(QString(""));
  addTitleBar(m_categoriesView, i18n("Categories"));
  connect(m_categoriesView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectAccount(const MyMoneyObject&)));
  connect(m_categoriesView, SIGNAL(selectObject(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectInstitution(const MyMoneyObject&)));
  connect(m_categoriesView, SIGNAL(openContextMenu(const MyMoneyObject&)), kmymoney2, SLOT(slotShowAccountContextMenu(const MyMoneyObject&)));
  connect(m_categoriesView, SIGNAL(openObject(const MyMoneyObject&)), kmymoney2, SLOT(slotAccountOpen(const MyMoneyObject&)));
  connect(m_categoriesView, SIGNAL(reparent(const MyMoneyAccount&, const MyMoneyAccount&)), kmymoney2, SLOT(slotReparentAccount(const MyMoneyAccount&, const MyMoneyAccount&)));

  // Page 5
  m_payeesView = new KPayeesView();
  m_payeesViewFrame = m_model->addPage( m_payeesView, i18n("Payees"));
  m_payeesViewFrame->setIcon(KIcon("system-users"));
  m_payeesViewFrame->setHeader(QString(""));
  addTitleBar(m_payeesView, i18n("Payees"));
  connect(kmymoney2, SIGNAL(payeeCreated(const QString&)), m_payeesView, SLOT(slotSelectPayeeAndTransaction(const QString&)));
  connect(kmymoney2, SIGNAL(payeeRename()), m_payeesView, SLOT(slotStartRename()));
  connect(m_payeesView, SIGNAL(openContextMenu(const MyMoneyObject&)), kmymoney2, SLOT(slotShowPayeeContextMenu()));
  connect(m_payeesView, SIGNAL(selectObjects(const QList<MyMoneyPayee>&)), kmymoney2, SLOT(slotSelectPayees(const QList<MyMoneyPayee>&)));
  connect(m_payeesView, SIGNAL(transactionSelected(const QString&, const QString&)),
          this, SLOT(slotLedgerSelected(const QString&, const QString&)));
  // Page 6
  m_ledgerView = new KGlobalLedgerView();
  m_ledgerViewFrame = m_model->addPage( m_ledgerView, i18n("Ledgers"));
  m_ledgerViewFrame->setIcon(KIcon("ledger"));
  m_ledgerViewFrame->setHeader(QString(""));
  connect(m_ledgerView, SIGNAL(accountSelected(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectAccount(const MyMoneyObject&)));
  connect(m_ledgerView, SIGNAL(openContextMenu()), kmymoney2, SLOT(slotShowTransactionContextMenu()));
  connect(m_ledgerView, SIGNAL(transactionsSelected(const KMyMoneyRegister::SelectedTransactions&)), kmymoney2, SLOT(slotSelectTransactions(const KMyMoneyRegister::SelectedTransactions&)));
  connect(m_ledgerView, SIGNAL(newTransaction()), kmymoney2, SLOT(slotTransactionsNew()));
  connect(m_ledgerView, SIGNAL(cancelOrEndEdit(bool&)), kmymoney2, SLOT(slotTransactionsCancelOrEnter(bool&)));
  connect(m_ledgerView, SIGNAL(startEdit()), kmymoney2, SLOT(slotTransactionsEdit()));
  connect(m_ledgerView, SIGNAL(endEdit()), kmymoney2, SLOT(slotTransactionsEnter()));
  connect(m_ledgerView, SIGNAL(toggleReconciliationFlag()), kmymoney2, SLOT(slotToggleReconciliationFlag()));
  connect(this, SIGNAL(reconciliationStarts(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&)), m_ledgerView, SLOT(slotSetReconcileAccount(const MyMoneyAccount&, const QDate&, const MyMoneyMoney&)));
  connect(kmymoney2, SIGNAL(selectAllTransactions()), m_ledgerView, SLOT(slotSelectAllTransactions()));


  // Page 7
  m_investmentView = new KInvestmentView();
  m_investmentViewFrame = m_model->addPage( m_investmentView, i18n("Investments"));
  m_investmentViewFrame->setIcon(KIcon("investments"));
  m_investmentViewFrame->setHeader(QString(""));
  addTitleBar(m_investmentView, i18n("Investments"));
  connect(m_investmentView, SIGNAL(accountSelected(const QString&, const QString&)),
      this, SLOT(slotLedgerSelected(const QString&, const QString&)));
  connect(m_investmentView, SIGNAL(accountSelected(const MyMoneyObject&)), kmymoney2, SLOT(slotSelectAccount(const MyMoneyObject&)));
  connect(m_investmentView, SIGNAL(investmentRightMouseClick()), kmymoney2, SLOT(slotShowInvestmentContextMenu()));

  // Page 8
  m_reportsView = new KReportsView();
  m_reportsViewFrame = m_model->addPage( m_reportsView, i18n("Reports"));
  m_reportsViewFrame->setIcon(KIcon("report"));
  m_reportsViewFrame->setHeader(QString(""));

  // Page 9
  m_budgetView = new KBudgetView();
  m_budgetViewFrame = m_model->addPage( m_budgetView, i18n("Budgets"));
  m_budgetViewFrame->setIcon(KIcon("budget"));
  m_budgetViewFrame->setHeader(QString(""));
  addTitleBar(m_budgetView, i18n("Budgets"));
  connect(kmymoney2, SIGNAL(fileLoaded(const KUrl&)), m_budgetView, SLOT(slotRefreshView()));
  connect(m_budgetView, SIGNAL(openContextMenu(const MyMoneyObject&)), kmymoney2, SLOT(slotShowBudgetContextMenu()));
  connect(m_budgetView, SIGNAL(selectObjects(const QList<MyMoneyBudget>&)), kmymoney2, SLOT(slotSelectBudget(const QList<MyMoneyBudget>&)));
  connect(kmymoney2, SIGNAL(budgetRename()), m_budgetView, SLOT(slotStartRename()));

  // Page 10
  m_forecastView = new KForecastView();
  m_forecastViewFrame = m_model->addPage( m_forecastView, i18n("Forecast"));
  m_forecastViewFrame->setIcon(KIcon("forcast"));
  m_forecastViewFrame->setHeader(QString(""));
  addTitleBar(m_forecastView, i18n("Forecast"));

  //set the model
  setModel(m_model);

  #warning "port to kde4"
  #if 0
  // get rid of the title text
  QWidget* widget = dynamic_cast<QWidget*>(child("KJanusWidgetTitleLabel", "QLabel"));
  if(widget)
    widget->hide();

  // and the separator below it
  widget = dynamic_cast<QWidget*>(child(0, "KSeparator"));
  if(widget)
    widget->hide();
  #endif

  // select the page first, before connecting the aboutToShow signal
  // because we don't want to override the information stored in the config file


  setCurrentPage(m_homeViewFrame);
  connect(this, SIGNAL(currentPageChanged(const QModelIndex, const QModelIndex)), this, SLOT(slotRememberPage(const QModelIndex, const QModelIndex)));


  m_inConstructor = false;
}

KMyMoneyView::~KMyMoneyView()
{
  removeStorage();
}

void KMyMoneyView::addTitleBar(QWidget* parent, const QString& title)
{
  KMyMoneyTitleLabel* label = new KMyMoneyTitleLabel(parent);
  label->setObjectName("titleLabel");
  label->setMinimumSize( QSize( 100, 30 ) );
  label->setRightImageFile("pics/titlelabel_background.png" );
  label->setText(title);
  QBoxLayout* pBoxLayout = dynamic_cast<QBoxLayout *>(parent->layout());
  if (pBoxLayout)
    pBoxLayout->insertWidget(0, label);
}

void KMyMoneyView::showTitleBar(bool show)
{
  QList<KMyMoneyTitleLabel *> l = findChildren<KMyMoneyTitleLabel *>("titleLabel");

  for (QList<KMyMoneyTitleLabel *>::iterator it = l.begin(); it != l.end(); ++it)
    (*it)->setVisible(show);
}

void KMyMoneyView::showPage(KPageWidgetItem* pageItem)
{

  // reset all selected items before showing the selected view
  // but not while we're in our own constructor
  if(!m_inConstructor && pageItem != currentPage()) {
    kmymoney2->slotResetSelections();
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

  if(!m_inConstructor) {
    // fixup some actions that are dependant on the view
    // this does not work during construction
    kmymoney2->slotUpdateActions();
  }
}

bool KMyMoneyView::canPrint(void)
{

  bool rc = (
              m_reportsViewFrame == currentPage()
              || m_homeViewFrame == currentPage()
            );
  return rc;
}

bool KMyMoneyView::canCreateTransactions(const KMyMoneyRegister::SelectedTransactions& /* list */, QString& tooltip) const
{
  // we can only create transactions in the ledger view so
  // we check that this is the active page
  bool rc = (m_ledgerViewFrame == currentPage());
  if(rc)
    rc = m_ledgerView->canCreateTransactions(tooltip);
  else
    tooltip = i18n("Creating transactions can only be performed in the ledger view");
  return rc;
}

bool KMyMoneyView::canModifyTransactions(const KMyMoneyRegister::SelectedTransactions& list, QString& tooltip) const
{
  // we can only modify transactions in the ledger view so
  // we check that this is the active page

  bool rc = (m_ledgerViewFrame == currentPage());

  if(rc) {
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
  bool rc = (m_ledgerViewFrame == currentPage());

  if(rc) {
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

  if((rc = canModifyTransactions(list, tooltip)) == true) {
    tooltip = i18n("Edit the current selected transactions");
    rc = m_ledgerView->canEditTransactions(list, tooltip);
  }
  return rc;
}

bool KMyMoneyView::createNewTransaction(void)
{
  bool rc = false;
  KMyMoneyRegister::SelectedTransactions list;
  QString txt;
  if(canCreateTransactions(list, txt)) {
    rc = m_ledgerView->selectEmptyTransaction();
  }
  return rc;
}

TransactionEditor* KMyMoneyView::startEdit(const KMyMoneyRegister::SelectedTransactions& list)
{
  TransactionEditor* editor = 0;
  QString txt;
  if(canEditTransactions(list, txt) || canCreateTransactions(list, txt)) {
    editor = m_ledgerView->startEdit(list);
  }
  return editor;
}

void KMyMoneyView::newStorage(storageTypeE t)
{
  removeStorage();
  MyMoneyFile* file = MyMoneyFile::instance();
  if (t == Memory)
    file->attachStorage(new MyMoneySeqAccessMgr);
  else
    file->attachStorage(new MyMoneyDatabaseMgr);
}

void KMyMoneyView::removeStorage(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  IMyMoneyStorage* p = file->storage();
  if(p != 0) {
    file->detachStorage(p);
    delete p;
  }
}

void KMyMoneyView::enableViews(int state)
{
  if(state == -1)
    state = m_fileOpen;

  m_accountsViewFrame->setEnabled(state);
  m_institutionsViewFrame->setEnabled(state);
  m_scheduleViewFrame->setEnabled(state);
  m_categoriesViewFrame->setEnabled(state);
  m_payeesViewFrame->setEnabled(state);
  m_budgetViewFrame->setEnabled(state);
  m_ledgerViewFrame->setEnabled(state);
  m_investmentViewFrame->setEnabled(state);
  m_reportsViewFrame->setEnabled(state);
  m_forecastViewFrame->setEnabled(state);

  emit viewStateChanged(state != 0);

}

void KMyMoneyView::slotLedgerSelected(const QString& _accId, const QString& transaction)
{
  MyMoneyAccount acc = MyMoneyFile::instance()->account(_accId);
  QString accId(_accId);

  switch(acc.accountType()) {
    case MyMoneyAccount::Stock:
      // if a stock account is selected, we show the
      // the corresponding parent (investment) account
      acc = MyMoneyFile::instance()->account(acc.parentAccountId());
      accId = acc.id();
      // tricky fall through here

    case MyMoneyAccount::Checkings:
    case MyMoneyAccount::Savings:
    case MyMoneyAccount::Cash:
    case MyMoneyAccount::CreditCard:
    case MyMoneyAccount::Loan:
    case MyMoneyAccount::Asset:
    case MyMoneyAccount::Liability:
    case MyMoneyAccount::AssetLoan:
    case MyMoneyAccount::Income:
    case MyMoneyAccount::Expense:
    case MyMoneyAccount::Investment:
    case MyMoneyAccount::Equity:
        setCurrentPage(m_ledgerViewFrame);
        m_ledgerView->slotSelectAccount(accId, transaction);
      break;

    case MyMoneyAccount::CertificateDep:
    case MyMoneyAccount::MoneyMarket:
    case MyMoneyAccount::Currency:
      qDebug("No ledger view available for account type %d", acc.accountType());
      break;

    default:
      qDebug("Unknown account type %d in KMyMoneyView::slotLedgerSelected", acc.accountType());
      break;
  }
}

void KMyMoneyView::slotPayeeSelected(const QString& payee, const QString& account, const QString& transaction)
{
  showPage(m_payeesViewFrame);
  m_payeesView->slotSelectPayeeAndTransaction(payee, account, transaction);
}

void KMyMoneyView::slotScheduleSelected(const QString& scheduleId)
{
  MyMoneySchedule sched = MyMoneyFile::instance()->schedule(scheduleId);
  kmymoney2->slotSelectSchedule(sched);
}

void KMyMoneyView::slotShowReport(const QString& reportid)
{
  showPage(m_reportsViewFrame);
  m_reportsView->slotOpenReport(reportid);
}

void KMyMoneyView::slotShowReport(const MyMoneyReport& report)
{
  showPage(m_reportsViewFrame);
  m_reportsView->slotOpenReport(report);
}

bool KMyMoneyView::fileOpen(void)
{
  return m_fileOpen;
}

void KMyMoneyView::closeFile(void)
{
  if ( m_reportsView )
    m_reportsView->slotCloseAll();

  emit kmmFilePlugin (preClose);
  if (isDatabase())
    MyMoneyFile::instance()->storage()->close(); // to log off a database user
  newStorage();

  slotShowHomePage();

  emit kmmFilePlugin (postClose);
  m_fileOpen = false;
}

void KMyMoneyView::ungetString(QIODevice *qfile, char *buf, int len)
{
  buf = &buf[len-1];
  while(len--) {
    qfile->ungetch(*buf--);
  }
}

bool KMyMoneyView::readFile(const KUrl& url)
{
  QString filename;

//  newStorage();
  m_fileOpen = false;
  bool isEncrypted = false;

  IMyMoneyStorageFormat* pReader = NULL;

  if(!url.isValid()) {
    qDebug("Invalid URL '%s'", qPrintable(url.url()));
    return false;
  }

  if (url.protocol() == "sql") { // handle reading of database
    //newStorage(Database);
    MyMoneyFile::instance()->detachStorage();
    m_fileType = KmmDb;
    // get rid of the mode parameter which is now redundant
    KUrl newUrl(url);
    if (!url.queryItem("mode").isNull()) {
      newUrl.removeQueryItem("mode");
    }
    return (openDatabase(newUrl)); // on error, any message will have been displayed
  }

  newStorage();

  if(url.isLocalFile()) {
    filename = url.path();
  } else {
    if(!KIO::NetAccess::download(url, filename, NULL)) {
      KMessageBox::detailedError(this,
             i18n("Error while loading file '%1'!",url.url()),
             KIO::NetAccess::lastErrorString(),
             i18n("File access error"));
      return false;
    }
  }

  // let's glimps into the file to figure out, if it's one
  // of the old (uncompressed) or new (compressed) files.
  QFile file(filename);
  QFileInfo info(file);
  if(!info.isFile()) {
    QString msg=i18n("<b>%1</b> is not a KMyMoney file.",filename);
    KMessageBox::error(this, QString("<p>")+msg, i18n("Filetype Error"));
    return false;
  }
  m_fmode = 0600;
  m_fmode |= info.permission(QFile::ReadGroup) ? 040 : 0;
  m_fmode |= info.permission(QFile::WriteGroup) ? 020 : 0;
  m_fmode |= info.permission(QFile::ReadOther) ? 004 : 0;
  m_fmode |= info.permission(QFile::WriteOther) ? 002 : 0;

  QIODevice *qfile = 0;
  bool rc = true;

  // There's a problem with the KFilterDev and KGPGFile classes:
  // One supports the at(n) member but not ungetch() together with
  // read() and the other does not provide an at(n) method but
  // supports read() that considers the ungetch() buffer. QFile
  // supports everything so this is not a problem. We solve the problem
  // for now by keeping track of which method can be used.
  bool haveAt = true;

  emit kmmFilePlugin (preOpen);
  ::timetrace("start reading file");
  if(file.open(QIODevice::ReadOnly)) {
    QByteArray hdr(2);
    int cnt;
    cnt = file.read(hdr.data(), 2);
    file.close();

    if(cnt == 2) {
      if(QString(hdr) == QString("\037\213")) {         // gzipped?
        ::timetrace("detected GZIP");
        qfile = KFilterDev::deviceForFile(filename, COMPRESSION_MIME_TYPE);
      } else if(QString(hdr) == QString("--")){         // PGP ASCII armored?
        ::timetrace("detected GPG");
        if(KGPGFile::GPGAvailable()) {
          ::timetrace("have GPG");
          qfile = new KGPGFile(filename);
          haveAt = false;
          isEncrypted = true;
        } else {
          KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("GPG is not available for decryption of file <b>%1</b>",filename)));
          qfile = new QFile(file.name());
        }
      } else {
        // we can't use file directly, as we delete qfile later on
        qfile = new QFile(file.name());
      }

      ::timetrace("open file");
      if(qfile->open(QIODevice::ReadOnly)) {
        try {
          hdr.resize(8);
          if(qfile->read(hdr.data(), 8) == 8) {
            if(haveAt)
              qfile->at(0);
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
            if((magic0 == MAGIC_0_50 && magic1 == MAGIC_0_51)
            || magic0 < 30) {
              // we do not support this file format anymore
              pReader = 0;
              m_fileType = KmmBinary;
            } else {
              ::timetrace("is not binary format");
              // Scan the first 70 bytes to see if we find something
              // we know. For now, we support our own XML format and
              // GNUCash XML format. If the file is smaller, then it
              // contains no valid data and we reject it anyway.
              hdr.resize(70);
              if(qfile->read(hdr.data(), 70) == 70) {
                if(haveAt)
                  qfile->at(0);
                else
                  ungetString(qfile, hdr.data(), 70);
                QRegExp kmyexp("<!DOCTYPE KMYMONEY-FILE>");
                QRegExp gncexp("<gnc-v(\\d+)");
                QByteArray txt(hdr, 70);
                if(kmyexp.indexIn(txt) != -1) {
                  ::timetrace("is XML format");
                  pReader = new MyMoneyStorageXML;
                  m_fileType = KmmXML;
                } else if(gncexp.indexIn(txt) != -1) {
                  ::timetrace("is GNC format");
                  MyMoneyFileTransaction ft;
                  loadDefaultCurrencies(); // currency list required for gnc
                  loadAncientCurrencies(); // these too
                  ft.commit();
                  pReader = new MyMoneyGncReader;
                  m_fileType = GncXML;
                }
              }
            }
            if(pReader) {
              pReader->setProgressCallback(&KMyMoneyView::progressCallback);
              ::timetrace("read data to memory");
              pReader->readFile(qfile, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
              ::timetrace("done reading to memory");
            } else {
              if(m_fileType == KmmBinary) {
                KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> contains the old binary format used by KMyMoney. Please use an older version of KMyMoney (0.8.x) that still supports this format to convert it to the new XML based format.",filename)));
              } else {
                KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> contains an unknown file format!",filename)));
              }
              rc = false;
            }
          } else {
            KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("Cannot read from file <b>%1</b>!",filename)));
            rc = false;
          }
        } catch (MyMoneyException *e) {
          KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("Cannot load file <b>%1</b>. Reason: %2",filename, e->what())));
          delete e;
          rc = false;
        }
        if(pReader) {
          pReader->setProgressCallback(0);
          delete pReader;
        }
        qfile->close();
      } else {
        KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> not found!",filename)));
        rc = false;
      }
      delete qfile;
    }
  } else {
    KMessageBox::sorry(this, QString("<qt>%1</qt>"). arg(i18n("File <b>%1</b> not found!",filename)));
    rc = false;
  }

  ::timetrace("done reading file");
  if(rc == false)
    return rc;

  // make sure we setup the encryption key correctly
  MyMoneyFileTransaction ft;
  if(isEncrypted && MyMoneyFile::instance()->value("kmm-encryption-key").isEmpty()) {
    MyMoneyFile::instance()->setValue("kmm-encryption-key", KMyMoneyGlobalSettings::gpgRecipientList().join(","));
  }

  // make sure we setup the name of the base accounts in translated form
  try {
    MyMoneyFile* file = MyMoneyFile::instance();
    checkAccountName(file->asset(), i18n("Asset"));
    checkAccountName(file->liability(), i18n("Liability"));
    checkAccountName(file->income(), i18n("Income"));
    checkAccountName(file->expense(), i18n("Expense"));
    checkAccountName(file->equity(), i18n("Equity"));
    ft.commit();
  } catch(MyMoneyException* e) {
    delete e;
  }

  // if a temporary file was constructed by NetAccess::download,
  // then it will be removed with the next call. Otherwise, it
  // stays untouched on the local filesystem
  KIO::NetAccess::removeTempFile(filename);
  return initializeStorage();
}

void KMyMoneyView::checkAccountName(const MyMoneyAccount& _acc, const QString& name) const
{
  MyMoneyFile* file = MyMoneyFile::instance();
  if(_acc.name() != name) {
    MyMoneyAccount acc(_acc);
    acc.setName(name);
    file->modifyAccount(acc);
  }
}

bool KMyMoneyView::openDatabase (const KUrl& url) {
      	::timetrace("start opening database");
  m_fileOpen = false;

  // open the database
  IMyMoneySerialize* pStorage = dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage());
  MyMoneyDatabaseMgr* pDBMgr = 0;
  if (! pStorage) {
    pDBMgr = new MyMoneyDatabaseMgr;
    pStorage = dynamic_cast<IMyMoneySerialize*> (pDBMgr);
  }
  KSharedPtr <MyMoneyStorageSql> reader = pStorage->connectToDatabase (url);
  KUrl dbURL (url);
  bool retry = true;
  while (retry) {
    switch (reader->open(dbURL, QIODevice::ReadWrite)) {
    case 0: // opened okay
      retry = false;
      break;
    case 1: // permanent error
      KMessageBox::detailedError (this, i18n("Can't open database %1\n",dbURL.prettyUrl()), reader->lastError());
      if (pDBMgr) {
        removeStorage();
        delete pDBMgr;
      }
      return false;
    case -1: // retryable error
      if (KMessageBox::warningYesNo (this, reader->lastError(), PACKAGE) == KMessageBox::No) {
        if (pDBMgr) {
          removeStorage();
          delete pDBMgr;
        }
        return false;
      } else {
        QString options = dbURL.queryItem("options") + ",override";
        dbURL.removeQueryItem("mode"); // now redundant
        dbURL.removeQueryItem("options");
        dbURL.addQueryItem("options", options);
      }
    }
  }
  if (pDBMgr) {
    removeStorage();
    MyMoneyFile::instance()->attachStorage(pDBMgr);
  }
  // single user mode; read some of the data into memory
  // FIXME - readFile no longer relevant?
  // tried removing it but then then got no indication that loading was complete
  // also, didn't show home page
  reader->setProgressCallback(&KMyMoneyView::progressCallback);
  if (!reader->readFile()) {
    KMessageBox::detailedError (0,
                                i18n("An unrecoverable error occurred while reading the database"),
                                reader->lastError().toLatin1(),
                                i18n("Database malfunction"));
    return false;
  }
  m_fileOpen = true;
  reader->setProgressCallback(0);
  ::timetrace("done opening database");
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
    loadDefaultCurrencies();
    loadAncientCurrencies();
    ft.commit();
  } catch(MyMoneyException *e) {
    delete e;
    MyMoneyFile::instance()->blockSignals(blocked);
    return false;
  }

  // make sure, we have a base currency and all accounts are
  // also assigned to a currency.
  if(MyMoneyFile::instance()->baseCurrency().id().isEmpty()) {
    // Stay in this endless loop until we have a base currency,
    // as without it the application does not work anymore.
    while(MyMoneyFile::instance()->baseCurrency().id().isEmpty())
      selectBaseCurrency();

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

  KSharedConfigPtr config = KGlobal::config();
  int pageIndex = 0;
  KPageWidgetItem* page;
  KConfigGroup grp = config->group("General Options");

  if(KMyMoneyGlobalSettings::startLastViewSelected() != 0) {
    KConfigGroup grp2 = config->group("Last Use Settings");
    pageIndex = grp2.readEntry("LastViewSelected", 0);
    page = m_model->item(m_model->index(pageIndex, 0));
  } else {
    page = m_homeViewFrame;
  }
  ::timetrace("start fixing file");

  // For debugging purposes, we can turn off the automatic fix manually
  // by setting the entry in kmymoney2rc to true
  grp = config->group("General Options");
  if(grp.readEntry("SkipFix", false) != true) {
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

          // add new levels above. Don't forget to increase currentFixVersion() for all
          // the storage backends this fix applies to
          default:
            throw new MYMONEYEXCEPTION(i18n("Unknown fix level in input file"));
        }
      }
      ft.commit();
    } catch(MyMoneyException *e) {
      delete e;
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
  // kmymoney2->createInitialAccount();

  ::timetrace("file open");
  m_fileOpen = true;
  emit kmmFilePlugin (postOpen);

  // inform everyone about new data
  MyMoneyFile::instance()->preloadCache();
  MyMoneyFile::instance()->forceDataChanged();

  // if we currently see a different page, then select the right one
  if(pageIndex != KPageView::currentPage().row()) {
    showPage(page);
  }
  return true;
}

void KMyMoneyView::saveToLocalFile(const QString& localFile, IMyMoneyStorageFormat* pWriter, bool plaintext, const QString& keyList)
{
  KSaveFile qfile(localFile);
  QIODevice *dev = &qfile;
  KFilterBase *base = 0;
  QIODevice *statusDevice = dev;

  bool encryptedOk = true;
  bool encryptRecover = false;
  if(!keyList.isEmpty()) {
    if(!KGPGFile::GPGAvailable()) {
      KMessageBox::sorry(this, i18n("GPG does not seem to be installed on your system. Please make sure, that GPG can be found using the standard search path. This time, encryption is disabled."), i18n("GPG not found"));
      encryptedOk = false;
    }

    if(KMyMoneyGlobalSettings::encryptRecover()) {
      encryptRecover = true;
      if(!KGPGFile::keyAvailable(QString(RECOVER_KEY_ID))) {
        KMessageBox::sorry(this, QString("<p>")+i18n("You have selected to encrypt your data also with the KMyMoney recover key, but the key with id</p><p><center><b>%1</b></center></p><p>has not been found in your keyring at this time. Please make sure to import this key into your keyring. You can find it on the <a href=\"http://kmymoney2.sourceforge.net/\">KMyMoney web-site</a>. This time your data will not be encrypted with the KMyMoney recover key.</p>",QString( RECOVER_KEY_ID) ), i18n("GPG-Key not found"));
        encryptRecover = false;
      }
    }

    const QStringList keys = keyList.split(',', QString::SkipEmptyParts);
    QStringList::const_iterator it_s;
    for(it_s = keys.constBegin(); it_s != keys.constEnd(); ++it_s) {
      if(!KGPGFile::keyAvailable(*it_s)) {
        KMessageBox::sorry(this, QString("<p>")+i18n("You have specified to encrypt your data for the user-id</p><p><center><b>%1</b>.</center></p><p>Unfortunately, a valid key for this user-id was not found in your keyring. Please make sure to import a valid key for this user-id. This time, encryption is disabled.</p>",*it_s), i18n("GPG-Key not found"));
        encryptedOk = false;
      }
    }

    if(encryptedOk == true) {
      QString msg = QString("<p>") + i18n("You have configured to save your data in encrypted form using GPG. Please be aware, that this is a brand new feature which is yet untested. Make sure, you have the necessary understanding that you might loose all your data if you store it encrypted and cannot decrypt it later on! If unsure, answer <b>No</b>.");

      if(KMessageBox::questionYesNo(this, msg, i18n("Store GPG encrypted"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "StoreEncrypted") == KMessageBox::No) {
        encryptedOk = false;
      }
    }
  }

  int mask = umask((~m_fmode) & 0777);
  bool blocked = MyMoneyFile::instance()->signalsBlocked();
  MyMoneyFile::instance()->blockSignals(true);
  MyMoneyFileTransaction ft;
  MyMoneyFile::instance()->deletePair("kmm-encryption-key");
  if(!keyList.isEmpty() && encryptedOk == true && !plaintext ) {
    base++;
    KGPGFile *kgpg = new KGPGFile(localFile);
    if(kgpg) {
      QStringList keys = keyList.split(',', QString::SkipEmptyParts);
      QStringList::const_iterator it_s;
      for(it_s = keys.constBegin(); it_s != keys.constEnd(); ++it_s) {
        kgpg->addRecipient((*it_s).toLatin1());
      }
      if(encryptRecover) {
        kgpg->addRecipient(RECOVER_KEY_ID);
      }
      MyMoneyFile::instance()->setValue("kmm-encryption-key", keyList);
    }
    statusDevice = dev = kgpg;
    if(!dev || !dev->open(QIODevice::WriteOnly)) {
      MyMoneyFile::instance()->blockSignals(blocked);
      delete dev;
      throw new MYMONEYEXCEPTION(i18n("Unable to open file '%1' for writing.", localFile));
    }

  } else if(!plaintext) {

    base = KFilterBase::findFilterByMimeType( COMPRESSION_MIME_TYPE );
    if(base) {
      base->setDevice(&qfile, false);
      // we need to reopen the file to set the mode inside the filter stuff
      dev = KFilterDev::deviceForFile(localFile, COMPRESSION_MIME_TYPE, true);
      if(!dev || !dev->open(QIODevice::WriteOnly)) {
        MyMoneyFile::instance()->blockSignals(blocked);
        delete dev;
        throw new MYMONEYEXCEPTION(i18n("Unable to open file '%1' for writing.", localFile));
      }
      statusDevice = base->device();
    }
  } else if (plaintext) {
    qfile.open();
    if(qfile.error() != QFile::NoError) {
      throw new MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'",localFile));
    }
  }

  umask(mask);
  ft.commit();

  pWriter->setProgressCallback(&KMyMoneyView::progressCallback);
  dev->resetStatus();
  pWriter->writeFile(dev, dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()));
  MyMoneyFile::instance()->blockSignals(blocked);
  if(statusDevice->status() != IO_Ok) {
    throw new MYMONEYEXCEPTION(i18n("Failure while writing to '%1'", localFile));
  }
  pWriter->setProgressCallback(0);

  if(base != 0) {
    dev->close();
    if(statusDevice->status() != IO_Ok) {
      delete dev;
      dev = 0;
      throw new MYMONEYEXCEPTION(i18n("Failure while writing to '%1'", localFile));
    }
    delete dev;
  } else
    qfile.close();
}

bool KMyMoneyView::saveFile(const KUrl& url, const QString& keyList)
{
  QString filename = url.path();

  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return false;
  }

  emit kmmFilePlugin (preSave);
  IMyMoneyStorageFormat* pWriter = NULL;

  // If this file ends in ".ANON.XML" then this should be written using the
  // anonymous writer.
  bool plaintext = filename.right(4).toLower() == ".xml";
  if (filename.right(9).toLower() == ".anon.xml")
  {
    pWriter = new MyMoneyStorageANON;
  }
  else
  {
    // only use XML writer. The binary format will be deprecated after 0.8
    pWriter = new MyMoneyStorageXML;
  }

  // actually, url should be the parameter to this function
  // but for now, this would involve too many changes
  bool rc = true;
  try {
    if(! url.isValid()) {
      throw new MYMONEYEXCEPTION(i18n("Malformed URL '%1'",url.url()));
    }

    if(url.isLocalFile()) {
      filename = url.path();
      int fmode = 0600;
      gid_t gid = static_cast<gid_t>(-1);      // don't change the group id (see "man 2 chown")
      QFileInfo fi(filename);
      if(fi.exists()) {
        fmode |= fi.permission(QFile::ReadGroup) ? 040 : 0;
        fmode |= fi.permission(QFile::WriteGroup) ? 020 : 0;
        fmode |= fi.permission(QFile::ReadOther) ? 004 : 0;
        fmode |= fi.permission(QFile::WriteOther) ? 002 : 0;
        if(fi.groupId() != static_cast<uint>(-2))
          gid = fi.groupId();
      }

      // create a new basic block here, so that the object qfile gets
      // deleted, before we reach the chown() call
      {
        //FIXME: Port to KDE4 - set the mode later
        int mask = umask((~fmode) & 0777);
        umask(mask);
        try {
          saveToLocalFile(filename, pWriter, plaintext, keyList);
        } catch (MyMoneyException* e) {
          delete e;
          throw new MYMONEYEXCEPTION(i18n("Unable to write changes to '%1'",filename));
        }
      }
      chown(filename.toLatin1(), static_cast<uid_t>(-1), gid);
    } else {
        KTemporaryFile tmpfile;
        tmpfile.open(); // to obtain the name
        saveToLocalFile(tmpfile.fileName(), pWriter, plaintext, keyList);
        if(!KIO::NetAccess::upload(tmpfile.fileName(), url, NULL))
          throw new MYMONEYEXCEPTION(i18n("Unable to upload to '%1'",url.url()));
        tmpfile.close();
    }
    m_fileType = KmmXML;
  } catch (MyMoneyException *e) {
    KMessageBox::error(this, e->what());
    delete e;
    MyMoneyFile::instance()->setDirty();
    rc = false;
  }
  delete pWriter;
  emit kmmFilePlugin (postSave);
  return rc;
}

bool KMyMoneyView::saveAsDatabase(const KUrl& url)
{
  bool rc = false;
  if (!fileOpen()) {
    KMessageBox::error(this, i18n("Tried to access a file when it's not open"));
    return (rc);
  }
  MyMoneyStorageSql *writer = new MyMoneyStorageSql(dynamic_cast<IMyMoneySerialize*> (MyMoneyFile::instance()->storage()), url);
  bool canWrite = false;
  switch (writer->open(url, QIODevice::WriteOnly)) {
      case 0:
        canWrite = true;
        break;
      case -1: // dbase already has data, see if he wants to clear it out
        if (KMessageBox::warningContinueCancel (0,
            i18n("Database contains data which must be removed before using SaveAs.\n"
                "Do you wish to continue?"), "Database not empty") == KMessageBox::Continue) {
          if (writer->open(url, QIODevice::WriteOnly, true) == 0) canWrite = true;
        } else {
          delete writer;
          return false;
        }
        break;
  }
  if (canWrite) {
    writer->setProgressCallback(&KMyMoneyView::progressCallback);
    if (!writer->writeFile()) {
      KMessageBox::detailedError (0,
                                i18n("An unrecoverable error occurred while writing to the database.\n"
                                    "It may well be corrupt."),
                                writer->lastError().toLatin1(),
                                i18n("Database malfunction"));
      rc =  false;
    }
    writer->setProgressCallback(0);
    rc = true;
  } else {
    KMessageBox::detailedError (this,
      i18n("Can't open or create database %1\n"
          "Retry SaveAsDatabase and click Help"
          " for further info",url.prettyUrl()), writer->lastError());
  }
  delete writer;
  return (rc);
}

bool KMyMoneyView::dirty(void)
{
  if (!fileOpen())
    return false;

  return MyMoneyFile::instance()->dirty();
}

bool KMyMoneyView::startReconciliation(const MyMoneyAccount& account, const QDate& reconciliationDate, const MyMoneyMoney& endingBalance)
{
  bool  ok = true;

  // we cannot reconcile standard accounts
  if(MyMoneyFile::instance()->isStandardAccount(account.id()))
    ok = false;

  // check if we can reconcile this account
  // it makes sense for asset and liability accounts only
  if(ok == true) {
    if(account.isAssetLiability()) {
      showPage(m_ledgerViewFrame);
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

void KMyMoneyView::newFile(void)
{
  closeFile();
  m_fileType = KmmXML; // assume native type until saved
  m_fileOpen = true;
}

void KMyMoneyView::slotSetBaseCurrency(const MyMoneySecurity& baseCurrency)
{
  if(!baseCurrency.id().isEmpty()) {
    if(baseCurrency.id() != MyMoneyFile::instance()->baseCurrency().id()) {
      MyMoneyFileTransaction ft;
      try {
        MyMoneyFile::instance()->setBaseCurrency(baseCurrency);
        ft.commit();
      } catch(MyMoneyException *e) {
        KMessageBox::sorry(this, i18n("Cannot set %1 as base currency: %2",baseCurrency.name(),e->what()), i18n("Set base currency"));
        delete e;
      }
    }
  }
}

void KMyMoneyView::selectBaseCurrency(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

  // check if we have a base currency. If not, we need to select one
  if(file->baseCurrency().id().isEmpty()) {
    QPointer<KCurrencyEditDlg> dlg = new KCurrencyEditDlg(this);
    connect(dlg, SIGNAL(selectBaseCurrency(const MyMoneySecurity&)), this, SLOT(slotSetBaseCurrency(const MyMoneySecurity&)));
    dlg->exec();
    delete dlg;
  }

  if(!file->baseCurrency().id().isEmpty()) {
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


    for(it = list.begin(); it != list.end(); ++it) {
      if((*it).currencyId().isEmpty() || (*it).currencyId().length() == 0) {
        (*it).setCurrencyId(file->baseCurrency().id());
        MyMoneyFileTransaction ft;
        try {
          file->modifyAccount(*it);
          ft.commit();
        } catch(MyMoneyException *e) {
          qDebug("Unable to setup base currency in account %s (%s): %s", qPrintable((*it).name()), qPrintable((*it).id()), qPrintable(e->what()));
          delete e;
        }
      }
    }
  }
}

void KMyMoneyView::loadDefaultCurrency(const MyMoneySecurity& currency, const bool create)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneySecurity sec;
  MyMoneyFileTransaction ft;
  try {
    sec = file->currency(currency.id());
    if(sec.name() != currency.name()) {
      sec.setName(currency.name());
      file->modifyCurrency(sec);
    }
    ft.commit();
  } catch (MyMoneyException* e) {
    delete e;
    try {
      if(create) {
        file->addCurrency(currency);
      }
      ft.commit();
    } catch (MyMoneyException* e) {
      qDebug("Error %s loading default currency", qPrintable(e->what()));
      delete e;
    }
  }
}

void KMyMoneyView::loadDefaultCurrencies(void)
{
  // more information can be obtained from http://en.wikipedia.org/wiki/Currency_codes

  bool create = MyMoneyFile::instance()->currencyList().count() == 0;
  loadDefaultCurrency(MyMoneySecurity("AFA", i18n("Afghanistan Afghani")), create);
  loadDefaultCurrency(MyMoneySecurity("ALL", i18n("Albanian Lek")), create);
  loadDefaultCurrency(MyMoneySecurity("ANG", i18n("Netherland Antillian Guilder")), create);
  loadDefaultCurrency(MyMoneySecurity("DZD", i18n("Algerian Dinar")), create);
  loadDefaultCurrency(MyMoneySecurity("ADF", i18n("Andorran Franc")), create);
  loadDefaultCurrency(MyMoneySecurity("ADP", i18n("Andorran Peseta")), create);
  loadDefaultCurrency(MyMoneySecurity("AON", i18n("Angolan New Kwanza")), create);
  loadDefaultCurrency(MyMoneySecurity("ARS", i18n("Argentine Peso"),         "$"), create);
  loadDefaultCurrency(MyMoneySecurity("AWG", i18n("Aruban Florin")), create);
  loadDefaultCurrency(MyMoneySecurity("AUD", i18n("Australian Dollar"),      "$"), create);
  loadDefaultCurrency(MyMoneySecurity("AZM", i18n("Azerbaijani Manat")), create);
  loadDefaultCurrency(MyMoneySecurity("BSD", i18n("Bahamian Dollar"),        "$"), create);
  loadDefaultCurrency(MyMoneySecurity("BHD", i18n("Bahraini Dinar"),         "BHD", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("BDT", i18n("Bangladeshi Taka")), create);
  loadDefaultCurrency(MyMoneySecurity("BBD", i18n("Barbados Dollar"),        "$"), create);
  loadDefaultCurrency(MyMoneySecurity("BYR", i18n("Belarusian Ruble"),      "BYR", 1, 1), create);
  loadDefaultCurrency(MyMoneySecurity("BZD", i18n("Belize Dollar"),          "$"), create);
  loadDefaultCurrency(MyMoneySecurity("BMD", i18n("Bermudian Dollar"),       "$"), create);
  loadDefaultCurrency(MyMoneySecurity("BTN", i18n("Bhutan Ngultrum")), create);
  loadDefaultCurrency(MyMoneySecurity("BOB", i18n("Bolivian Boliviano")), create);
  loadDefaultCurrency(MyMoneySecurity("BAM", i18n("Bosnian Convertible Mark")), create);
  loadDefaultCurrency(MyMoneySecurity("BWP", i18n("Botswana Pula")), create);
  loadDefaultCurrency(MyMoneySecurity("BRL", i18n("Brazilian Real"),         "R$"), create);
  loadDefaultCurrency(MyMoneySecurity("GBP", i18n("British Pound"),          QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("BND", i18n("Brunei Dollar"),          "$"), create);
  loadDefaultCurrency(MyMoneySecurity("BGL", i18n("Bulgarian Lev")), create);
  loadDefaultCurrency(MyMoneySecurity("BIF", i18n("Burundi Franc")), create);
  loadDefaultCurrency(MyMoneySecurity("XAF", i18n("CFA Franc BEAC")), create);
  loadDefaultCurrency(MyMoneySecurity("XOF", i18n("CFA Franc BCEAO")), create);
  loadDefaultCurrency(MyMoneySecurity("XPF", i18n("CFP Franc Pacifique"), "F", 1, 1, 100), create);
  loadDefaultCurrency(MyMoneySecurity("KHR", i18n("Cambodia Riel")), create);
  loadDefaultCurrency(MyMoneySecurity("CAD", i18n("Canadian Dollar"),        "$"), create);
  loadDefaultCurrency(MyMoneySecurity("CVE", i18n("Cape Verde Escudo")), create);
  loadDefaultCurrency(MyMoneySecurity("KYD", i18n("Cayman Islands Dollar"),  "$"), create);
  loadDefaultCurrency(MyMoneySecurity("CLP", i18n("Chilean Peso")), create);
  loadDefaultCurrency(MyMoneySecurity("CNY", i18n("Chinese Yuan Renminbi")), create);
  loadDefaultCurrency(MyMoneySecurity("COP", i18n("Colombian Peso")), create);
  loadDefaultCurrency(MyMoneySecurity("KMF", i18n("Comoros Franc")), create);
  loadDefaultCurrency(MyMoneySecurity("CRC", i18n("Costa Rican Colon"),      QChar(0x20A1)), create);
  loadDefaultCurrency(MyMoneySecurity("HRK", i18n("Croatian Kuna")), create);
  loadDefaultCurrency(MyMoneySecurity("CUP", i18n("Cuban Peso")), create);
  loadDefaultCurrency(MyMoneySecurity("CZK", i18n("Czech Koruna")), create);
  loadDefaultCurrency(MyMoneySecurity("DKK", i18n("Danish Krone"),           "kr"), create);
  loadDefaultCurrency(MyMoneySecurity("DJF", i18n("Djibouti Franc")), create);
  loadDefaultCurrency(MyMoneySecurity("DOP", i18n("Dominican Peso")), create);
  loadDefaultCurrency(MyMoneySecurity("XCD", i18n("East Caribbean Dollar"),  "$"), create);
  loadDefaultCurrency(MyMoneySecurity("EGP", i18n("Egyptian Pound"),         QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("SVC", i18n("El Salvador Colon")), create);
  loadDefaultCurrency(MyMoneySecurity("ERN", i18n("Eritrean Nakfa")), create);
  loadDefaultCurrency(MyMoneySecurity("EEK", i18n("Estonian Kroon")), create);
  loadDefaultCurrency(MyMoneySecurity("ETB", i18n("Ethiopian Birr")), create);
  loadDefaultCurrency(MyMoneySecurity("EUR", i18n("Euro"),                   QChar(0x20ac)), true);
  loadDefaultCurrency(MyMoneySecurity("FKP", i18n("Falkland Islands Pound"), QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("FJD", i18n("Fiji Dollar"),            "$"), create);
  loadDefaultCurrency(MyMoneySecurity("GMD", i18n("Gambian Dalasi")), create);
  loadDefaultCurrency(MyMoneySecurity("GEL", i18n("Georgian Lari")), create);
  loadDefaultCurrency(MyMoneySecurity("GHC", i18n("Ghanaian Cedi")), create);
  loadDefaultCurrency(MyMoneySecurity("GIP", i18n("Gibraltar Pound"),        QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("GTQ", i18n("Guatemalan Quetzal")), create);
  loadDefaultCurrency(MyMoneySecurity("GWP", i18n("Guinea-Bissau Peso")), create);
  loadDefaultCurrency(MyMoneySecurity("GYD", i18n("Guyanan Dollar"),         "$"), create);
  loadDefaultCurrency(MyMoneySecurity("HTG", i18n("Haitian Gourde")), create);
  loadDefaultCurrency(MyMoneySecurity("HNL", i18n("Honduran Lempira")), create);
  loadDefaultCurrency(MyMoneySecurity("HKD", i18n("Hong Kong Dollar"),       "$"), create);
  loadDefaultCurrency(MyMoneySecurity("HUF", i18n("Hungarian Forint"),       "HUF", 1, 1, 100), create);
  loadDefaultCurrency(MyMoneySecurity("ISK", i18n("Iceland Krona")), create);
  loadDefaultCurrency(MyMoneySecurity("INR", i18n("Indian Rupee"),           QChar(0x20A8)), create);
  loadDefaultCurrency(MyMoneySecurity("IDR", i18n("Indonesian Rupiah"),      "IDR", 100, 1), create);
  loadDefaultCurrency(MyMoneySecurity("IRR", i18n("Iranian Rial"),           "IRR", 1, 1), create);
  loadDefaultCurrency(MyMoneySecurity("IQD", i18n("Iraqi Dinar"),            "IQD", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("ILS", i18n("Israeli New Shekel"),     QChar(0x20AA)), create);
  loadDefaultCurrency(MyMoneySecurity("JMD", i18n("Jamaican Dollar"),        "$"), create);
  loadDefaultCurrency(MyMoneySecurity("JPY", i18n("Japanese Yen"),           QChar(0x00A5), 100, 1), create);
  loadDefaultCurrency(MyMoneySecurity("JOD", i18n("Jordanian Dinar"),        "JOD", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("KZT", i18n("Kazakhstan Tenge")), create);
  loadDefaultCurrency(MyMoneySecurity("KES", i18n("Kenyan Shilling")), create);
  loadDefaultCurrency(MyMoneySecurity("KWD", i18n("Kuwaiti Dinar"),          "KWD", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("KGS", i18n("Kyrgyzstan Som")), create);
  loadDefaultCurrency(MyMoneySecurity("LAK", i18n("Laos Kip"),               QChar(0x20AD)), create);
  loadDefaultCurrency(MyMoneySecurity("LVL", i18n("Latvian Lats")), create);
  loadDefaultCurrency(MyMoneySecurity("LBP", i18n("Lebanese Pound"),         QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("LSL", i18n("Lesotho Loti")), create);
  loadDefaultCurrency(MyMoneySecurity("LRD", i18n("Liberian Dollar"),        "$"), create);
  loadDefaultCurrency(MyMoneySecurity("LYD", i18n("Libyan Dinar"),           "LYD", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("LTL", i18n("Lithuanian Litas")), create);
  loadDefaultCurrency(MyMoneySecurity("MOP", i18n("Macau Pataca")), create);
  loadDefaultCurrency(MyMoneySecurity("MKD", i18n("Macedonian Denar")), create);
  loadDefaultCurrency(MyMoneySecurity("MGF", i18n("Malagasy Franc"),         "MGF", 500, 500), create);
  loadDefaultCurrency(MyMoneySecurity("MWK", i18n("Malawi Kwacha")), create);
  loadDefaultCurrency(MyMoneySecurity("MYR", i18n("Malaysian Ringgit")), create);
  loadDefaultCurrency(MyMoneySecurity("MVR", i18n("Maldive Rufiyaa")), create);
  loadDefaultCurrency(MyMoneySecurity("MLF", i18n("Mali Republic Franc")), create);
  loadDefaultCurrency(MyMoneySecurity("MRO", i18n("Mauritanian Ouguiya"),    "MRO", 5, 5), create);
  loadDefaultCurrency(MyMoneySecurity("MUR", i18n("Mauritius Rupee")), create);
  loadDefaultCurrency(MyMoneySecurity("MXN", i18n("Mexican Peso"),           "$"), create);
  loadDefaultCurrency(MyMoneySecurity("MDL", i18n("Moldavian Leu")), create);
  loadDefaultCurrency(MyMoneySecurity("MNT", i18n("Mongolian Tugrik"),       QChar(0x20AE)), create);
  loadDefaultCurrency(MyMoneySecurity("MAD", i18n("Moroccan Dirham")), create);
  loadDefaultCurrency(MyMoneySecurity("MZM", i18n("Mozambique Metical")), create);
  loadDefaultCurrency(MyMoneySecurity("MMK", i18n("Myanmar Kyat")), create);
  loadDefaultCurrency(MyMoneySecurity("NAD", i18n("Namibian Dollar"),        "$"), create);
  loadDefaultCurrency(MyMoneySecurity("NPR", i18n("Nepalese Rupee")), create);
  loadDefaultCurrency(MyMoneySecurity("NZD", i18n("New Zealand Dollar"),     "$"), create);
  loadDefaultCurrency(MyMoneySecurity("NIC", i18n("Nicaraguan Cordoba Oro")), create);
  loadDefaultCurrency(MyMoneySecurity("NGN", i18n("Nigerian Naira"),         QChar(0x20A6)), create);
  loadDefaultCurrency(MyMoneySecurity("KPW", i18n("North Korean Won"),       QChar(0x20A9)), create);
  loadDefaultCurrency(MyMoneySecurity("NOK", i18n("Norwegian Kroner"),       "kr"), create);
  loadDefaultCurrency(MyMoneySecurity("OMR", i18n("Omani Rial"),             "OMR", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("PKR", i18n("Pakistan Rupee")), create);
  loadDefaultCurrency(MyMoneySecurity("PAB", i18n("Panamanian Balboa")), create);
  loadDefaultCurrency(MyMoneySecurity("PGK", i18n("Papua New Guinea Kina")), create);
  loadDefaultCurrency(MyMoneySecurity("PYG", i18n("Paraguay Guarani")), create);
  loadDefaultCurrency(MyMoneySecurity("PEN", i18n("Peruvian Nuevo Sol")), create);
  loadDefaultCurrency(MyMoneySecurity("PHP", i18n("Philippine Peso"),        QChar(0x20B1)), create);
  loadDefaultCurrency(MyMoneySecurity("PLN", i18n("Polish Zloty")), create);
  loadDefaultCurrency(MyMoneySecurity("QAR", i18n("Qatari Rial")), create);
  loadDefaultCurrency(MyMoneySecurity("RON", i18n("Romanian Leu (new)")), true);
  loadDefaultCurrency(MyMoneySecurity("RUB", i18n("Russian Ruble")), true);
  loadDefaultCurrency(MyMoneySecurity("RWF", i18n("Rwanda Franc")), create);
  loadDefaultCurrency(MyMoneySecurity("WST", i18n("Samoan Tala")), create);
  loadDefaultCurrency(MyMoneySecurity("STD", i18n("Sao Tome and Principe Dobra")), create);
  loadDefaultCurrency(MyMoneySecurity("SAR", i18n("Saudi Riyal")), create);
  loadDefaultCurrency(MyMoneySecurity("SCR", i18n("Seychelles Rupee")), create);
  loadDefaultCurrency(MyMoneySecurity("SLL", i18n("Sierra Leone Leone")), create);
  loadDefaultCurrency(MyMoneySecurity("SGD", i18n("Singapore Dollar"),       "$"), create);
  // loadDefaultCurrency(MyMoneySecurity("SKK", i18n("Slovak Koruna")), create);
  // loadDefaultCurrency(MyMoneySecurity("SIT", i18n("Slovenian Tolar")), create);
  loadDefaultCurrency(MyMoneySecurity("SBD", i18n("Solomon Islands Dollar"), "$"), create);
  loadDefaultCurrency(MyMoneySecurity("SOS", i18n("Somali Shilling")), create);
  loadDefaultCurrency(MyMoneySecurity("ZAR", i18n("South African Rand")), create);
  loadDefaultCurrency(MyMoneySecurity("KRW", i18n("South Korean Won"),       QChar(0x20A9)), create);
  loadDefaultCurrency(MyMoneySecurity("LKR", i18n("Sri Lanka Rupee")), create);
  loadDefaultCurrency(MyMoneySecurity("SHP", i18n("St. Helena Pound"),       QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("SDD", i18n("Sudanese Dinar")), create);
  loadDefaultCurrency(MyMoneySecurity("SRG", i18n("Suriname Guilder")), create);
  loadDefaultCurrency(MyMoneySecurity("SZL", i18n("Swaziland Lilangeni")), create);
  loadDefaultCurrency(MyMoneySecurity("SEK", i18n("Swedish Krona")), create);
  loadDefaultCurrency(MyMoneySecurity("CHF", i18n("Swiss Franc"),            "SFr"), create);
  loadDefaultCurrency(MyMoneySecurity("SYP", i18n("Syrian Pound"),           QChar(0x00A3)), create);
  loadDefaultCurrency(MyMoneySecurity("TWD", i18n("Taiwan Dollar"),          "$"), create);
  loadDefaultCurrency(MyMoneySecurity("TJS", i18n("Tajikistan Somani")), create);
  loadDefaultCurrency(MyMoneySecurity("TZS", i18n("Tanzanian Shilling")), create);
  loadDefaultCurrency(MyMoneySecurity("THB", i18n("Thai Baht"),              QChar(0x0E3F)), create);
  loadDefaultCurrency(MyMoneySecurity("TOP", i18n("Tongan Pa'anga")), create);
  loadDefaultCurrency(MyMoneySecurity("TTD", i18n("Trinidad and Tobago Dollar"), "$"), create);
  loadDefaultCurrency(MyMoneySecurity("TND", i18n("Tunisian Dinar"),         "TND", 1000, 1000), create);
  loadDefaultCurrency(MyMoneySecurity("TRY", i18n("Turkish Lira (new)"), "YTL"), true);
  loadDefaultCurrency(MyMoneySecurity("TMM", i18n("Turkmenistan Manat")), create);
  loadDefaultCurrency(MyMoneySecurity("USD", i18n("US Dollar"),              "$"), create);
  loadDefaultCurrency(MyMoneySecurity("UGX", i18n("Uganda Shilling")), create);
  loadDefaultCurrency(MyMoneySecurity("UAH", i18n("Ukraine Hryvnia")), create);
  loadDefaultCurrency(MyMoneySecurity("AED", i18n("United Arab Emirates Dirham")), create);
  loadDefaultCurrency(MyMoneySecurity("UYU", i18n("Uruguayan Peso")), create);
  loadDefaultCurrency(MyMoneySecurity("UZS", i18n("Uzbekistani Sum")), create);
  loadDefaultCurrency(MyMoneySecurity("VUV", i18n("Vanuatu Vatu")), create);
  loadDefaultCurrency(MyMoneySecurity("VEB", i18n("Venezuelan Bolivar")), create);
  loadDefaultCurrency(MyMoneySecurity("VND", i18n("Vietnamese Dong"),        QChar(0x20AB)), create);
  loadDefaultCurrency(MyMoneySecurity("YUM", i18n("Yugoslav Dinar")), create);
  loadDefaultCurrency(MyMoneySecurity("ZMK", i18n("Zambian Kwacha")), create);
  loadDefaultCurrency(MyMoneySecurity("ZWD", i18n("Zimbabwe Dollar"),        "$"), create);

  loadDefaultCurrency(MyMoneySecurity("XAU", i18n("Gold"),       "XAU", 1, 1000000), create);
  loadDefaultCurrency(MyMoneySecurity("XPD", i18n("Palladium"),  "XPD", 1, 1000000), create);
  loadDefaultCurrency(MyMoneySecurity("XPT", i18n("Platinum"),   "XPT", 1, 1000000), create);
  loadDefaultCurrency(MyMoneySecurity("XAG", i18n("Silver"),     "XAG", 1, 1000000), create);
}

void KMyMoneyView::loadAncientCurrency(const QString& id, const QString& name, const QString& sym, const QDate& date, const MyMoneyMoney& rate, const QString& newId, const int partsPerUnit, const int smallestCashFraction, const int smallestAccountFraction)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyPrice price(id, newId, date, rate, "KMyMoney");
  MyMoneyFileTransaction ft;
  try {
    // make sure if entry exists
    file->currency(id);
    // make sure we have the right price
    if(file->price(id, newId, date, true) != price) {
      file->addPrice(price);
    }
    ft.commit();
  } catch(MyMoneyException *e) {
    delete e;
    try {
      file->addCurrency(MyMoneySecurity(id, name, sym, partsPerUnit, smallestCashFraction, smallestAccountFraction));
      if(date.isValid()) {
        file->addPrice(price);
      }
      ft.commit();
    } catch(MyMoneyException *e) {
      qDebug("Error loading currency: %s",qPrintable( e->what()));
      delete e;
    }
  }
}

void KMyMoneyView::loadAncientCurrencies(void)
{
  loadAncientCurrency("ATS", i18n("Austrian Schilling"), "ÖS", QDate(1998,12,31), MyMoneyMoney(10000, 137603), "EUR");
  loadAncientCurrency("DEM", i18n("German Mark"), "DM", QDate(1998,12,31), MyMoneyMoney(100000, 195583), "EUR");
  loadAncientCurrency("FRF", i18n("French Franc"), "FF", QDate(1998,12,31), MyMoneyMoney(100000, 655957), "EUR");
  loadAncientCurrency("ITL", i18n("Italian Lira"), QChar(0x20A4), QDate(1998,12,31), MyMoneyMoney(100, 193627), "EUR");
  loadAncientCurrency("ESP", i18n("Spanish Peseta"), QString(), QDate(1998,12,31), MyMoneyMoney(1000, 166386), "EUR");
  loadAncientCurrency("NLG", i18n("Dutch Guilder"), QString(), QDate(1998,12,31), MyMoneyMoney(100000, 220371), "EUR");
  loadAncientCurrency("BEF", i18n("Belgian Franc"), "Fr", QDate(1998,12,31), MyMoneyMoney(10000, 403399), "EUR");
  loadAncientCurrency("LUF", i18n("Luxembourg Franc"), "Fr", QDate(1998,12,31), MyMoneyMoney(10000, 403399), "EUR");
  loadAncientCurrency("PTE", i18n("Portuguese Escudo"), QString(), QDate(1998,12,31), MyMoneyMoney(1000, 200482), "EUR");
  loadAncientCurrency("IEP", i18n("Irish Pound"), QChar(0x00A3), QDate(1998,12,31), MyMoneyMoney(1000000, 787564), "EUR");
  loadAncientCurrency("FIM", i18n("Finnish Markka"), QString(), QDate(1998,12,31), MyMoneyMoney(100000, 594573), "EUR");
  loadAncientCurrency("GRD", i18n("Greek Drachma"), QChar(0x20AF), QDate(1998,12,31), MyMoneyMoney(100, 34075), "EUR");

  loadAncientCurrency("ROL", i18n("Romanian Leu"), "ROL", QDate(2005,6,30), MyMoneyMoney(1, 10000), "RON");

  loadAncientCurrency("RUR", i18n("Russian Ruble (old)"), "RUR", QDate(1998, 1, 1), MyMoneyMoney(1, 1000), "RUB");

  loadAncientCurrency("SIT", i18n("Slovenian Tolar"), "SIT", QDate(2006,12,31), MyMoneyMoney(100, 23964), "EUR");

  // Source: http://www.tf-portfoliosolutions.net/products/turkishlira.aspx
  loadAncientCurrency("TRL", i18n("Turkish Lira"), "TL", QDate(2004,12,31), MyMoneyMoney(1,1000000), "TRY");

  // Source: http://www.focus.de/finanzen/news/malta-und-zypern_aid_66058.html
  loadAncientCurrency("MTL", i18n("Maltese Lira"), "MTL", QDate(2008,1,1), MyMoneyMoney(429300,1000000), "EUR");
  loadAncientCurrency("CYP", i18n("Cyprus Pound"), QString("C%1").arg(QChar(0x00A3)), QDate(2008,1,1), MyMoneyMoney(585274,1000000), "EUR");

  // Source: http://www.focus.de/finanzen/news/waehrungszone-slowakei-ist-neuer-euro-staat_aid_359025.html
  loadAncientCurrency("SKK", i18n("Slovak Koruna"), "SKK", QDate(2008,12,31), MyMoneyMoney(1000,30126), "EUR");
}

void KMyMoneyView::viewUp(void)
{
  if (!fileOpen())
    return;
}

void KMyMoneyView::viewAccountList(const QString& /*selectAccount*/)
{
  if(m_accountsViewFrame != currentPage())
    showPage(m_accountsViewFrame);
  m_accountsView->show();
}

void KMyMoneyView::slotRefreshViews()
{
  // turn off sync between ledger and investment view
  disconnect(m_investmentView, SIGNAL(accountSelected(const MyMoneyObject&)), m_ledgerView, SLOT(slotSelectAccount(const MyMoneyObject&)));
  disconnect(m_ledgerView, SIGNAL(accountSelected(const MyMoneyObject&)), m_investmentView, SLOT(slotSelectAccount(const MyMoneyObject&)));


  // TODO turn sync between ledger and investment view if selected by user
  if(KMyMoneyGlobalSettings::syncLedgerInvestment()) {
    connect(m_investmentView, SIGNAL(accountSelected(const MyMoneyObject&)), m_ledgerView, SLOT(slotSelectAccount(const MyMoneyObject&)));
    connect(m_ledgerView, SIGNAL(accountSelected(const MyMoneyObject&)), m_investmentView, SLOT(slotSelectAccount(const MyMoneyObject&)));
  }

  showTitleBar(KMyMoneyGlobalSettings::showTitleBar());

  m_accountsView->slotLoadAccounts();
  m_institutionsView->slotLoadAccounts();
  m_categoriesView->slotLoadAccounts();
  m_payeesView->slotLoadPayees();
  m_ledgerView->slotLoadView();
  m_budgetView->slotRefreshView();
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
  kmymoney2->progressCallback(current, total, msg);
}

void KMyMoneyView::slotRememberPage(const QModelIndex current, const QModelIndex)
{
  KSharedConfigPtr config = KGlobal::config();
  KConfigGroup grp = config->group("Last Use Settings");
  grp.writeEntry("LastViewSelected", current.row());
  config->sync();
}

/* DO NOT ADD code to this function or any of it's called ones.
   Instead, create a new function, fixFile_n, and modify the initializeStorage()
   logic above to call it */

void KMyMoneyView::fixFile_2(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits( false );
  QList<MyMoneyTransaction> transactionList;
  file->transactionList(transactionList, filter);

  // scan the transactions and modify transactions with two splits
  // which reference an account and a category to have the memo text
  // of the account.
  QList<MyMoneyTransaction>::Iterator it_t;
  int count = 0;
  for(it_t = transactionList.begin(); it_t != transactionList.end(); ++it_t) {
    if((*it_t).splitCount() == 2) {
      QString accountId;
      QString categoryId;
      QString accountMemo;
      QString categoryMemo;
      const QList<MyMoneySplit>& splits = (*it_t).splits();
      QList<MyMoneySplit>::const_iterator it_s;
      for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if(acc.isIncomeExpense()) {
          categoryId = (*it_s).id();
          categoryMemo = (*it_s).memo();
        } else {
          accountId = (*it_s).id();
          accountMemo = (*it_s).memo();
        }
      }

      if(!accountId.isEmpty() && !categoryId.isEmpty()
      && accountMemo != categoryMemo) {
        MyMoneyTransaction t(*it_t);
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

void KMyMoneyView::fixFile_1(void)
{
  // we need to fix reports. If the account filter list contains
  // investment accounts, we need to add the stock accounts to the list
  // as well if we don't have the expert mode enabled
  if(!KMyMoneyGlobalSettings::expertMode()) {
    try {
      QList<MyMoneyReport> reports = MyMoneyFile::instance()->reportList();
      QList<MyMoneyReport>::iterator it_r;
      for(it_r = reports.begin(); it_r != reports.end(); ++it_r) {
        QStringList list;
        (*it_r).accounts(list);
        QStringList missing;
        QStringList::const_iterator it_a, it_b;
        for(it_a = list.constBegin(); it_a != list.constEnd(); ++it_a) {
          MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
          if(acc.accountType() == MyMoneyAccount::Investment) {
            for(it_b = acc.accountList().begin(); it_b != acc.accountList().end(); ++it_b) {
              if(!list.contains(*it_b)) {
                missing.append(*it_b);
              }
            }
          }
        }
        if(!missing.isEmpty()) {
          (*it_r).addAccount(missing);
          MyMoneyFile::instance()->modifyReport(*it_r);
        }
      }
    } catch(MyMoneyException* e) {
      delete e;
    }
  }
}

#if 0
  if(!m_accountsView->allItemsSelected()) {
    // retrieve a list of selected accounts
          QStringList list;
          m_accountsView->selectedItems(list);

    // if we're not in expert mode, we need to make sure
    // that all stock accounts for the selected investment
    // account are also selected
          if(!KMyMoneyGlobalSettings::expertMode()) {
            QStringList missing;
            QStringList::const_iterator it_a, it_b;
            for(it_a = list.begin(); it_a != list.end(); ++it_a) {
              MyMoneyAccount acc = MyMoneyFile::instance()->account(*it_a);
              if(acc.accountType() == MyMoneyAccount::Investment) {
                for(it_b = acc.accountList().begin(); it_b != acc.accountList().end(); ++it_b) {
                  if(!list.contains(*it_b)) {
                    missing.append(*it_b);
                  }
                }
              }
            }
            list += missing;
          }

          m_filter.addAccount(list);
        }

#endif





void KMyMoneyView::fixFile_0(void)
{
  /* (Ace) I am on a crusade against file fixups.  Whenever we have to fix the
   * file, it is really a warning.  So I'm going to print a debug warning, and
   * then go track them down when I see them to figure out how they got saved
   * out needing fixing anyway.
   */

  MyMoneyFile* file = MyMoneyFile::instance();
  QList<MyMoneyAccount> accountList;
  file->accountList(accountList);
  ::timetrace("Have account list");
  QList<MyMoneyAccount>::Iterator it_a;
  QList<MyMoneySchedule> scheduleList = file->scheduleList();
  ::timetrace("Have schedule list");
  QList<MyMoneySchedule>::Iterator it_s;

  MyMoneyAccount equity = file->equity();
  MyMoneyAccount asset = file->asset();
  bool equityListEmpty = equity.accountList().count() == 0;

  ::timetrace("Fix accounts start");
  for(it_a = accountList.begin(); it_a != accountList.end(); ++it_a) {
    if((*it_a).accountType() == MyMoneyAccount::Loan
    || (*it_a).accountType() == MyMoneyAccount::AssetLoan) {
      fixLoanAccount_0(*it_a);
    }
    // until early before 0.8 release, the equity account was not saved to
    // the file. If we have an equity account with no sub-accounts but
    // find and equity account that has equity() as it's parent, we reparent
    // this account. Need to move it to asset() first, because otherwise
    // MyMoneyFile::reparent would act as NOP.
    if(equityListEmpty && (*it_a).accountType() == MyMoneyAccount::Equity) {
      if((*it_a).parentAccountId() == equity.id()) {
        MyMoneyAccount acc = *it_a;
        // tricky, force parent account to be empty so that we really
        // can re-parent it
        acc.setParentAccountId(QString());
        file->reparentAccount(acc, equity);
        kDebug(2) << __func__ << " fixed account " << acc.id() << " reparented to " << equity.id();
      }
    }
  }

  ::timetrace("Fix schedules start");
  for(it_s = scheduleList.begin(); it_s != scheduleList.end(); ++it_s) {
    fixSchedule_0(*it_s);
  }

  ::timetrace("Fix transactions start");
  fixTransactions_0();
  ::timetrace("Fix transactions done");
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
    for(it_s = splitList.constBegin(); it_s != splitList.constEnd(); ++it_s) {
      // the first split is always the account on which this transaction operates
      // and if the transaction commodity is not set, we take this
      if(it_s == splitList.constBegin() && t.commodity().isEmpty()) {
        kDebug(2) << __func__ << " " << t.id() << " has no commodity";
        try {
          MyMoneyAccount acc = MyMoneyFile::instance()->account((*it_s).accountId());
          t.setCommodity(acc.currencyId());
          updated = true;
        } catch(MyMoneyException *e) {
          delete e;
        }
      }
      // make sure the account exists. If not, remove the split
      try {
        MyMoneyFile::instance()->account((*it_s).accountId());
      } catch(MyMoneyException *e) {
        kDebug(2) << __func__ << " " << sched.id() << " " << (*it_s).id() << " removed, because account '" << (*it_s).accountId() << "' does not exist.";
        t.removeSplit(*it_s);
        updated = true;
        delete e;
      }
      if((*it_s).reconcileFlag() != MyMoneySplit::NotReconciled) {
        kDebug(2) << __func__ << " " << sched.id() << " " << (*it_s).id() << " should be 'not reconciled'";
        MyMoneySplit split = *it_s;
        split.setReconcileDate(QDate());
        split.setReconcileFlag(MyMoneySplit::NotReconciled);
        t.modifySplit(split);
        updated = true;
      }
      // the schedule logic used to operate only on the value field.
      // This is now obsolete.
      if((*it_s).shares().isZero() && !(*it_s).value().isZero()) {
        MyMoneySplit split = *it_s;
        split.setShares(split.value());
        t.modifySplit(split);
        updated = true;
      }
    }

    // If there have been changes, update the schedule and
    // the engine data.
    if(updated) {
      sched.setTransaction(t);
      MyMoneyFile::instance()->modifySchedule(sched);
    }
  } catch(MyMoneyException *e) {
    qWarning("Unable to update broken schedule: %s", qPrintable(e->what()));
    delete e;
  }
}

void KMyMoneyView::fixLoanAccount_0(MyMoneyAccount acc)
{
  if(acc.value("final-payment").isEmpty()
  || acc.value("term").isEmpty()
  || acc.value("periodic-payment").isEmpty()
  || acc.value("loan-amount").isEmpty()
  || acc.value("interest-calculation").isEmpty()
  || acc.value("schedule").isEmpty()
  || acc.value("fixed-interest").isEmpty()) {
    KMessageBox::information(this,
        i18n("<p>The account \"%1\" was previously created as loan account but some information is missing.</p><p>The new loan wizard will be started to collect all relevant information.</p><p>Please use a KMyMoney version >= 0.8.7 and < 0.9 to correct the problem.</p>"
             ,acc.name()),
        i18n("Account problem"));

    throw new MYMONEYEXCEPTION("Fix LoanAccount0 not supported anymore");
  }
}

void KMyMoneyView::createSchedule(MyMoneySchedule newSchedule, MyMoneyAccount& newAccount)
{
  // Add the schedule only if one exists
  //
  // Remember to modify the first split to reference the newly created account
  if (!newSchedule.name().isEmpty())
  {
    MyMoneyFileTransaction ft;
    try
    {
      // We assume at least 2 splits in the transaction
      MyMoneyTransaction t = newSchedule.transaction();
      if(t.splitCount() < 2) {
        throw new MYMONEYEXCEPTION("Transaction for schedule has less than 2 splits!");
      }
      // now search the split that does not have an account reference
      // and set it up to be the one of the account we just added
      // to the account pool. Note: the schedule code used to leave
      // this always the first split, but the loan code leaves it as
      // the second one. So I thought, searching is a good alternative ....
      QList<MyMoneySplit>::ConstIterator it_s;
      for(it_s = t.splits().constBegin(); it_s != t.splits().constEnd(); ++it_s) {
        if((*it_s).accountId().isEmpty()) {
          MyMoneySplit s = (*it_s);
          s.setAccountId(newAccount.id());
          t.modifySplit(s);
          break;
        }
      }
      newSchedule.setTransaction(t);

      MyMoneyFile::instance()->addSchedule(newSchedule);

      // in case of a loan account, we keep a reference to this
      // schedule in the account
      if(newAccount.isLoan()) {
        newAccount.setValue("schedule", newSchedule.id());
        MyMoneyFile::instance()->modifyAccount(newAccount);
      }
      ft.commit();
    }
    catch (MyMoneyException *e)
    {
      KMessageBox::information(this, i18n("Unable to add schedule: "), e->what());
      delete e;
    }
  }
}

void KMyMoneyView::fixTransactions_0(void)
{
  MyMoneyFile* file = MyMoneyFile::instance();

#if 0
  ::timetrace("Start alloc memory");
  int * p = new int [10000];
  delete p;
  ::timetrace("Done alloc memory");
#endif

  ::timetrace("fixTransactions: get schedule list");
  QList<MyMoneySchedule> scheduleList = file->scheduleList();
  ::timetrace("fixTransactions: get transaction list");
  MyMoneyTransactionFilter filter;
  filter.setReportAllSplits( false );
  QList<MyMoneyTransaction> transactionList;
  file->transactionList(transactionList, filter);
  ::timetrace("fixTransactions: have list");

  QList<MyMoneySchedule>::Iterator it_x;
  QStringList interestAccounts;

  KMSTATUS(i18n("Fix transactions"));
  kmymoney2->slotStatusProgressBar(0, scheduleList.count() + transactionList.count());

  int cnt = 0;
  // scan the schedules to find interest accounts
  for(it_x = scheduleList.begin(); it_x != scheduleList.end(); ++it_x) {
    MyMoneyTransaction t = (*it_x).transaction();
    QList<MyMoneySplit>::ConstIterator it_s;
    QStringList accounts;
    bool hasDuplicateAccounts = false;

    for(it_s = t.splits().constBegin(); it_s != t.splits().constEnd(); ++it_s) {
      if(accounts.contains((*it_s).accountId())) {
        hasDuplicateAccounts = true;
        kDebug(2) << __func__ << " " << t.id() << " has multiple splits with account " << (*it_s).accountId();
      } else {
        accounts << (*it_s).accountId();
      }

      if((*it_s).action() == MyMoneySplit::ActionInterest) {
        if(interestAccounts.contains((*it_s).accountId()) == 0) {
          interestAccounts << (*it_s).accountId();
        }
      }
    }
    if(hasDuplicateAccounts) {
      fixDuplicateAccounts_0(t);
    }
    ++cnt;
    if(!(cnt % 10))
      kmymoney2->slotStatusProgressBar(cnt);
  }

  ::timetrace("fixTransactions: start loop");
  // scan the transactions and modify loan transactions
  QList<MyMoneyTransaction>::Iterator it_t;
  for(it_t = transactionList.begin(); it_t != transactionList.end(); ++it_t) {
    const char *defaultAction = 0;
    QList<MyMoneySplit> splits = (*it_t).splits();
    QList<MyMoneySplit>::Iterator it_s;
    QStringList accounts;

    // check if base commodity is set. if not, set baseCurrency
    if((*it_t).commodity().isEmpty()) {
      kDebug(2) << __func__ << " " << (*it_t).id() << " has no base currency";
      (*it_t).setCommodity(file->baseCurrency().id());
      file->modifyTransaction(*it_t);
    }

    bool isLoan = false;
    // Determine default action
    if((*it_t).splitCount() == 2) {
      // check for transfer
      int accountCount = 0;
      MyMoneyMoney val;
      for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        MyMoneyAccount acc = file->account((*it_s).accountId());
        if(acc.accountGroup() == MyMoneyAccount::Asset
        || acc.accountGroup() == MyMoneyAccount::Liability) {
          val = (*it_s).value();
          accountCount++;
          if(acc.accountType() == MyMoneyAccount::Loan
          || acc.accountType() == MyMoneyAccount::AssetLoan)
            isLoan = true;
        } else
          break;
      }
      if(accountCount == 2) {
        if(isLoan)
          defaultAction = MyMoneySplit::ActionAmortization;
        else
          defaultAction = MyMoneySplit::ActionTransfer;
      } else {
        if(val.isNegative())
          defaultAction = MyMoneySplit::ActionWithdrawal;
        else
          defaultAction = MyMoneySplit::ActionDeposit;
      }
    }

    isLoan = false;
    for(it_s = splits.begin(); defaultAction == 0 && it_s != splits.end(); ++it_s) {
      MyMoneyAccount acc = file->account((*it_s).accountId());
      MyMoneyMoney val = (*it_s).value();
      if(acc.accountGroup() == MyMoneyAccount::Asset
      || acc.accountGroup() == MyMoneyAccount::Liability) {
        if(!val.isPositive())
          defaultAction = MyMoneySplit::ActionWithdrawal;
        else
          defaultAction = MyMoneySplit::ActionDeposit;
      }
    }

#if 0
    // Check for correct actions in transactions referencing credit cards
    bool needModify = false;
    // The action fields are actually not used anymore in the ledger view logic
    // so we might as well skip this whole thing here!
    for(it_s = splits.begin(); needModify == false && it_s != splits.end(); ++it_s) {
      MyMoneyAccount acc = file->account((*it_s).accountId());
      MyMoneyMoney val = (*it_s).value();
      if(acc.accountType() == MyMoneyAccount::CreditCard) {
        if(val < 0 && (*it_s).action() != MyMoneySplit::ActionWithdrawal && (*it_s).action() != MyMoneySplit::ActionTransfer )
          needModify = true;
        if(val >= 0 && (*it_s).action() != MyMoneySplit::ActionDeposit && (*it_s).action() != MyMoneySplit::ActionTransfer)
          needModify = true;
      }
    }

    // (Ace) Extended the #endif down to cover this conditional, because as-written
    // it will ALWAYS be skipped.

    if(needModify == true) {
      for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
        (*it_s).setAction(defaultAction);
        (*it_t).modifySplit(*it_s);
        file->modifyTransaction(*it_t);
      }
      splits = (*it_t).splits();    // update local copy
      qDebug("Fixed credit card assignment in %s", (*it_t).id().data());
    }
#endif

    bool hasDuplicateAccounts = false;
    // Check for correct assignment of ActionInterest in all splits
    // and check if there are any duplicates in this transactions
    for(it_s = splits.begin(); it_s != splits.end(); ++it_s) {
      MyMoneyAccount splitAccount = file->account((*it_s).accountId());
      if(accounts.contains((*it_s).accountId())) {
        hasDuplicateAccounts = true;
      } else {
        accounts << (*it_s).accountId();
      }
      // if this split references an interest account, the action
      // must be of type ActionInterest
      if(interestAccounts.contains((*it_s).accountId())) {
        if((*it_s).action() != MyMoneySplit::ActionInterest) {
          kDebug(2) << __func__ << " " << (*it_t).id() << " contains an interest account (" << (*it_s).accountId() << ") but does not have ActionInterest";
          (*it_s).setAction(MyMoneySplit::ActionInterest);
          (*it_t).modifySplit(*it_s);
          file->modifyTransaction(*it_t);
          qDebug("Fixed interest action in %s", qPrintable((*it_t).id()));
        }
      // if it does not reference an interest account, it must not be
      // of type ActionInterest
      } else {
        if((*it_s).action() == MyMoneySplit::ActionInterest) {
          kDebug(2) << __func__ << " " << (*it_t).id() << " does not contain an interest account so it should not have ActionInterest";
          (*it_s).setAction(defaultAction);
          (*it_t).modifySplit(*it_s);
          file->modifyTransaction(*it_t);
          qDebug("Fixed interest action in %s", qPrintable((*it_t).id()));
        }
      }

      // check that for splits referencing an account that has
      // the same currency as the transactions commodity the value
      // and shares field are the same.
      if((*it_t).commodity() == splitAccount.currencyId()
      && (*it_s).value() != (*it_s).shares()) {
        kDebug(2) << __func__ << " " << (*it_t).id() << " " << (*it_s).id() << " uses the transaction currency, but shares != value";
        (*it_s).setShares((*it_s).value());
        (*it_t).modifySplit(*it_s);
        file->modifyTransaction(*it_t);
      }

      // fix the shares and values to have the correct fraction
      if(!splitAccount.isInvest()) {
        try {
          int fract = splitAccount.fraction();
          if((*it_s).shares() != (*it_s).shares().convert(fract)) {
            qDebug("adjusting fraction in %s,%s", qPrintable((*it_t).id()), qPrintable((*it_s).id()));
            (*it_s).setShares((*it_s).shares().convert(fract));
            (*it_s).setValue((*it_s).value().convert(fract));
            (*it_t).modifySplit(*it_s);
            file->modifyTransaction(*it_t);
          }
        } catch(MyMoneyException* e) {
          qDebug("Missing security '%s', split not altered", qPrintable(splitAccount.currencyId()));
          delete e;
        }
      }
    }

/*
    // if there are at least two splits referencing the same account,
    // we need to combine them into one and get rid of the others
    if(hasDuplicateAccounts) {
      fixDuplicateAccounts(*it_t);
    }
*/
    ++cnt;
    if(!(cnt % 10))
      kmymoney2->slotStatusProgressBar(cnt);
  }

  kmymoney2->slotStatusProgressBar(-1, -1);
}

void KMyMoneyView::fixDuplicateAccounts_0(MyMoneyTransaction& t)
{
  qDebug("Duplicate account in transaction %s", qPrintable(t.id()));
}

void KMyMoneyView::slotPrintView(void)
{
  if(m_reportsViewFrame == currentPage())
    m_reportsView->slotPrintView();
  else if(m_homeViewFrame == currentPage())
    m_homeView->slotPrintView();
}

KMyMoneyViewBase* KMyMoneyView::addBasePage(const QString& title, const QString& icon)
{
  KMyMoneyViewBase* viewBase = new KMyMoneyViewBase(this, title, title);
  const int iconSize = (KMyMoneyGlobalSettings::iconSize()+1)*16;
  KPageWidgetItem* frm = m_model->addPage(viewBase, title);
  frm->setIcon(KIcon(icon));
  frm->setHeader(QString("")); // hide the header and let the title bar do it's job
  return viewBase;
}


/* ------------------------------------------------------------------------ */
/*                 KMyMoneyViewBase                                         */
/* ------------------------------------------------------------------------ */

// ----------------------------------------------------------------------------
// QT Includes

#include <QLayout>
#include <q3vbox.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneytitlelabel.h"

class KMyMoneyViewBase::Private {
  public:
    Q3Frame* m_titleLine;
    KMyMoneyTitleLabel* m_titleLabel;
    QVBoxLayout* m_viewLayout;
};

KMyMoneyViewBase::KMyMoneyViewBase(QWidget* parent, const QString& name, const QString& title) :
  QWidget(parent),
  d(new Private)
{
  setAccessibleName(name);
  d->m_viewLayout = new QVBoxLayout(this);
  d->m_viewLayout->setSpacing( 6 );
  d->m_viewLayout->setMargin( 0 );

  d->m_titleLabel = new KMyMoneyTitleLabel(this);
  d->m_titleLabel->setObjectName("titleLabel");
  d->m_titleLabel->setMinimumSize( QSize( 100, 30 ) );
  d->m_titleLabel->setRightImageFile("pics/titlelabel_background.png" );
  d->m_titleLabel->setText(title);
  d->m_viewLayout->addWidget( d->m_titleLabel );

#if 0
  d->m_titleLine = new Q3Frame( this, "titleLine" );
  d->m_titleLine->setFrameShape( Q3Frame::HLine );
  d->m_titleLine->setFrameShadow( Q3Frame::Sunken );
  d->m_titleLine->setFrameShape( Q3Frame::HLine );
  d->m_viewLayout->addWidget( d->m_titleLine );
#endif
}

KMyMoneyViewBase::~KMyMoneyViewBase()
{
  delete d;
}

void KMyMoneyViewBase::addWidget(QWidget* w)
{
  d->m_viewLayout->addWidget(w);
}

QVBoxLayout* KMyMoneyViewBase::layout(void) const
{
  return d->m_viewLayout;
}

#include "kmymoneyview.moc"
