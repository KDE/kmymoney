/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "additionalfeeswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>
#include <KGuiItem>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_additionalfeeswizardpage.h"

#include "knewloanwizard.h"
#include "knewloanwizard_p.h"
#include "mymoneyaccount.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "splitdialog.h"

AdditionalFeesWizardPage::AdditionalFeesWizardPage(QWidget *parent)
    : QWizardPage(parent),
      ui(new Ui::AdditionalFeesWizardPage)
{
    ui->setupUi(this);

    registerField("additionalCost", ui->m_additionalCost, "text");
    registerField("periodicPayment", ui->m_periodicPayment, "text");
    registerField("basePayment", ui->m_basePayment, "text");
    // load button icons
    KGuiItem additionalFeeButtonItem(i18n("&Additional fees..."),
                                     0, //QIcon::fromTheme("document-new"),
                                     i18n("Enter additional fees"),
                                     i18n("Use this to add any additional fees other than principal and interest contained in your periodical payments."));
    KGuiItem::assign(ui->m_additionalFeeButton, additionalFeeButtonItem);
    connect(ui->m_additionalFeeButton, &QAbstractButton::clicked, this, &AdditionalFeesWizardPage::slotAdditionalFees);
}

AdditionalFeesWizardPage::~AdditionalFeesWizardPage()
{
    delete ui;
}

void AdditionalFeesWizardPage::slotAdditionalFees()
{
    const auto d = qobject_cast<KNewLoanWizard*>(wizard())->d_func();

    const auto transactionFactor(MyMoneyMoney::ONE);
    const auto commodity = MyMoneyFile::instance()->currency(d->m_additionalFeesTransaction.commodity());
    const auto payeeId = field("payeeEdit").toString();

    SplitModel dlgSplitModel(this, nullptr, d->m_feeSplitModel);

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
        d->m_feeSplitModel = dlgSplitModel;

        // create the phony transaction with those additional splits
        d->m_additionalFeesTransaction.removeSplits();
        d->m_phonySplit.clearId();
        d->m_additionalFeesTransaction.addSplit(d->m_phonySplit);
        d->m_feeSplitModel.addSplitsToTransaction(d->m_additionalFeesTransaction);
        setField("additionalCost", d->m_feeSplitModel.valueSum().formatMoney(commodity.smallestAccountFraction()));
    }
    if (splitDialog) {
        splitDialog->deleteLater();
    }
    updatePeriodicPayment(qobject_cast<KNewLoanWizard*>(wizard())->d_func()->m_account);
}

void AdditionalFeesWizardPage::updatePeriodicPayment(const MyMoneyAccount& account)
{
    MyMoneyMoney base(ui->m_basePayment->text());
    MyMoneyMoney add(ui->m_additionalCost->text());

    ui->m_periodicPayment->setText((base + add).formatMoney(account.fraction(MyMoneyFile::instance()->security(account.currencyId()))));
}
