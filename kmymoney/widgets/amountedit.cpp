/*
    SPDX-FileCopyrightText: 2010-2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "amountedit.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QFrame>
#include <QKeyEvent>
#include <QLocale>
#include <QStyle>
#include <QToolButton>

// ----------------------------------------------------------------------------
// KDE Includes

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

// ----------------------------------------------------------------------------
// Project Includes

#include "amountvalidator.h"
#include "kmymoneycalculator.h"
#include "mymoneysecurity.h"
#include "icons.h"
#include "popuppositioner.h"

using namespace Icons;

class AmountEditHelper
{
public:
    AmountEditHelper() : q(nullptr) {}
    ~AmountEditHelper() {
        delete q;
    }
    AmountEdit *q;
};

Q_GLOBAL_STATIC(AmountEditHelper, s_globalAmountEdit)

AmountEdit* AmountEdit::global()
{
    if (!s_globalAmountEdit()->q) {
        s_globalAmountEdit()->q = new AmountEdit(0, 2);
    }

    return s_globalAmountEdit()->q;
}

class AmountEditPrivate
{
    Q_DISABLE_COPY(AmountEditPrivate)
    Q_DECLARE_PUBLIC(AmountEdit)

public:
    enum Item {
        NoItem = 0x0,
        ShowCalculator = 0x1,
        ShowCurrencySymbol = 0x02,
        ShowAll = 0x3,
    };
    Q_DECLARE_FLAGS(Items, Item)

    explicit AmountEditPrivate(AmountEdit* qq)
        : q_ptr(qq)
        , m_calculatorFrame(nullptr)
        , m_calculator(nullptr)
        , m_calculatorButton(nullptr)
        , m_prec(2)
        , m_allowEmpty(false)
        , m_actionIcons(NoItem)
        , m_initialExchangeRate(MyMoneyMoney::ONE)
        , m_state(AmountEdit::DisplayValue)
        , sharesSet(false)
        , valueSet(false)
        , m_isCashAmount(false)
    {
        m_calculatorFrame = new QFrame;
        m_calculatorFrame->setWindowFlags(Qt::Popup);

        m_calculatorFrame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        m_calculatorFrame->setLineWidth(3);

        m_calculator = new KMyMoneyCalculator(m_calculatorFrame);
        m_calculatorFrame->hide();
    }

    void init()
    {
        Q_Q(AmountEdit);
        // Yes, just a simple double validator !
        auto validator = new AmountValidator(q);
        q->setValidator(validator);
        q->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        const auto btnSize = q->sizeHint().height() - 5;

        m_calculatorButton = new QToolButton(q);
        m_calculatorButton->setIcon(Icons::get(Icon::Calculator));
        m_calculatorButton->setCursor(Qt::ArrowCursor);
        m_calculatorButton->setStyleSheet("QToolButton { border: none; padding: 2px}");
        m_calculatorButton->setFixedSize(btnSize, btnSize);
        m_calculatorButton->setFocusPolicy(Qt::ClickFocus);

        q->connect(m_calculatorButton, &QAbstractButton::clicked, q, &AmountEdit::slotCalculatorOpen);

        m_currencyButton = new QToolButton(q);
        m_currencyButton->setCursor(Qt::ArrowCursor);
        m_currencyButton->setAutoRaise(true);
        m_currencyButton->hide();
        m_currencyButton->setFocusPolicy(Qt::ClickFocus);

        // setup items
        KSharedConfig::Ptr kconfig = KSharedConfig::openConfig();
        KConfigGroup grp = kconfig->group("General Options");
        q->setCalculatorButtonVisible(!grp.readEntry("DontShowCalculatorButton", false));
        q->setCurrencySymbol(QString(), QString());

        updateLineEditSize(m_currencyButton, true);

        q->connect(q, &QLineEdit::textChanged, q, &AmountEdit::theTextChanged);
        q->connect(m_calculator, &KMyMoneyCalculator::signalResultAvailable, q, &AmountEdit::slotCalculatorResult);
        q->connect(m_calculator, &KMyMoneyCalculator::signalQuit, q, &AmountEdit::slotCalculatorClose);
    }

    /**
      * Internal helper function for value() and ensureFractionalPart().
      */
    void ensureFractionalPart(QString& s) const
    {
        s = MyMoneyMoney(s).formatMoney(QString(), precision(m_state), false);
    }

    /**
      * This method opens the calculator and replays the key
      * event pointed to by @p ev. If @p ev is 0, then no key
      * event is replayed.
      *
      * @param ev pointer to QKeyEvent that started the calculator.
      */
    void calculatorOpen(QKeyEvent* k)
    {
        Q_Q(AmountEdit);
        m_calculator->setInitialValues(q->text(), k);

        // do not open the calculator in read-only mode
        if (q->isReadOnly())
            return;

        // show calculator and update size
        m_calculatorFrame->show();
        m_calculatorFrame->setGeometry(m_calculator->geometry());

        PopupPositioner pos(q, m_calculatorFrame, PopupPositioner::BottemLeft);
        m_calculator->setFocus();
    }

    void updateLineEditSize(QWidget* widget, int w, int h)
    {
        Q_Q(AmountEdit);
        widget->resize(w, h);

        int height = q->sizeHint().height();
        int frameWidth = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        const int btnHeight = height - 2*frameWidth;
        const int btnX = (height - btnHeight) / 2;
        const int currencyX = (height - h) / 2;

        int gaps = 0;
        gaps += m_actionIcons.testFlag(ShowCalculator) ? 1 : 0;
        gaps += m_actionIcons.testFlag(ShowCurrencySymbol) ? 1 : 0;

        m_calculatorButton->move(q->width() - m_calculatorButton->width() - frameWidth, btnX);
        widget->move(q->width() - m_calculatorButton->width() - widget->width() - gaps * frameWidth, currencyX);

        const int padding = m_calculatorButton->width() + widget->width() + ((gaps - 1) * frameWidth);
        q->setStyleSheet(QString("QLineEdit { padding-right: %1px }").arg(padding));
        q->setMinimumHeight(height);
    }

    void cut()
    {
        Q_Q(AmountEdit);
        // only cut if parts of the text are selected
        if (q->hasSelectedText() && (q->text() != q->selectedText())) {
            cut();
        }
    }

    bool hasMultipleCurrencies() const
    {
        return m_sharesCommodity.id().compare(m_valueCommodity.id());
    }

    void updateLineEditSize(QToolButton* button, bool ofs = false)
    {
        Q_Q(AmountEdit);
        const int currencyWidth = q->fontMetrics().boundingRect(button->text()).width() + 10;
        const auto frameWidth = ofs ? q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) : 0;

        const int currencyHeight = button->height() - frameWidth;

        AmountEditPrivate::updateLineEditSize(button, currencyWidth, currencyHeight);
    }

    void setCurrencySymbol(const QString& symbol, const QString& name)
    {
        m_currencyButton->setText(symbol);
        m_currencyButton->setToolTip(name);
        m_currencyButton->setHidden(symbol.isEmpty());
        m_actionIcons.setFlag(AmountEditPrivate::ShowCurrencySymbol, !symbol.isEmpty());
        updateLineEditSize(m_currencyButton);
    }

    void updateWidgets()
    {
        Q_Q(AmountEdit);
        if (hasMultipleCurrencies()) {
            m_currencyButton->setEnabled(true);
            QString currentCurrency;
            QString otherCurrency;
            // prevent to change the values due to emitted textChanged() signals
            QSignalBlocker block(q);
            if (m_state == AmountEdit::DisplayShares) {
                currentCurrency = m_sharesCommodity.name();
                otherCurrency = m_valueCommodity.name();
                setCurrencySymbol(m_sharesCommodity.tradingSymbol(), currentCurrency);
                q->setText(m_sharesText);
            } else {
                currentCurrency = m_valueCommodity.name();
                otherCurrency = m_sharesCommodity.name();
                setCurrencySymbol(m_valueCommodity.tradingSymbol(), m_valueCommodity.name());
                q->setText(m_valueText);
            }

            m_currencyButton->setToolTip(
                i18nc("@info:tooltip Swap currencies for entry/display", "Value are presented in %1. Press this button to switch the currency to %2.")
                    .arg(currentCurrency, otherCurrency));

        } else {
            if (!m_valueCommodity.id().isEmpty()) {
                m_currencyButton->setEnabled(false);
                m_currencyButton->setToolTip(
                    i18nc("@info:tooltip Swap currencies for entry/display", "Values are presented in %1.").arg(m_valueCommodity.name()));
                setCurrencySymbol(m_valueCommodity.tradingSymbol(), m_valueCommodity.name());

            } else {
                // hide the currency symbol
                setCurrencySymbol(QString(), QString());
            }
        }
    }

    void swapCommodities()
    {
        Q_Q(AmountEdit);
        q->setDisplayState((m_state == AmountEdit::DisplayShares) ? AmountEdit::DisplayValue : AmountEdit::DisplayShares);
    }

    MyMoneyMoney adjustToPrecision(AmountEdit::DisplayState state, const MyMoneyMoney& amount) const
    {
        auto money(amount);
        const MyMoneySecurity& sec((state == AmountEdit::DisplayValue) ? m_valueCommodity : m_sharesCommodity);
        if (!sec.id().isEmpty()) {
            const auto fraction = m_isCashAmount ? sec.smallestCashFraction() : sec.smallestAccountFraction();
            money = money.convert(fraction);
        } else if (m_prec != -1)
            money = money.convert(MyMoneyMoney::precToDenom(m_prec));
        return money;
    }

    int precision(AmountEdit::DisplayState state) const
    {
        const MyMoneySecurity& sec((state == AmountEdit::DisplayValue) ? m_valueCommodity : m_sharesCommodity);
        auto prec(m_prec);

        if (!sec.id().isEmpty()) {
            prec = MyMoneyMoney::denomToPrec(m_isCashAmount ? sec.smallestCashFraction() : sec.smallestAccountFraction());
        }
        return prec;
    }

    void setValueText(const QString& txt)
    {
        Q_Q(AmountEdit);
        m_valueText = txt;
        if (q->isEnabled() && !txt.isEmpty()) {
            ensureFractionalPart(m_valueText);
        }
        // only update text if it really differs
        if (m_state == AmountEdit::DisplayValue && m_valueText.compare(q->QLineEdit::text())) {
            // prevent to change the values due to emitted textChanged() signals
            QSignalBlocker block(q);
            q->QLineEdit::setText(m_valueText);
        }
    }

    void setSharesText(const QString& txt)
    {
        Q_Q(AmountEdit);
        m_sharesText = txt;
        if (q->isEnabled() && !txt.isEmpty()) {
            ensureFractionalPart(m_sharesText);
        }
        // only update text if it really differs
        if (m_state == AmountEdit::DisplayShares && m_sharesText.compare(q->QLineEdit::text())) {
            // prevent to change the values due to emitted textChanged() signals
            QSignalBlocker block(q);
            q->QLineEdit::setText(m_sharesText);
        }
    }

    AmountEdit* q_ptr;
    QFrame* m_calculatorFrame;
    KMyMoneyCalculator* m_calculator;
    QToolButton* m_calculatorButton;
    QToolButton* m_currencyButton;
    int m_prec;
    bool m_allowEmpty;
    QString m_previousText; // keep track of what has been typed

    QString m_valueText; // keep track of what was the original value
    QString m_sharesText; // keep track of what was the original value

    QFlags<Item> m_actionIcons;

    MyMoneyMoney m_value; // original amount when starting editing
    MyMoneyMoney m_shares; // original amount when starting editing
    MyMoneyMoney m_initialExchangeRate;

    MyMoneySecurity m_sharesCommodity;
    MyMoneySecurity m_valueCommodity;

    /**
     * Which part is displayed
     */
    AmountEdit::DisplayState m_state;

    bool sharesSet;
    bool valueSet;
    bool m_isCashAmount;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AmountEditPrivate::Items)

void AmountEdit::setReadOnly(bool ro)
{
    Q_D(AmountEdit);
    d->m_calculatorButton->setEnabled(!ro);
    QLineEdit::setReadOnly(ro);
}

AmountEdit::AmountEdit(QWidget *parent, const int prec) :
    QLineEdit(parent),
    d_ptr(new AmountEditPrivate(this))
{
    Q_D(AmountEdit);
    d->m_prec = prec;
    if (prec < -1 || prec > 20) {
        d->m_prec = AmountEdit::global()->standardPrecision();
    }

    d->init();

    connect(d->m_currencyButton, &QToolButton::clicked, this, [&]() {
        Q_D(AmountEdit);
        d->swapCommodities();
    });
}

AmountEdit::AmountEdit(const MyMoneySecurity& sec, QWidget *parent) :
    QLineEdit(parent),
    d_ptr(new AmountEditPrivate(this))
{
    Q_D(AmountEdit);
    d->m_prec = MyMoneyMoney::denomToPrec(sec.smallestAccountFraction());
    d->init();
}

AmountEdit::AmountEdit(QWidget* parent, const int prec, AmountEditPrivate* dd)
    : QLineEdit(parent)
    , d_ptr(dd)
{
    Q_D(AmountEdit);
    d->m_prec = prec;
    if (prec < -1 || prec > 20) {
        d->m_prec = AmountEdit::global()->standardPrecision();
    }
    d->init();
}

AmountEdit::~AmountEdit()
{
    Q_D(AmountEdit);
    delete d;
}

void AmountEdit::setStandardPrecision(int prec)
{
    if (prec >= 0 && prec < 20) {
        global()->d_ptr->m_prec = prec;
    }
}

int AmountEdit::standardPrecision()
{
    return global()->d_ptr->m_prec;
}


void AmountEdit::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);
    Q_D(AmountEdit);
    d->updateLineEditSize(d->m_currencyButton);
}

void AmountEdit::focusInEvent(QFocusEvent* event)
{
    QLineEdit::focusInEvent(event);
    if (event->reason() == Qt::MouseFocusReason) {
        if (!hasSelectedText()) {
            // we need to wait until all processing is done before
            // we can successfully call selectAll. Hence the
            // delayed execution when we return back to the event loop
            metaObject()->invokeMethod(this, &QLineEdit::selectAll, Qt::QueuedConnection);
        }
    }
}

void AmountEdit::focusOutEvent(QFocusEvent* event)
{
    Q_D(AmountEdit);
    QLineEdit::focusOutEvent(event);

    // make sure we have a zero value in case the current text
    // is empty but this is not allowed
    if (text().isEmpty() && !d->m_allowEmpty) {
        QLineEdit::setText(QLatin1String("0"));
    }

    // make sure we have a fractional part
    if (!text().isEmpty())
        ensureFractionalPart();

    // in case the widget contains a different value we emit
    // the amountChanged signal
    if ((d->m_value != value()) || (d->m_shares != shares())) {
        d->m_value = value();
        d->m_shares = shares();
        emit amountChanged();
    }
}

void AmountEdit::keyPressEvent(QKeyEvent* event)
{
    Q_D(AmountEdit);
    switch(event->key()) {
    case Qt::Key_Plus:
    case Qt::Key_Minus:
        d->cut();
        if (text().length() == 0) {
            break;
        }
        // in case of '-' we do not enter the calculator when
        // the current position is the beginning and there is
        // no '-' sign at the first position.
        if (event->key() == Qt::Key_Minus) {
            if (cursorPosition() == 0 && text().at(0) != '-') {
                break;
            }
        }
        // intentional fall through

    case Qt::Key_Slash:
    case Qt::Key_Asterisk:
    case Qt::Key_Percent:
        d->cut();
        d->calculatorOpen(event);
        return;

    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Enter:
        break;

    case Qt::Key_Space:
        if (event->modifiers() == Qt::ControlModifier) {
            event->accept();
            d->m_currencyButton->animateClick();
            return;
        }
        break;

    default:
        // make sure to use the locale's decimalPoint when the
        // keypad comma/dot is pressed
        auto keyText = event->text();
        auto key = event->key();
        if (event->modifiers() & Qt::KeypadModifier) {
            if ((key == Qt::Key_Period) || (key == Qt::Key_Comma)) {
                key = QLocale().decimalPoint().unicode();
                keyText = QLocale().decimalPoint();
            }
        }
        // create a (possibly adjusted) copy of the event
        QKeyEvent newEvent(event->type(),
                           key,
                           event->modifiers(),
                           event->nativeScanCode(),
                           event->nativeVirtualKey(),
                           event->nativeModifiers(),
                           keyText,
                           event->isAutoRepeat(),
                           event->count());

        // in case all text is selected and the user presses the decimal point
        // we fill the widget with the leading "0". The outcome of this will be
        // that the widget then contains "0.".
        if ((newEvent.key() == QLocale().decimalPoint()) && (selectedText() == text())) {
            QLineEdit::setText(QLatin1String("0"));
        }
        QLineEdit::keyPressEvent(&newEvent);
        return;
    }

    // in case we have not processed anything, we
    // need to call the base class implementation
    QLineEdit::keyPressEvent(event);
}


void AmountEdit::setPrecision(const int prec)
{
    Q_D(AmountEdit);
    if (prec >= -1 && prec <= 20) {
        if (prec != d->m_prec) {
            d->m_prec = prec;
            // update current display
            setValue(value());
        }
    }
}

int AmountEdit::precision() const
{
    Q_D(const AmountEdit);
    return d->m_prec;
}

int AmountEdit::precision(MultiCurrencyEdit::DisplayState state) const
{
    Q_D(const AmountEdit);
    return d->precision(state);
}

MultiCurrencyEdit::DisplayState AmountEdit::displayState() const
{
    Q_D(const AmountEdit);
    return d->m_state;
}

bool AmountEdit::isValid() const
{
    return !(text().isEmpty());
}

MyMoneyMoney AmountEdit::value() const
{
    Q_D(const AmountEdit);
    MyMoneyMoney money(d->m_valueText);
    return d->adjustToPrecision(AmountEdit::DisplayValue, money);
}

void AmountEdit::setValue(const MyMoneyMoney& amount)
{
    Q_D(AmountEdit);
    if (d->sharesSet && !d->valueSet) {
        qWarning() << objectName() << "Call AmountEdit::setValue() before AmountEdit::setShares(). Fix source code!";
    }
    if (d->valueSet && (amount == d->m_value))
        return;

    d->valueSet = true;
    d->m_value = amount;
    const auto txt(amount.formatMoney(QString(), d->precision(AmountEdit::DisplayValue), false));
    d->setValueText(txt);

    if (!hasMultipleCurrencies()) {
        setShares(amount);
    }
}

MyMoneyMoney AmountEdit::shares() const
{
    Q_D(const AmountEdit);
    MyMoneyMoney money(d->m_sharesText);
    return d->adjustToPrecision(AmountEdit::DisplayShares, money);
}

void AmountEdit::setShares(const MyMoneyMoney& amount)
{
    Q_D(AmountEdit);
    d->sharesSet = !d->valueSet;
    d->m_shares = amount;

    const auto txt(amount.formatMoney(QString(), d->precision(AmountEdit::DisplayShares), false));
    d->setSharesText(txt);

    if (!d->m_shares.isZero()) {
        d->m_initialExchangeRate = d->m_value / d->m_shares;
    }
}

void AmountEdit::clear()
{
    Q_D(AmountEdit);
    d->m_sharesText.clear();
    d->m_valueText.clear();
    QLineEdit::clear();
    d->m_previousText.clear();
}

void AmountEdit::theTextChanged(const QString & theText)
{
    Q_D(AmountEdit);
    QLocale locale;
    QString dec = locale.groupSeparator();
    QString l_text = theText;
    QString nsign, psign;
    nsign = locale.negativeSign();
    psign = locale.positiveSign();

    auto i = 0;
    if (isEnabled()) {
        QValidator::State state =  validator()->validate(l_text, i);
        if (state == QValidator::Intermediate) {
            if (l_text.length() == 1) {
                if (l_text != dec && l_text != nsign && l_text != psign)
                    state = QValidator::Invalid;
            }
        }
        if (state == QValidator::Invalid)
            QLineEdit::setText(d->m_previousText);
        else {
            d->m_previousText = l_text;

            if (!l_text.isEmpty()) {
                // adjust value or shares depending on state
                // by using the initialExchangeRate
                if (d->m_state == AmountEdit::DisplayValue) {
                    d->m_valueText = l_text;
                    MyMoneyMoney amount(l_text);
                    d->adjustToPrecision(AmountEdit::DisplayValue, amount);
                    amount /= d->m_initialExchangeRate;
                    d->m_sharesText = amount.formatMoney(QString(), d->precision(AmountEdit::DisplayShares), false);
                } else {
                    d->m_sharesText = l_text;
                    MyMoneyMoney amount(l_text);
                    d->adjustToPrecision(AmountEdit::DisplayShares, amount);
                    amount *= d->m_initialExchangeRate;
                    d->m_valueText = amount.formatMoney(QString(), d->precision(AmountEdit::DisplayValue), false);
                }
            }
            emit validatedTextChanged(text());
        }
    }
}

void AmountEdit::slotCalculatorOpen()
{
    Q_D(AmountEdit);
    d->calculatorOpen(0);
}

void AmountEdit::slotCalculatorClose()
{
    Q_D(AmountEdit);
    if (d->m_calculator != 0) {
        d->m_calculatorFrame->hide();
    }
}

void AmountEdit::slotCalculatorResult()
{
    Q_D(AmountEdit);
    slotCalculatorClose();
    if (d->m_calculator != 0) {
        MyMoneyMoney amount(d->m_calculator->result());
        d->adjustToPrecision(d->m_state, amount);
        theTextChanged(amount.formatMoney(QString(), d->precision(d->m_state), false));
    }
}

void AmountEdit::setCalculatorButtonVisible(const bool show)
{
    Q_D(AmountEdit);
    d->m_calculatorButton->setVisible(show);
    d->m_actionIcons.setFlag(AmountEditPrivate::ShowCalculator, show);
    d->updateLineEditSize(d->m_currencyButton);
}

void AmountEdit::setAllowEmpty(bool allowed)
{
    Q_D(AmountEdit);
    d->m_allowEmpty = allowed;
}

bool AmountEdit::isEmptyAllowed() const
{
    Q_D(const AmountEdit);
    return d->m_allowEmpty;
}

bool AmountEdit::isCalculatorButtonVisible() const
{
    Q_D(const AmountEdit);
    return d->m_calculatorButton->isVisible();
}

void AmountEdit::ensureFractionalPart()
{
    Q_D(AmountEdit);
    QString s(text());
    d->ensureFractionalPart(s);
    // by setting the text only when it's different then the one that it is already there
    // we preserve the edit widget's state (like the selection for example) during a
    // call to ensureFractionalPart() that does not change anything
    if (s != text())
        QLineEdit::setText(s);
}

void AmountEdit::setCurrencySymbol(const QString& symbol, const QString& name)
{
    Q_D(AmountEdit);
    d->setCurrencySymbol(symbol, name);
}

void AmountEdit::setDisplayState(AmountEdit::DisplayState state)
{
    Q_D(AmountEdit);
    if (state != d->m_state) {
        d->m_state = state;
        d->updateWidgets();
        emit displayStateChanged(state);
    }
}

void AmountEdit::setShowShares(bool show)
{
    if (!show) {
        setShowValue();
        return;
    }
    setDisplayState(AmountEdit::DisplayShares);
}

void AmountEdit::setShowValue(bool show)
{
    if (!show) {
        setShowShares();
        return;
    }
    setDisplayState(AmountEdit::DisplayValue);
}

void AmountEdit::setCommodity(const MyMoneySecurity& commodity)
{
    Q_D(AmountEdit);
    d->m_sharesCommodity = commodity;
    d->m_valueCommodity = commodity;
    d->updateWidgets();
}

void AmountEdit::setSharesCommodity(const MyMoneySecurity& commodity)
{
    Q_D(AmountEdit);
    if (d->m_sharesCommodity.id() != commodity.id()) {
        d->m_sharesCommodity = commodity;
        d->updateWidgets();
    }
}

MyMoneySecurity AmountEdit::sharesCommodity() const
{
    Q_D(const AmountEdit);
    return d->m_sharesCommodity;
}

void AmountEdit::setValueCommodity(const MyMoneySecurity& commodity)
{
    Q_D(AmountEdit);
    if (d->m_valueCommodity.id() != commodity.id()) {
        d->m_valueCommodity = commodity;
        d->updateWidgets();
    }
}

MyMoneySecurity AmountEdit::valueCommodity() const
{
    Q_D(const AmountEdit);
    return d->m_valueCommodity;
}

void AmountEdit::setInitialExchangeRate(const MyMoneyMoney& price)
{
    Q_D(AmountEdit);
    d->m_initialExchangeRate = price;
}

MyMoneyMoney AmountEdit::initialExchangeRate() const
{
    Q_D(const AmountEdit);
    return d->m_initialExchangeRate;
}

QWidget* AmountEdit::widget()
{
    return this;
}

bool AmountEdit::hasMultipleCurrencies() const
{
    Q_D(const AmountEdit);
    return d->hasMultipleCurrencies();
}
