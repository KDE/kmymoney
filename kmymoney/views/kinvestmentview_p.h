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

#ifndef KINVESTMENTVIEW_P_H
#define KINVESTMENTVIEW_P_H

#include "kinvestmentview.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QIcon>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmentview.h"
#include "kmymoneyviewbase_p.h"

#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "mymoneyaccount.h"
#include "kmymoneyglobalsettings.h"
#include "kmymoneyaccountcombo.h"
#include "accountsmodel.h"
#include "models.h"
#include "equitiesmodel.h"
#include "securitiesmodel.h"
#include "icons.h"
#include "modelenums.h"
#include "mymoneyenums.h"

using namespace Icons;

namespace eView {
  namespace Investment {
    enum Tab { Equities = 0, Securities };
  }
}

class KInvestmentViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KInvestmentView)

public:
  explicit KInvestmentViewPrivate(KInvestmentView *qq) :
    KMyMoneyViewBasePrivate(),
    q_ptr(qq),
    ui(new Ui::KInvestmentView),
    m_idInvAcc(QString()),
    m_needLoad(true),
    m_accountsProxyModel(nullptr),
    m_equitiesProxyModel(nullptr),
    m_securitiesProxyModel(nullptr)
  {
  }

  ~KInvestmentViewPrivate()
  {
    if (!m_needLoad) {
        // save the header state of the equities list
        auto cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Equities");
        auto cfgHeader = ui->m_equitiesTree->header()->saveState();
        auto visEColumns = m_equitiesProxyModel->getVisibleColumns();

        QList<int> cfgColumns;
        foreach (const auto visColumn, visEColumns)
          cfgColumns.append(static_cast<int>(visColumn));

        cfgGroup.writeEntry("HeaderState", cfgHeader);
        cfgGroup.writeEntry("ColumnsSelection", cfgColumns);

        // save the header state of the securities list
        cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Securities");
        cfgHeader = ui->m_securitiesTree->header()->saveState();
        auto visSColumns = m_securitiesProxyModel->getVisibleColumns();
        cfgColumns.clear();
        foreach (const auto visColumn, visSColumns)
          cfgColumns.append(static_cast<int>(visColumn));

        cfgGroup.writeEntry("HeaderState", cfgHeader);
        cfgGroup.writeEntry("ColumnsSelection", cfgColumns);
      }
    delete ui;
  }

  void init()
  {
    Q_Q(KInvestmentView);
    m_needLoad = false;
    ui->setupUi(q);

    // Equities tab
    m_accountsProxyModel = new AccountNamesFilterProxyModel(q);
    m_accountsProxyModel->addAccountType(eMyMoney::Account::Type::Investment);
    m_accountsProxyModel->setHideEquityAccounts(false);
    auto const model = Models::instance()->accountsModel();
    m_accountsProxyModel->setSourceModel(model);
    m_accountsProxyModel->setSourceColumns(model->getColumns());
    m_accountsProxyModel->sort((int)eAccountsModel::Column::Account);
    ui->m_accountComboBox->setModel(m_accountsProxyModel);
    ui->m_accountComboBox->expandAll();

    auto cfgGroup = KSharedConfig::openConfig()->group("KInvestmentView_Equities");
    auto cfgHeader = cfgGroup.readEntry("HeaderState", QByteArray());
    auto cfgColumns = cfgGroup.readEntry("ColumnsSelection", QList<int>());
    QList<EquitiesModel::Column> visEColumns {EquitiesModel::Equity};
    foreach (const auto cfgColumn, cfgColumns) {
        const auto visColumn = static_cast<EquitiesModel::Column>(cfgColumn);
        if (!visEColumns.contains(visColumn))
          visEColumns.append(visColumn);
      }

    m_equitiesProxyModel = new EquitiesFilterProxyModel(q, Models::instance()->equitiesModel(), visEColumns);
    ui->m_equitiesTree->setModel(m_equitiesProxyModel);
    ui->m_equitiesTree->header()->restoreState(cfgHeader);
    ui->m_equitiesTree->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    q->connect(ui->m_equitiesTree, &QWidget::customContextMenuRequested, q, &KInvestmentView::slotInvestmentMenuRequested);
    q->connect(ui->m_equitiesTree->selectionModel(), &QItemSelectionModel::currentRowChanged, q, &KInvestmentView::slotEquitySelected);
    q->connect(ui->m_equitiesTree, &QTreeView::doubleClicked, q, &KInvestmentView::slotEditInvestment);
    q->connect(ui->m_equitiesTree->header(), &QWidget::customContextMenuRequested, m_equitiesProxyModel, &EquitiesFilterProxyModel::slotColumnsMenu);
    q->connect(ui->m_accountComboBox, &KMyMoneyAccountCombo::accountSelected, q, &KInvestmentView::slotLoadAccount);

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

    m_securitiesProxyModel = new SecuritiesFilterProxyModel(q, Models::instance()->securitiesModel(), visSColumns);
    ui->m_securitiesTree->setModel(m_securitiesProxyModel);
    ui->m_securitiesTree->header()->restoreState(cfgHeader);

    ui->m_searchSecurities->setProxy(m_securitiesProxyModel);
    ui->m_deleteSecurityButton->setIcon(Icons::get(Icon::EditDelete));
    ui->m_editSecurityButton->setIcon(Icons::get(Icon::DocumentEdit));

    q->connect(ui->m_securitiesTree->selectionModel(), &QItemSelectionModel::currentRowChanged, q, &KInvestmentView::slotSecuritySelected);
    q->connect(ui->m_editSecurityButton, &QAbstractButton::clicked, q, &KInvestmentView::slotEditSecurity);
    q->connect(ui->m_deleteSecurityButton, &QAbstractButton::clicked, q, &KInvestmentView::slotDeleteSecurity);
    ui->m_securitiesTree->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    q->connect(ui->m_securitiesTree->header(), &QWidget::customContextMenuRequested, m_securitiesProxyModel, &SecuritiesFilterProxyModel::slotColumnsMenu);

    // Investment Page
    m_needReload[eView::Investment::Tab::Equities] = m_needReload[eView::Investment::Tab::Securities] = true;
    q->connect(ui->m_tab, &QTabWidget::currentChanged, q, &KInvestmentView::slotLoadTab);
    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KInvestmentView::refresh);
  }

  void loadInvestmentTab()
  {
    Q_Q(KInvestmentView);
    m_equitiesProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !KMyMoneyGlobalSettings::showAllAccounts());
    m_equitiesProxyModel->setHideZeroBalanceAccounts(KMyMoneyGlobalSettings::hideZeroBalanceEquities());
    m_equitiesProxyModel->invalidate();

    m_accountsProxyModel->setHideClosedAccounts(KMyMoneyGlobalSettings::hideClosedAccounts() && !KMyMoneyGlobalSettings::showAllAccounts());
    m_accountsProxyModel->invalidate();

    if (!m_idInvAcc.isEmpty()) {                                          // check if account to be selected exist
        try {                                                                  // it could not exist anymore (e.g. another file has been opened)
          const auto acc = MyMoneyFile::instance()->account(m_idInvAcc);    // then this should throw an exception
          if (acc.accountType() == eMyMoney::Account::Type::Investment)                 // it could be that id exists but account in new file isn't investment account anymore
            q->slotSelectAccount(m_idInvAcc);                                  // otherwise select preset account
          else
            m_idInvAcc.clear();
        } catch (const MyMoneyException &) {
          m_idInvAcc.clear();                                               // account is invalid
        }
      }

    if (m_idInvAcc.isEmpty())                                             // if account is invalid select default one
      selectDefaultInvestmentAccount();

    ui->m_accountComboBox->expandAll();
  }

  void loadSecuritiesTab()
  {
    ui->m_deleteSecurityButton->setEnabled(false);
    ui->m_editSecurityButton->setEnabled(false);

    m_securitiesProxyModel->invalidate();
    // securities model contains both securities and currencies, so...
    // ...search here for securities node and show only this
    const auto indexList = m_securitiesProxyModel->match(m_securitiesProxyModel->index(0, 0), Qt::DisplayRole, QLatin1String("Securities"), 1, Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap));
    if (!indexList.isEmpty())
      ui->m_securitiesTree->setRootIndex(indexList.first());
  }

  /**
    * This slot is used to programatically preselect default account in investment view
    */
  void selectDefaultInvestmentAccount()
  {
    Q_Q(KInvestmentView);
    if (m_accountsProxyModel->rowCount() > 0) {
        auto firsitem = m_accountsProxyModel->index(0, 0, QModelIndex());
        if (m_accountsProxyModel->hasChildren(firsitem)) {
            auto seconditem = m_accountsProxyModel->index(0, 0, firsitem);
            q->slotSelectAccount(seconditem.data(EquitiesModel::EquityID).toString());
          }
      }
  }

  /**
    * This slots returns security currently selected in tree view
    */
  MyMoneySecurity currentSecurity()
  {
    MyMoneySecurity sec;
    auto treeItem = ui->m_securitiesTree->currentIndex();
    if (treeItem.isValid()) {
      auto mdlItem = m_securitiesProxyModel->index(treeItem.row(), SecuritiesModel::Security, treeItem.parent());
      sec = MyMoneyFile::instance()->security(mdlItem.data(Qt::UserRole).toString());
    }
    return sec;
  }

  /**
    * This slots returns equity currently selected in tree view
    */
  MyMoneyAccount currentEquity()
  {
    MyMoneyAccount equ;
    auto treeItem = ui->m_equitiesTree->currentIndex();
    if (treeItem.isValid()) {
      auto mdlItem = m_equitiesProxyModel->index(treeItem.row(), EquitiesModel::Equity, treeItem.parent());
      equ = MyMoneyFile::instance()->account(mdlItem.data(EquitiesModel::EquityID).toString());
    }
    return equ;
  }

  KInvestmentView     *q_ptr;
  Ui::KInvestmentView *ui;
  QString             m_idInvAcc;

  /**
    * This member holds the load state of page
    */
  bool m_needLoad;

  bool m_needReload[2];
  AccountNamesFilterProxyModel *m_accountsProxyModel;
  EquitiesFilterProxyModel     *m_equitiesProxyModel;
  SecuritiesFilterProxyModel   *m_securitiesProxyModel;

  MyMoneyAccount m_currentEquity;
};

#endif
