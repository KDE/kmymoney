/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACCOUNTSUMMARYPAGE_P_H
#define KACCOUNTSUMMARYPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountsummarypage.h"

#include "wizardpage_p.h"

namespace NewAccountWizard
{
class Wizard;

class AccountSummaryPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(AccountSummaryPagePrivate)

public:
    explicit AccountSummaryPagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent),
        ui(new Ui::KAccountSummaryPage)
    {
    }

    ~AccountSummaryPagePrivate()
    {
        delete ui;
    }

    Ui::KAccountSummaryPage *ui;
};
}

#endif
