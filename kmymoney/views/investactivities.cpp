/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
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
#include "kmymoneyaccountcombo.h"
#include "amountedit.h"
#include "kmymoneyaccountselector.h"
#include "kmymoneycompletion.h"
#include "kmymoneysettings.h"
#include "mymoneyfile.h"
#include "mymoneysplit.h"
#include "mymoneyaccount.h"
#include "mymoneysecurity.h"
#include "dialogenums.h"
#include "mymoneyenums.h"
#include "widgethintframe.h"
#include "splitmodel.h"

using namespace Invest;

class Invest::ActivityPrivate
{
    Q_DISABLE_COPY(ActivityPrivate)

public:
    ActivityPrivate(InvestTransactionEditor* parent)
        : editor(parent)
    {
    }

    template <typename T>
    inline T* haveWidget(const QString &aName) const
    {
        return editor->findChild<T*>(aName);
    }

    template <typename T>
    inline T* haveVisibleWidget(const QString &aName) const
    {
        auto widget = editor->findChild<T*>(aName);
        if (!widget) {
            qDebug() << "Widget with name" << aName << "not found";
        }
        if (widget && widget->isVisible())
            return widget;
        return nullptr;
    }

    void createAssetAccountSplit(MyMoneySplit& split, const MyMoneySplit& stockSplit) const
    {
        auto cat = haveWidget<KMyMoneyAccountCombo>("assetAccountCombo");
        if (cat) {
            auto categoryId = cat->getSelected();
            split.setAccountId(categoryId);
        }
        split.setMemo(stockSplit.memo());
    }

    MyMoneyMoney sumSplits(const MyMoneySplit& s0, const SplitModel* feesModel, const SplitModel* interestModel) const
    {
        auto total = s0.value();

        if (feesModel) {
            total += feesModel->valueSum();
        }
        if (interestModel) {
            total += interestModel->valueSum();
        }
        return total;
    }

    eDialogs::PriceMode priceMode() const
    {
        eDialogs::PriceMode mode = eDialogs::PriceMode::Price;
        auto sec = haveWidget<QComboBox>("securityAccountCombo");

        QString accId;
        if (sec && !sec->currentText().isEmpty()) {
            const auto idx = sec->model()->index(sec->currentIndex(), 0);
            accId = idx.data(eMyMoney::Model::IdRole).toString();
        }
        while (!accId.isEmpty() && mode == eDialogs::PriceMode::Price) {
            auto acc = MyMoneyFile::instance()->account(accId);
            if (acc.value("priceMode").isEmpty())
                accId = acc.parentAccountId();
            else
                mode = static_cast<eDialogs::PriceMode>(acc.value("priceMode").toInt());
        }

        // if mode is still <Price> then use that
        if (mode == eDialogs::PriceMode::Price) {
            mode = eDialogs::PriceMode::PricePerShare;
        }
        return mode;
    }

    MyMoneyMoney valueAllShares() const
    {
        const auto shares = haveVisibleWidget<AmountEdit>("sharesAmountEdit");
        const auto price = haveVisibleWidget<AmountEdit>("priceAmountEdit");
        MyMoneyMoney result;
        if (shares && price) {
            if (priceMode() == eDialogs::PriceMode::PricePerShare) {
                result = shares->value() * price->value();
            } else {
                result = price->value();
            }
        }
        return result;
    }

    InvestTransactionEditor* editor;
    QString actionString;
};

Activity::Activity(InvestTransactionEditor* editor, const QString& action)
    : d_ptr(new ActivityPrivate(editor))
{
    Q_D(Activity);
    d->actionString = action;
}

Activity::~Activity()
{
    Q_D(Activity);
    delete d;
}

void Activity::setupWidgets(const QStringList& activityWidgets) const
{
    static const QStringList dynamicWidgetNames = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
        // clang-format on
    };

    setWidgetVisibility(dynamicWidgetNames, false);

    setLabelText("priceLabel", priceLabelText());
    setLabelText("sharesLabel", sharesLabelText());

    static const QStringList standardWidgetNames = {
        // clang-format off
        "activityLabel", "activityCombo",
        "dateLabel", "dateEdit",
        "securityLabel", "securityAccountCombo",

        // the ones in between are dynamically handled

        "memoLabel", "memoEdit",
        "statusLabel", "statusCombo",
        "enterButton", "cancelButton",
        // clang-format on
    };

    setWidgetVisibility(standardWidgetNames, true);
    setWidgetVisibility(activityWidgets, true);
}

bool Activity::haveCategoryAndAmount(const QString& categoryWidget, const QString& amountWidget, fieldRequired_t optional) const
{
    Q_D(const Activity);
    const auto cat = d->haveVisibleWidget<KMyMoneyAccountCombo>(categoryWidget);
    const auto amount = d->haveVisibleWidget<AmountEdit>(amountWidget);
    auto rc = true;

    if (cat && amount) {
        switch(optional) {
        case Unused:
            break;
        case Optional:
            // both must be filled or empty to be OK
            rc = cat->currentText().isEmpty() == amount->value().isZero();
            break;
        case Mandatory:
            // both must be filled to be OK
            rc = !(cat->currentText().isEmpty() || amount->value().isZero());
            break;
        }
    }
    return rc;
}

bool Activity::haveFees(fieldRequired_t optional) const
{
    return haveCategoryAndAmount("feesCombo", "feesAmountEdit", optional);
}

bool Activity::haveInterest(fieldRequired_t optional) const
{
    return haveCategoryAndAmount("interestCombo", "interestAmountEdit", optional);
}

MyMoneyMoney Activity::totalAmount(const MyMoneySplit& stockSplit, const SplitModel* feesModel, const SplitModel* interestModel) const
{
    auto result = stockSplit.value();

    if (feesModel && (feesRequired() != Unused)) {
        result += feesModel->valueSum();
    }

    if (interestModel && (interestRequired() != Unused)) {
        result += interestModel->valueSum();
    }

    return result;
}

void Activity::setLabelText(const QString& idx, const QString& txt) const
{
    Q_D(const Activity);
    auto w = d->haveWidget<QLabel>(idx);
    if (w) {
        w->setText(txt);
    }
}

void Activity::setWidgetVisibility(const QStringList& widgetIds, bool visible) const
{
    Q_D(const Activity);
    for (const auto& name : qAsConst(widgetIds)) {
        auto w = d->haveWidget<QWidget>(name);
        if (w) {
            w->setVisible(visible);
        } else {
            qCritical() << "Activity::setWidgetVisibility unknown widget" << name;
        }
    }
}

QString Invest::Activity::sharesLabelText() const
{
    return i18nc("@label:textbox", "Shares");
}

QString Activity::priceLabelText() const
{
    Q_D(const Activity);
    switch (d->priceMode()) {
    default:
    case eDialogs::PriceMode::Price:
        break;
    case eDialogs::PriceMode::PricePerShare:
        return i18nc("@label:textbox", "Price/share");
    case eDialogs::PriceMode::PricePerTransaction:
        return i18nc("@label:textbox", "Transaction amount");
    }
    return i18nc("@label:textbox", "Price");
}

void Activity::loadPriceWidget(const MyMoneySplit & split)
{
    Q_D(const Activity);
    auto priceEdit = d->haveWidget<AmountEdit>("priceAmountEdit");

    if (priceEdit && !split.accountId().isEmpty()) {
        const auto account = MyMoneyFile::instance()->account(split.accountId());
        const auto currency = MyMoneyFile::instance()->currency(account.tradingCurrencyId());
        priceEdit->setCommodity(currency);
        if (d->priceMode() == eDialogs::PriceMode::PricePerTransaction) {
            priceEdit->setValue(split.value());
        } else {
            priceEdit->setValue(split.price());
        }
    }
}

MyMoneyMoney Activity::sharesFactor() const
{
    return MyMoneyMoney::ONE;
}

MyMoneyMoney Activity::feesFactor() const
{
    return MyMoneyMoney::ONE;
}

MyMoneyMoney Activity::interestFactor() const
{
    return MyMoneyMoney::MINUS_ONE;
}

QString Activity::actionString() const
{
    Q_D(const Activity);
    return d->actionString;
}

MyMoneyMoney Activity::valueAllShares() const
{
    Q_D(const Activity);
    return d->valueAllShares();
}

eDialogs::PriceMode Activity::priceMode() const
{
    Q_D(const Activity);
    return d->priceMode();
}

Buy::Buy(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Buy"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}

Sell::Sell(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Buy"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}

Invest::Activity::fieldRequired_t Invest::Sell::assetAccountRequired() const
{
    Q_D(const Activity);
    return d->editor->totalAmount().isZero() ? Unused : Mandatory;
}

MyMoneyMoney Sell::sharesFactor() const
{
    return MyMoneyMoney::MINUS_ONE;
}

Div::Div(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Dividend"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "assetAccountLabel", "assetAccountCombo",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}

Reinvest::Reinvest(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Reinvest"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}


MyMoneyMoney Invest::Reinvest::totalAmount(const MyMoneySplit & stockSplit, const SplitModel * feesModel, const SplitModel * interestModel) const
{
    Q_UNUSED(stockSplit)
    Q_UNUSED(feesModel)
    Q_UNUSED(interestModel)

    return {};
}

Add::Add(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Add"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}


void Add::adjustStockSplit(MyMoneySplit& stockSplit)
{
    stockSplit.setValue(MyMoneyMoney());
    stockSplit.setPrice(MyMoneyMoney());
}

MyMoneyMoney Invest::Add::totalAmount(const MyMoneySplit & stockSplit, const SplitModel * feesModel, const SplitModel * interestModel) const
{
    Q_UNUSED(stockSplit)
    Q_UNUSED(feesModel)
    Q_UNUSED(interestModel)

    return {};
}

Remove::Remove(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Add"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}

void Remove::adjustStockSplit(MyMoneySplit& stockSplit)
{
    stockSplit.setValue(MyMoneyMoney());
    stockSplit.setPrice(MyMoneyMoney());
}

MyMoneyMoney Invest::Remove::totalAmount(const MyMoneySplit & stockSplit, const SplitModel * feesModel, const SplitModel * interestModel) const
{
    Q_UNUSED(stockSplit)
    Q_UNUSED(feesModel)
    Q_UNUSED(interestModel)

    return {};
}


MyMoneyMoney Remove::sharesFactor() const
{
    return MyMoneyMoney::MINUS_ONE;
}

Invest::Split::Split(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("Split"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "sharesLabel", "sharesAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}



QString Invest::Split::sharesLabelText() const
{
    return i18nc("@label:textbox", "Ratio 1/");
}

void Invest::Split::adjustStockSplit(MyMoneySplit & stockSplit)
{
    stockSplit.setValue(MyMoneyMoney());
    stockSplit.setPrice(MyMoneyMoney());
}

MyMoneyMoney Invest::Split::totalAmount(const MyMoneySplit & stockSplit, const SplitModel * feesModel, const SplitModel * interestModel) const
{
    Q_UNUSED(stockSplit)
    Q_UNUSED(feesModel)
    Q_UNUSED(interestModel)

    return {};
}

IntInc::IntInc(InvestTransactionEditor* editor)
    : Activity(editor, QLatin1String("IntIncome"))
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
    static const QStringList activityWidgets = {
        // clang-format off
        "assetAccountLabel", "assetAccountCombo",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
        // clang-format on
    };

    setupWidgets(activityWidgets);
}
