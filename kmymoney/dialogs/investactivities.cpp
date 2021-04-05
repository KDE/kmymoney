/*
    SPDX-FileCopyrightText: 2007-2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "investactivities.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLabel>
#include <QList>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KLocalizedString>

// ----------------------------------------------------------------------------
// Project Includes

#include "investtransactioneditor.h"
#include "mymoneymoney.h"
#include "kmymoneycategory.h"
#include "amountedit.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneycompletion.h"
#include <kmymoneysettings.h>
#include "mymoneyfile.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "dialogenums.h"
#include "mymoneyenums.h"

using namespace Invest;
using namespace KMyMoneyRegister;

class Invest::ActivityPrivate
{
    Q_DISABLE_COPY(ActivityPrivate)

public:
    ActivityPrivate() :
        m_parent(nullptr),
        m_memoChanged(false)
    {
    }

    InvestTransactionEditor     *m_parent;
    QMap<QString, MyMoneyMoney>  m_priceInfo;
    bool                         m_memoChanged;
    QString                      m_memoText;
};


Activity::Activity(InvestTransactionEditor* editor) :
    d_ptr(new ActivityPrivate)
{
    Q_D(Activity);
    d->m_memoChanged = false;
    d->m_parent = editor;
}

Activity::~Activity()
{
    Q_D(Activity);
    delete d;
}

bool& Activity::memoChanged()
{
    Q_D(Activity);
    return d->m_memoChanged;
}

QString& Activity::memoText()
{
    Q_D(Activity);
    return d->m_memoText;
}

bool Activity::isComplete(QString& reason) const
{
    Q_D(const Activity);
    Q_UNUSED(reason)

    auto rc = false;
    auto security = dynamic_cast<KMyMoneySecurity*>(haveWidget("security"));
    if (security && !security->currentText().isEmpty()) {
        rc = (security->selector()->contains(security->currentText()) || (isMultiSelection() && d->m_memoChanged));
    }
    return rc;
}

QWidget* Activity::haveWidget(const QString& name) const
{
    Q_D(const Activity);
    return d->m_parent->haveWidget(name);
}

bool Activity::haveAssetAccount() const
{
    auto rc = true;
    auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
    if (!cat)
        return false;

    if (!isMultiSelection())
        rc = !cat->currentText().isEmpty();

    if (rc && !cat->currentText().isEmpty())
        rc = cat->selector()->contains(cat->currentText());

    return rc;
}

bool Activity::haveCategoryAndAmount(const QString& category, const QString& amount, bool optional) const
{
    Q_D(const Activity);
    auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget(category));

    auto rc = true;

    if (cat && !cat->currentText().isEmpty()) {
        rc = cat->selector()->contains(cat->currentText()) || cat->isSplitTransaction();
        if (rc && !amount.isEmpty() && !isMultiSelection()) {
            if (cat->isSplitTransaction()) {
                QList<MyMoneySplit>::const_iterator split;
                QList<MyMoneySplit>::const_iterator splitEnd;

                if (category == "fee-account") {
                    split = d->m_parent->feeSplits().cbegin();
                    splitEnd = d->m_parent->feeSplits().cend();
                } else if (category == "interest-account") {
                    split = d->m_parent->interestSplits().cbegin();
                    splitEnd = d->m_parent->interestSplits().cend();
                }

                for (; split != splitEnd; ++split) {
                    if ((*split).value().isZero())
                        rc = false;
                }
            } else {
                if (auto valueWidget = dynamic_cast<AmountEdit*>(haveWidget(amount)))
                    rc = !valueWidget->value().isZero();
            }
        }
    } else if (!isMultiSelection() && !optional) {
        rc = false;
    }
    return rc;
}

bool Activity::haveFees(bool optional) const
{
    return haveCategoryAndAmount("fee-account", "fee-amount", optional);
}

bool Activity::haveInterest(bool optional) const
{
    return haveCategoryAndAmount("interest-account", "interest-amount", optional);
}

bool Activity::haveShares() const
{
    if (auto amount = dynamic_cast<AmountEdit*>(haveWidget("shares"))) {
        if (isMultiSelection() && amount->value().isZero())
            return true;

        return !amount->value().isZero();
    }
    return false;
}

bool Activity::havePrice() const
{
    if (auto amount = dynamic_cast<AmountEdit*>(haveWidget("price"))) {
        if (isMultiSelection() && amount->value().isZero())
            return true;

        return !amount->value().isZero();
    }
    return false;
}

bool Activity::isMultiSelection() const
{
    Q_D(const Activity);
    return d->m_parent->isMultiSelection();
}

bool Activity::createCategorySplits(const MyMoneyTransaction& t, KMyMoneyCategory* cat, AmountEdit* amount, MyMoneyMoney factor, QList<MyMoneySplit>&splits, const QList<MyMoneySplit>& osplits) const
{
    Q_D(const Activity);
    auto rc = true;
    if (!isMultiSelection() || !cat->currentText().isEmpty()) {
        if (!cat->isSplitTransaction()) {
            splits.clear();
            MyMoneySplit s1;
            // base the resulting split on the original split
            // that was provided which avoids loosing data
            // stored in the split
            if (!osplits.isEmpty()) {
                s1 = osplits.first();
                s1.clearId();
            }
            QString categoryId;
            categoryId = cat->selectedItem();
            if (!categoryId.isEmpty()) {
                s1.setAccountId(categoryId);
                s1.setValue(amount->value() * factor);
                if (!s1.value().isZero()) {
                    rc = d->m_parent->setupPrice(t, s1);
                }
                splits.append(s1);
            }
        } else {
            splits = osplits;
        }
    }
    return rc;
}

void Activity::createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const
{
    auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
    if (cat && (!isMultiSelection() || !cat->currentText().isEmpty())) {
        auto categoryId = cat->selectedItem();
        split.setAccountId(categoryId);
    }
    split.setMemo(stockSplit.memo());
}

MyMoneyMoney Activity::sumSplits(const MyMoneySplit& s0, const QList<MyMoneySplit>& feeSplits, const QList<MyMoneySplit>& interestSplits) const
{
    auto total = s0.value();
    foreach (const auto feeSplit, feeSplits)
        total += feeSplit.value();

    foreach (const auto interestSplit, interestSplits)
        total += interestSplit.value();

    return total;
}

void Activity::setLabelText(const QString& idx, const QString& txt) const
{
    auto w = dynamic_cast<QLabel*>(haveWidget(idx));
    if (w) {
        w->setText(txt);
    } else {
        if (KMyMoneySettings::transactionForm()) {
            // labels are only used in the transaction form
            qDebug("Unknown QLabel named '%s'", qPrintable(idx));
        }
    }
}

void Activity::preloadAssetAccount()
{
    Q_D(Activity);
    auto cat = dynamic_cast<KMyMoneyCategory*>(haveWidget("asset-account"));
    if (cat && cat->isVisible()) {
        if (cat->currentText().isEmpty()) {
            MyMoneyAccount acc = MyMoneyFile::instance()->accountByName(i18n("%1 (Brokerage)", d->m_parent->account().name()));
            if (!acc.id().isEmpty()) {
                bool blocked = cat->signalsBlocked();
                // block signals, so that the focus does not go crazy
                cat->blockSignals(true);
                cat->completion()->setSelected(acc.id());
                cat->slotItemSelected(acc.id());
                cat->blockSignals(blocked);
            }
        }
    }
}

void Activity::setWidgetVisibility(const QStringList& widgetIds, bool visible) const
{
    for (QStringList::const_iterator it_w = widgetIds.constBegin(); it_w != widgetIds.constEnd(); ++it_w) {
        auto w = haveWidget(*it_w);
        if (w) {
            // in case we hit a category with a split button,
            // we need to manipulate the enclosing QFrame
            auto cat = dynamic_cast<KMyMoneyCategory*>(w);
            if (cat && cat->splitButton()) {
                cat->parentWidget()->setVisible(visible);
            } else {
                w->setVisible(visible);
            }
        }
    }
}

eDialogs::PriceMode Activity::priceMode() const
{
    Q_D(const Activity);
    return d->m_parent->priceMode();
}

QString Activity::priceLabel() const
{
    QString label;
    if (priceMode() == eDialogs::PriceMode::Price) {
        label = i18n("Price");
    } else if (priceMode() == eDialogs::PriceMode::PricePerShare) {
        label = i18n("Price/share");
    } else if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
        label = i18n("Transaction amount");
    }
    return label;
}

Buy::Buy(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Buy::~Buy()
{
}

eMyMoney::Split::InvestmentTransactionType Buy::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::BuyShares;
}

void Buy::showWidgets() const
{
    static const QStringList visibleWidgetIds = {"asset-account", "shares", "price", "total", "fee-account", "fee-amount", "interest-account", "interest-amount",};
    setWidgetVisibility(visibleWidgetIds, true);

    setLabelText("interest-amount-label", i18n("Interest"));
    setLabelText("interest-label", i18n("Interest"));
    setLabelText("fee-amount-label", i18n("Fees"));
    setLabelText("fee-label", i18n("Fees"));
    setLabelText("asset-label", i18n("Account"));
    setLabelText("shares-label", i18n("Shares"));
    setLabelText("price-label", priceLabel());
    setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Buy::isComplete(QString& reason) const
{
    auto rc = Activity::isComplete(reason);
    rc &= haveAssetAccount();
    rc &= haveFees(true);
    rc &= haveShares();
    rc &= havePrice();

    return rc;
}

bool Buy::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_D(Activity);
    Q_UNUSED(m_interestSplits);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"));
    auto priceEdit = dynamic_cast<AmountEdit*>(haveWidget("price"));

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);

    MyMoneyMoney shares = s0.shares();
    MyMoneyMoney price;
    if (!s0.shares().isZero())
        price = (s0.value() / s0.shares()).reduce();

    if (sharesEdit && (!isMultiSelection() || !sharesEdit->value().isZero())) {
        shares = sharesEdit->value().abs();
        s0.setShares(shares);
        s0.setValue((shares * price).reduce());
        s0.setPrice(price);
    }
    if (priceEdit && (!isMultiSelection() || !priceEdit->value().isZero())) {
        price = priceEdit->value().abs();
        if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
            s0.setValue(price.reduce());
            if (!s0.shares().isZero())
                s0.setPrice((price / s0.shares()).reduce());
        } else {
            s0.setValue((shares * price).reduce());
            s0.setPrice(price);
        }
    }

    auto feeAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
    auto feeAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
    if (!feeAccountWidget || !feeAmountWidget //
            || !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
        return false;

    createAssetAccountSplit(assetAccountSplit, s0);

    MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

    //  Clear any leftover value from previous Dividend.
    interestSplits.clear();

    assetAccountSplit.setValue(-total);

    if (!d->m_parent->setupPrice(t, assetAccountSplit))
        return false;

    return true;
}

Sell::Sell(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Sell::~Sell()
{
}

eMyMoney::Split::InvestmentTransactionType Sell::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::SellShares;
}

void Sell::showWidgets() const
{
    Q_D(const Activity);
    static const QStringList visibleWidgetIds = QStringList{"asset-account", "interest-amount", "fee-amount", "shares", "price", "total", "interest-account", "fee-account"};
    setWidgetVisibility(visibleWidgetIds, true);

    if (auto shareEdit = dynamic_cast<AmountEdit*>(haveWidget("shares")))
        shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));

    setLabelText("interest-amount-label", i18n("Interest"));
    setLabelText("interest-label", i18n("Interest"));
    setLabelText("fee-amount-label", i18n("Fees"));
    setLabelText("fee-label", i18n("Fees"));
    setLabelText("asset-label", i18n("Account"));
    setLabelText("shares-label", i18n("Shares"));
    setLabelText("price-label", priceLabel());
    setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Sell::isComplete(QString& reason) const
{
    Q_D(const Activity);

    auto rc = Activity::isComplete(reason);
    rc &= haveFees(true);
    rc &= haveInterest(true);
    rc &= haveShares();
    rc &= havePrice();

    // Allow a sell operation to be saved without specifying a brokerage
    // account, when the proceeds equal the fees. This will handle sales
    // made solely to cover annual account fees, where there is no money
    // transferred.
    if (rc) {
        if (!d->m_parent->totalAmount().isZero()) {
            rc &= haveAssetAccount();
        }
    }
    return rc;
}

bool Sell::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_D(Activity);
    Q_UNUSED(m_interestSplits);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"));
    auto priceEdit = dynamic_cast<AmountEdit*>(haveWidget("price"));

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::BuyShares);

    MyMoneyMoney shares = s0.shares();
    MyMoneyMoney price;
    if (!s0.shares().isZero())
        price = (s0.value() / s0.shares()).reduce();

    if (sharesEdit && (!isMultiSelection() || !sharesEdit->value().isZero())) {
        shares = -sharesEdit->value().abs();
        s0.setShares(shares);
        s0.setValue((shares * price).reduce());
        s0.setPrice(price);
    }
    if (priceEdit && (!isMultiSelection() || !priceEdit->value().isZero())) {
        price = priceEdit->value().abs();
        if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
            price = -price;
            s0.setValue(price.reduce());
            if (!s0.shares().isZero())
                s0.setPrice((price / s0.shares()).reduce());
        } else {
            s0.setValue((shares * price).reduce());
            s0.setPrice(price);
        }
    }

    auto feeAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
    auto feeAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
    if (!feeAccountWidget || !feeAmountWidget //
            || !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
        return false;

    auto interestAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
    auto interestAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("interest-amount"));
    if (!interestAccountWidget || !interestAmountWidget //
            || !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
        return false;

    const auto total = sumSplits(s0, feeSplits, interestSplits);
    if (!total.isZero()) {
        createAssetAccountSplit(assetAccountSplit, s0);
        assetAccountSplit.setValue(-total);

        if (!d->m_parent->setupPrice(t, assetAccountSplit))
            return false;
    }

    return true;
}

Div::Div(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Div::~Div()
{
}

eMyMoney::Split::InvestmentTransactionType Div::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::Dividend;
}

void Div::showWidgets() const
{
    static const QStringList visibleWidgetIds = {"asset-account", "interest-amount", "fee-amount", "total", "interest-account", "fee-account"};
    setWidgetVisibility(visibleWidgetIds, true);
    static const QStringList hiddenWidgetIds = {"shares", "price"};
    setWidgetVisibility(hiddenWidgetIds, false);

    setLabelText("interest-amount-label", i18n("Interest"));
    setLabelText("interest-label", i18n("Interest"));
    setLabelText("fee-amount-label", i18n("Fees"));
    setLabelText("fee-label", i18n("Fees"));
    setLabelText("asset-label", i18n("Account"));
    setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Div::isComplete(QString& reason) const
{
    Q_UNUSED(reason)

    auto rc = Activity::isComplete(reason);
    rc &= haveAssetAccount();
    rc &= haveCategoryAndAmount("interest-account", QString(), false);
    rc &= haveInterest(false);
    return rc;
}

bool Div::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_D(Activity);
    Q_UNUSED(m_feeSplits);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::Dividend);

    // for dividends, we only use the stock split as a marker
    MyMoneyMoney shares;
    s0.setShares(shares);
    s0.setValue(shares);
    s0.setPrice(MyMoneyMoney::ONE);

    auto feeAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
    auto feeAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
    if (!feeAccountWidget || !feeAmountWidget //
            || !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
        return false;

    auto interestAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
    auto interestAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("interest-amount"));
    if (!interestAccountWidget || !interestAmountWidget //
            || !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
        return false;

    createAssetAccountSplit(assetAccountSplit, s0);

    MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
    assetAccountSplit.setValue(-total);

    if (!d->m_parent->setupPrice(t, assetAccountSplit))
        return false;

    return true;
}

Reinvest::Reinvest(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Reinvest::~Reinvest()
{
}

eMyMoney::Split::InvestmentTransactionType Reinvest::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::ReinvestDividend;
}

void Reinvest::showWidgets() const
{
    Q_D(const Activity);
    static const QStringList visibleWidgetIds = QStringList{"price", "fee-account", "interest-account"};
    setWidgetVisibility(visibleWidgetIds, true);

    if (auto shareEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"))) {
        shareEdit->show();
        shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
    }

    setLabelText("interest-amount-label", i18n("Interest"));
    setLabelText("interest-label", i18n("Interest"));
    setLabelText("fee-amount-label", i18n("Fees"));
    setLabelText("fee-label", i18n("Fees"));
    setLabelText("interest-label", i18n("Interest"));
    setLabelText("shares-label", i18n("Shares"));
    setLabelText("price-label", priceLabel());
    setLabelText("total-label", i18nc("Total value", "Total"));
}

bool Reinvest::isComplete(QString& reason) const
{
    auto rc = Activity::isComplete(reason);
    rc &= haveCategoryAndAmount("interest-account", QString(), false);
    rc &= haveFees(true);
    rc &= haveShares();
    rc &= havePrice();
    return rc;
}

bool Reinvest::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_D(Activity);
    Q_UNUSED(assetAccountSplit);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"));
    auto priceEdit = dynamic_cast<AmountEdit*>(haveWidget("price"));

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::ReinvestDividend);

    MyMoneyMoney shares = s0.shares();
    MyMoneyMoney price;
    if (!s0.shares().isZero())
        price = (s0.value() / s0.shares()).reduce();

    if (sharesEdit && (!isMultiSelection() || !sharesEdit->value().isZero())) {
        shares = sharesEdit->value().abs();
        s0.setShares(shares);
        s0.setValue((shares * price).reduce());
        s0.setPrice(price);
    }
    if (priceEdit && (!isMultiSelection() || !priceEdit->value().isZero())) {
        price = priceEdit->value().abs();
        if (priceMode() == eDialogs::PriceMode::PricePerTransaction) {
            s0.setValue(price.reduce());
            if (!s0.shares().isZero())
                s0.setPrice((price / s0.shares()).reduce());
        } else {
            s0.setValue((shares * price).reduce());
            s0.setPrice(price);
        }
    }

    auto feeAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
    auto feeAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
    if (feeAmountWidget && feeAccountWidget) {
        if (!createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
            return false;
    }

    auto interestAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
    auto interestAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("interest-amount"));
    if (!interestAccountWidget || !interestAmountWidget //
            || !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
        return false;

    if (interestSplits.count() != 1) {
        qDebug("more or less than one interest split in Reinvest::createTransaction. Not created.");
        return false;
    }
    assetAccountSplit.setAccountId(QString());

    MyMoneySplit& s1 = interestSplits[0];

    MyMoneyMoney total = sumSplits(s0, feeSplits, QList<MyMoneySplit>());

    s1.setValue(-total);

    if (!d->m_parent->setupPrice(t, s1))
        return false;

    return true;
}

Add::Add(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Add::~Add()
{
}

eMyMoney::Split::InvestmentTransactionType Add::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::AddShares;
}

void Add::showWidgets() const
{
    Q_D(const Activity);
    if (auto shareEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"))) {
        shareEdit->show();
        shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
    }

    setLabelText("shares-label", i18n("Shares"));
}

bool Add::isComplete(QString& reason) const
{
    auto rc = Activity::isComplete(reason);
    rc &= haveShares();
    return rc;
}

bool Add::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_UNUSED(t);
    Q_UNUSED(assetAccountSplit);
    Q_UNUSED(m_feeSplits);
    Q_UNUSED(m_interestSplits);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"));

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
    if (sharesEdit)
        s0.setShares(sharesEdit->value().abs());
    s0.setValue(MyMoneyMoney());
    s0.setPrice(MyMoneyMoney());

    assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

    feeSplits.clear();
    interestSplits.clear();

    return true;
}

Remove::Remove(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Remove::~Remove()
{
}

eMyMoney::Split::InvestmentTransactionType Remove::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::RemoveShares;
}

void Remove::showWidgets() const
{
    Q_D(const Activity);
    if (auto shareEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"))) {
        shareEdit->show();
        shareEdit->setPrecision(MyMoneyMoney::denomToPrec(d->m_parent->security().smallestAccountFraction()));
    }
    setLabelText("shares-label", i18n("Shares"));
}

bool Remove::isComplete(QString& reason) const
{
    auto rc = Activity::isComplete(reason);
    rc &= haveShares();
    return rc;
}

bool Remove::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_UNUSED(t);
    Q_UNUSED(assetAccountSplit);
    Q_UNUSED(m_feeSplits);
    Q_UNUSED(m_interestSplits);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::AddShares);
    if (auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares")))
        s0.setShares(-(sharesEdit->value().abs()));
    s0.setValue(MyMoneyMoney());
    s0.setPrice(MyMoneyMoney());

    assetAccountSplit.setValue(MyMoneyMoney());//  Clear any leftover value from previous Dividend.

    feeSplits.clear();
    interestSplits.clear();

    return true;
}

Invest::Split::Split(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

Invest::Split::~Split()
{
}

eMyMoney::Split::InvestmentTransactionType Invest::Split::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::SplitShares;
}

void Invest::Split::showWidgets() const
{
    // TODO do we need a special split ratio widget?
    // TODO maybe yes, currently the precision is the one of the fraction and might differ from it
    if (auto shareEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"))) {
        shareEdit->show();
        shareEdit->setPrecision(-1);
    }
    setLabelText("shares-label", i18n("Ratio 1/"));
}

bool Invest::Split::isComplete(QString& reason) const
{
    auto rc = Activity::isComplete(reason);
    rc &= haveShares();
    return rc;
}

bool Invest::Split::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_UNUSED(t);
    Q_UNUSED(assetAccountSplit);
    Q_UNUSED(m_feeSplits);
    Q_UNUSED(m_interestSplits);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    auto sharesEdit = dynamic_cast<AmountEdit*>(haveWidget("shares"));

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::SplitShares);
    if (sharesEdit)
        s0.setShares(sharesEdit->value().abs());
    s0.setValue(MyMoneyMoney());
    s0.setPrice(MyMoneyMoney());

    feeSplits.clear();
    interestSplits.clear();

    return true;
}

IntInc::IntInc(InvestTransactionEditor* editor) :
    Activity(editor)
{
}

IntInc::~IntInc()
{
}

eMyMoney::Split::InvestmentTransactionType IntInc::type() const
{
    return eMyMoney::Split::InvestmentTransactionType::InterestIncome;
}

void IntInc::showWidgets() const
{
    static const QStringList visibleWidgetIds = {"asset-account", "interest-amount", "total", "interest-account", "fee-amount", "fee-account"};
    setWidgetVisibility(visibleWidgetIds, true);
    static const QStringList hiddenWidgetIds = {"shares", "price"};
    setWidgetVisibility(hiddenWidgetIds, false);

    setLabelText("interest-amount-label", i18n("Interest"));
    setLabelText("interest-label", i18n("Interest"));
    setLabelText("fee-amount-label", i18n("Fees"));
    setLabelText("fee-label", i18n("Fees"));
    setLabelText("asset-label", i18n("Account"));
    setLabelText("total-label", i18nc("Total value", "Total"));
}

bool IntInc::isComplete(QString& reason) const
{
    Q_UNUSED(reason)

    auto rc = Activity::isComplete(reason);
    rc &= haveAssetAccount();
    rc &= haveCategoryAndAmount("interest-account", QString(), false);
    rc &= haveInterest(false);
    return rc;
}

bool IntInc::createTransaction(MyMoneyTransaction& t, MyMoneySplit& s0, MyMoneySplit& assetAccountSplit, QList<MyMoneySplit>& feeSplits, QList<MyMoneySplit>& m_feeSplits, QList<MyMoneySplit>& interestSplits, QList<MyMoneySplit>& m_interestSplits, MyMoneySecurity& security, MyMoneySecurity& currency)
{
    Q_D(Activity);
    Q_UNUSED(security);
    Q_UNUSED(currency);

    QString reason;
    if (!isComplete(reason))
        return false;

    s0.setAction(eMyMoney::Split::InvestmentTransactionType::InterestIncome);

    // for dividends, we only use the stock split as a marker
    MyMoneyMoney shares;
    s0.setShares(shares);
    s0.setValue(shares);
    s0.setPrice(MyMoneyMoney::ONE);

    auto feeAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("fee-account"));
    auto feeAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("fee-amount"));
    if (!feeAccountWidget || !feeAmountWidget //
            || !createCategorySplits(t, feeAccountWidget, feeAmountWidget, MyMoneyMoney::ONE, feeSplits, m_feeSplits))
        return false;

    auto interestAccountWidget = dynamic_cast<KMyMoneyCategory*>(haveWidget("interest-account"));
    auto interestAmountWidget = dynamic_cast<AmountEdit*>(haveWidget("interest-amount"));
    if (!interestAccountWidget || !interestAmountWidget //
            || !createCategorySplits(t, interestAccountWidget, interestAmountWidget, MyMoneyMoney::MINUS_ONE, interestSplits, m_interestSplits))
        return false;

    createAssetAccountSplit(assetAccountSplit, s0);

    MyMoneyMoney total = sumSplits(s0, feeSplits, interestSplits);
    assetAccountSplit.setValue(-total);

    if (!d->m_parent->setupPrice(t, assetAccountSplit))
        return false;

    return true;
}
