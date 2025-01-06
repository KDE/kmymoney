/*
    SPDX-FileCopyrightText: 2004 Thomas Baumgart <kmymoney-devel@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knewinvestmentwizard.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QList>
#include <QPointer>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KMessageBox>
#include <KHelpClient>
#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "ui_knewinvestmentwizard.h"

#include "kmymoneyutils.h"
#include "mymoneyaccount.h"
#include "mymoneyenums.h"
#include "mymoneyexception.h"
#include "mymoneyfile.h"
#include "mymoneymoney.h"
#include "mymoneysecurity.h"

#include "kmmyesno.h"

class KNewInvestmentWizardPrivate
{
    Q_DISABLE_COPY(KNewInvestmentWizardPrivate)
    Q_DECLARE_PUBLIC(KNewInvestmentWizard)

public:
    explicit KNewInvestmentWizardPrivate(KNewInvestmentWizard *qq) :
        q_ptr(qq),
        ui(new Ui::KNewInvestmentWizard),
        m_createAccount(false)
    {
    }

    ~KNewInvestmentWizardPrivate()
    {
        delete ui;
    }

    void init()
    {
        Q_Q(KNewInvestmentWizard);
        ui->setupUi(q);

        // make sure, the back button does not clear fields
        q->setOption(QWizard::IndependentPages, true);

        // enable the help button
        q->setOption(q->HaveHelpButton, true);
        q->connect(q, &KNewInvestmentWizard::helpRequested, q, [&]() {
            KHelpClient::invokeHelp("details.investments.newinvestmentwizard");
        });

        m_createAccount = true;

        // Update label in case of edit
        ui->m_investmentTypePage->setIntroLabelText(i18n("This wizard allows you to modify the selected investment."));

        KMyMoneyUtils::updateWizardButtons(q);

        ui->m_investmentTypePage->init(m_account, m_security);
        ui->m_investmentDetailsPage->init(m_account, m_security);
        ui->m_onlineUpdatePage->init(m_security);

        // setup the signal here, so that the initialization does not interfer.
        q->connect(ui->m_investmentDetailsPage, &KInvestmentDetailsWizardPage::securityIdChanged, q, [&](const QString& id) {
            m_security = MyMoneySecurity(id, m_security);
        });
    }

    int pageId(const QWizardPage* page) const
    {
        const auto pageIds = q_ptr->pageIds();
        for (const auto id : qAsConst(pageIds)) {
            if (q_ptr->page(id) == page) {
                return id;
            }
        }
        return -1;
    }

    void createAccountAndSecurity(const QString& parentAccountId)
    {
        Q_Q(KNewInvestmentWizard);
        const auto file = MyMoneyFile::instance();

        const auto type = static_cast<eMyMoney::Security::Type>(q->field("securityType").toInt());
        const auto roundingMethod = static_cast<AlkValue::RoundingMethod>(q->field("roundingMethod").toInt());
        MyMoneyFileTransaction ft;
        try {
            // update all relevant attributes only, if we create a stock
            // account and the security is unknown or we modify the security
            MyMoneySecurity newSecurity(m_security);
            // if user really wishes to create the same symbol again, we allow it
            if (q->field("duplicateSymbol").toBool()) {
                newSecurity.clearId();
            }
            newSecurity.setName(q->field("investmentName").toString());
            newSecurity.setTradingSymbol(q->field("investmentSymbol").toString());
            newSecurity.setTradingMarket(q->field("tradingMarket").toString());
            newSecurity.setSmallestAccountFraction(q->field("fraction").value<MyMoneyMoney>().formatMoney("", 0, false).toUInt());
            newSecurity.setPricePrecision(q->field("pricePrecision").toUInt());
            newSecurity.setTradingCurrency(q->field("tradingCurrencyEdit").value<MyMoneySecurity>().id());
            newSecurity.setSecurityType(type);
            newSecurity.setRoundingMethod(roundingMethod);
            newSecurity.deletePair("kmm-online-source");
            newSecurity.deletePair("kmm-online-quote-system");
            newSecurity.deletePair("kmm-online-factor");

            if (!q->field("onlineSourceCombo").toString().isEmpty()) {
                if (q->field("useFinanceQuote").toBool()) {
                    newSecurity.setValue("kmm-online-quote-system", "Finance::Quote");
                }
                newSecurity.setValue("kmm-online-source", q->field("onlineSourceCombo").toString());
            }
            if (!q->field("onlineSourceCombo").toString().isEmpty() && (q->field("onlineFactor").value<MyMoneyMoney>() != MyMoneyMoney::ONE))
                newSecurity.setValue("kmm-online-factor", q->field("onlineFactor").value<MyMoneyMoney>().toString());

            newSecurity.setValue("kmm-security-id", q->field("investmentIdentification").toString());

            if (m_security.id().isEmpty() || newSecurity != m_security) {
                m_security = newSecurity;

                // add or update it
                if (m_security.id().isEmpty()) {
                    file->addSecurity(m_security);
                } else {
                    file->modifySecurity(m_security);
                }
            }

            if (m_createAccount) {
                // now that the security exists, we can add the account to store it
                m_account.setName(q->field("accountName").toString());
                if (m_account.accountType() == eMyMoney::Account::Type::Unknown)
                    m_account.setAccountType(eMyMoney::Account::Type::Stock);

                m_account.setCurrencyId(m_security.id());
                if (!q->field("priceMode").toString().isEmpty())
                    m_account.setPriceMode(q->field("priceMode").value<eMyMoney::Invest::PriceMode>());
                else
                    m_account.setPriceMode(eMyMoney::Invest::PriceMode::Price);

                // update account's fraction in case its security fraction has changed
                // otherwise KMM restart is required because this won't happen automatically
                m_account.fraction(m_security);
                if (m_account.id().isEmpty()) {
                    MyMoneyAccount parent = file->account(parentAccountId);
                    file->addAccount(m_account, parent);
                } else
                    file->modifyAccount(m_account);
            }
            ft.commit();
        } catch (const MyMoneyException& e) {
            KMessageBox::detailedError(q, i18n("Unexpected error occurred while adding new investment"), QString::fromLatin1(e.what()));
        }
    }

    KNewInvestmentWizard      *q_ptr;
    Ui::KNewInvestmentWizard  *ui;

    MyMoneyAccount    m_account;
    MyMoneySecurity   m_security;
    bool              m_createAccount;
};

KNewInvestmentWizard::KNewInvestmentWizard(QWidget *parent) :
    QWizard(parent),
    d_ptr(new KNewInvestmentWizardPrivate(this))
{
    Q_D(KNewInvestmentWizard);
    d->init();
}

KNewInvestmentWizard::KNewInvestmentWizard(const MyMoneyAccount& account, QWidget* parent)
    : QWizard(parent)
    , d_ptr(new KNewInvestmentWizardPrivate(this))
{
    Q_D(KNewInvestmentWizard);
    d->m_account = account;
    d->m_security = MyMoneyFile::instance()->security(account.currencyId());

    setWindowTitle(i18nc("@info:window", "Investment detail wizard"));
    d->init();

    // load the widgets with the data
    setField("accountName", account.name());
    setField("investmentName", d->m_security.name());
    setField("investmentIdentification", d->m_security.value("kmm-security-id"));
    setField("priceMode", QVariant::fromValue<eMyMoney::Invest::PriceMode>(d->m_account.priceMode()));
}

KNewInvestmentWizard::KNewInvestmentWizard(const MyMoneySecurity& security, QWidget *parent) :
    QWizard(parent),
    d_ptr(new KNewInvestmentWizardPrivate(this))
{
    Q_D(KNewInvestmentWizard);
    d->m_security = security;
    setWindowTitle(i18nc("@info:window", "Security detail wizard"));
    d->init();

    d->ui->m_investmentDetailsPage->setTitle(i18nc("@info:tab Security edit wizard detail page", "Security details"));

    // We are editing the security, so we cannot create an account
    d->m_createAccount = false;

    // load the widgets with the data
    setField("investmentName", security.name());
    setField("investmentIdentification", security.value("kmm-security-id"));

    // no chance to change the price mode here
    setField("priceMode", QVariant::fromValue<eMyMoney::Invest::PriceMode>(eMyMoney::Invest::PriceMode::Price));

    // start with the details page if editing a security
    setStartId(d->pageId(d->ui->m_investmentDetailsPage));
    d->ui->m_investmentTypePage->hide();
}

KNewInvestmentWizard::~KNewInvestmentWizard()
{
}

void KNewInvestmentWizard::newInvestment(const MyMoneyAccount& parent)
{
    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard;
    if (dlg->exec() == QDialog::Accepted)
        dlg->d_func()->createAccountAndSecurity(parent.id());
    delete dlg;
}

void KNewInvestmentWizard::newInvestment(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
    QString dontShowAgain = "CreateNewInvestments";
    if (KMessageBox::questionTwoActions(nullptr,
                                        i18n("<qt>The security <b>%1</b> currently does not exist as sub-account of <b>%2</b>. "
                                             "Do you want to create it?</qt>",
                                             account.name(),
                                             parent.name()),
                                        i18n("Create security"),
                                        KMMYesNo::yes(),
                                        KMMYesNo::no(),
                                        dontShowAgain)
        == KMessageBox::PrimaryAction) {
        QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard;
        dlg->setField("accountName", account.name());
        if (dlg->exec() == QDialog::Accepted) {
            dlg->d_func()->createAccountAndSecurity(parent.id());
            account = dlg->d_func()->m_account;
        }
        delete dlg;
    } else {
        // in case the user said no but turned on the don't show again selection, we will enable
        // the message no matter what. Otherwise, the user is not able to use this feature
        // in the future anymore.
        KMessageBox::enableMessage(dontShowAgain);
    }
}

void KNewInvestmentWizard::editInvestment(const MyMoneyAccount& investment)
{
    if (!investment.id().isEmpty()) {
        QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(investment);
        if (dlg->exec() == QDialog::Accepted)
            dlg->d_func()->createAccountAndSecurity(investment.id());
        delete dlg;
    }
}

void KNewInvestmentWizard::editSecurity(const MyMoneySecurity& security)
{
    if (!security.id().isEmpty()) {
        QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(security);
        if (dlg->exec() == QDialog::Accepted)
            dlg->d_func()->createAccountAndSecurity(QString());
        delete dlg;
    }
}
