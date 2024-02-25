/*
    SPDX-FileCopyrightText: 2010 Fernando Vilas <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konlineupdatewizardpage.h"

// ----------------------------------------------------------------------------
// QT Includes
#include <QSortFilterProxyModel>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLazyLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_konlineupdatewizardpage.h"

#include <alkimia/alkonlinequotesprofile.h>

#include "kmmonlinequotesprofilemanager.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

KOnlineUpdateWizardPage::KOnlineUpdateWizardPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::KOnlineUpdateWizardPage)
{
    ui->setupUi(this);
    ui->m_onlineFactor->setPrecision(4);
    ui->m_onlineFactor->setValue(MyMoneyMoney::ONE);

    // make ui->m_onlineSourceCombo sortable
    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->m_onlineSourceCombo);
    proxy->setSourceModel(ui->m_onlineSourceCombo->model());
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    ui->m_onlineSourceCombo->model()->setParent(proxy);
    ui->m_onlineSourceCombo->setModel(proxy);

    // Connect signals-slots
    connect(ui->m_useFinanceQuote, &QAbstractButton::toggled, this, &KOnlineUpdateWizardPage::slotSourceChanged);

    // Register the fields with the QWizard and connect the
    // appropriate signals to update the "Next" button correctly
    registerField("onlineFactor", ui->m_onlineFactor, "value");
    registerField("onlineSourceCombo", ui->m_onlineSourceCombo, "currentText", SIGNAL(currentIndexChanged(QString)));
    registerField("useFinanceQuote", ui->m_useFinanceQuote);
    connect(ui->m_onlineSourceCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int idx) {
        slotCheckPage(ui->m_onlineSourceCombo->itemText(idx));
    });
    connect(ui->m_onlineFactor, &AmountEdit::textChanged,
            this, &QWizardPage::completeChanged);

    connect(ui->m_onlineSourceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &QWizardPage::completeChanged);

    connect(ui->m_useFinanceQuote, &QAbstractButton::toggled,
            this, &QWizardPage::completeChanged);
}

KOnlineUpdateWizardPage::~KOnlineUpdateWizardPage()
{
    delete ui;
}

/**
 * Set the values based on the @param security
 */
void KOnlineUpdateWizardPage::init2(const MyMoneySecurity& security)
{
    const auto onlineQuoteProfileName = security.value(QLatin1String("kmm-online-quote-system"), QLatin1String("kmymoney5"));

    KMMOnlineQuotesProfileManager& manager = KMMOnlineQuotesProfileManager::instance();
    AlkOnlineQuotesProfile* onlineQuoteProfile;
    onlineQuoteProfile = manager.profile(onlineQuoteProfileName);

    // Allow Finance::Quote only if fully available
    const auto tip = manager.availabilityHint(QLatin1String("Finance::Quote"));
    ui->m_useFinanceQuote->setToolTip(tip);
    ui->m_useFinanceQuote->setEnabled(tip.isEmpty());

    int idx = -1;
    if (onlineQuoteProfile) {
        ui->m_useFinanceQuote->setChecked(onlineQuoteProfile->type() == AlkOnlineQuotesProfile::Type::Script);
        idx = ui->m_onlineSourceCombo->findText(security.value("kmm-online-source"));
    }
    // in case we did not find the entry, we use the empty one
    if (idx == -1) {
        idx = ui->m_onlineSourceCombo->findText(QString());
    }
    ui->m_onlineSourceCombo->setCurrentIndex(idx);

    if (!security.value("kmm-online-factor").isEmpty()) {
        ui->m_onlineFactor->setValue(MyMoneyMoney(security.value("kmm-online-factor")));
    }
}

/**
 * Update the "Next" button
 */
bool KOnlineUpdateWizardPage::isComplete() const
{
    return !(ui->m_onlineFactor->isEnabled()
             && ui->m_onlineFactor->value().isZero());
}

bool KOnlineUpdateWizardPage::isOnlineFactorEnabled() const
{
    return ui->m_onlineFactor->isEnabled();
}

void KOnlineUpdateWizardPage::slotCheckPage(const QString& txt)
{
    ui->m_onlineFactor->setEnabled(!txt.isEmpty());
}

void KOnlineUpdateWizardPage::slotSourceChanged(bool useFQ)
{
    ui->m_onlineSourceCombo->clear();
    ui->m_onlineSourceCombo->insertItem(0, QString());

    const QString profileName = useFQ ? QLatin1String("Finance::Quote") : QLatin1String("kmymoney5");
    QWidget* widget = useFQ ? ui->m_useFinanceQuote : nullptr;

    KMMOnlineQuotesProfileManager& manager = KMMOnlineQuotesProfileManager::instance();
    AlkOnlineQuotesProfile* quoteProfile = manager.profile(profileName);

    if (quoteProfile) {
        ui->m_onlineSourceCombo->addItems(quoteProfile->quoteSources());
    }

    if (widget) {
        ui->m_useFinanceQuote->setEnabled(quoteProfile != nullptr);
        widget->setToolTip(manager.availabilityHint(profileName));
    }

    ui->m_onlineSourceCombo->setEnabled(quoteProfile != nullptr);
    ui->m_onlineSourceCombo->model()->sort(0);
}
