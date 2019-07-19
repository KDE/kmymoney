/***************************************************************************
                          kinstitutionsview_p.h
                             -------------------
    copyright            : (C) 2007 by Thomas Baumgart <ipwizard@users.sourceforge.net>
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
#include "kmymoneyviewbase_p.h"

#include "accountsproxymodel.h"
#include "mymoneyinstitution.h"
#include "icons.h"

using namespace Icons;

class KInstitutionsViewPrivate : public KMyMoneyViewBasePrivate
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
/// @todo port to new model code
#if 0
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
#endif
  }

  KInstitutionsView       *q_ptr;
  Ui::KInstitutionsView   *ui;
  MyMoneyInstitution       m_currentInstitution;
};

#endif
