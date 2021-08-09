/*
    SPDX-FileCopyrightText: 2006-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "khierarchypage.h"

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

#include "mymoneyfile.h"
#include "accountsmodel.h"
#include "accountsproxymodel.h"
#include "kmymoneyaccounttreeview.h"
#include "kmymoneysettings.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccountsummarypage.h"
#include "kaccounttypepage.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "wizardpage.h"
#include "columnselector.h"

using namespace eMyMoney;

namespace NewAccountWizard
{
class Wizard;

class HierarchyPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(HierarchyPagePrivate)

public:
    explicit HierarchyPagePrivate(QObject* parent)
        : WizardPagePrivate<Wizard>(parent)
        , ui(new Ui::KHierarchyPage)
        , m_columnSelector(nullptr)
    {
    }

    ~HierarchyPagePrivate()
    {
        delete ui;
    }

    Ui::KHierarchyPage        *ui;
    ColumnSelector*           m_columnSelector;
};
}


namespace NewAccountWizard
{
HierarchyPage::HierarchyPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new HierarchyPagePrivate(wizard), StepParentAccount, this, wizard)
{
    Q_D(HierarchyPage);
    d->ui->setupUi(this);
    // setup the filter model first
    auto proxyModel = d->ui->m_parentAccounts->proxyModel();
    proxyModel->setHideClosedAccounts(true);
    proxyModel->setHideEquityAccounts(!KMyMoneySettings::expertMode());
    proxyModel->addAccountGroup(QVector<Account::Type> {Account::Type::Asset, Account::Type::Liability});
    // setup source model
    d->ui->m_parentAccounts->setModel(MyMoneyFile::instance()->accountsModel());

    // force to only show the name column
    d->m_columnSelector = new ColumnSelector(d->ui->m_parentAccounts);
    d->m_columnSelector->setModel(proxyModel);
    auto columns = d->m_columnSelector->columns();
    columns.remove(AccountsModel::Column::AccountName);
    d->m_columnSelector->setAlwaysHidden(columns);
    columns.clear();
    columns.append(AccountsModel::Column::AccountName);
    d->m_columnSelector->setAlwaysVisible(columns);

    d->ui->m_parentAccounts->sortByColumn(AccountsModel::Column::AccountName, Qt::AscendingOrder);
    proxyModel->setDynamicSortFilter(true);

    connect(d->ui->m_parentAccounts->selectionModel(), &QItemSelectionModel::currentChanged, this, &HierarchyPage::parentAccountChanged);
    connect(d->ui->m_parentAccounts, &KMyMoneyAccountTreeView::startEdit, wizard, &KMyMoneyWizard::selectNextPage);
}

HierarchyPage::~HierarchyPage()
{
}

void HierarchyPage::enterPage()
{
    Q_D(HierarchyPage);
    // Ensure that the list reflects the selected Account Type
    MyMoneyAccount topAccount = d->m_wizard->d_func()->m_accountTypePage->parentAccount();
    d->ui->m_parentAccounts->proxyModel()->clear();
    d->ui->m_parentAccounts->proxyModel()->addAccountGroup(QVector<Account::Type> {topAccount.accountGroup()});
    d->ui->m_parentAccounts->expandAll();

    const auto file = MyMoneyFile::instance();
    const auto baseIdx = file->accountsModel()->indexById(topAccount.id());
    const auto idx = file->accountsModel()->mapFromBaseSource(d->ui->m_parentAccounts->model(), baseIdx);
    d->ui->m_parentAccounts->selectionModel()->select(idx, QItemSelectionModel::SelectCurrent);
    d->ui->m_parentAccounts->setCurrentIndex(idx);
    d->ui->m_parentAccounts->scrollTo(idx, QAbstractItemView::PositionAtCenter);
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

MyMoneyAccount HierarchyPage::parentAccount() const
{
    Q_D(const HierarchyPage);
    const auto accountId = d->ui->m_parentAccounts->currentIndex().data(eMyMoney::Model::Roles::IdRole).toString();
    return MyMoneyFile::instance()->accountsModel()->itemById(accountId);
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
