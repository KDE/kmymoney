/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KSEARCHTRANSACTIONDLG_H
#define KSEARCHTRANSACTIONDLG_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QScopedPointer>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class SelectedObjects;
class KSearchTransactionDlgPrivate;
class KMM_BASE_DIALOGS_EXPORT KSearchTransactionDlg : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KSearchTransactionDlg);
    Q_DISABLE_COPY(KSearchTransactionDlg)

public:
    explicit KSearchTransactionDlg(QWidget* parent);
    virtual ~KSearchTransactionDlg();

    bool eventFilter(QObject* watched, QEvent* event) override;

Q_SIGNALS:
    void requestSelectionChange(const SelectedObjects& selections);

private:
    const QScopedPointer<KSearchTransactionDlgPrivate> d_ptr;
};

#endif // KSEARCHTRANSACTIONDLG_H
