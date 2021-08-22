/*
    SPDX-FileCopyrightText: 2006 Thomas Baumgart <ipwizard@users.sourceforge.net>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kaccountpage.h"
#include "kaccountpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QDate>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLineEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountpage.h"

#include "kmymoneydateinput.h"
#include "knewuserwizard.h"
#include "knewuserwizard_p.h"
#include "kcategoriespage.h"
#include "wizardpage.h"
#include "kguiutils.h"

namespace NewUserWizard
{
AccountPage::AccountPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new AccountPagePrivate(wizard), stepCount, this, wizard)       // don't inc. the step count here
{
    Q_D(AccountPage);
    d->ui->setupUi(this);
    d->m_mandatoryGroup->add(d->ui->m_accountNameEdit);
    connect(d->m_mandatoryGroup, static_cast<void (KMandatoryFieldGroup::*)()>(&KMandatoryFieldGroup::stateChanged), object(), &KMyMoneyWizardPagePrivate::completeStateChanged);
    connect(d->ui->m_haveCheckingAccountButton, &QAbstractButton::toggled, object(), &KMyMoneyWizardPagePrivate::completeStateChanged);
    d->ui->m_openingDateEdit->setDate(QDate(QDate::currentDate().year(), 1, 1));
}

AccountPage::~AccountPage()
{
}

void AccountPage::enterPage()
{
    Q_D(AccountPage);
    d->ui->m_accountNameEdit->setFocus();
}

KMyMoneyWizardPage* AccountPage::nextPage() const
{
    Q_D(const AccountPage);
    return d->m_wizard->d_func()->m_categoriesPage;
}

bool AccountPage::isComplete() const
{
    Q_D(const AccountPage);
    return !d->ui->m_haveCheckingAccountButton->isChecked() || d->m_mandatoryGroup->isEnabled();
}

}
