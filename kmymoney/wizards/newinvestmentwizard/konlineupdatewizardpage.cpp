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

class KOnlineUpdateWizardPage::Private
{
public:
    Private(KOnlineUpdateWizardPage* qq)
        : q(qq)
        , ui(new Ui::KOnlineUpdateWizardPage)
    {
        ui->setupUi(q);
        ui->m_onlineFactor->setPrecision(4);
        ui->m_onlineFactor->setValue(MyMoneyMoney::ONE);

        // make ui->m_onlineSourceCombo sortable
        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(ui->m_onlineSourceCombo);
        proxy->setSourceModel(ui->m_onlineSourceCombo->model());
        proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
        ui->m_onlineSourceCombo->model()->setParent(proxy);
        ui->m_onlineSourceCombo->setModel(proxy);

        // Register the fields with the QWizard and connect the
        // appropriate signals to update the "Next" button correctly
        q->registerField("onlineFactor", ui->m_onlineFactor, "value");
        q->registerField("onlineSourceCombo", ui->m_onlineSourceCombo, "currentText", SIGNAL(currentIndexChanged(QString)));
        q->registerField("useFinanceQuote", ui->m_useFinanceQuote);

        q->connect(ui->m_useFinanceQuote, &QAbstractButton::toggled, q, [&](bool checked) {
            slotSourceChanged(checked);
            q->completeChanged();
        });

        q->connect(ui->m_onlineSourceCombo, qOverload<int>(&QComboBox::currentIndexChanged), q, [&](int idx) {
            setFactorEnabled(ui->m_onlineSourceCombo->itemText(idx));
        });
        q->connect(ui->m_onlineSourceCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), q, &QWizardPage::completeChanged);

        q->connect(ui->m_onlineFactor, &AmountEdit::textChanged, q, &QWizardPage::completeChanged);

        slotSourceChanged(false);
    }

    /**
     * This enables the factor widget when @a txt is not empty
     */
    void setFactorEnabled(const QString& txt)
    {
        ui->m_onlineFactor->setEnabled(!txt.isEmpty());
    }

    void slotSourceChanged(bool useFQ)
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

    ~Private()
    {
        delete ui;
    }

    KOnlineUpdateWizardPage* q;
    Ui::KOnlineUpdateWizardPage* ui;
};

KOnlineUpdateWizardPage::KOnlineUpdateWizardPage(QWidget* parent)
    : QWizardPage(parent)
    , d(new Private(this))
{
}

KOnlineUpdateWizardPage::~KOnlineUpdateWizardPage()
{
    delete d;
}

/**
 * Set the values based on the @param security
 */
void KOnlineUpdateWizardPage::init(const MyMoneySecurity& security)
{
    d->ui->m_onlineFactor->setEnabled(false);

    const auto onlineQuoteProfileName = security.value(QLatin1String("kmm-online-quote-system"), QStringLiteral("kmymoney5"));

    KMMOnlineQuotesProfileManager& manager = KMMOnlineQuotesProfileManager::instance();
    AlkOnlineQuotesProfile* onlineQuoteProfile;
    onlineQuoteProfile = manager.profile(onlineQuoteProfileName);

    // Allow Finance::Quote only if fully available
    const auto tip = manager.availabilityHint(QLatin1String("Finance::Quote"));
    d->ui->m_useFinanceQuote->setToolTip(tip);
    d->ui->m_useFinanceQuote->setEnabled(tip.isEmpty());

    int idx = -1;
    if (onlineQuoteProfile) {
        d->ui->m_useFinanceQuote->setChecked(onlineQuoteProfile->type() == AlkOnlineQuotesProfile::Type::Script);
        idx = d->ui->m_onlineSourceCombo->findText(security.value("kmm-online-source"));
    }
    // in case we did not find the entry, we use the empty one
    if (idx == -1) {
        idx = d->ui->m_onlineSourceCombo->findText(QString());
    }
    d->ui->m_onlineSourceCombo->setCurrentIndex(idx);

    d->setFactorEnabled(security.value("kmm-online-source"));

    if (!security.value("kmm-online-factor").isEmpty()) {
        d->ui->m_onlineFactor->setValue(MyMoneyMoney(security.value("kmm-online-factor")));
    }
}

/**
 * Update the "Next" button
 */
bool KOnlineUpdateWizardPage::isComplete() const
{
    return !(d->ui->m_onlineFactor->isEnabled() && d->ui->m_onlineFactor->value().isZero());
}
