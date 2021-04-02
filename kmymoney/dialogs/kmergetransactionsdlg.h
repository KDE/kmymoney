/*
    SPDX-FileCopyrightText: 2006 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMERGETRANSACTIONSDLG_H
#define KMERGETRANSACTIONSDLG_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kselecttransactionsdlg.h"

class MyMoneyAccount;
class KMergeTransactionsDlg: public KSelectTransactionsDlg
{
    Q_OBJECT
public:
    explicit KMergeTransactionsDlg(const MyMoneyAccount& account, QWidget* parent = nullptr);

    bool eventFilter(QObject*, QEvent*) override;

public Q_SLOTS:
    void slotHelp() override;
};

#endif // KMERGETRANSACTIONSDLG_H
