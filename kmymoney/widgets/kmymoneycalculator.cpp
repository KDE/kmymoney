/*
    SPDX-FileCopyrightText: 2002-2017 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmymoneycalculator.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QRegularExpression>
#include <QSignalMapper>

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class KMyMoneyCalculatorPrivate
{
    Q_DISABLE_COPY(KMyMoneyCalculatorPrivate)

public:
    KMyMoneyCalculatorPrivate() :
        op0(0.0),
        op1(0.0),
        op(0),
        stackedOp(0),
        display(nullptr),
        m_clearOperandOnDigit(false)
    {
        for (auto& button : buttons) {
            button = nullptr;
        }
    }

    void updateOperand()
    {
        if (operand.length() > 16) {
            operand = operand.left(16);
        }
        changeDisplay(operand);
    }

    void changeDisplay(const QString& str)
    {
        auto txt = str;
        txt.replace(QRegularExpression(QLatin1String("\\.")), m_comma);
        display->setText("<b>" + txt + "</b>");
    }

    /**
      * This member variable stores the current (second) operand
      */
    QString operand;

    /**
      * This member variable stores the last result
      */
    QString m_result;

    /**
      * This member variable stores the representation of the
      * character to be used to separate the integer and fractional
      * part of numbers. The internal representation is always a period.
      */
    QString m_comma;

    /**
      * The numeric representation of a stacked first operand
      */
    double op0;

    /**
      * The numeric representation of the first operand
      */
    double op1;

    /**
      * This member stores the operation to be performed between
      * the first and the second operand.
      */
    int op;

    /**
     * This member stores a pending addition operation
     */
    int stackedOp;

    /**
      * This member stores a pointer to the display area
      */
    QLabel *display;

    /**
      * This member array stores the pointers to the various
      * buttons of the calculator. It is setup during the
      * constructor of this object
      */
    QPushButton *buttons[20];

    /**
      * This enumeration type stores the values used for the
      * various keys internally
      */
    enum {
        /* 0-9 are used by digits */
        COMMA = 10,
        /*
         * make sure, that PLUS through EQUAL remain in
         * the order they are. Otherwise, check the calculation
         * signal mapper
         */
        PLUS,
        MINUS,
        SLASH,
        STAR,
        EQUAL,
        PLUSMINUS,
        PERCENT,
        CLEAR,
        CLEARALL,
        /* insert new buttons before this line */
        MAX_BUTTONS,
    };

    /**
      * This flag signals, if the operand should be replaced upon
      * a digit key pressure. Defaults to false and will be set, if
      * setInitialValues() is called without an operation.
      */
    bool m_clearOperandOnDigit;
};

KMyMoneyCalculator::KMyMoneyCalculator(QWidget* parent) :
    QFrame(parent),
    d_ptr(new KMyMoneyCalculatorPrivate)
{
    Q_D(KMyMoneyCalculator);
    d->m_comma = QLocale().decimalPoint();
    d->m_clearOperandOnDigit = false;

    QGridLayout* grid = new QGridLayout(this);

    d->display = new QLabel(this);
    QPalette palette;
    palette.setColor(d->display->backgroundRole(), QColor("#BDFFB4"));
    d->display->setPalette(palette);

    d->display->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    d->display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    grid->addWidget(d->display, 0, 0, 1, 5);

    d->buttons[0] = new QPushButton("0", this);
    d->buttons[1] = new QPushButton("1", this);
    d->buttons[2] = new QPushButton("2", this);
    d->buttons[3] = new QPushButton("3", this);
    d->buttons[4] = new QPushButton("4", this);
    d->buttons[5] = new QPushButton("5", this);
    d->buttons[6] = new QPushButton("6", this);
    d->buttons[7] = new QPushButton("7", this);
    d->buttons[8] = new QPushButton("8", this);
    d->buttons[9] = new QPushButton("9", this);
    d->buttons[KMyMoneyCalculatorPrivate::PLUS] = new QPushButton("+", this);
    d->buttons[KMyMoneyCalculatorPrivate::MINUS] = new QPushButton("-", this);
    d->buttons[KMyMoneyCalculatorPrivate::STAR] = new QPushButton("X", this);
    d->buttons[KMyMoneyCalculatorPrivate::COMMA] = new QPushButton(d->m_comma, this);
    d->buttons[KMyMoneyCalculatorPrivate::EQUAL] = new QPushButton("=", this);
    d->buttons[KMyMoneyCalculatorPrivate::SLASH] = new QPushButton("/", this);
    d->buttons[KMyMoneyCalculatorPrivate::CLEAR] = new QPushButton("C", this);
    d->buttons[KMyMoneyCalculatorPrivate::CLEARALL] = new QPushButton("AC", this);
    d->buttons[KMyMoneyCalculatorPrivate::PLUSMINUS] = new QPushButton("+-", this);
    d->buttons[KMyMoneyCalculatorPrivate::PERCENT] = new QPushButton("%", this);

    grid->addWidget(d->buttons[7], 1, 0);
    grid->addWidget(d->buttons[8], 1, 1);
    grid->addWidget(d->buttons[9], 1, 2);
    grid->addWidget(d->buttons[4], 2, 0);
    grid->addWidget(d->buttons[5], 2, 1);
    grid->addWidget(d->buttons[6], 2, 2);
    grid->addWidget(d->buttons[1], 3, 0);
    grid->addWidget(d->buttons[2], 3, 1);
    grid->addWidget(d->buttons[3], 3, 2);
    grid->addWidget(d->buttons[0], 4, 1);

    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::COMMA], 4, 0);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::PLUS], 3, 3);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::MINUS], 4, 3);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::STAR], 3, 4);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::SLASH], 4, 4);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::EQUAL], 4, 2);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::PLUSMINUS], 2, 3);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::PERCENT], 2, 4);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::CLEAR], 1, 3);
    grid->addWidget(d->buttons[KMyMoneyCalculatorPrivate::CLEARALL], 1, 4);

    d->buttons[KMyMoneyCalculatorPrivate::EQUAL]->setFocus();

    d->op1 = d->op0 = 0.0;
    d->stackedOp = d->op = 0;
    d->operand.clear();
    d->changeDisplay("0");

    // connect the digit signals through a signal mapper
    QSignalMapper* mapper = new QSignalMapper(this);
    for (auto i = 0; i < 10; ++i) {
        mapper->setMapping(d->buttons[i], i);
        connect(d->buttons[i], &QAbstractButton::clicked, mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    }
    connect(mapper, &QSignalMapper::mappedInt, this, &KMyMoneyCalculator::digitClicked);

    // connect the calculation operations through another mapper
    mapper = new QSignalMapper(this);
    for (int i = KMyMoneyCalculatorPrivate::PLUS; i <= KMyMoneyCalculatorPrivate::EQUAL; ++i) {
        mapper->setMapping(d->buttons[i], i);
        connect(d->buttons[i], &QAbstractButton::clicked, mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    }
    connect(mapper, &QSignalMapper::mappedInt, this, &KMyMoneyCalculator::calculationClicked);

    // connect all remaining signals
    connect(d->buttons[KMyMoneyCalculatorPrivate::COMMA], &QAbstractButton::clicked, this, &KMyMoneyCalculator::commaClicked);
    connect(d->buttons[KMyMoneyCalculatorPrivate::PLUSMINUS], &QAbstractButton::clicked, this, &KMyMoneyCalculator::plusminusClicked);
    connect(d->buttons[KMyMoneyCalculatorPrivate::PERCENT], &QAbstractButton::clicked, this, &KMyMoneyCalculator::percentClicked);
    connect(d->buttons[KMyMoneyCalculatorPrivate::CLEAR], &QAbstractButton::clicked, this, &KMyMoneyCalculator::clearClicked);
    connect(d->buttons[KMyMoneyCalculatorPrivate::CLEARALL], &QAbstractButton::clicked, this, &KMyMoneyCalculator::clearAllClicked);

    for (auto i = 0; i < KMyMoneyCalculatorPrivate::MAX_BUTTONS; ++i) {
        d->buttons[i]->setMinimumSize(40, 30);
        d->buttons[i]->setMaximumSize(40, 30);
    }
    // keep the size determined by the size of the contained buttons no matter what
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

KMyMoneyCalculator::~KMyMoneyCalculator()
{
    Q_D(KMyMoneyCalculator);
    delete d;
}

void KMyMoneyCalculator::digitClicked(int button)
{
    Q_D(KMyMoneyCalculator);
    if (d->m_clearOperandOnDigit) {
        d->operand.clear();
        d->m_clearOperandOnDigit = false;
    }

    d->operand += QChar(button + 0x30);
    d->updateOperand();
}

void KMyMoneyCalculator::commaClicked()
{
    Q_D(KMyMoneyCalculator);
    if (d->operand.length() == 0) {
        d->operand = '0';
    }
    if (d->operand.contains('.', Qt::CaseInsensitive) == 0)
        d->operand.append('.');

    d->updateOperand();
}

void KMyMoneyCalculator::plusminusClicked()
{
    Q_D(KMyMoneyCalculator);
    if (d->operand.length() == 0 && d->m_result.length() > 0) {
        d->operand = d->m_result;
    }

    if (d->operand.length() > 0) {
        if (d->operand.indexOf('-') != -1)
            d->operand.remove('-');
        else
            d->operand.prepend('-');
        d->changeDisplay(d->operand);
    }
}

void KMyMoneyCalculator::calculationClicked(int button)
{
    Q_D(KMyMoneyCalculator);
    if (d->operand.length() == 0 && d->op != 0 && button == KMyMoneyCalculatorPrivate::EQUAL) {
        d->op = 0;
        d->m_result = normalizeString(d->op1);
        d->changeDisplay(d->m_result);

    } else if (d->operand.length() > 0 && d->op != 0) {
        // perform operation
        double op2 = d->operand.toDouble();
        bool error = false;

        // if the pending operation is addition and we now do multiplication
        // we just stack op1 and remember the operation in
        if ((d->op == KMyMoneyCalculatorPrivate::PLUS || d->op == KMyMoneyCalculatorPrivate::MINUS) && (button == KMyMoneyCalculatorPrivate::STAR || button == KMyMoneyCalculatorPrivate::SLASH)) {
            d->op0 = d->op1;
            d->stackedOp = d->op;
            d->op = 0;
        }

        switch (d->op) {
        case KMyMoneyCalculatorPrivate::PLUS:
            op2 = d->op1 + op2;
            break;
        case KMyMoneyCalculatorPrivate::MINUS:
            op2 = d->op1 - op2;
            break;
        case KMyMoneyCalculatorPrivate::STAR:
            op2 = d->op1 * op2;
            break;
        case KMyMoneyCalculatorPrivate::SLASH:
            if (op2 == 0.0)
                error = true;
            else
                op2 = d->op1 / op2;
            break;
        }

        // if we have a pending addition operation, and the next operation is
        // not multiplication, we calculate the stacked operation
        if (d->stackedOp && button != KMyMoneyCalculatorPrivate::STAR && button != KMyMoneyCalculatorPrivate::SLASH) {
            switch (d->stackedOp) {
            case KMyMoneyCalculatorPrivate::PLUS:
                op2 = d->op0 + op2;
                break;
            case KMyMoneyCalculatorPrivate::MINUS:
                op2 = d->op0 - op2;
                break;
            }
            d->stackedOp = 0;
        }

        if (error) {
            d->op = 0;
            d->changeDisplay("Error");
            d->operand.clear();
        } else {
            d->op1 = op2;
            d->m_result = normalizeString(d->op1);
            d->changeDisplay(d->m_result);
        }
    } else if (d->operand.length() > 0 && d->op == 0) {
        d->op1 = d->operand.toDouble();
        d->m_result = normalizeString(d->op1);
        d->changeDisplay(d->m_result);
    }

    if (button != KMyMoneyCalculatorPrivate::EQUAL) {
        d->op = button;
    } else {
        d->op = 0;
        Q_EMIT signalResultAvailable();
    }
    d->operand.clear();
}

QString KMyMoneyCalculator::normalizeString(const double& val)
{
    QString str;
    str.setNum(val, 'f');
    int i = str.length();
    while (i > 1 && str[i-1] == '0') {
        --i;
    }
    // cut off trailing 0's
    str.remove(i, str.length());
    if (str.length() > 0) {
        // possibly remove trailing period
        if (str[str.length()-1] == '.') {
            str.remove(str.length() - 1, 1);
        }
    }
    return str;
}

void KMyMoneyCalculator::clearClicked()
{
    Q_D(KMyMoneyCalculator);
    if (d->operand.length() > 0) {
        d->operand = d->operand.left(d->operand.length() - 1);
    }
    if (d->operand.length() == 0)
        d->changeDisplay("0");
    else
        d->changeDisplay(d->operand);
}

void KMyMoneyCalculator::clearAllClicked()
{
    Q_D(KMyMoneyCalculator);
    d->operand.clear();
    d->op = 0;
    d->changeDisplay("0");
}

void KMyMoneyCalculator::percentClicked()
{
    Q_D(KMyMoneyCalculator);
    if (d->op != 0) {
        double op2 = d->operand.toDouble();
        switch (d->op) {
        case KMyMoneyCalculatorPrivate::PLUS:
        case KMyMoneyCalculatorPrivate::MINUS:
            op2 = (d->op1 * op2) / 100;
            break;

        case KMyMoneyCalculatorPrivate::STAR:
        case KMyMoneyCalculatorPrivate::SLASH:
            op2 /= 100;
            break;
        }
        d->operand = normalizeString(op2);
        d->changeDisplay(d->operand);
    }
}

QString KMyMoneyCalculator::result() const
{
    Q_D(const KMyMoneyCalculator);
    auto txt = d->m_result;
    txt.replace(QRegularExpression(QLatin1String("\\.")), d->m_comma);
    if (txt[0] == '-') {
        txt = txt.mid(1); // get rid of the minus sign
        QString mask;
        // TODO: port this to kf5 (support for paren around negative numbers)
#if 0
        switch (KLocale::global()->negativeMonetarySignPosition()) {
        case KLocale::ParensAround:
            mask = "(%1)";
            break;
        case KLocale::AfterQuantityMoney:
            mask = "%1-";
            break;
        case KLocale::AfterMoney:
        case KLocale::BeforeMoney:
            mask = "%1 -";
            break;
        case KLocale::BeforeQuantityMoney:
            mask = "-%1";
            break;
        }
#else
        mask = "-%1";
#endif
        txt = QString(mask).arg(txt);
    }
    return txt;
}

void KMyMoneyCalculator::setComma(const QChar ch)
{
    Q_D(KMyMoneyCalculator);
    d->m_comma = ch;
}

void KMyMoneyCalculator::keyPressEvent(QKeyEvent* ev)
{
    Q_D(KMyMoneyCalculator);
    int button = -1;

    if (ev->matches(QKeySequence::Paste)) {
        const auto clipboard = qApp->clipboard();
        auto txt = clipboard->text();

        // check that the clipboard content is valid
        bool ok;
        QLocale().toDouble(txt, &ok);

        // Now convert the localized command into a dot
        txt.replace(d->m_comma, QLatin1String("."));

        // since toDouble() allows scientific notation, we
        // make sure the letter e is not contained
        if (ok && !(txt.toLower()).contains(QLatin1Char('e'))) {
            if (d->m_clearOperandOnDigit) {
                d->operand.clear();
                d->m_clearOperandOnDigit = false;
            }
            d->operand += txt;
            d->updateOperand();
        }
        return;
    }

    switch (ev->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        if (d->m_clearOperandOnDigit) {
            d->operand.clear();
            d->m_clearOperandOnDigit = false;
        }
        button = ev->key() - Qt::Key_0;
        break;
    case Qt::Key_Plus:
        button = KMyMoneyCalculatorPrivate::PLUS;
        break;
    case Qt::Key_Minus:
        button = KMyMoneyCalculatorPrivate::MINUS;
        break;
    case Qt::Key_Comma:
    case Qt::Key_Period:
        if (d->m_clearOperandOnDigit) {
            d->operand.clear();
            d->m_clearOperandOnDigit = false;
        }
        button = KMyMoneyCalculatorPrivate::COMMA;
        break;
    case Qt::Key_Slash:
        button = KMyMoneyCalculatorPrivate::SLASH;
        break;
    case Qt::Key_Backspace:
        button = KMyMoneyCalculatorPrivate::CLEAR;
        if(ev->modifiers() & Qt::ShiftModifier) {
            button = KMyMoneyCalculatorPrivate::CLEARALL;
        }
        break;
    case Qt::Key_Asterisk:
        button = KMyMoneyCalculatorPrivate::STAR;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Equal:
        button = KMyMoneyCalculatorPrivate::EQUAL;
        break;
    case Qt::Key_Escape:
        Q_EMIT signalQuit();
        break;
    case Qt::Key_Percent:
        button = KMyMoneyCalculatorPrivate::PERCENT;
        break;
    default:
        ev->ignore();
        break;
    }
    if (button != -1)
        d->buttons[button]->animateClick();

    d->m_clearOperandOnDigit = false;
}

void KMyMoneyCalculator::setInitialValues(const QString& value, QKeyEvent* ev)
{
    Q_D(KMyMoneyCalculator);
    bool negative = false;
    // setup operand
    d->operand = value;
    // make sure the group/thousands separator is removed ...
    d->operand.replace(QRegularExpression(QStringLiteral("\\%1").arg(QLocale().groupSeparator())), QChar());
    // ... and the decimal is represented by a dot
    d->operand.replace(QRegularExpression(QStringLiteral("\\%1").arg(d->m_comma)), QChar('.'));
    if (d->operand.contains('(')) {
        negative = true;
        d->operand.remove('(');
        d->operand.remove(')');
    }
    if (d->operand.contains('-')) {
        negative = true;
        d->operand.remove('-');
    }
    if (d->operand.isEmpty()) {
        d->operand.clear();
        d->changeDisplay(QLatin1String("0"));
    } else {
        if (negative) {
            d->operand = QStringLiteral("-%1").arg(d->operand);
        }
        d->changeDisplay(d->operand);
    }

    // and operation
    d->op = 0;
    if (ev)
        keyPressEvent(ev);
    else
        d->m_clearOperandOnDigit = true;
}
