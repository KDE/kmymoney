/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kloanpayoutpage.h"
#include "kloanpayoutpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QFrame>
#include <QHash>
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KGuiItem>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccounttypepage.h"
#include "ui_kloanpayoutpage.h"

#include <kguiutils.h>
#include "icons.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneycategory.h"
#include "kmymoneydateinput.h"
#include "kmymoneywizardpage.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "kaccountsummarypage.h"
#include "kaccounttypepage.h"
#include "kaccounttypepage_p.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "wizardpage.h"

using namespace NewAccountWizard;
using namespace Icons;
using namespace eMyMoney;

namespace NewAccountWizard
{
LoanPayoutPage::LoanPayoutPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new LoanPayoutPagePrivate(wizard), StepPayout, this, wizard)
{
    Q_D(LoanPayoutPage);
    d->ui->setupUi(this);
    d->m_mandatoryGroup->add(d->ui->m_assetAccount->lineEdit());
    d->m_mandatoryGroup->add(d->ui->m_loanAccount->lineEdit());

    KGuiItem createAssetButtenItem(i18n("&Create..."),
                                   Icons::get(Icon::DocumentNew),
                                   i18n("Create a new asset account"),
                                   i18n("If the asset account does not yet exist, press this button to create it."));
    KGuiItem::assign(d->ui->m_createAssetButton, createAssetButtenItem);
    d->ui->m_createAssetButton->setToolTip(createAssetButtenItem.toolTip());
    d->ui->m_createAssetButton->setWhatsThis(createAssetButtenItem.whatsThis());
    connect(d->ui->m_createAssetButton, &QAbstractButton::clicked, this, &LoanPayoutPage::slotCreateAssetAccount);

    connect(d->ui->m_noPayoutTransaction, &QAbstractButton::toggled, this, &LoanPayoutPage::slotButtonsToggled);
    connect(d->ui->m_refinanceLoan, &QAbstractButton::toggled, this, &LoanPayoutPage::slotButtonsToggled);

    connect(MyMoneyFile::instance(), &MyMoneyFile::dataChanged, this, &LoanPayoutPage::slotLoadWidgets);
    slotLoadWidgets();
}

LoanPayoutPage::~LoanPayoutPage()
{
}

void LoanPayoutPage::slotButtonsToggled()
{
    Q_D(LoanPayoutPage);
    // we don't go directly, as the order of the emission of signals to slots is
    // not defined. Using a single shot timer postpones the call of m_mandatoryGroup::changed()
    // until the next round of the main loop so we can be sure to see all relevant changes
    // that happened in the meantime (eg. widgets are enabled and disabled)
    QTimer::singleShot(0, d->m_mandatoryGroup, SLOT(changed()));
}

void LoanPayoutPage::slotCreateAssetAccount()
{
    Q_D(LoanPayoutPage);
    MyMoneyAccount acc;
    acc.setAccountType(Account::Type::Asset);
    acc.setOpeningDate(d->m_wizard->d_func()->m_accountTypePage->d_func()->ui->m_openingDate->date());

    emit d->m_wizard->createAccount(acc);

    if (!acc.id().isEmpty()) {
        d->ui->m_assetAccount->setSelectedItem(acc.id());
    }
}

void LoanPayoutPage::slotLoadWidgets()
{
    Q_D(LoanPayoutPage);
    AccountSet set;
    set.addAccountGroup(Account::Type::Asset);
    set.load(d->ui->m_assetAccount->selector());

    set.clear();
    set.addAccountType(Account::Type::Loan);
    set.load(d->ui->m_loanAccount->selector());
}

void LoanPayoutPage::enterPage()
{
    Q_D(LoanPayoutPage);
    // only allow to create new asset accounts for liability loans
    d->ui->m_createAssetButton->setEnabled(d->m_wizard->moneyBorrowed());
    d->ui->m_refinanceLoan->setEnabled(d->m_wizard->moneyBorrowed());
    if (!d->m_wizard->moneyBorrowed()) {
        d->ui->m_refinanceLoan->setChecked(false);
    }
    d->ui->m_payoutDetailFrame->setDisabled(d->ui->m_noPayoutTransaction->isChecked());
}

KMyMoneyWizardPage* LoanPayoutPage::nextPage() const
{
    Q_D(const LoanPayoutPage);
    return d->m_wizard->d_func()->m_accountSummaryPage;
}

QWidget* LoanPayoutPage::initialFocusWidget() const
{
    Q_D(const LoanPayoutPage);
    return d->ui->m_noPayoutTransaction;
}

bool LoanPayoutPage::isComplete() const
{
    Q_D(const LoanPayoutPage);
    return KMyMoneyWizardPage::isComplete() | d->ui->m_noPayoutTransaction->isChecked();
}

QString LoanPayoutPage::payoutAccountId() const
{
    Q_D(const LoanPayoutPage);
    if (d->ui->m_refinanceLoan->isChecked()) {
        return d->ui->m_loanAccount->selectedItem();
    } else {
        return d->ui->m_assetAccount->selectedItem();
    }
}

}
