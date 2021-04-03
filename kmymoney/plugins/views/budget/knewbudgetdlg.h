/*
    SPDX-FileCopyrightText: 2000-2004 Darren Gould <darren_gould@gmx.de>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNEWBUDGETDLG_H
#define KNEWBUDGETDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KNewBudgetDlgPrivate;
class KNewBudgetDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KNewBudgetDlg)

public:
    explicit KNewBudgetDlg(QWidget* parent = nullptr);
    ~KNewBudgetDlg();

    QString getYear() const;
    QString getName() const;

public Q_SLOTS:
    void m_pbCancel_clicked();
    void m_pbOk_clicked();

private:
    KNewBudgetDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KNewBudgetDlg)
};

#endif // KNEWBUDGETDLG_H
