/*
 *    SPDX-FileCopyrightText: 2026 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef FIXUNCATEGORIZEDTRANSACTIONS_H
#define FIXUNCATEGORIZEDTRANSACTIONS_H

#include "kmm_mymoney_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QObject>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

class FixUncategorizedTransactionsPrivate;
class KMM_MYMONEY_EXPORT FixUncategorizedTransactions : public QObject
{
    Q_OBJECT

public:
    FixUncategorizedTransactions(QObject* parent);
    ~FixUncategorizedTransactions();

    /**
     * Returns the number of uncategorized income transactions.
     */
    int unCategorizedIncomeCount() const;

    /**
     * Returns the number of uncategorized income transactions
     */
    int unCategorizedExpensesCount() const;

    /**
     * Create a split with either @a incomeCategoryId or @a expenseCategoryId for each
     * transaction that contains a single split and is unbalanced. The category used
     * depends on the type of transaction (income or expense). If the id passed is
     * empty, a respective matching transaction will not be modified.
     */
    void assignCategories(const QString& incomeCategoryId, const QString& expenseCategoryId);

Q_SIGNALS:
    void progressUpdate(int current, int maximum);

private:
    FixUncategorizedTransactionsPrivate* d;
};

#endif // FIXUNCATEGORIZEDTRANSACTIONS_H
