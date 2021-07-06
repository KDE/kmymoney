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

#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "mymoneyfile.h"
#include "webpricequote.h"
#include "kmymoneyutils.h"
#include "mymoneyexception.h"

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

    void init1()
    {
        Q_Q(KNewInvestmentWizard);
        ui->m_onlineUpdatePage->slotSourceChanged(false);

        // make sure, the back button does not clear fields
        q->setOption(QWizard::IndependentPages, true);

        // enable the help button
        q->setOption(q->HaveHelpButton, true);
        q->connect(q, &KNewInvestmentWizard::helpRequested, q, &KNewInvestmentWizard::slotHelp);

        m_createAccount = true;

        // Update label in case of edit
        if (!m_account.id().isEmpty()) {
            ui->m_investmentTypePage->setIntroLabelText(i18n("This wizard allows you to modify the selected investment."));
        }
        if (!m_security.id().isEmpty()) {
            ui->m_investmentTypePage->setIntroLabelText(i18n("This wizard allows you to modify the selected security."));
        }

        KMyMoneyUtils::updateWizardButtons(q);
    }

    void init2()
    {
        ui->m_investmentTypePage->init2(m_security);
        ui->m_investmentDetailsPage->init2(m_security);
        ui->m_onlineUpdatePage->init2(m_security);
        ui->m_onlineUpdatePage->slotCheckPage(m_security.value("kmm-online-source"));
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
    d->ui->setupUi(this);
    d->init1();
    d->ui->m_onlineUpdatePage->slotCheckPage(QString());

    d->ui->m_investmentDetailsPage->setupInvestmentSymbol();

    connect(d->ui->m_investmentDetailsPage, &KInvestmentDetailsWizardPage::checkForExistingSymbol, this, &KNewInvestmentWizard::slotCheckForExistingSymbol);
}

KNewInvestmentWizard::KNewInvestmentWizard(const MyMoneyAccount& acc, QWidget *parent) :
    QWizard(parent),
    d_ptr(new KNewInvestmentWizardPrivate(this))
{
    Q_D(KNewInvestmentWizard);
    d->ui->setupUi(this);
    d->m_account = acc;
    setWindowTitle(i18n("Investment detail wizard"));
    d->init1();

    // load the widgets with the data
    setName(d->m_account.name());
    d->m_security = MyMoneyFile::instance()->security(d->m_account.currencyId());

    d->init2();

    int priceMode = 0;
    if (!d->m_account.value("priceMode").isEmpty())
        priceMode = d->m_account.value("priceMode").toInt();
    d->ui->m_investmentDetailsPage->setCurrentPriceMode(priceMode);

}

KNewInvestmentWizard::KNewInvestmentWizard(const MyMoneySecurity& security, QWidget *parent) :
    QWizard(parent),
    d_ptr(new KNewInvestmentWizardPrivate(this))
{
    Q_D(KNewInvestmentWizard);
    d->ui->setupUi(this);
    d->m_security = security;
    setWindowTitle(i18n("Security detail wizard"));
    d->init1();
    d->m_createAccount = false;

    // load the widgets with the data
    setName(security.name());

    d->init2();

    // no chance to change the price mode here
    d->ui->m_investmentDetailsPage->setCurrentPriceMode(0);
    d->ui->m_investmentDetailsPage->setPriceModeEnabled(false);
}

KNewInvestmentWizard::~KNewInvestmentWizard()
{
}

void KNewInvestmentWizard::setName(const QString& name)
{
    Q_D(KNewInvestmentWizard);
    d->ui->m_investmentDetailsPage->setName(name);
}

void KNewInvestmentWizard::slotCheckForExistingSymbol(const QString& symbol)
{
    Q_D(KNewInvestmentWizard);
    Q_UNUSED(symbol);

    if (field("investmentName").toString().isEmpty()) {
        QList<MyMoneySecurity> list = MyMoneyFile::instance()->securityList();
        auto type = static_cast<eMyMoney::Security::Type>(field("securityType").toInt());

        foreach (const MyMoneySecurity& it_s, list) {
            if (it_s.securityType() == type
                    && it_s.tradingSymbol() == field("investmentSymbol").toString()) {
                d->m_security = MyMoneySecurity();
                if (KMessageBox::questionYesNo(this, i18n("The selected symbol is already on file. Do you want to reuse the existing security?"), i18n("Security found")) == KMessageBox::Yes) {
                    d->m_security = it_s;
                    d->init2();
                    d->ui->m_investmentDetailsPage->loadName(d->m_security.name());
                }
                break;
            }
        }
    }
}

void KNewInvestmentWizard::slotHelp()
{
    KHelpClient::invokeHelp("details.investments.newinvestmentwizard");
}

void KNewInvestmentWizard::createObjects(const QString& parentId)
{
    Q_D(KNewInvestmentWizard);
    auto file = MyMoneyFile::instance();

    auto type = static_cast<eMyMoney::Security::Type>(field("securityType").toInt());
    auto roundingMethod = static_cast<AlkValue::RoundingMethod>(field("roundingMethod").toInt());
    MyMoneyFileTransaction ft;
    try {
        // update all relevant attributes only, if we create a stock
        // account and the security is unknown or we modify the security
        MyMoneySecurity newSecurity(d->m_security);
        newSecurity.setName(field("investmentName").toString());
        newSecurity.setTradingSymbol(field("investmentSymbol").toString());
        newSecurity.setTradingMarket(field("tradingMarket").toString());
        newSecurity.setSmallestAccountFraction(field("fraction").value<MyMoneyMoney>().formatMoney("", 0, false).toUInt());
        newSecurity.setPricePrecision(MyMoneyMoney(field("pricePrecision").toUInt()).formatMoney("", 0, false).toUInt());
        newSecurity.setTradingCurrency(field("tradingCurrencyEdit").value<MyMoneySecurity>().id());
        newSecurity.setSecurityType(type);
        newSecurity.setRoundingMethod(roundingMethod);
        newSecurity.deletePair("kmm-online-source");
        newSecurity.deletePair("kmm-online-quote-system");
        newSecurity.deletePair("kmm-online-factor");
        newSecurity.deletePair("kmm-security-id");

        if (!field("onlineSourceCombo").toString().isEmpty()) {
            if (field("useFinanceQuote").toBool()) {
                FinanceQuoteProcess p;
                newSecurity.setValue("kmm-online-quote-system", "Finance::Quote");
                newSecurity.setValue("kmm-online-source", p.crypticName(field("onlineSourceCombo").toString()));
            } else {
                newSecurity.setValue("kmm-online-source", field("onlineSourceCombo").toString());
            }
        }
        if (d->ui->m_onlineUpdatePage->isOnlineFactorEnabled() && (field("onlineFactor").value<MyMoneyMoney>() != MyMoneyMoney::ONE))
            newSecurity.setValue("kmm-online-factor", field("onlineFactor").value<MyMoneyMoney>().toString());
        if (!field("investmentIdentification").toString().isEmpty())
            newSecurity.setValue("kmm-security-id", field("investmentIdentification").toString());

        if (d->m_security.id().isEmpty() || newSecurity != d->m_security) {
            d->m_security = newSecurity;

            // add or update it
            if (d->m_security.id().isEmpty()) {
                file->addSecurity(d->m_security);
            } else {
                file->modifySecurity(d->m_security);
            }
        }

        if (d->m_createAccount) {
            // now that the security exists, we can add the account to store it
            d->m_account.setName(field("investmentName").toString());
            if (d->m_account.accountType() == eMyMoney::Account::Type::Unknown)
                d->m_account.setAccountType(eMyMoney::Account::Type::Stock);

            d->m_account.setCurrencyId(d->m_security.id());
            switch (d->ui->m_investmentDetailsPage->priceMode()) {
            case 0:
                d->m_account.deletePair("priceMode");
                break;
            case 1:
            case 2:
                d->m_account.setValue("priceMode", QString("%1").arg(d->ui->m_investmentDetailsPage->priceMode()));
                break;
            }
            // update account's fraction in case its security fraction has changed
            // otherwise KMM restart is required because this won't happen automatically
            d->m_account.fraction(d->m_security);
            if (d->m_account.id().isEmpty()) {
                MyMoneyAccount parent = file->account(parentId);
                file->addAccount(d->m_account, parent);
            } else
                file->modifyAccount(d->m_account);
        }
        ft.commit();
    } catch (const MyMoneyException &e) {
        KMessageBox::detailedSorry(this, i18n("Unexpected error occurred while adding new investment"), QString::fromLatin1(e.what()));
    }
}

void KNewInvestmentWizard::newInvestment(const MyMoneyAccount& parent)
{
    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard;
    if (dlg->exec() == QDialog::Accepted)
        dlg->createObjects(parent.id());
    delete dlg;
}

void KNewInvestmentWizard::newInvestment(MyMoneyAccount& account, const MyMoneyAccount& parent)
{
    QString dontShowAgain = "CreateNewInvestments";
    if (KMessageBox::questionYesNo(nullptr,
                                   i18n("<qt>The security <b>%1</b> currently does not exist as sub-account of <b>%2</b>. "
                                        "Do you want to create it?</qt>", account.name(), parent.name()), i18n("Create security"),
                                   KStandardGuiItem::yes(), KStandardGuiItem::no(), dontShowAgain) == KMessageBox::Yes) {
        QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard;
        dlg->setName(account.name());
        if (dlg->exec() == QDialog::Accepted) {
            dlg->createObjects(parent.id());
            account = dlg->account();
        }
        delete dlg;
    } else {
        // in case the user said no but turned on the don't show again selection, we will enable
        // the message no matter what. Otherwise, the user is not able to use this feature
        // in the future anymore.
        KMessageBox::enableMessage(dontShowAgain);
    }
}

void KNewInvestmentWizard::editInvestment(const MyMoneyAccount& parent)
{
    QPointer<KNewInvestmentWizard> dlg = new KNewInvestmentWizard(parent);
    if (dlg->exec() == QDialog::Accepted)
        dlg->createObjects(parent.id());
    delete dlg;
}

MyMoneyAccount KNewInvestmentWizard::account() const
{
    Q_D(const KNewInvestmentWizard);
    return d->m_account;
}
