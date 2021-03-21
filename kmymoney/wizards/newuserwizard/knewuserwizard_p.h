/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KNEWUSERWIZARD_P_H
#define KNEWUSERWIZARD_P_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "knewuserwizard.h"
#include "kmymoneywizard_p.h"
#include "mymoneysecurity.h"

namespace NewUserWizard
{
class IntroPage;
class GeneralPage;
class CurrencyPage;
class AccountPage;
class CategoriesPage;
class PreferencePage;

class WizardPrivate : public KMyMoneyWizardPrivate
{
    Q_DISABLE_COPY(WizardPrivate)

public:
    explicit WizardPrivate(Wizard *qq):
        KMyMoneyWizardPrivate(qq),
        m_introPage(nullptr),
        m_generalPage(nullptr),
        m_currencyPage(nullptr),
        m_accountPage(nullptr),
        m_categoriesPage(nullptr),
        m_preferencePage(nullptr)
    {
    }

    ~WizardPrivate()
    {
    }

    MyMoneySecurity   m_baseCurrency;
    IntroPage*        m_introPage;
    GeneralPage*      m_generalPage;
    CurrencyPage*     m_currencyPage;
    AccountPage*      m_accountPage;
    CategoriesPage*   m_categoriesPage;
    PreferencePage*   m_preferencePage;
};

} // namespace

#endif
