/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTRANSACTIONSELECTDLG_H
#define KTRANSACTIONSELECTDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

/**
 * @author Thomas Baumgart
 */
class KTransactionSelectDlgPrivate;
class KTransactionSelectDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KTransactionSelectDlg)

public:
    explicit KTransactionSelectDlg(QWidget* parent = nullptr);
    ~KTransactionSelectDlg();

    void addTransaction(const QString& journalEntryId);

protected:
    KTransactionSelectDlgPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(KTransactionSelectDlg)
};

class KTransactionMergeDlg : public KTransactionSelectDlg
{
    Q_OBJECT
    Q_DISABLE_COPY(KTransactionMergeDlg)

public:
    explicit KTransactionMergeDlg(QWidget* parent = nullptr);

    void addTransaction(const QString& journalEntryId);
};
#endif // KTRANSACTIONSELECTDLG_H
