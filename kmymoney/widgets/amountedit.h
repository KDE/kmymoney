/*
    SPDX-FileCopyrightText: 2010-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AMOUNTEDIT_H
#define AMOUNTEDIT_H

#include "kmm_base_widgets_export.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QLineEdit>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "multicurrencyedit.h"
#include "mymoneymoney.h"

class MyMoneySecurity;

/**
 * This class represents a widget to enter monetary values
 * in at most two currencies. It supports two amounts for
 * the value() and the shares(). They may differ, in case
 * two different commodities have been setup using
 * setValueCommodity() and setSharesCommodity(). A
 * convenience method setCommodity() sets both to the
 * same commodity.
 *
 * It has an edit field and a button to select a popup
 * calculator. The result of the calculator (if used) is
 * stored in the edit field.
 *
 * @author Thomas Baumgart
 */
class AmountEditPrivate;
class KMM_BASE_WIDGETS_EXPORT AmountEdit : public QLineEdit, public MultiCurrencyEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(AmountEdit)

    Q_PROPERTY(bool calculatorButtonVisibility READ isCalculatorButtonVisible WRITE setCalculatorButtonVisible)
    Q_PROPERTY(bool allowEmpty READ isEmptyAllowed WRITE setAllowEmpty)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(MyMoneyMoney value READ value WRITE setValue DESIGNABLE false STORED false USER true)
    Q_PROPERTY(bool valid READ isValid DESIGNABLE false STORED false)

protected Q_SLOTS:
    void theTextChanged(const QString & text);
    void slotCalculatorResult();
    void slotCalculatorOpen();
    void slotCalculatorClose();

public:
    explicit AmountEdit(QWidget* parent = nullptr, const int prec = -2);
    explicit AmountEdit(const MyMoneySecurity& eq, QWidget* parent = nullptr);
    virtual ~AmountEdit();

    bool isValid() const;

    QWidget* widget() override;

    DisplayState displayState() const override;

    /**
      * This method returns the value of the edit field in "numerator/denominator" format.
      * If you want to get the text of the edit field, use lineedit()->text() instead.
      */
    // QString numericalText() const;

    /**
      * Set the number of fractional digits that should be shown
      *
      * @param prec number of fractional digits.
      *
      * @note should be used prior to calling setText()
      * @sa precision
      */
    void setPrecision(const int prec);

    /**
      * return the number of fractional digits
      * @sa setPrecision
      */
    int precision() const;

    /**
     * Returns the number of fractional digits for the @a state.
     */
    int precision(DisplayState state) const override;

    /**
      * This method allows to modify the behavior of the widget
      * such that it accepts an empty value (all blank) or not.
      * The default is to not accept an empty input and to
      * convert an empty field into 0.00 upon loss of focus.
      *
      * @param allowed if @a true, empty input is allowed, if @a false
      *                empty input will be converted to 0.00
      */
    void setAllowEmpty(bool allowed = true);

    bool isEmptyAllowed() const;

    bool isCalculatorButtonVisible() const;

    /**
     * This allows to setup the standard precision (number of decimal places)
     * to be used when no other information is available. @a prec must be in
     * the range of 0..19. If never set, the default precision is 2.
     *
     * @sa standardPrecision
     */
    void setStandardPrecision(int prec);

    /**
     * This returns the global selected standard precision
     *
     * @sa setStandardPrecision
     */
    int standardPrecision();

    /**
     * Show the symbol next to the edit field in case
     * @a symbol is not empty. Hide it, in case it
     * is empty.
     */
    void setCurrencySymbol(const QString& symbol, const QString& name);

    /**
     * Use @a commodity for amounts in the value portion.
     */
    void setValueCommodity(const MyMoneySecurity& commodity) override;
    MyMoneySecurity valueCommodity() const override;

    /**
     * Use @a commodity for amounts in the shares portion.
     */
    void setSharesCommodity(const MyMoneySecurity& commodity) override;
    MyMoneySecurity sharesCommodity() const override;

    /**
     * Use @a commodity for amounts in the values and shares portion.
     * This is a convenience method for single currency amounts
     */
    void setCommodity(const MyMoneySecurity& commodity) override;

    /**
     * This returns the amount entered in the valueCommodity.
     */
    MyMoneyMoney value() const override;

    /**
     * This returns the amount entered in the sharesCommodity.
     */
    MyMoneyMoney shares() const override;

    /**
     * Sets the value portion to @a amount. This method calls
     * setShares internally and sets the initialExchangeRate to 1.
     *
     * @note This method does not emit the valueChanged() signal
     */
    void setValue(const MyMoneyMoney& amount) override;

    /**
     * Sets the shares portion to @a amount. This method
     * calls setInitialExchangeRate() with a price calculated
     * based on the values passed by setValue() and setShares().
     *
     * @note This method does not emit the sharesChanged() signal
     */
    void setShares(const MyMoneyMoney& amount) override;

    /**
     * Allows to setup an initial @a price for the two
     * selected currencies. The following equation is used
     *
     *    value = shares * price
     *
     * @note This method should be called after setValue() and
     * setShares() because setShares overwrites the @a price provided
     * as argument.
     */
    void setInitialExchangeRate(const MyMoneyMoney& price) override;

    /**
     * Returns the initial exchange rate set by setInitialExchangeRate()
     * or calculated by setShares().
     */
    MyMoneyMoney initialExchangeRate() const override;

    /**
     * Show the amount in the shares commodity if @a show
     * is @c true, show in value commodity otherwise.
     *
     * @note The default is to show amount in value commodity
     */
    void setShowShares(bool show = true);

    /**
     * Show the amount in the value commodity if @a show
     * is @c true, show in share commodity otherwise.
     *
     * @note This is the default setting
     */
    void setShowValue(bool show = true);

    /**
     * Overridden for internal reasons. It clears both
     * the cached text content for values and shares as
     * well as the widget
     */
    void clear();

    bool hasMultipleCurrencies() const override;

    /**
     * Behaves like QLineEdit::setText() when not in
     * multi-currency mode. Otherwise assigns the text
     * to the currently visible currency.
     */
    void setText(const QString& txt);

private:
public Q_SLOTS:

    /**
      * This method allows to show/hide the calculator button of the widget.
      * The parameter @p show controls the behavior. Default is to show the
      * button.
      *
      * @param show if true, button is shown, if false it is hidden
      */
    void setCalculatorButtonVisible(const bool show);

    /**
     * Sets the display state to @a state. In case it changes,
     * this emits displayStateChanged().
     *
     * @sa setShowShares(), setShowValue()
     */
    void setDisplayState(DisplayState state) override;

    /**
     * overridden for internal reasons (keep state of calculator button)
     */
    void setReadOnly(bool ro);

Q_SIGNALS:
    /**
     * This signal is emitted, when the focus leaves this widget and
     * the amount has been changed by user during this focus possession.
     */
    void amountChanged();

    /**
     * This signal is emitted when the contents of the widget
     * changed and was validated. Use this in favor of textChanged()
     * in your application.
     */
    void validatedTextChanged(const QString& text);

    /**
     * This signal is emitted when the @a state changes either
     * by a call to setDisplayState(), setShowShares() or
     * setShowValue() or by user activity.
     */
    void displayStateChanged(DisplayState state);

protected:
    explicit AmountEdit(QWidget* parent, const int prec, AmountEditPrivate* dd);

    /**
      * This method ensures that the text version contains a
      * fractional part.
      */
    void ensureFractionalPart();

    /**
     * return a pointer to the global object for the settings
     */
    AmountEdit* global();

    /**
     * Overridden to support calculator button.
     */
    virtual void resizeEvent(QResizeEvent* event) override;

    /**
     * Overridden to support full selection upon entry.
     */
    virtual void focusInEvent(QFocusEvent* event) override;

    /**
     * Overridden to support ensureFractionalPart().
     */
    virtual void focusOutEvent(QFocusEvent* event) override;

    /**
     * Overridden to support calculator button.
     */
    virtual void keyPressEvent(QKeyEvent* event) override;

protected:
    AmountEditPrivate * const d_ptr;
    Q_DECLARE_PRIVATE(AmountEdit)
};

#endif
