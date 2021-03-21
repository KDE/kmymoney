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
        if (mode == eDialogs::PriceMode::Price)
            mode = eDialogs::PriceMode::PricePerShare;
        return mode;
    }

    MyMoneyMoney shareValue() const
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

    InvestTransactionEditor*     editor;
};


Activity::Activity(InvestTransactionEditor* editor) :
    d_ptr(new ActivityPrivate(editor))
{
}

Activity::~Activity()
{
    Q_D(Activity);
    delete d;
}

void Invest::Activity::setupWidgets(const QStringList& activityWidgets) const
{
    Q_D(const Activity);

    static const QStringList dynamicWidgetNames = {
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
    };

    setWidgetVisibility(dynamicWidgetNames, false);

    setLabelText("priceLabel", priceLabelText());
    setLabelText("sharesLabel", sharesLabelText());

    static const QStringList standardWidgetNames = {
        "activityLabel", "activityCombo",
        "dateLabel", "dateEdit",
        "securityLabel", "securityAccountCombo",

        // the ones in between are dynamically handled

        "memoLabel", "memoEdit",
        "statusLabel", "statusCombo",
        "enterButton", "cancelButton",
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
    QString label;
    if (d->priceMode() == eDialogs::PriceMode::Price) {
        label = i18nc("@label:textbox", "Price");
    } else if (d->priceMode() == eDialogs::PriceMode::PricePerShare) {
        label = i18nc("@label:textbox", "Price/share");
    } else if (d->priceMode() == eDialogs::PriceMode::PricePerTransaction) {
        label = i18nc("@label:textbox", "Transaction amount");
    }
    return label;
}

void Activity::loadPriceWidget(const MyMoneySplit & split)
{
    Q_D(const Activity);
    auto priceEdit = d->haveWidget<AmountEdit>("priceAmountEdit");

    if (priceEdit) {
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
    Q_D(const Activity);
    static const QStringList activityWidgets = {
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "totalLabel", "totalAmountEdit",
    };

    setupWidgets(activityWidgets);
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

    static const QStringList activityWidgets = {
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
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
    static const QStringList activityWidgets = {
        "assetAccountLabel", "assetAccountCombo",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
    };

    setupWidgets(activityWidgets);
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

    static const QStringList activityWidgets = {
        "sharesLabel", "sharesAmountEdit",
        "assetAccountLabel", "assetAccountCombo",
        "priceLabel", "priceAmountEdit",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
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
    static const QStringList activityWidgets = {
        "sharesLabel", "sharesAmountEdit",
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
    static const QStringList activityWidgets = {
        "sharesLabel", "sharesAmountEdit",
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
    Q_D(const Activity);
    static const QStringList activityWidgets = {
        "sharesLabel", "sharesAmountEdit",
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
    static const QStringList activityWidgets = {
        "assetAccountLabel", "assetAccountCombo",
        "feesLabel", "feesCombo", "feesAmountLabel", "feesAmountEdit",
        "interestLabel", "interestCombo", "interestAmountLabel", "interestAmountEdit",
        "totalLabel", "totalAmountEdit",
    };

    setupWidgets(activityWidgets);
}
