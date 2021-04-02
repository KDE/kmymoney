/*
    SPDX-FileCopyrightText: 2000-2002 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNEWBANKDLG_H
#define KNEWBANKDLG_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyInstitution;
class KJob;

/// This dialog lets the user create or edit an institution
class KNewBankDlgPrivate;
class KNewBankDlg : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KNewBankDlg)

public:
    explicit KNewBankDlg(MyMoneyInstitution& institution, QWidget *parent = nullptr);
    ~KNewBankDlg();
    const MyMoneyInstitution& institution();

    static void newInstitution(MyMoneyInstitution& institution);

private Q_SLOTS:
    void okClicked();
    void institutionNameChanged(const QString &);
    void slotUrlChanged(const QString&);
    void slotLoadIcon();
    void slotIconLoaded(KJob* job);
    void killIconLoad();

private:
    KNewBankDlgPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KNewBankDlg)
};

#endif
