/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MULTICURRENCYEDIT_H
#define MULTICURRENCYEDIT_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

class QWidget;

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyMoney;
class MyMoneySecurity;

class MultiCurrencyEdit
{
public:
    typedef enum {
        DisplayValue,
        DisplayShares,
    } DisplayState;

    virtual ~MultiCurrencyEdit() = default;

    /**
     * Use @a commodity for amounts in the value portion.
     */
    virtual void setValueCommodity(const MyMoneySecurity& commodity) = 0;

    /**
     * Returns the commodity used for amounts in the value portion.
     */
    virtual MyMoneySecurity valueCommodity() const = 0;

    /**
     * Use @a commodity for amounts in the shares portion.
     */
    virtual void setSharesCommodity(const MyMoneySecurity& commodity) = 0;

    /**
     * Returns the commodity used for amounts in the shares portion.
     */
    virtual MyMoneySecurity sharesCommodity() const = 0;

    /**
     * Use @a commodity for amounts in the values and shares portion.
     * This is a convenience method for single currency amounts
     */
    virtual void setCommodity(const MyMoneySecurity& commodity) = 0;

    /**
     * This returns the amount entered in the valueCommodity.
     */
    virtual MyMoneyMoney value() const = 0;

    /**
     * This returns the amount entered in the sharesCommodity.
     */
    virtual MyMoneyMoney shares() const = 0;

    /**
     * Sets the value portion to @a amount.
     *
     * @note This method does not emit the amountChanged() signal
     */
    virtual void setValue(const MyMoneyMoney& amount) = 0;

    /**
     * Sets the shares portion to @a amount.
     *
     * @note This method does not emit the amountChanged() signal
     */
    virtual void setShares(const MyMoneyMoney& amount) = 0;

    /**
     * Allows to setup an initial @a price for the two
     * selected currencies. The following equation is used
     *
     *    value = shares * price
     */
    virtual void setInitialExchangeRate(const MyMoneyMoney& price) = 0;

    virtual MyMoneyMoney initialExchangeRate() const = 0;

    /**
     * Returns a pointer to the edit widget
     */
    virtual QWidget* widget() = 0;

    /**
     * Sets the current display state of the widget to
     * be in @a state.
     */
    virtual void setDisplayState(DisplayState state) = 0;

    /**
     * Returns the current display state of the widget.
     *
     * @retval DisplayState::Value when display is showing amount in valueCommodity
     * @retval DisplayState::Shares when display is showing amount in sharesCommodity
     */
    virtual DisplayState displayState() const = 0;

    /**
     * Returns the precision in number of fraction digits for
     * the given display state passed in @a state.
     */
    virtual int precision(DisplayState state) const = 0;

    /**
     * Checks if multiple currencies are used.
     *
     * @retval false sharesCommodity and valueCommodity are identical
     * @retval true sharesCommodity and valueCommodity differ
     */
    virtual bool hasMultipleCurrencies() const = 0;
};

#endif // MULTICURRENCYEDIT_H
