/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kaccountsummarypage.h"
#include "kaccountsummarypage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>
#include <QDate>
#include <QFont>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>
#include <KTextEdit>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kaccountsummarypage.h"

#include "kaccounttypepage.h"
#include "kbrokeragepage.h"
#include "kcreditcardschedulepage.h"
#include "kgeneralloaninfopage.h"
#include "kinstitutionpage.h"
#include "kloandetailspage.h"
#include "kloanpaymentpage.h"
#include "kloanpayoutpage.h"
#include "kloanschedulepage.h"

#include "kaccounttypepage_p.h"
#include "kbrokeragepage_p.h"
#include "kcreditcardschedulepage_p.h"
#include "kgeneralloaninfopage_p.h"
#include "kloandetailspage_p.h"
#include "kloanpayoutpage_p.h"
#include "kloanschedulepage_p.h"

#include "kmymoneycategory.h"
#include "kmymoneycurrencyselector.h"
#include "kmymoneydateinput.h"
#include "kmymoneygeneralcombo.h"
#include "kmymoneypayeecombo.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyfile.h"
#include "mymoneyinstitution.h"
#include "mymoneymoney.h"
#include "mymoneyprice.h"
#include "mymoneyschedule.h"
#include "mymoneysecurity.h"
#include "mymoneyutils.h"
#include "wizardpage.h"

using namespace NewAccountWizard;
using namespace Icons;
using namespace eMyMoney;

namespace NewAccountWizard
{
AccountSummaryPage::AccountSummaryPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new AccountSummaryPagePrivate(wizard), StepFinish, this, wizard)
{
    Q_D(AccountSummaryPage);
    d->ui->setupUi(this);
}

AccountSummaryPage::~AccountSummaryPage()
{
}

void AccountSummaryPage::enterPage()
{
    Q_D(AccountSummaryPage);
    MyMoneyAccount acc = d->m_wizard->account();
    MyMoneySecurity sec = d->m_wizard->d_func()->currency();
    acc.fraction(sec);

    // assign an id to the account inside the wizard which is required for a schedule
    // get the schedule and clear the id again in the wizards object.
    MyMoneyAccount tmp(QLatin1String("Phony-ID"), acc);
    d->m_wizard->setAccount(tmp);
    MyMoneySchedule sch = d->m_wizard->schedule();
    d->m_wizard->setAccount(acc);

    d->ui->m_dataList->clear();

    // Account data
    d->ui->m_dataList->setFontWeight(QFont::Bold);
    d->ui->m_dataList->append(i18n("Account information"));
    d->ui->m_dataList->setFontWeight(QFont::Normal);
    d->ui->m_dataList->append(i18nc("Account name", "Name: %1", acc.name()));
    if (!acc.isLoan())
        d->ui->m_dataList->append(i18n("Subaccount of %1", d->m_wizard->parentAccount().name()));
    QString accTypeText;
    if (acc.accountType() == Account::Type::AssetLoan)
        accTypeText = i18n("Loan");
    else
        accTypeText = d->m_wizard->d_func()->m_accountTypePage->d_func()->ui->m_typeSelection->currentText();
    d->ui->m_dataList->append(i18n("Type: %1", accTypeText));

    d->ui->m_dataList->append(i18n("Currency: %1", d->m_wizard->d_func()->currency().name()));
    d->ui->m_dataList->append(i18n("Opening date: %1", QLocale().toString(acc.openingDate())));
    if (d->m_wizard->d_func()->currency().id() != MyMoneyFile::instance()->baseCurrency().id()) {
        d->ui->m_dataList->append(i18n("Conversion rate: %1", d->m_wizard->conversionRate().rate(QString()).formatMoney(QString(), d->m_wizard->d_func()->currency().pricePrecision())));
    }
    if ((!acc.isLoan() && acc.accountType() != Account::Type::Investment) || !d->m_wizard->openingBalance().isZero())
        d->ui->m_dataList->append(i18n("Opening balance: %1", MyMoneyUtils::formatMoney(d->m_wizard->openingBalance(), acc, sec)));

    if (!d->m_wizard->d_func()->m_institutionPage->institution().id().isEmpty()) {
        d->ui->m_dataList->append(i18n("Institution: %1", d->m_wizard->d_func()->m_institutionPage->institution().name()));
        if (!acc.number().isEmpty()) {
            d->ui->m_dataList->append(i18n("Number: %1", acc.number()));
        }
        if (!acc.value("iban").isEmpty()) {
            d->ui->m_dataList->append(i18n("IBAN: %1", acc.value("iban")));
        }
    }

    if (acc.accountType() == Account::Type::Investment) {
        if (d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_createBrokerageButton->isChecked()) {
            d->ui->m_dataList->setFontWeight(QFont::Bold);
            d->ui->m_dataList->append(i18n("Brokerage Account"));
            d->ui->m_dataList->setFontWeight(QFont::Normal);

            d->ui->m_dataList->append(i18nc("Account name", "Name: %1 (Brokerage)", acc.name()));
            d->ui->m_dataList->append(i18n("Currency: %1", d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_brokerageCurrency->security().name()));
            if (d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_accountNumber->isEnabled() && !d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_accountNumber->text().isEmpty())
                d->ui->m_dataList->append(i18n("Number: %1", d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_accountNumber->text()));
            if (d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_iban->isEnabled() && !d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_iban->text().isEmpty())
                d->ui->m_dataList->append(i18n("IBAN: %1", d->m_wizard->d_func()->m_brokeragepage->d_func()->ui->m_iban->text()));
        }
    }

    // Loan
    if (acc.isLoan()) {
        d->ui->m_dataList->setFontWeight(QFont::Bold);
        d->ui->m_dataList->append(i18n("Loan information"));
        d->ui->m_dataList->setFontWeight(QFont::Normal);
        if (d->m_wizard->moneyBorrowed()) {
            d->ui->m_dataList->append(i18n("Amount borrowed: %1", d->m_wizard->d_func()->m_loanDetailsPage->d_func()->ui->m_loanAmount->value().formatMoney(d->m_wizard->d_func()->currency().tradingSymbol(), d->m_wizard->d_func()->precision())));
        } else {
            d->ui->m_dataList->append(i18n("Amount lent: %1", d->m_wizard->d_func()->m_loanDetailsPage->d_func()->ui->m_loanAmount->value().formatMoney(d->m_wizard->d_func()->currency().tradingSymbol(), d->m_wizard->d_func()->precision())));
        }
        d->ui->m_dataList->append(i18n("Interest rate: %1 %", d->m_wizard->d_func()->m_loanDetailsPage->d_func()->ui->m_interestRate->value().formatMoney(QString(), -1)));
        d->ui->m_dataList->append(i18n("Interest rate is %1", d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_interestType->currentText()));
        d->ui->m_dataList->append(i18n("Principal and interest: %1", MyMoneyUtils::formatMoney(d->m_wizard->d_func()->m_loanDetailsPage->d_func()->ui->m_paymentAmount->value(), acc, sec)));
        d->ui->m_dataList->append(i18n("Additional Fees: %1", MyMoneyUtils::formatMoney(d->m_wizard->d_func()->m_loanPaymentPage->additionalFees(), acc, sec)));
        d->ui->m_dataList->append(i18n("Payment frequency: %1", d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_paymentFrequency->currentText()));
        d->ui->m_dataList->append(i18n("Payment account: %1", d->m_wizard->d_func()->m_loanSchedulePage->d_func()->ui->m_paymentAccount->currentText()));

        if (!d->m_wizard->d_func()->m_loanPayoutPage->d_func()->ui->m_noPayoutTransaction->isChecked() && d->m_wizard->openingBalance().isZero()) {
            d->ui->m_dataList->setFontWeight(QFont::Bold);
            d->ui->m_dataList->append(i18n("Payout information"));
            d->ui->m_dataList->setFontWeight(QFont::Normal);
            if (d->m_wizard->d_func()->m_loanPayoutPage->d_func()->ui->m_refinanceLoan->isChecked()) {
                d->ui->m_dataList->append(i18n("Refinance: %1", d->m_wizard->d_func()->m_loanPayoutPage->d_func()->ui->m_loanAccount->currentText()));
            } else {
                if (d->m_wizard->moneyBorrowed())
                    d->ui->m_dataList->append(i18n("Transfer amount to %1", d->m_wizard->d_func()->m_loanPayoutPage->d_func()->ui->m_assetAccount->currentText()));
                else
                    d->ui->m_dataList->append(i18n("Transfer amount from %1", d->m_wizard->d_func()->m_loanPayoutPage->d_func()->ui->m_assetAccount->currentText()));
            }
            d->ui->m_dataList->append(i18n("Payment date: %1 ", QLocale().toString(d->m_wizard->d_func()->m_loanPayoutPage->d_func()->ui->m_payoutDate->date())));
        }
    }

    // Schedule
    if (!(sch == MyMoneySchedule())) {
        d->ui->m_dataList->setFontWeight(QFont::Bold);
        d->ui->m_dataList->append(i18n("Schedule information"));
        d->ui->m_dataList->setFontWeight(QFont::Normal);
        d->ui->m_dataList->append(i18nc("Schedule name", "Name: %1", sch.name()));
        if (acc.accountType() == Account::Type::CreditCard) {
            MyMoneyAccount paymentAccount = MyMoneyFile::instance()->account(d->m_wizard->d_func()->m_schedulePage->d_func()->ui->m_paymentAccount->selectedItem());
            d->ui->m_dataList->append(i18n("Occurrence: Monthly"));
            d->ui->m_dataList->append(i18n("Paid from %1", paymentAccount.name()));
            d->ui->m_dataList->append(i18n("Pay to %1", d->m_wizard->d_func()->m_schedulePage->d_func()->ui->m_payee->currentText()));
            d->ui->m_dataList->append(i18n("Amount: %1", MyMoneyUtils::formatMoney(d->m_wizard->d_func()->m_schedulePage->d_func()->ui->m_amount->value(), acc, sec)));
            d->ui->m_dataList->append(i18n("First payment due on %1", QLocale().toString(sch.nextDueDate())));
            d->ui->m_dataList->append(i18n("Payment method: %1", d->m_wizard->d_func()->m_schedulePage->d_func()->ui->m_method->currentText()));
        }
        if (acc.isLoan()) {
            d->ui->m_dataList->append(i18n("Occurrence: %1", d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_paymentFrequency->currentText()));
            d->ui->m_dataList->append(i18n("Amount: %1", MyMoneyUtils::formatMoney(d->m_wizard->d_func()->m_loanPaymentPage->basePayment() + d->m_wizard->d_func()->m_loanPaymentPage->additionalFees(), acc, sec)));
            d->ui->m_dataList->append(i18n("First payment due on %1", QLocale().toString(d->m_wizard->d_func()->m_loanSchedulePage->firstPaymentDueDate())));
        }
    }
}

QWidget* AccountSummaryPage::initialFocusWidget() const
{
    Q_D(const AccountSummaryPage);
    return d->ui->m_dataList;
}

}
