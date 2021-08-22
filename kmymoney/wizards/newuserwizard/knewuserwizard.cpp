/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knewuserwizard.h"
#include "knewuserwizard_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "kaccountpage.h"
#include "kaccountpage_p.h"
#include "kcategoriespage.h"
#include "kcurrencypage.h"
#include "kgeneralpage.h"
#include "kintropage.h"
#include "kpreferencepage.h"
#include "kpreferencepage_p.h"

#include "mymoneysecurity.h"
#include "mymoneypayee.h"
#include "mymoneymoney.h"
#include "mymoneyinstitution.h"
#include "mymoneyaccount.h"
#include "kmymoneydateinput.h"
#include "kmymoneysettings.h"
#include "mymoneytemplate.h"
#include "mymoneyenums.h"

namespace NewUserWizard
{
int stepCount = 0;

Wizard::Wizard(QWidget *parent, bool modal, Qt::WindowFlags flags) :
    KMyMoneyWizard(*new WizardPrivate(this), parent, modal, flags)
{
    Q_D(Wizard);
    bool isFirstTime = KMyMoneySettings::firstTimeRun();

    stepCount = 1;

    setTitle(i18n("KMyMoney New File Setup"));
    if (isFirstTime)
        addStep(i18nc("New file wizard introduction", "Introduction"));
    addStep(i18n("Personal Data"));
    addStep(i18n("Select Currency"));
    addStep(i18n("Select Accounts"));
    addStep(i18nc("Finish the wizard", "Finish"));

    if (isFirstTime)
        d->m_introPage = new IntroPage(this);
    d->m_generalPage = new GeneralPage(this);
    d->m_currencyPage = new CurrencyPage(this);
    d->m_accountPage = new AccountPage(this);
    d->m_categoriesPage = new CategoriesPage(this);
    d->m_preferencePage = new PreferencePage(this);

    d->m_accountPage->d_func()->ui->m_haveCheckingAccountButton->setChecked(true);
    if (isFirstTime)
        d->setFirstPage(d->m_introPage);
    else
        d->setFirstPage(d->m_generalPage);

    setHelpContext("firsttime-3");
}

Wizard::~Wizard()
{
}

MyMoneyPayee Wizard::user() const
{
    Q_D(const Wizard);
    return d->m_generalPage->user();
}

MyMoneyInstitution Wizard::institution() const
{
    Q_D(const Wizard);
    MyMoneyInstitution inst;
    if (d->m_accountPage->d_func()->ui->m_haveCheckingAccountButton->isChecked()) {
        if (d->m_accountPage->d_func()->ui->m_institutionNameEdit->text().length()) {
            inst.setName(d->m_accountPage->d_func()->ui->m_institutionNameEdit->text());
            if (d->m_accountPage->d_func()->ui->m_institutionNumberEdit->text().length())
                inst.setSortcode(d->m_accountPage->d_func()->ui->m_institutionNumberEdit->text());
        }
    }
    return inst;
}

MyMoneyAccount Wizard::account() const
{
    Q_D(const Wizard);
    MyMoneyAccount acc;
    if (d->m_accountPage->d_func()->ui->m_haveCheckingAccountButton->isChecked()) {
        acc.setName(d->m_accountPage->d_func()->ui->m_accountNameEdit->text());
        if (d->m_accountPage->d_func()->ui->m_accountNumberEdit->text().length())
            acc.setNumber(d->m_accountPage->d_func()->ui->m_accountNumberEdit->text());
        acc.setOpeningDate(d->m_accountPage->d_func()->ui->m_openingDateEdit->date());
        acc.setCurrencyId(d->m_baseCurrency.id());
        acc.setAccountType(eMyMoney::Account::Type::Checkings);
    }
    return acc;
}

MyMoneyMoney Wizard::openingBalance() const
{
    Q_D(const Wizard);
    return d->m_accountPage->d_func()->ui->m_openingBalanceEdit->value();
}

MyMoneySecurity Wizard::baseCurrency() const
{
    Q_D(const Wizard);
    return d->m_baseCurrency;
}

QList<MyMoneyTemplate> Wizard::templates() const
{
    Q_D(const Wizard);
    return d->m_categoriesPage->selectedTemplates();
}

bool Wizard::startSettingsAfterFinished() const
{
    Q_D(const Wizard);
    return d->m_preferencePage->d_func()->ui->m_openConfigAfterFinished->checkState() == Qt::Checked;
}
}
