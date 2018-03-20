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
#include "kmymoneyaccountsviewbase_p.h"

#include "accountsviewproxymodel.h"
#include "mymoneyaccount.h"
#include "icons.h"

using namespace Icons;

class KCategoriesViewPrivate : public KMyMoneyAccountsViewBasePrivate
{
  Q_DECLARE_PUBLIC(KCategoriesView)

public:
  explicit KCategoriesViewPrivate(KCategoriesView *qq) :
    q_ptr(qq),
    ui(new Ui::KCategoriesView),
    m_haveUnusedCategories(false)
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
  }

  KCategoriesView       *q_ptr;
  Ui::KCategoriesView   *ui;
  bool                  m_haveUnusedCategories;
  MyMoneyAccount        m_currentCategory;
};

#endif
