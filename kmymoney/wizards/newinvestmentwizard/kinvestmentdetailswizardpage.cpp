/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinvestmentdetailswizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes


// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmentdetailswizardpage.h"

#include "mymoneymoney.h"
#include "mymoneyfile.h"
#include "mymoneysecurity.h"
#include "kmymoneymoneyvalidator.h"

KInvestmentDetailsWizardPage::KInvestmentDetailsWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::KInvestmentDetailsWizardPage)
{
    ui->setupUi(this);
    ui->m_fraction->setPrecision(0);
    ui->m_fraction->setValue(MyMoneyMoney(100, 1));
    KMyMoneyMoneyValidator* fractionValidator = new KMyMoneyMoneyValidator(1, 100000, 0, this);
    ui->m_fraction->setValidator(fractionValidator);

    // load the price mode combo
    ui->m_priceMode->insertItem(i18nc("default price mode", "(default)"), 0);
    ui->m_priceMode->insertItem(i18n("Price per share"), 1);
    ui->m_priceMode->insertItem(i18n("Total for all shares"), 2);

    // load the widget with the available currencies
    ui->m_tradingCurrencyEdit->update(QString());

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("investmentName", ui->m_investmentName);
    connect(ui->m_investmentName, &QLineEdit::textChanged,
            this, &QWizardPage::completeChanged);

    registerField("investmentIdentification", ui->m_investmentIdentification);
    connect(ui->m_investmentIdentification, &QLineEdit::textChanged,
            this, &QWizardPage::completeChanged);

    registerField("investmentSymbol", ui->m_investmentSymbol);
    connect(ui->m_investmentSymbol, &QLineEdit::textChanged,
            this, &QWizardPage::completeChanged);

    registerField("tradingCurrencyEdit", ui->m_tradingCurrencyEdit, "security");

    registerField("tradingMarket", ui->m_tradingMarket, "currentText", SIGNAL(currentIndexChanged(QString)));

    ui->m_roundingMethod->addItem(i18nc("Rounding method", "Round"), AlkValue::RoundRound);
    ui->m_roundingMethod->addItem(i18nc("Rounding method", "Ceil"), AlkValue::RoundCeil);
    ui->m_roundingMethod->addItem(i18nc("Rounding method", "Floor"), AlkValue::RoundFloor);
    ui->m_roundingMethod->addItem(i18nc("Rounding method", "Truncate"), AlkValue::RoundTruncate);
    registerField("roundingMethod", ui->m_roundingMethod, "currentData", SIGNAL(currentIndexChanged(int)));

    registerField("fraction", ui->m_fraction, "value", SIGNAL(textChanged()));
    connect(ui->m_fraction, &AmountEdit::textChanged,
            this, &QWizardPage::completeChanged);

    registerField("pricePrecision", ui->m_pricePrecision, "value", SIGNAL(valueChanged()));
}

KInvestmentDetailsWizardPage::~KInvestmentDetailsWizardPage()
{
    delete ui;
}

/**
 * Set the values based on the @param security
 */
void KInvestmentDetailsWizardPage::init2(const MyMoneySecurity& security)
{
    const auto file = MyMoneyFile::instance();
    MyMoneySecurity tradingCurrency = file->currency(security.tradingCurrency());
    ui->m_investmentSymbol->setText(security.tradingSymbol());
    // add any trading market found in the data to the combo box
    const auto securities = file->securityList();
    for(const auto& sec: securities) {
        if (!sec.tradingMarket().isEmpty()) {
            if (ui->m_tradingMarket->findText(sec.tradingMarket(), Qt::MatchExactly) == -1) {
                ui->m_tradingMarket->addItem(sec.tradingMarket());
            }
        }
    }
    ui->m_tradingMarket->setCurrentIndex(ui->m_tradingMarket->findText(security.tradingMarket(), Qt::MatchExactly));
    if (security.roundingMethod() == AlkValue::RoundNever)
        ui->m_roundingMethod->setCurrentIndex(0);
    else
        ui->m_roundingMethod->setCurrentIndex(ui->m_roundingMethod->findData(security.roundingMethod()));
    ui->m_fraction->setValue(MyMoneyMoney(security.smallestAccountFraction(), 1));
    ui->m_pricePrecision->setValue(security.pricePrecision());
    ui->m_tradingCurrencyEdit->setSecurity(tradingCurrency);

    ui->m_investmentIdentification->setText(security.value("kmm-security-id"));
}

/**
 * Update the "Next" button
 */
bool KInvestmentDetailsWizardPage::isComplete() const
{
    return (!ui->m_investmentName->text().isEmpty() //
            && !ui->m_investmentSymbol->text().isEmpty() //
            && !ui->m_fraction->value().isZero());
}

int KInvestmentDetailsWizardPage::priceMode() const
{
    return ui->m_priceMode->currentItem();
}

void KInvestmentDetailsWizardPage::setCurrentPriceMode(int mode)
{
    ui->m_priceMode->setCurrentItem(mode);
}

void KInvestmentDetailsWizardPage::loadName(const QString& name)
{
    ui->m_investmentName->loadText(name);
}

void KInvestmentDetailsWizardPage::setName(const QString& name)
{
    ui->m_investmentName->setText(name);
}

void KInvestmentDetailsWizardPage::setPriceModeEnabled(bool enabled)
{
    ui->m_priceMode->setEnabled(enabled);
}

void KInvestmentDetailsWizardPage::setupInvestmentSymbol()
{
    ui->m_investmentSymbol->setFocus();
    connect(ui->m_investmentSymbol, &KMyMoneyLineEdit::lineChanged, this, &KInvestmentDetailsWizardPage::checkForExistingSymbol);
}
