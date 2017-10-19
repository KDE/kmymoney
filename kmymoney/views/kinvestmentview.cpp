/***************************************************************************
                          kinvestmentview.cpp  -  description
                             -------------------
    begin                : Mon Mar 12 2007
    copyright            : (C) 2007 by Thomas Baumgart
    email                : Thomas Baumgart <ipwizard@users.sourceforge.net>
                           (C) 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kinvestmentview.h"

#include <typeinfo>

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>
#include <KActionCollection>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyaccount.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneyaccountcombo.h"
#include "knewinvestmentwizard.h"
#include "kmymoney.h"
#include "kmymoneyview.h"
#include "accountsmodel.h"
#include "models.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"
#include "icons.h"

using namespace Icons;

class KInvestmentView::Private
{
public:
  Private() :
      m_idInvAcc(QString()),
      m_accountsProxyModel(nullptr),
      m_equitiesProxyModel(nullptr),
      m_securitiesProxyModel(nullptr) {}

  QString           m_idInvAcc;

  bool              m_needReload[2];
  AccountNamesFilterProxyModel *m_accountsProxyModel;
  EquitiesFilterProxyModel     *m_equitiesProxyModel;
  SecuritiesFilterProxyModel   *m_securitiesProxyModel;
};

KInvestmentView::KInvestmentView(KMyMoneyApp *kmymoney, KMyMoneyView *kmymoneyview) :
    QWidget(nullptr),
    d(new Private),
    m_kmymoney(kmymoney),
    m_kmymoneyview(kmymoneyview),
    m_needLoad(true)
{
}

KInvestmentView::~KInvestmentView()
{
  if (!m_needLoad) {
    // save the header state of the equities list
    auto cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Equities");
    auto cfgHeader = m_equitiesTree->header()->saveState();
    auto visEColumns = d->m_equitiesProxyModel->getVisibleColumns();

    QList<int> cfgColumns;
    foreach (const auto visColumn, visEColumns)
      cfgColumns.append(static_cast<int>(visColumn));

    cfgGroup.writeEntry("HeaderState", cfgHeader);
    cfgGroup.writeEntry("ColumnsSelection", cfgColumns);

    // save the header state of the securities list
    cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Securities");
    cfgHeader = m_securitiesTree->header()->saveState();
    auto visSColumns = d->m_securitiesProxyModel->getVisibleColumns();
    cfgColumns.clear();
    foreach (const auto visColumn, visSColumns)
      cfgColumns.append(static_cast<int>(visColumn));

    cfgGroup.writeEntry("HeaderState", cfgHeader);
    cfgGroup.writeEntry("ColumnsSelection", cfgColumns);
  }

  delete d;
}

void KInvestmentView::setDefaultFocus()
{
  auto tab = static_cast<Tab>(m_tab->currentIndex());

  switch (tab) {
    case Tab::Equities:
      QTimer::singleShot(0, m_equitiesTree, SLOT(setFocus()));
      break;
    case Tab::Securities:
      QTimer::singleShot(0, m_securitiesTree, SLOT(setFocus()));
      break;
  }
}

void KInvestmentView::init()
{
  m_needLoad = false;
  setupUi(this);

  // Equities tab
  d->m_accountsProxyModel = new AccountNamesFilterProxyModel(this);
  d->m_accountsProxyModel->addAccountType(MyMoneyAccount::Investment);
  d->m_accountsProxyModel->setHideEquityAccounts(false);
  auto const model = Models::instance()->accountsModel();
  d->m_accountsProxyModel->setSourceModel(model);
  d->m_accountsProxyModel->setSourceColumns(model->getColumns());
  d->m_accountsProxyModel->sort((int)eAccountsModel::Column::Account);
  m_accountComboBox->setModel(d->m_accountsProxyModel);
  m_accountComboBox->expandAll();

  auto cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Equities");
  auto cfgHeader = cfgGroup.readEntry("HeaderState", QByteArray());
  auto cfgColumns = cfgGroup.readEntry("ColumnsSelection", QList<int>());
  QList<EquitiesModel::Column> visEColumns {EquitiesModel::Equity};
  foreach (const auto cfgColumn, cfgColumns) {
    const auto visColumn = static_cast<EquitiesModel::Column>(cfgColumn);
    if (!visEColumns.contains(visColumn))
      visEColumns.append(visColumn);
  }

  d->m_equitiesProxyModel = new EquitiesFilterProxyModel(this, Models::instance()->equitiesModel(), visEColumns);
  m_equitiesTree->setModel(d->m_equitiesProxyModel);
  m_equitiesTree->header()->restoreState(cfgHeader);

  connect(m_equitiesTree, &QWidget::customContextMenuRequested, this, &KInvestmentView::slotEquityRightClicked);
//  connect(m_equitiesTree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &KInvestmentView::slotEquitySelected);
  connect(m_equitiesTree, &QTreeView::doubleClicked, this, &KInvestmentView::slotEquityDoubleClicked);
  m_equitiesTree->header()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_equitiesTree->header(), &QWidget::customContextMenuRequested, d->m_equitiesProxyModel, &EquitiesFilterProxyModel::slotColumnsMenu);
  connect(m_accountComboBox, SIGNAL(accountSelected(QString)),
          this, SLOT(slotLoadAccount(QString)));

  // Securities tab
  cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Securities");
  cfgHeader = cfgGroup.readEntry("HeaderState", QByteArray());
  cfgColumns = cfgGroup.readEntry("ColumnsSelection", QList<int>());
  QList<SecuritiesModel::Column> visSColumns {SecuritiesModel::Security};
  foreach (const auto cfgColumn, cfgColumns) {
    const auto visColumn = static_cast<SecuritiesModel::Column>(cfgColumn);
    if (!visSColumns.contains(visColumn))
      visSColumns.append(visColumn);
  }

  d->m_securitiesProxyModel = new SecuritiesFilterProxyModel(this, Models::instance()->securitiesModel(), visSColumns);
  m_securitiesTree->setModel(d->m_securitiesProxyModel);
  m_securitiesTree->header()->restoreState(cfgHeader);

  m_searchSecurities->setProxy(d->m_securitiesProxyModel);
  m_deleteSecurityButton->setIcon(QIcon::fromTheme(g_Icons[Icon::EditDelete]));
  m_editSecurityButton->setIcon(QIcon::fromTheme(g_Icons[Icon::DocumentEdit]));

  connect(m_securitiesTree->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &KInvestmentView::slotSecuritySelected);
  connect(m_editSecurityButton, &QAbstractButton::clicked, this, &KInvestmentView::slotEditSecurity);
  connect(m_deleteSecurityButton, &QAbstractButton::clicked, this, &KInvestmentView::slotDeleteSecurity);
  m_securitiesTree->header()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_securitiesTree->header(), &QWidget::customContextMenuRequested, d->m_securitiesProxyModel, &SecuritiesFilterProxyModel::slotColumnsMenu);

  // Investment Page
  d->m_needReload[Tab::Equities] = d->m_needReload[Tab::Securities] = true;
  connect(m_tab, &QTabWidget::currentChanged, this, &KInvestmentView::slotLoadTab);
  connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &KInvestmentView::slotLoadView);

  connect(this, SIGNAL(accountSelected(MyMoneyObject)), m_kmymoney, SLOT(slotSelectAccount(MyMoneyObject)));
  connect(this, &KInvestmentView::equityRightClicked, m_kmymoney, &KMyMoneyApp::slotShowInvestmentContextMenu);
  connect(this, &KInvestmentView::aboutToShow, m_kmymoneyview, &KMyMoneyView::aboutToChangeView);
}

void KInvestmentView::slotLoadTab(int index)
{
  auto tab = static_cast<Tab>(index);
  if (d->m_needReload[tab]) {
    switch (tab) {
      case Tab::Equities:
        loadInvestmentTab();
        break;
      case Tab::Securities:
        loadSecuritiesTab();
        break;
    }
    d->m_needReload[tab] = false;
  }
}

void KInvestmentView::slotEquitySelected(const QModelIndex &current, const QModelIndex &previous)
{
  Q_UNUSED(current);
  Q_UNUSED(previous);
  MyMoneyAccount acc;

  auto treeItem = m_equitiesTree->currentIndex();
  if (treeItem.isValid()) {
    auto mdlItem = d->m_equitiesProxyModel->index(treeItem.row(), EquitiesModel::Equity, treeItem.parent());
    acc = MyMoneyFile::instance()->account(mdlItem.data(EquitiesModel::EquityID).toString());
  }
  m_kmymoney->slotSelectInvestment(acc);
}

void KInvestmentView::slotEquityRightClicked(const QPoint&)
{
  slotEquitySelected(QModelIndex(), QModelIndex());
  emit equityRightClicked();
}

void KInvestmentView::slotEquityDoubleClicked()
{
  slotEquitySelected(QModelIndex(), QModelIndex());
  m_kmymoney->actionCollection()->action(m_kmymoney->s_Actions[Action::InvestmentEdit])->trigger();
}

void KInvestmentView::slotSecuritySelected(const QModelIndex &current, const QModelIndex &previous)
{
  Q_UNUSED(current);
  Q_UNUSED(previous);
  const auto sec = currentSecurity();
  if (!sec.id().isEmpty()) {
    MyMoneyFileBitArray skip(IMyMoneyStorage::MaxRefCheckBits);
    skip.fill(false);
    skip.setBit(IMyMoneyStorage::RefCheckPrice);
    m_editSecurityButton->setEnabled(true);
    m_deleteSecurityButton->setEnabled(!MyMoneyFile::instance()->isReferenced(sec, skip));
  } else {
    m_editSecurityButton->setEnabled(false);
    m_deleteSecurityButton->setEnabled(false);
  }
}

void KInvestmentView::slotLoadView()
{
  d->m_needReload[Tab::Equities] = d->m_needReload[Tab::Securities] = true;
  if (isVisible())
    slotLoadTab(m_tab->currentIndex());
}

void KInvestmentView::selectDefaultInvestmentAccount()
{
  if (d->m_accountsProxyModel->rowCount() > 0) {
    auto firsitem = d->m_accountsProxyModel->index(0, 0, QModelIndex());
    if (d->m_accountsProxyModel->hasChildren(firsitem)) {
      auto seconditem = d->m_accountsProxyModel->index(0, 0, firsitem);
      slotSelectAccount(seconditem.data(EquitiesModel::EquityID).toString());
    }
  }
}

void KInvestmentView::slotSelectAccount(const QString &id)
{
  if (!id.isEmpty()) {
    d->m_idInvAcc = id;
    if (isVisible())
      m_accountComboBox->setSelected(id);
  }
}

void KInvestmentView::slotSelectAccount(const MyMoneyObject &obj)
{
  if (typeid(obj) != typeid(MyMoneyAccount))
     return;
  const auto acc = dynamic_cast<const MyMoneyAccount &>(obj);

  if (acc.accountType() == MyMoneyAccount::Investment)
    slotSelectAccount(acc.id());
}

void KInvestmentView::slotLoadAccount(const QString &id)
{
  const auto indexList = d->m_equitiesProxyModel->match(d->m_equitiesProxyModel->index(0,0), EquitiesModel::InvestmentID, id, 1,
                                                   Qt::MatchFlags(Qt::MatchExactly | Qt::MatchRecursive | Qt::MatchWrap));
  if (!indexList.isEmpty()) {
    m_equitiesTree->setRootIndex(indexList.first());
    d->m_idInvAcc = id;
    if (isVisible())
      emit accountSelected(MyMoneyFile::instance()->account(id));
  }
}

void KInvestmentView::loadInvestmentTab()
{
  d->m_equitiesProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !m_kmymoney->isActionToggled(Action::ViewShowAll));
  d->m_equitiesProxyModel->setHideZeroBalanceAccounts(KMyMoneyGlobalSettings::hideZeroBalanceEquities());
  d->m_equitiesProxyModel->invalidate();

  d->m_accountsProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !m_kmymoney->isActionToggled(Action::ViewShowAll));
  d->m_accountsProxyModel->invalidate();

  if (!d->m_idInvAcc.isEmpty()) {                                          // check if account to be selected exist
    try {                                                                  // it could not exist anymore (e.g. another file has been opened)
      const auto acc = MyMoneyFile::instance()->account(d->m_idInvAcc);    // then this should throw an exception
      if (acc.accountType() == MyMoneyAccount::Investment)                 // it could be that id exists but account in new file isn't investment account anymore
        slotSelectAccount(d->m_idInvAcc);                                  // otherwise select preset account
      else
        d->m_idInvAcc.clear();
    } catch (const MyMoneyException &) {
      d->m_idInvAcc.clear();                                               // account is invalid
    }
  }

  if (d->m_idInvAcc.isEmpty())                                             // if account is invalid select default one
    selectDefaultInvestmentAccount();

  m_accountComboBox->expandAll();
}

void KInvestmentView::showEvent(QShowEvent* event)
{
  if (m_needLoad)
    init();

  emit aboutToShow();

  d->m_needReload[Tab::Equities] = true;  // ensure tree view will be reloaded after selecting account in ledger view
  slotLoadTab(m_tab->currentIndex());

  // don't forget base class implementation
  QWidget::showEvent(event);
}

void KInvestmentView::loadSecuritiesTab()
{
  m_deleteSecurityButton->setEnabled(false);
  m_editSecurityButton->setEnabled(false);

  d->m_securitiesProxyModel->invalidate();
  // securities model contains both securities and currencies, so...
  // ...search here for securities node and show only this
  const auto indexList = d->m_securitiesProxyModel->match(d->m_securitiesProxyModel->index(0, 0), Qt::DisplayRole, QLatin1String("Securities"), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap));
  if (!indexList.isEmpty())
    m_securitiesTree->setRootIndex(indexList.first());
}

void KInvestmentView::slotEditSecurity()
{
  auto sec = currentSecurity();

  if (!sec.id().isEmpty()) {
    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(sec, this);
    dlg->setObjectName("KNewInvestmentWizard");
    if (dlg->exec() == QDialog::Accepted)
      dlg->createObjects(QString());
    delete dlg;
  }
}

MyMoneySecurity KInvestmentView::currentSecurity()
{
  MyMoneySecurity sec;

  auto treeItem = m_securitiesTree->currentIndex();
  if (treeItem.isValid()) {
    auto mdlItem = d->m_securitiesProxyModel->index(treeItem.row(), SecuritiesModel::Security, treeItem.parent());
    try {
      sec = MyMoneyFile::instance()->security(mdlItem.data(Qt::UserRole).toString());
    } catch (const MyMoneyException &) {}
  }
  return sec;
}

void KInvestmentView::slotDeleteSecurity()
{
  auto sec = currentSecurity();
  if (!sec.id().isEmpty())
    KMyMoneyUtils::deleteSecurity(sec, this);
}
