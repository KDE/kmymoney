/*
 *    SPDX-FileCopyrightText: 2026 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "fixuncategroziedtransactions.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

#include "journalmodel.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"

using namespace eMyMoney;

class FixUncategorizedTransactionsPrivate
{
public:
    FixUncategorizedTransactionsPrivate()
    {
    }

    void collectTransactions()
    {
        m_uncategorizedIncome = 0;
        m_uncategorizedExpenses = 0;
        const auto model = MyMoneyFile::instance()->journalModel();
        const auto rows = model->rowCount();
        for (int row = 0; row < rows;) {
            const auto index = model->index(row, 0);
            const auto splitCount = index.data(eMyMoney::Model::TransactionSplitCountRole).toInt();
            if (splitCount == 1) {
                const auto amount = index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
                if (!amount.isZero()) {
                    if (amount.isNegative()) {
                        ++m_uncategorizedExpenses;
                    } else {
                        ++m_uncategorizedIncome;
                    }
                }
            }
            row += splitCount;
        }
    }

    int m_uncategorizedIncome{-1};
    int m_uncategorizedExpenses{-1};
};

FixUncategorizedTransactions::FixUncategorizedTransactions(QObject* parent)
    : QObject(parent)
    , d(new FixUncategorizedTransactionsPrivate)
{
}

FixUncategorizedTransactions::~FixUncategorizedTransactions()
{
    delete d;
}

int FixUncategorizedTransactions::unCategorizedIncomeCount() const
{
    if (d->m_uncategorizedIncome == -1) {
        d->collectTransactions();
    }
    return d->m_uncategorizedIncome;
}

int FixUncategorizedTransactions::unCategorizedExpensesCount() const
{
    if (d->m_uncategorizedExpenses == -1) {
        d->collectTransactions();
    }
    return d->m_uncategorizedExpenses;
}

void FixUncategorizedTransactions::assignCategories(const QString& incomeCategoryId, const QString& expenseCategoryId)
{
    const auto file = MyMoneyFile::instance();
    const auto model = file->journalModel();
    auto rows = model->rowCount();
    MyMoneyFileTransaction ft;

    // make sure the progress bar becomes visible
    Q_EMIT progressUpdate(0, rows);
    for (int row = 0; row < rows;) {
        const auto index = model->index(row, 0);
        const auto splitCount = index.data(eMyMoney::Model::TransactionSplitCountRole).toInt();
        if (splitCount == 1) {
            const auto amount = index.data(eMyMoney::Model::SplitSharesRole).value<MyMoneyMoney>();
            if (!amount.isZero()) {
                QString counterAccountId;
                if (amount.isNegative()) {
                    counterAccountId = expenseCategoryId;
                } else {
                    counterAccountId = incomeCategoryId;
                }
                if (!counterAccountId.isEmpty()) {
                    // make sure the counterAccount uses the same currency
                    const auto currencyId = index.data(eMyMoney::Model::JournalSplitSecurityIdRole).toString();
                    const auto counterAccount = file->account(counterAccountId);
                    if (currencyId == counterAccount.currencyId()) {
                        const auto transactionId = index.data(eMyMoney::Model::JournalTransactionIdRole).toString();
                        auto t = file->transaction(transactionId);

                        // create the counterSplit
                        MyMoneySplit sp;
                        sp.setShares(-amount);
                        sp.setValue(-amount);
                        sp.setPrice(MyMoneyMoney::ONE);
                        sp.setAccountId(counterAccountId);
                        t.addSplit(sp);
                        try {
                            file->modifyTransaction(t);
                            ++rows;
                            ++row;
                        } catch (MyMoneyException& e) {
                            qDebug() << "Adding category failed" << e.what();
                        }
                    }
                }
            }
        }
        row += splitCount;
        Q_EMIT progressUpdate(row, rows);
    }
    ft.commit();
}
