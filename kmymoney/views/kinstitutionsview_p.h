/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KINSTITUTIONSVIEW_P_H
#define KINSTITUTIONSVIEW_P_H

#include "kinstitutionsview.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinstitutionsview.h"
#include "kmymoneyaccountsviewbase_p.h"

#include "accountsviewproxymodel.h"
#include "mymoneyinstitution.h"
#include "icons.h"

using namespace Icons;

class KInstitutionsViewPrivate : public KMyMoneyAccountsViewBasePrivate
{
  Q_DECLARE_PUBLIC(KInstitutionsView)

public:
  explicit KInstitutionsViewPrivate(KInstitutionsView *qq) :
    q_ptr(qq),
    ui(new Ui::KInstitutionsView)
  {
  }

  ~KInstitutionsViewPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KInstitutionsView);
    m_accountTree = &ui->m_accountTree;

    // setup icons for collapse and expand button
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    // the proxy filter model
    m_proxyModel = ui->m_accountTree->init(View::Institutions);
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KInstitutionsView::selectByObject);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByVariant, q, &KInstitutionsView::selectByVariant);
    q->connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, q, &KInstitutionsView::refresh);
  }

  KInstitutionsView       *q_ptr;
  Ui::KInstitutionsView   *ui;
  MyMoneyInstitution       m_currentInstitution;
};

#endif
