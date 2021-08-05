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

// ----------------------------------------------------------------------------
// Alkimia Includes
#include <alkimia/alkonlinequotesprofilemanager.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_konlineupdatewizardpage.h"

#include "mymoneymoney.h"
#include "mymoneysecurity.h"
#include "webpricequote.h"

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
    connect(ui->m_onlineSourceCombo, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &KOnlineUpdateWizardPage::slotCheckPage);
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
    int idx = -1;
    if (security.value("kmm-online-quote-system") == "Finance::Quote") {
        //    AlkFinanceQuoteProcess p;
        //    m_useFinanceQuote->setChecked(true);
        //    idx = m_onlineSourceCombo->findText(p.niceName(security.value("kmm-online-source")));
    } else {
        idx = ui->m_onlineSourceCombo->findText(security.value("kmm-online-source"));
    }

    // in case we did not find the entry, we use the empty one
    if (idx == -1)
        idx = ui->m_onlineSourceCombo->findText(QString());
    ui->m_onlineSourceCombo->setCurrentIndex(idx);

    if (!security.value("kmm-online-factor").isEmpty())
        ui->m_onlineFactor->setValue(MyMoneyMoney(security.value("kmm-online-factor")));
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
    ui->m_onlineSourceCombo->addItems(AlkOnlineQuotesProfileManager::instance().profiles().first()->quoteSources());
    ui->m_onlineSourceCombo->model()->sort(0);
}
