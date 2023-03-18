/*
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INVESTACTIVITIES_H
#define INVESTACTIVITIES_H

// ----------------------------------------------------------------------------
// QT Includes

#include "qcontainerfwd.h"
#include <qglobal.h>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class MyMoneySplit;
class SplitModel;
class InvestTransactionEditor;

namespace eMyMoney {
namespace Split {
enum class InvestmentTransactionType;
}
}

namespace eDialogs {
enum class PriceMode;
}

namespace Invest
{

class ActivityPrivate;
class Activity
{
    Q_DISABLE_COPY(Activity)

public:
    typedef enum {
        Unused,
        Optional,
        Mandatory,
    } fieldRequired_t;

    virtual eMyMoney::Split::InvestmentTransactionType type() const = 0;
    virtual void showWidgets() const = 0;

    virtual void adjustStockSplit(MyMoneySplit& stockSplit) = 0;

    virtual ~Activity();

    virtual void loadPriceWidget(const MyMoneySplit& split);
    virtual MyMoneyMoney totalAmount(const MyMoneySplit& stockSplit, const SplitModel* feesModel, const SplitModel* interestModel) const;

    virtual MyMoneyMoney sharesFactor() const;

    /**
     * This method returns the total value of the shares
     * The default is to return MyMoneyMoney().
     *
     * Widgets with names @c sharesAmountEdit and @c priceAmountEdit must
     * be present. If the priceMode is eDialogs::PriceMode::PricePerShare
     * the product of shares times the price will be returned. In other
     * cases, the value of price is returned.
     */
    virtual MyMoneyMoney valueAllShares() const;

    virtual MyMoneyMoney feesFactor() const;
    virtual MyMoneyMoney interestFactor() const;

    virtual fieldRequired_t feesRequired() const {
        return Unused;
    }
    virtual fieldRequired_t interestRequired() const {
        return Unused;
    }
    virtual fieldRequired_t assetAccountRequired() const {
        return Unused;
    }
    virtual fieldRequired_t priceRequired() const {
        return Unused;
    }

    eDialogs::PriceMode priceMode() const;

    bool haveFees( fieldRequired_t = Mandatory) const;
    bool haveInterest( fieldRequired_t = Mandatory) const;

    QString actionString() const;

protected:
    explicit Activity(InvestTransactionEditor* editor, const QString& action);
    virtual QString priceLabelText() const;
    virtual QString sharesLabelText() const;

    bool haveCategoryAndAmount(const QString& category, const QString& amount, fieldRequired_t optional) const;
    void setLabelText(const QString& idx, const QString& txt) const;
    void setWidgetVisibility(const QStringList& widgetIds, bool visible) const;
    void setupWidgets(const QStringList& activityWidgets) const;

protected:
    ActivityPrivate* d_ptr;
    Q_DECLARE_PRIVATE(Activity)
};

class Buy : public Activity
{
public:
    explicit Buy(InvestTransactionEditor* editor);
    ~Buy() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit&) override {}

    fieldRequired_t feesRequired() const override
    {
        return Optional;
    }
    fieldRequired_t interestRequired() const override
    {
        return Optional;
    }
    fieldRequired_t assetAccountRequired() const override
    {
        return Mandatory;
    }
    fieldRequired_t priceRequired() const override
    {
        return Mandatory;
    }
};

class Sell : public Activity
{
public:
    explicit Sell(InvestTransactionEditor* editor);
    ~Sell() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit&) override {}

    MyMoneyMoney sharesFactor() const override;
    fieldRequired_t feesRequired() const override {
        return Optional;
    }
    fieldRequired_t interestRequired() const override {
        return Optional;
    }
    fieldRequired_t assetAccountRequired() const override;
    fieldRequired_t priceRequired() const override {
        return Mandatory;
    }
};

class Div : public Activity
{
public:
    explicit Div(InvestTransactionEditor* editor);
    ~Div() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit&) override
    {
    }
    fieldRequired_t feesRequired() const override
    {
        return Optional;
    }
    fieldRequired_t interestRequired() const override
    {
        return Mandatory;
    }
    fieldRequired_t assetAccountRequired() const override
    {
        return Mandatory;
    }
};

class Yield : public Activity
{
public:
    explicit Yield(InvestTransactionEditor* editor);
    ~Yield() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit&) override
    {
    }
    fieldRequired_t feesRequired() const override
    {
        return Optional;
    }
    fieldRequired_t interestRequired() const override
    {
        return Mandatory;
    }
    fieldRequired_t assetAccountRequired() const override
    {
        return Mandatory;
    }
};

class Reinvest : public Activity
{
public:
    explicit Reinvest(InvestTransactionEditor* editor);
    ~Reinvest() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit&) override {}
    MyMoneyMoney totalAmount(const MyMoneySplit& stockSplit, const SplitModel* feesModel, const SplitModel* interestModel) const override;

    fieldRequired_t feesRequired() const override {
        return Optional;
    }
    fieldRequired_t interestRequired() const override {
        return Mandatory;
    }
    fieldRequired_t priceRequired() const override {
        return Mandatory;
    }
};

class Add : public Activity
{
public:
    explicit Add(InvestTransactionEditor* editor);
    ~Add() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    void adjustStockSplit(MyMoneySplit& stockSplit) override;
    MyMoneyMoney totalAmount(const MyMoneySplit& stockSplit, const SplitModel* feesModel, const SplitModel* interestModel) const override;
};

class Remove : public Activity
{
public:
    explicit Remove(InvestTransactionEditor* editor);
    ~Remove() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    void adjustStockSplit(MyMoneySplit& stockSplit) override;
    MyMoneyMoney totalAmount(const MyMoneySplit& stockSplit, const SplitModel* feesModel, const SplitModel* interestModel) const override;

    virtual MyMoneyMoney sharesFactor() const override;
};

class Split : public Activity
{
public:
    explicit Split(InvestTransactionEditor* editor);
    ~Split() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit& stockSplit) override;
    MyMoneyMoney totalAmount(const MyMoneySplit& stockSplit, const SplitModel* feesModel, const SplitModel* interestModel) const override;

protected:
    QString sharesLabelText() const override;
};

class IntInc : public Activity
{
public:
    explicit IntInc(InvestTransactionEditor* editor);
    ~IntInc() override;
    eMyMoney::Split::InvestmentTransactionType type() const override;
    void showWidgets() const override;
    virtual void adjustStockSplit(MyMoneySplit&) override {}

    fieldRequired_t feesRequired() const override {
        return Optional;
    }
    fieldRequired_t interestRequired() const override {
        return Mandatory;
    }
    fieldRequired_t assetAccountRequired() const override {
        return Mandatory;
    }
};

} // namespace Invest

#endif // INVESTACTIVITIES_H

