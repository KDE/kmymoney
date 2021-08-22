/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KACCOUNTPAGE_H
#define KACCOUNTPAGE_H

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// Project Includes

#include "wizardpage.h"

class KMyMoneyWizardPage;

namespace NewUserWizard
{
class Wizard;

/**
* Wizard page collecting information about the checking account
*/
class AccountPagePrivate;
class AccountPage : public QWidget, public WizardPage<Wizard>
{
    Q_OBJECT
    Q_DISABLE_COPY(AccountPage)

public:
    explicit AccountPage(Wizard* parent);
    ~AccountPage() override;

    KMyMoneyWizardPage* nextPage() const override;

    bool isComplete() const override;

    void enterPage() override;

private:
    Q_DECLARE_PRIVATE_D(WizardPage<Wizard>::d_ptr, AccountPage)
    friend class Wizard;
    friend class CurrencyPage;
};
} // namespace

#endif
