/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "khierarchypage.h"
#include "khierarchypage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QVector>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_khierarchypage.h"

#include "accountsmodel.h"
#include "hierarchyfilterproxymodel.h"
#include "kmymoneyaccounttreeview.h"
#include "kmymoneysettings.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccountsummarypage.h"
#include "kaccounttypepage.h"
#include "modelenums.h"
#include "models.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "wizardpage.h"

using namespace eMyMoney;

namespace NewAccountWizard
{
HierarchyPage::HierarchyPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new HierarchyPagePrivate(wizard), StepParentAccount, this, wizard)
{
    Q_D(HierarchyPage);
    d->ui->setupUi(this);
    d->m_filterProxyModel = nullptr;
    // the proxy filter model
    d->m_filterProxyModel = new HierarchyFilterProxyModel(this);
    d->m_filterProxyModel->setHideClosedAccounts(true);
    d->m_filterProxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    d->m_filterProxyModel->addAccountGroup(QVector<Account::Type> {Account::Type::Asset, Account::Type::Liability});
    auto const model = Models::instance()->accountsModel();
    d->m_filterProxyModel->setSourceColumns(model->getColumns());
    d->m_filterProxyModel->setSourceModel(model);
    d->m_filterProxyModel->setDynamicSortFilter(true);

    d->ui->m_parentAccounts->setModel(d->m_filterProxyModel);
    d->ui->m_parentAccounts->sortByColumn((int)eAccountsModel::Column::Account, Qt::AscendingOrder);

    connect(d->ui->m_parentAccounts->selectionModel(), &QItemSelectionModel::currentChanged, this, &HierarchyPage::parentAccountChanged);
}

HierarchyPage::~HierarchyPage()
{
}

void HierarchyPage::enterPage()
{
    Q_D(HierarchyPage);
    // Ensure that the list reflects the Account Type
    MyMoneyAccount topAccount = d->m_wizard->d_func()->m_accountTypePage->parentAccount();
    d->m_filterProxyModel->clear();
    d->m_filterProxyModel->addAccountGroup(QVector<Account::Type> {topAccount.accountGroup()});
    d->ui->m_parentAccounts->expandAll();
}

KMyMoneyWizardPage* HierarchyPage::nextPage() const
{
    Q_D(const HierarchyPage);
    return d->m_wizard->d_func()->m_accountSummaryPage;
}

QWidget* HierarchyPage::initialFocusWidget() const
{
    Q_D(const HierarchyPage);
    return d->ui->m_parentAccounts;
}

const MyMoneyAccount& HierarchyPage::parentAccount()
{
    Q_D(HierarchyPage);
    auto dataVariant = d->ui->m_parentAccounts->model()->data(d->ui->m_parentAccounts->currentIndex(), (int)eAccountsModel::Role::Account);
    if (dataVariant.isValid()) {
        d->m_parentAccount = dataVariant.value<MyMoneyAccount>();
    } else {
        d->m_parentAccount = MyMoneyAccount();
    }
    return d->m_parentAccount;
}

bool HierarchyPage::isComplete() const
{
    Q_D(const HierarchyPage);
    return d->ui->m_parentAccounts->currentIndex().isValid();
}

void HierarchyPage::parentAccountChanged()
{
    completeStateChanged();
}
}
