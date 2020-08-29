/*
 * Copyright 2007-2019  Thomas Baumgart <tbaumgart@kde.org>
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
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
#include "institutionsmodel.h"
#include "accountsproxymodel.h"
#include "institutionsproxymodel.h"
#include "mymoneyinstitution.h"
#include "icons.h"
#include "columnselector.h"

using namespace Icons;

class KInstitutionsViewPrivate : public KMyMoneyViewBasePrivate
{
  Q_DECLARE_PUBLIC(KInstitutionsView)

public:
  explicit KInstitutionsViewPrivate(KInstitutionsView *qq)
    : KMyMoneyViewBasePrivate(qq)
    , ui(new Ui::KInstitutionsView)
    , m_proxyModel(nullptr)
  {
  }

  ~KInstitutionsViewPrivate()
  {
    delete ui;
  }

  void init()
  {
    Q_Q(KInstitutionsView);
    ui->setupUi(q);

    // setup icons for collapse and expand button
    ui->m_collapseButton->setIcon(Icons::get(Icon::ListCollapse));
    ui->m_expandButton->setIcon(Icons::get(Icon::ListExpand));

    ui->m_accountTree->setProxyModel(new InstitutionsProxyModel);
    m_proxyModel = ui->m_accountTree->proxyModel();
    q->connect(ui->m_searchWidget, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterFixedString);

    auto columnSelector = new ColumnSelector(ui->m_accountTree, q->metaObject()->className());
    columnSelector->setAlwaysVisible(QVector<int>({ AccountsModel::Column::AccountName }));
    columnSelector->setAlwaysHidden(QVector<int>({ AccountsModel::Column::Balance, AccountsModel::Column::PostedValue }));

    ui->m_accountTree->setModel(MyMoneyFile::instance()->institutionsModel());
    m_proxyModel->addAccountGroup(AccountsProxyModel::assetLiabilityEquity());

    columnSelector->setModel(m_proxyModel);
    q->slotSettingsChanged();

    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByObject, q, &KInstitutionsView::selectByObject);
    q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::selectByVariant, q, &KInstitutionsView::selectByVariant);
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

  Ui::KInstitutionsView   *ui;
  MyMoneyInstitution      m_currentInstitution;
  AccountsProxyModel*     m_proxyModel;
};

#endif
