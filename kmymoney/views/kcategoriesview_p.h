/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Javier Campos Morales <javi_c@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 John C <thetacoturtle@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000-2002 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

        q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestCustomContextMenu, q, &KCategoriesView::requestCustomContextMenu);
        q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestSelectionChange, q, &KCategoriesView::requestSelectionChange);
        q->connect(ui->m_accountTree, &KMyMoneyAccountTreeView::requestActionTrigger, q, &KCategoriesView::requestActionTrigger);
    }

    Ui::KCategoriesView   *ui;
    bool                  m_haveUnusedCategories;
    MyMoneyAccount        m_currentCategory;
    AccountsProxyModel*   m_proxyModel;
};

#endif
