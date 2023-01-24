/*
    SPDX-FileCopyrightText: 2023 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTRANSACTIONSORTOPTIONSDLG
#define KTRANSACTIONSORTOPTIONSDLG

#include "kmm_base_dialogs_export.h"

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
class KTransactionSortOptionsDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KTransactionSortOptionsDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KTransactionSortOptionsDlg)
    Q_DECLARE_PRIVATE(KTransactionSortOptionsDlg)

public:
    explicit KTransactionSortOptionsDlg(QWidget* parent = nullptr);
    ~KTransactionSortOptionsDlg();

    void setSortOption(const QString& option, const QString& def);
    QString sortOption() const;
    void hideDefaultButton();

private:
    KTransactionSortOptionsDlgPrivate* d_ptr;
};

#endif // KTRANSACTIONSORTOPTIONSDLG
