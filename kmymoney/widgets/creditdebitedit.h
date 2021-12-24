/*
    SPDX-FileCopyrightText: 2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CREDITDEBITEDIT_H
#define CREDITDEBITEDIT_H

// #include <qobjectdefs.h>

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QWidget>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "multicurrencyedit.h"

class MyMoneyMoney;
class MyMoneySecurity;

class CreditDebitEditPrivate;
class KMM_BASE_WIDGETS_EXPORT CreditDebitEdit : public QWidget, public MultiCurrencyEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(CreditDebitEdit)

public:
    explicit CreditDebitEdit(QWidget* parent);
    ~CreditDebitEdit();

    /**
     * Returns the value of the widget that is filled.
     * A credit is returned as negative, a debit as positive value.
     */
    MyMoneyMoney value() const override;
    MyMoneyMoney shares() const override;

    /**
     * Loads the value of the widgets with the @a amount passed. If
     * @a amount is negative it is loaded into the credit
     * widget, otherwise into the debit widget.
     */
    void setValue(const MyMoneyMoney& amount) override;
    void setShares(const MyMoneyMoney& amount) override;

    /**
     * This method returns true if at least one
     * of the two widgets is filled with text.
     * It returns false if both widgets are empty.
     */
    bool haveValue() const;

    void setCommodity(const MyMoneySecurity& commodity) override;
    void setValueCommodity(const MyMoneySecurity& commodity) override;
    MyMoneySecurity valueCommodity() const override;
    void setSharesCommodity(const MyMoneySecurity& commodity) override;
    MyMoneySecurity sharesCommodity() const override;

    void setInitialExchangeRate(const MyMoneyMoney& price) override;
    MyMoneyMoney initialExchangeRate() const override;

    QWidget* widget() override;

    void swapCreditDebit();

    /**
     * @sa AmountEdit::setAllowEmpty()
     */
    void setAllowEmpty(bool allowed = true);

    void setPlaceholderText(const QString& creditText, const QString& debitText);

    DisplayState displayState() const override;

    void setDisplayState(MultiCurrencyEdit::DisplayState state) override;

    bool hasMultipleCurrencies() const override;

    int precision(MultiCurrencyEdit::DisplayState state) const override;

public Q_SLOTS:
    /**
     * Show the symbol next to the edit field in case
     * @a symbol is not empty. Hide it, in case it
     * is empty.
     */
    void setCurrencySymbol(const QString& symbol, const QString& name);

Q_SIGNALS:
    void amountChanged();

private Q_SLOTS:
    void creditChanged();
    void debitChanged();

private:
    CreditDebitEditPrivate* const d_ptr;
    Q_DECLARE_PRIVATE(CreditDebitEdit)
};

#endif
