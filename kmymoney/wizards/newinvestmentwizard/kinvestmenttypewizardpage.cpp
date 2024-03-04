/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kinvestmenttypewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_kinvestmenttypewizardpage.h"

#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneysecurity.h"

KInvestmentTypeWizardPage::KInvestmentTypeWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::KInvestmentTypeWizardPage)
{
    ui->setupUi(this);
    ui->m_securityType->addItem(i18nc("Security type", "Stock"), (int)eMyMoney::Security::Type::Stock);
    ui->m_securityType->addItem(i18nc("Security type", "Mutual Fund"), (int)eMyMoney::Security::Type::MutualFund);
    ui->m_securityType->addItem(i18nc("Security type", "Bond"), (int)eMyMoney::Security::Type::Bond);
    registerField("securityType", ui->m_securityType, "currentData", SIGNAL(currentIndexChanged(int)));
    registerField("accountName", ui->m_accountName);
    connect(ui->m_accountName, &QLineEdit::textChanged, this, &QWizardPage::completeChanged);
}

KInvestmentTypeWizardPage::~KInvestmentTypeWizardPage()
{
    delete ui;
}

void KInvestmentTypeWizardPage::init(const MyMoneyAccount& account, const MyMoneySecurity& security)
{
    if (!security.id().isEmpty()) {
        // get the current text of the security and set the combo index accordingly
        auto text = MyMoneySecurity::securityTypeToString(security.securityType());
        for (auto i = 0; i < ui->m_securityType->count(); ++i) {
            if (ui->m_securityType->itemText(i) == text)
                ui->m_securityType->setCurrentIndex(i);
        }
    }
    ui->m_accountName->setText(account.name());
    QMetaObject::invokeMethod(ui->m_accountName, "setFocus", Qt::QueuedConnection);
}

void KInvestmentTypeWizardPage::setIntroLabelText(const QString& text)
{
    ui->m_introLabel->setText(text);
}

bool KInvestmentTypeWizardPage::isComplete() const
{
    return !ui->m_accountName->text().isEmpty();
}
