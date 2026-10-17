/*
 *    SPDX-FileCopyrightText: 2026 Thomas Baumgart <tbaumgart@kde.org>
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KFIXUNCATEGORIZEDTRANSACTIONSDLG_H
#define KFIXUNCATEGORIZEDTRANSACTIONSDLG_H

#include "kmm_extended_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Headers

// ----------------------------------------------------------------------------
// Project Includes

class KFixUncategorizedTransactionsDlgPrivate;
class KMM_EXTENDED_DIALOGS_EXPORT KFixUncategorizedTransactionsDlg : public QDialog
{
    Q_OBJECT

public:
    KFixUncategorizedTransactionsDlg(QWidget* parent);

    void setUncategorizedCount(int incomeCount, int expenseCount);

public Q_SLOTS:
    void setProgress(int actual, int maximum);
    void accept() override;

Q_SIGNALS:
    void startProcess(const QString& incomeCategoryId, const QString& expenseCategoryId);

private:
    KFixUncategorizedTransactionsDlgPrivate* d;
};

#endif // KFIXUNCATEGORIZEDTRANSACTIONSDLG_H
