/***************************************************************************
                          kcategoriesview.cpp  -  description
                             -------------------
    begin                : Sun Jan 20 2002
    copyright            : (C) 2000-2002 by Michael Edwardes <mte@users.sourceforge.net>
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
                           (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KCATEGORIESVIEW_P_H
#define KCATEGORIESVIEW_P_H

#include "kcategoriesview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kcategoriesview.h"
#include "kmymoneyviewbase_p.h"
#include "accountsproxymodel.h"
#include "mymoneyaccount.h"
#include "icons.h"
#include "columnselector.h"

using namespace Icons;

class KCategoriesViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KCategoriesView)

public:
  explicit KCategoriesViewPrivate(KCategoriesView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KCategoriesView)
    , m_haveUnusedCategories(false)
    , m_proxyModel(nullptr)
  {
  }

  ~KCategoriesViewPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KCategoriesView);
    ui->setupUi(q);

    // setup icons for collapse and expand button
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    // setup filter
    m_proxyModel = ui->m_accountTree->proxyModel();
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);

    auto columnSelector = new ColumnSelector(ui->m_accountTree, q->metaObject()->className());
    columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));
    columnSelector->setAlwaysHidden(QVector<int>({ AccountsModel::Column::Balance,
                                                   AccountsModel::Column::PostedValue,
                                                   AccountsModel::Column::Iban,
                                                   AccountsModel::Column::Bic,
                                                   AccountsModel::Column::BankCode,
                                                   AccountsModel::Column::Number,
                                                   AccountsModel::Column::HasOnlineMapping }));

    ui->m_accountTree->setModel(MyMoneyFile::instance()->accountsModel());
    m_proxyModel->addAccountGroup(AccountsProxyModel::incomeExpense());

    columnSelector->setModel(m_proxyModel);
    q->slotSettingsChanged();

    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KCategoriesView::selectByObject);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByVariant, q, &KCategoriesView::selectByVariant);

    /// @todo port to new model code or cleanup
#if 0
    Q_Q(KCategoriesView);
    m_accountTree = &ui->m_accountTree;

    // setup icons for collapse and expand button
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    m_proxyModel = ui->m_accountTree->init(View::Categories);

    q->connect(m_proxyModel, &AccountsProxyModel::unusedIncomeExpenseAccountHidden, q, &KCategoriesView::slotUnusedIncomeExpenseAccountHidden);
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KCategoriesView::selectByObject);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByVariant, q, &KCategoriesView::selectByVariant);
    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KCategoriesView::refresh);
#endif
  }

  Ui::KCategoriesView   *ui;
  bool                  m_haveUnusedCategories;
  MyMoneyAccount        m_currentCategory;
  AccountsProxyModel*   m_proxyModel;
};

#endif
