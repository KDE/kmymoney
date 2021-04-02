/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACCOUNTPAGE_P_H
#define KACCOUNTPAGE_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountpage.h"
#include "wizardpage_p.h"

namespace NewUserWizard
{
class Wizard;

class AccountPagePrivate : public WizardPagePrivate<Wizard>
{
    Q_DISABLE_COPY(AccountPagePrivate)

public:
    explicit AccountPagePrivate(QObject* parent) :
        WizardPagePrivate<Wizard>(parent),
        ui(new Ui::KAccountPage)
    {
    }

    ~AccountPagePrivate()
    {
        delete ui;
    }

    Ui::KAccountPage *ui;
};
}
#endif
