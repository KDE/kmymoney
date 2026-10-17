/*
 *    SPDX-FileCopyrightText: 2026 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kfixuncategorizedtransactionsdlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QProgressBar>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kfixuncategorizedtransactionsdlg.h"

#include "accountsmodel.h"
#include "accountsproxymodel.h"
#include "kmymoneysettings.h"
#include "mymoneyfile.h"

using namespace eMyMoney;

class KFixUncategorizedTransactionsDlgPrivate
{
public:
    KFixUncategorizedTransactionsDlgPrivate(KFixUncategorizedTransactionsDlg* qq)
        : q(qq)
        , ui(new Ui::FixUncategorizedTransactions)
        , m_refreshCounter(0)
    {
        ui->setupUi(q);

        ui->m_progressBar->hide();

        m_filterProxyModel = new AccountNamesFilterProxyModel(q);
        m_filterProxyModel->setHideZeroBalancedAccounts(KMyMoneySettings::hideZeroBalanceAccounts());
        m_filterProxyModel->addAccountGroup(QVector<eMyMoney::Account::Type>{eMyMoney::Account::Type::Income, eMyMoney::Account::Type::Expense});

        m_filterProxyModel->setSourceModel(MyMoneyFile::instance()->accountsModel());
        m_filterProxyModel->sort(AccountsModel::Column::AccountName);

        ui->m_incomeCategory->setModel(m_filterProxyModel);
        ui->m_incomeCategory->setShowFullAccountName(true);

        ui->m_expenseCategory->setModel(m_filterProxyModel);
        ui->m_expenseCategory->setShowFullAccountName(true);
    }

    ~KFixUncategorizedTransactionsDlgPrivate()
    {
        delete ui;
    }

    KFixUncategorizedTransactionsDlg* q;
    Ui::FixUncategorizedTransactions* ui;
    AccountNamesFilterProxyModel* m_filterProxyModel;
    int m_refreshCounter;
};

KFixUncategorizedTransactionsDlg::KFixUncategorizedTransactionsDlg(QWidget* parent)
    : QDialog(parent)
    , d(new KFixUncategorizedTransactionsDlgPrivate(this))
{
}

void KFixUncategorizedTransactionsDlg::setUncategorizedCount(int incomeCount, int expenseCount)
{
    d->ui->m_incomeLabel->setText(i18ncp("", "Category for unassigned income, %1 transaction", "Category for unassigned income, %1 transactions", incomeCount));
    d->ui->m_incomeCategory->setEnabled(incomeCount > 0);
    d->ui->m_expenseLabel->setText(
        i18ncp("", "Category for unassigned expense, %1 transaction", "Category for unassigned expenses, %1 transactions", expenseCount));
    d->ui->m_expenseCategory->setEnabled(expenseCount > 0);

    const auto haveUncategorized = d->ui->m_incomeCategory->isEnabled() || d->ui->m_expenseCategory->isEnabled();
    const auto pageWidget = haveUncategorized ? d->ui->fixPage : d->ui->noFixPage;
    d->ui->m_stackWidget->setCurrentWidget(pageWidget);
    d->ui->m_buttonBox->button(QDialogButtonBox::Ok)->setVisible(haveUncategorized);
    d->ui->m_buttonBox->button(QDialogButtonBox::Cancel)->setVisible(haveUncategorized);
    d->ui->m_buttonBox->button(QDialogButtonBox::Close)->setVisible(!haveUncategorized);
}

void KFixUncategorizedTransactionsDlg::accept()
{
    d->ui->m_buttonBox->setDisabled(true);
    d->ui->m_progressBar->show();
    Q_EMIT startProcess(d->ui->m_incomeCategory->getSelected(), d->ui->m_expenseCategory->getSelected());
    QDialog::accept();
}

void KFixUncategorizedTransactionsDlg::setProgress(int count, int maximum)
{
    if (d->ui->m_progressBar->maximum() != maximum) {
        d->ui->m_progressBar->setMaximum(maximum);
    }
    d->ui->m_progressBar->setValue(count);

    if ((d->m_refreshCounter % 10) == 0) {
        QCoreApplication::processEvents();
    }
    ++d->m_refreshCounter;
}
