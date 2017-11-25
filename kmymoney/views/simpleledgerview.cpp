/***************************************************************************
                          simpleledgerview.cpp
                             -------------------
    begin                : Sat Aug 8 2015
    copyright            : (C) 2015 by Thomas Baumgart
    email                : Thomas Baumgart <tbaumgart@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simpleledgerview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTabBar>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ledgerviewpage.h"
#include "models.h"
#include "accountsmodel.h"
#include "kmymoneyaccountcombo.h"
#include "ui_simpleledgerview.h"
#include "icons/icons.h"
#include "kmymoneyview.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "modelenums.h"

using namespace Icons;

class SimpleLedgerView::Private
{
public:
  Private(SimpleLedgerView* p)
  : parent(p)
  , ui(new Ui_SimpleLedgerView)
  , accountsModel(new AccountNamesFilterProxyModel(parent))
  , newTabWidget(0)
  , lastIdx(-1)
  , inModelUpdate(false)
  , m_needLoad(true)
  {}

  ~Private() {}

  SimpleLedgerView*             parent;
  Ui_SimpleLedgerView*          ui;
  AccountNamesFilterProxyModel* accountsModel;
  QWidget*                      newTabWidget;
  int                           lastIdx;
  bool                          inModelUpdate;
  bool                          m_needLoad;
};


SimpleLedgerView::SimpleLedgerView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview)
  : QWidget(nullptr)
  , d(new Private(this))
  , m_kmymoney(kmymoney)
  , m_kmymoneyview(kmymoneyview)
{
}

SimpleLedgerView::~SimpleLedgerView()
{
  if (!d->m_needLoad)
    delete d->ui;
  delete d;
}

void SimpleLedgerView::init()
{
  d->m_needLoad = false;
  d->ui->setupUi(this);
  d->ui->ledgerTab->setTabIcon(0, QIcon::fromTheme(g_Icons[Icon::ListAdd]));
  d->ui->ledgerTab->setTabText(0, QString());
  d->newTabWidget = d->ui->ledgerTab->widget(0);

  // remove close button from new page
  QTabBar* bar = d->ui->ledgerTab->findChild<QTabBar*>();
  if(bar) {
    QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, d->newTabWidget);
    QWidget *w = bar->tabButton(0, closeSide);
    bar->setTabButton(0, closeSide, 0);
    w->deleteLater();
    connect(bar, SIGNAL(tabMoved(int,int)), this, SLOT(checkTabOrder(int,int)));
  }

  connect(d->ui->accountCombo, SIGNAL(accountSelected(QString)), this, SLOT(openNewLedger(QString)));
  connect(d->ui->ledgerTab, &QTabWidget::currentChanged, this, &SimpleLedgerView::tabSelected);
  connect(Models::instance(), &Models::modelsLoaded, this, &SimpleLedgerView::updateModels);
  connect(d->ui->ledgerTab, &QTabWidget::tabCloseRequested, this, &SimpleLedgerView::closeLedger);
  connect(m_kmymoneyview, &KMyMoneyView::fileClosed, this, &SimpleLedgerView::closeLedgers);
  connect(m_kmymoneyview, &KMyMoneyView::fileOpened, this, &SimpleLedgerView::openFavoriteLedgers);

  d->accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Equity});

  d->accountsModel->setHideEquityAccounts(false);
  auto const model = Models::instance()->accountsModel();
  d->accountsModel->setSourceModel(model);
  d->accountsModel->setSourceColumns(model->getColumns());
  d->accountsModel->sort((int)eAccountsModel::Column::Account);
  d->ui->accountCombo->setModel(d->accountsModel);

  tabSelected(0);
  updateModels();
  openFavoriteLedgers();
}

void SimpleLedgerView::openNewLedger(QString accountId)
{
  if(d->inModelUpdate || accountId.isEmpty())
    return;

  LedgerViewPage* view = 0;
  // check if ledger is already opened
  for(int idx=0; idx < d->ui->ledgerTab->count()-1; ++idx) {
    view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->widget(idx));
    if(view) {
      if(accountId == view->accountId()) {
        d->ui->ledgerTab->setCurrentIndex(idx);
        return;
      }
    }
  }

  // need a new tab, we insert it before the rightmost one
  QModelIndex index = Models::instance()->accountsModel()->accountById(accountId);
  if(index.isValid()) {

    // create new ledger view page
    MyMoneyAccount acc = Models::instance()->accountsModel()->data(index, (int)eAccountsModel::Role::Account).value<MyMoneyAccount>();
    view = new LedgerViewPage(this);
    view->setAccount(acc);

    view->setShowEntryForNewTransaction();
    /// @todo setup current global setting for form visibility
    // view->showTransactionForm(...);

    // insert new ledger view page in tab view
    int newIdx = d->ui->ledgerTab->insertTab(d->ui->ledgerTab->count()-1, view, acc.name());
    d->ui->ledgerTab->setCurrentIndex(d->ui->ledgerTab->count()-1);
    d->ui->ledgerTab->setCurrentIndex(newIdx);
  }
}

void SimpleLedgerView::tabSelected(int idx)
{
  // qDebug() << "tabSelected" << idx << (d->ui->ledgerTab->count()-1);
  if(idx != (d->ui->ledgerTab->count()-1)) {
    d->lastIdx = idx;
  }
}

void SimpleLedgerView::updateModels()
{
  d->inModelUpdate = true;
  // d->ui->accountCombo->
  d->ui->accountCombo->expandAll();
  d->ui->accountCombo->setSelected(MyMoneyFile::instance()->asset().id());
  d->inModelUpdate = false;
}

void SimpleLedgerView::closeLedger(int idx)
{
  // don't react on the close request for the new ledger function
  if(idx != (d->ui->ledgerTab->count()-1)) {
    d->ui->ledgerTab->removeTab(idx);
  }
}

void SimpleLedgerView::checkTabOrder(int from, int to)
{
  if(d->inModelUpdate)
    return;

  QTabBar* bar = d->ui->ledgerTab->findChild<QTabBar*>();
  if(bar) {
    const int rightMostIdx = d->ui->ledgerTab->count()-1;

    if(from == rightMostIdx) {
      // someone tries to move the new account tab away from the rightmost position
      d->inModelUpdate = true;
      bar->moveTab(to, from);
      d->inModelUpdate = false;
    }
  }
}

void SimpleLedgerView::showTransactionForm(bool show)
{
  emit showForms(show);
}

void SimpleLedgerView::closeLedgers()
{
  int tabCount = d->ui->ledgerTab->count();
  // check that we have a least one tab that can be closed
  if(tabCount > 1) {
    // we keep the tab with the selector open at all times
    // which is located in the right most position
    --tabCount;
    do {
      --tabCount;
      closeLedger(tabCount);
    } while(tabCount > 0);
  }
}

void SimpleLedgerView::openFavoriteLedgers()
{
  AccountsModel* model = Models::instance()->accountsModel();
  QModelIndex start = model->index(0, 0);
  QModelIndexList indexes = model->match(start, (int)eAccountsModel::Role::Favorite, QVariant(true), -1, Qt::MatchRecursive);

  // indexes now has a list of favorite accounts but two entries for each.
  // that doesn't matter here, since openNewLedger() can handle duplicates
  Q_FOREACH(QModelIndex index, indexes) {
    openNewLedger(model->data(index, (int)eAccountsModel::Role::ID).toString());
  }
  d->ui->ledgerTab->setCurrentIndex(0);
}

void SimpleLedgerView::showEvent(QShowEvent* event)
{
  if (d->m_needLoad)
    init();

  // don't forget base class implementation
  QWidget::showEvent(event);
}
