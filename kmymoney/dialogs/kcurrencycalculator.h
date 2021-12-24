/*
    SPDX-FileCopyrightText: 2004-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCURRENCYCALCULATOR_H
#define KCURRENCYCALCULATOR_H

#include "kmm_base_dialogs_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDialog>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class QDate;

class MyMoneyMoney;
class MyMoneySplit;
class MyMoneyTransaction;
class MyMoneySecurity;
class MultiCurrencyEdit;

namespace Ui {
class KCurrencyCalculator;
}

typedef qint64 signed64;

/**
  * @author Thomas Baumgart
  */

class KCurrencyCalculatorPrivate;
class KMM_BASE_DIALOGS_EXPORT KCurrencyCalculator : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(KCurrencyCalculator)

public:
    explicit KCurrencyCalculator(QWidget* parent = nullptr);

    /**
      * @param from the @p from currency
      * @param to   the @p to currency
      * @param value the value to be converted
      * @param shares the number of foreign currency units
      * @param date the date when the conversion took place
      * @param resultFraction the smallest fraction of the result (default 100)
      * @param parent see QWidget constructor
      *
      * @note @p value must not be 0!
      */
    KCurrencyCalculator(const MyMoneySecurity& from,
                        const MyMoneySecurity& to,
                        const MyMoneyMoney& value,
                        const MyMoneyMoney& shares,
                        const QDate& date,
                        const signed64 resultFraction = 100,
                        QWidget* parent = nullptr);

    ~KCurrencyCalculator();

    /**
     * This method returns the price determined by the method selected by the user
     * which is either
     *
     * a) based on the resulting amount or
     * b) based on direct price entry.
     *
     * In case a) the price is returned without precision loss as the division
     * of the amount entered by the user and the @a value passed as argument.
     * In case b) it is returned with the selected global price precision.
     */
    MyMoneyMoney price() const;

    void setupPriceEditor();

    static bool setupSplitPrice(MyMoneyMoney& shares,
                                const MyMoneyTransaction& t,
                                const MyMoneySplit& s,
                                const QMap<QString,
                                MyMoneyMoney>& priceInfo,
                                QWidget* parentWidget);

    void setFromCurrency(const MyMoneySecurity& sec);
    void setToCurrency(const MyMoneySecurity& sec);
    void setFromAmount(const MyMoneyMoney& amount);
    void setToAmount(const MyMoneyMoney& amount);
    void setDate(const QDate& date);
    void setResultFraction(signed64 fraction);

    /**
     * This method asks the user for exchange rate information
     * according to the data found in the @a amountEdit widget.
     * In case the two commodities in @a amountEdit are identical
     * the method returns immediately.
     */
    static void updateConversion(MultiCurrencyEdit* amountEdit, const QDate date);

protected Q_SLOTS:
    void slotSetToAmount();
    void slotSetExchangeRate();
    void slotUpdateResult(const QString& txt);
    void slotUpdateRate(const QString& txt);
    void accept() override;

private:
    KCurrencyCalculatorPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(KCurrencyCalculator)
};

#endif
