/*
 * Copyright 2015-2019  Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "simpleledgerview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QTabBar>
#include <QToolButton>
#include <QUrl>
#include <QDesktopServices>

// ----------------------------------------------------------------------------
// KDE Includes


// ----------------------------------------------------------------------------
// Project Includes

#include "ui_simpleledgerview.h"
#include "kmymoneyviewbase_p.h"
#include "ledgerviewpage.h"
#include "accountsmodel.h"
#include "institutionsmodel.h"
#include "kmymoneyaccountcombo.h"
#include "icons.h"
#include "mymoneyfile.h"
#include "mymoneyaccount.h"
#include "mymoneyinstitution.h"
#include "mymoneyenums.h"
#include "modelenums.h"
#include "kmymoneysettings.h"

using namespace Icons;

class SimpleLedgerViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(SimpleLedgerView)

public:
  explicit SimpleLedgerViewPrivate(SimpleLedgerView* qq)
  : q_ptr(qq)
  , ui(new Ui_SimpleLedgerView)
  , accountsModel(nullptr)
  , newTabWidget(nullptr)
  , webSiteButton(nullptr)
  , lastIdx(-1)
  , inModelUpdate(false)
  , m_needLoad(true)
  {}

  ~SimpleLedgerViewPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(SimpleLedgerView);
    m_needLoad = false;
    ui->setupUi(q);
    ui->ledgerTab->setTabIcon(0, Icons::get(Icon::ListAdd));
    ui->ledgerTab->setTabText(0, QString());
    newTabWidget = ui->ledgerTab->widget(0);

    accountsModel = new AccountNamesFilterProxyModel(q);
    q->slotSettingsChanged();

    // remove close button from new page
    QTabBar* bar = ui->ledgerTab->findChild<QTabBar*>();
    if(bar) {
      QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)q->style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, newTabWidget);
      QWidget *w = bar->tabButton(0, closeSide);
      bar->setTabButton(0, closeSide, 0);
      w->deleteLater();
      q->connect(bar, SIGNAL(tabMoved(int,int)), q, SLOT(checkTabOrder(int,int)));
    }

    webSiteButton = new QToolButton;
    ui->ledgerTab->setCornerWidget(webSiteButton);
    q->connect(webSiteButton, &QToolButton::pressed, q,
            [=] {
              QDesktopServices::openUrl(webSiteUrl);
            });

    q->connect(ui->accountCombo, SIGNAL(accountSelected(QString)), q, SLOT(openNewLedger(QString)));
    q->connect(ui->ledgerTab, &QTabWidget::currentChanged, q, &SimpleLedgerView::tabSelected);
    q->connect(MyMoneyFile::instance(), &MyMoneyFile::modelsLoaded, q, &SimpleLedgerView::updateModels);
    q->connect(ui->ledgerTab, &QTabWidget::tabCloseRequested, q, &SimpleLedgerView::closeLedger);
    // we reload the icon if the institution data changed
    q->connect(MyMoneyFile::instance()->institutionsModel(), &InstitutionsModel::dataChanged, q, &SimpleLedgerView::setupCornerWidget);

    accountsModel->addAccountGroup(QVector<eMyMoney::Account::Type> {eMyMoney::Account::Type::Asset, eMyMoney::Account::Type::Liability, eMyMoney::Account::Type::Equity});

    accountsModel->setHideEquityAccounts(false);
    auto const model = MyMoneyFile::instance()->accountsModel();
    accountsModel->setSourceModel(model);
    accountsModel->sort(AccountsModel::Column::AccountName);
    ui->accountCombo->setModel(accountsModel);

    q->tabSelected(0);
    q->updateModels();
    q->openFavoriteLedgers();
  }

  SimpleLedgerView*             q_ptr;
  Ui_SimpleLedgerView*          ui;
  AccountNamesFilterProxyModel* accountsModel;
  QWidget*                      newTabWidget;
  QToolButton*                  webSiteButton;
  QUrl                          webSiteUrl;
  int                           lastIdx;
  bool                          inModelUpdate;
  bool                          m_needLoad;
};


SimpleLedgerView::SimpleLedgerView(QWidget *parent) :
    KMyMoneyViewBase(*new SimpleLedgerViewPrivate(this), parent)
{
}

SimpleLedgerView::~SimpleLedgerView()
{
}

void SimpleLedgerView::openNewLedger(QString accountId)
{
  openNewLedger(accountId, true);
}

void SimpleLedgerView::openNewLedger(QString accountId, bool makeCurrentLedger)
{
  Q_D(SimpleLedgerView);
  if(d->inModelUpdate || accountId.isEmpty())
    return;

  // in case a stock account is selected, we switch to the parent which
  // is the investment account
  MyMoneyAccount acc = MyMoneyFile::instance()->accountsModel()->itemById(accountId);
  if (acc.isInvest()) {
    acc = MyMoneyFile::instance()->accountsModel()->itemById(acc.parentAccountId());
    accountId = acc.id();
  }

  LedgerViewPage* view = 0;
  // check if ledger is already opened
  for(int idx = 0; idx < d->ui->ledgerTab->count()-1; ++idx) {
    view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->widget(idx));
    if(view) {
      if(accountId == view->accountId()) {
        d->ui->ledgerTab->setCurrentIndex(idx);
        return;
      }
    }
  }

  // need a new tab, we insert it before the rightmost one
  if(!acc.id().isEmpty()) {

    QString configGroupName;
    switch(acc.accountType()) {
      case eMyMoney::Account::Type::Investment:
        configGroupName = QStringLiteral("InvestmentLedger");
        break;
      default:
        configGroupName = QStringLiteral("StandardLedger");
        break;
    }
    // create new ledger view page
    view = new LedgerViewPage(this, configGroupName);
    view->setAccount(acc);
    view->setShowEntryForNewTransaction();

    /// @todo setup current global setting for form visibility
    // view->showTransactionForm(...);

    // insert new ledger view page in tab view
    int newIdx = d->ui->ledgerTab->insertTab(d->ui->ledgerTab->count()-1, view, acc.name());
    if (makeCurrentLedger) {
      d->ui->ledgerTab->setCurrentIndex(d->ui->ledgerTab->count()-1);
      d->ui->ledgerTab->setCurrentIndex(newIdx);
    }
    connect(this, &SimpleLedgerView::settingsChanged, view, &LedgerViewPage::slotSettingsChanged);
  }
}

void SimpleLedgerView::tabSelected(int idx)
{
  Q_D(SimpleLedgerView);
  // qDebug() << "tabSelected" << idx << (d->ui->ledgerTab->count()-1);
  if(idx != (d->ui->ledgerTab->count()-1)) {
    d->lastIdx = idx;
  }
  setupCornerWidget();
}

void SimpleLedgerView::updateModels()
{
  Q_D(SimpleLedgerView);
  d->inModelUpdate = true;
  // d->ui->accountCombo->
  d->ui->accountCombo->expandAll();
  d->ui->accountCombo->setSelected(MyMoneyFile::instance()->asset().id());
  d->inModelUpdate = false;
}

void SimpleLedgerView::closeLedger(int idx)
{
  Q_D(SimpleLedgerView);
  // don't react on the close request for the new ledger function
  if(idx != (d->ui->ledgerTab->count()-1)) {
    auto tab = d->ui->ledgerTab->widget(idx);
    d->ui->ledgerTab->removeTab(idx);
    delete tab;
  }
}

void SimpleLedgerView::checkTabOrder(int from, int to)
{
  Q_D(SimpleLedgerView);
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
  Q_D(SimpleLedgerView);
  if (d->m_needLoad)
    return;
  auto tabCount = d->ui->ledgerTab->count();
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
  Q_D(SimpleLedgerView);
  if (d->m_needLoad)
    return;

  AccountsModel* model = MyMoneyFile::instance()->accountsModel();
  // retrieve all items in the favorite subtree
  QModelIndexList indexes = model->match(model->index(0, 0, model->favoriteIndex()), Qt::DisplayRole, QString("*"), -1, Qt::MatchWildcard);

  // indexes now has a list of favorite accounts
  foreach (const auto idx, indexes) {
    openNewLedger(idx.data(eMyMoney::Model::Roles::IdRole).toString(), false);
  }
  d->ui->ledgerTab->setCurrentIndex(0);
}

void SimpleLedgerView::showEvent(QShowEvent* event)
{
  Q_D(SimpleLedgerView);
  if (d->m_needLoad)
    d->init();

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void SimpleLedgerView::setupCornerWidget()
{
  Q_D(SimpleLedgerView);

  // check if we already have the button, quit if not
  if (!d->webSiteButton)
    return;

  d->webSiteButton->hide();
  auto view = qobject_cast<LedgerViewPage*>(d->ui->ledgerTab->currentWidget());
  if (view) {
    const auto accountsModel = MyMoneyFile::instance()->accountsModel();
    auto index = accountsModel->indexById(view->accountId());
    if(index.isValid()) {
      // get icon name and url via account and institution object
      const auto acc = accountsModel->itemByIndex(index);
      if (!acc.institutionId().isEmpty()) {
        const auto institutionsModel = MyMoneyFile::instance()->institutionsModel();
        index = institutionsModel->indexById(acc.institutionId());
        const auto institution = institutionsModel->itemByIndex(index);
        const auto url = institution.value(QStringLiteral("url"));
        const auto iconName = institution.value(QStringLiteral("icon"));
        if (!url.isEmpty() && !iconName.isEmpty()) {
          const auto favIcon = Icons::loadIconFromApplicationCache(iconName);
          if (!favIcon.isNull()) {
            d->webSiteButton->show();
            d->webSiteButton->setIcon(favIcon);
            d->webSiteButton->setText(url);
            d->webSiteButton->setToolTip(i18n("Open website of <b>%1</b> in your browser.", institution.name()));
            d->webSiteUrl.setUrl(QString::fromLatin1("https://%1/").arg(url));
          }
        }
      }
    }
  }
}

void SimpleLedgerView::slotSettingsChanged()
{
  Q_D(SimpleLedgerView);
  if (d->accountsModel) {
    d->accountsModel->setHideClosedAccounts(KMyMoneySettings::hideClosedAccounts());
    d->accountsModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    d->accountsModel->setHideFavoriteAccounts(false);
  }
  emit settingsChanged();
}
