/*
    SPDX-FileCopyrightText: 2007 Thomas Baumgart <Thomas Baumgart <ipwizard@users.sourceforge.net>>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kloanpaymentpage.h"
#include "kloanpaymentpage_p.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QPointer>
#include <QPushButton>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kloanpaymentpage.h"

#include "kgeneralloaninfopage.h"
#include "kgeneralloaninfopage_p.h"
#include "kloandetailspage.h"
#include "kloandetailspage_p.h"
#include "kloanschedulepage.h"
#include "knewaccountwizard.h"
#include "knewaccountwizard_p.h"
#include "mymoneyaccount.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "mymoneysplit.h"
#include "mymoneytransaction.h"
#include "splitdialog.h"
#include "wizardpage.h"

namespace NewAccountWizard
{
LoanPaymentPage::LoanPaymentPage(Wizard* wizard) :
    QWidget(wizard),
    WizardPage<Wizard>(*new LoanPaymentPagePrivate(wizard), StepFees, this, wizard)
{
    Q_D(LoanPaymentPage);
    d->ui->setupUi(this);
    d->phonyAccount = MyMoneyAccount(QLatin1String("Phony-ID"), MyMoneyAccount());

    d->phonySplit.setAccountId(d->phonyAccount.id());
    d->phonySplit.setValue(MyMoneyMoney());
    d->phonySplit.setShares(MyMoneyMoney());

    d->additionalFeesTransaction.addSplit(d->phonySplit);

    connect(d->ui->m_additionalFeesButton, &QAbstractButton::clicked, this, &LoanPaymentPage::slotAdditionalFees);
}

LoanPaymentPage::~LoanPaymentPage()
{
}

MyMoneyMoney LoanPaymentPage::basePayment() const
{
    Q_D(const LoanPaymentPage);
    return d->m_wizard->d_func()->m_loanDetailsPage->d_func()->ui->m_paymentAmount->value();
}

MyMoneyMoney LoanPaymentPage::additionalFees() const
{
    Q_D(const LoanPaymentPage);
    return d->additionalFees;
}

void LoanPaymentPage::additionalFeesSplits(QList<MyMoneySplit>& list)
{
    Q_D(LoanPaymentPage);
    list = d->m_splitModel.splitList();
}

void LoanPaymentPage::updateAmounts()
{
    Q_D(LoanPaymentPage);
    d->ui->m_additionalFees->setText(d->additionalFees.formatMoney(d->m_wizard->d_func()->currency().tradingSymbol(), d->m_wizard->d_func()->precision()));
    d->ui->m_totalPayment->setText((basePayment() + d->additionalFees).formatMoney(d->m_wizard->d_func()->currency().tradingSymbol(), d->m_wizard->d_func()->precision()));
}

void LoanPaymentPage::enterPage()
{
    Q_D(LoanPaymentPage);
    const MyMoneySecurity& currency = d->m_wizard->d_func()->currency();

    d->ui->m_basePayment->setText(basePayment().formatMoney(currency.tradingSymbol(), d->m_wizard->d_func()->precision()));
    d->phonyAccount.setCurrencyId(currency.id());
    d->additionalFeesTransaction.setCommodity(currency.id());

    updateAmounts();
}

void LoanPaymentPage::slotAdditionalFees()
{
    Q_D(LoanPaymentPage);

    const auto transactionFactor(MyMoneyMoney::ONE);
    const auto commodity = d->m_wizard->d_func()->currency();
    const auto payeeId = d->m_wizard->d_func()->m_generalLoanInfoPage->d_func()->ui->m_payee->selectedItem();

    SplitModel dlgSplitModel(this, nullptr, d->m_splitModel);
    // create an empty split at the end
    // used to create new splits
    dlgSplitModel.appendEmptySplit();

    QPointer<SplitDialog> splitDialog = new SplitDialog(commodity, MyMoneyMoney::autoCalc, commodity.smallestAccountFraction(), transactionFactor, this);
    splitDialog->setTransactionPayeeId(payeeId);
    splitDialog->setModel(&dlgSplitModel);

    int rc = splitDialog->exec();
    if (splitDialog && (rc == QDialog::Accepted)) {
        // remove that empty split again before we update the splits
        dlgSplitModel.removeEmptySplit();

        // copy the splits model contents
        d->m_splitModel = dlgSplitModel;

        // create the phony transaction with those additional splits
        d->additionalFeesTransaction.removeSplits();
        d->phonySplit.clearId();
        d->additionalFeesTransaction.addSplit(d->phonySplit);
        d->m_splitModel.addSplitsToTransaction(d->additionalFeesTransaction);
        d->additionalFees = d->m_splitModel.valueSum();
        updateAmounts();
    }
    if (splitDialog) {
        splitDialog->deleteLater();
    }
}

KMyMoneyWizardPage* LoanPaymentPage::nextPage() const
{
    Q_D(const LoanPaymentPage);
    return d->m_wizard->d_func()->m_loanSchedulePage;
}

}
