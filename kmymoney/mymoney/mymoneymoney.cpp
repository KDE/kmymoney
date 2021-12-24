/*
    SPDX-FileCopyrightText: 2000-2004 Michael Edwardes <mte@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001-2021 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-FileCopyrightText: 2001-2002 Felix Rodriguez <frodriguez@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002-2004 Kevin Tambascio <ktambascio@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005 Ace Jones <acejones@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007-2010 Alvaro Soliverez <asoliverez@gmail.com>
    SPDX-FileCopyrightText: 2011 Carlos Eduardo da Silva <kaduardo@gmail.com>
    SPDX-FileCopyrightText: 2017-2018 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

// make sure, that this is defined before we even include any other header file
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS         // force definition of min and max values
#endif

#include "mymoneymoney.h"

#include <alkimia/alkversion.h>
#include <gmpxx.h>
#include <stdint.h>

// ----------------------------------------------------------------------------
// QT Includes

#include <QDebug>
#include <QString>

// ----------------------------------------------------------------------------
// Project Includes

#include "mymoneyexception.h"
#include "mymoneyenums.h"

const MyMoneyMoney MyMoneyMoney::ONE = MyMoneyMoney(1, 1);
const MyMoneyMoney MyMoneyMoney::MINUS_ONE = MyMoneyMoney(-1, 1);
namespace eMyMoney
{
namespace Money {

enum fileVersionE : int {
    FILE_4_BYTE_VALUE = 0,
    FILE_8_BYTE_VALUE,
};

QChar _thousandSeparator = QLatin1Char(',');
QChar _decimalSeparator = QLatin1Char('.');
eMyMoney::Money::signPosition _negativeMonetarySignPosition = PreceedQuantityAndSymbol;
eMyMoney::Money::signPosition _positiveMonetarySignPosition = PreceedQuantityAndSymbol;
bool _negativePrefixCurrencySymbol = false;
bool _positivePrefixCurrencySymbol = false;
bool _negativeSpaceSeparatesSymbol = true;
bool _positiveSpaceSeparatesSymbol = true;
eMyMoney::Money::fileVersionE _fileVersion = fileVersionE::FILE_4_BYTE_VALUE;
}
}

MyMoneyMoney MyMoneyMoney::maxValue = MyMoneyMoney(INT64_MAX, 100);
MyMoneyMoney MyMoneyMoney::minValue = MyMoneyMoney(INT64_MIN, 100);
MyMoneyMoney MyMoneyMoney::autoCalc = MyMoneyMoney(INT64_MIN + 1, 100);

void MyMoneyMoney::setNegativeSpaceSeparatesSymbol(const bool flag)
{
    eMyMoney::Money::_negativeSpaceSeparatesSymbol= flag;
}

void MyMoneyMoney::setPositiveSpaceSeparatesSymbol(const bool flag)
{
    eMyMoney::Money::_positiveSpaceSeparatesSymbol= flag;
}

void MyMoneyMoney::setNegativePrefixCurrencySymbol(const bool flag)
{
    eMyMoney::Money::_negativePrefixCurrencySymbol = flag;
}

void MyMoneyMoney::setPositivePrefixCurrencySymbol(const bool flag)
{
    eMyMoney::Money::_positivePrefixCurrencySymbol = flag;
}

void MyMoneyMoney::setNegativeMonetarySignPosition(const eMyMoney::Money::signPosition pos)
{
    eMyMoney::Money::_negativeMonetarySignPosition = pos;
}

eMyMoney::Money::signPosition MyMoneyMoney::negativeMonetarySignPosition()
{
    return eMyMoney::Money::_negativeMonetarySignPosition;
}

void MyMoneyMoney::setPositiveMonetarySignPosition(const eMyMoney::Money::signPosition pos)
{
    eMyMoney::Money::_positiveMonetarySignPosition = pos;
}

eMyMoney::Money::signPosition MyMoneyMoney::positiveMonetarySignPosition()
{
    return eMyMoney::Money::_positiveMonetarySignPosition;
}

void MyMoneyMoney::setThousandSeparator(const QChar &separator)
{
    if (separator != QLatin1Char(' '))
        eMyMoney::Money::_thousandSeparator = separator;
    else
        eMyMoney::Money::_thousandSeparator = 0;
}

const QChar MyMoneyMoney::thousandSeparator()
{
    return eMyMoney::Money::_thousandSeparator;
}

void MyMoneyMoney::setDecimalSeparator(const QChar &separator)
{
    if (separator != QLatin1Char(' '))
        eMyMoney::Money::_decimalSeparator = separator;
    else
        eMyMoney::Money::_decimalSeparator = 0;
}

const QChar MyMoneyMoney::decimalSeparator()
{
    return eMyMoney::Money::_decimalSeparator;
}

MyMoneyMoney::MyMoneyMoney(const QString& pszAmount)
    : AlkValue(pszAmount, eMyMoney::Money::_decimalSeparator)
{
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a signed64 value
//   Returns: None
//    Throws: Nothing.
// Arguments: Amount - signed 64 object containing amount
//            denom  - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
MyMoneyMoney::MyMoneyMoney(qint64 Amount, const unsigned int denom)
{
    if (denom == 0)
        throw MYMONEYEXCEPTION_CSTRING("Denominator 0 not allowed!");

    *this = AlkValue(QString::fromLatin1("%1/%2").arg(Amount).arg(denom), eMyMoney::Money::_decimalSeparator);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a integer value
//   Returns: None
//    Throws: Nothing.
// Arguments: iAmount - integer object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
MyMoneyMoney::MyMoneyMoney(const int iAmount, const unsigned int denom)
{
    if (denom == 0)
        throw MYMONEYEXCEPTION_CSTRING("Denominator 0 not allowed!");
    *this = AlkValue(iAmount, denom);
}

////////////////////////////////////////////////////////////////////////////////
//      Name: MyMoneyMoney
//   Purpose: Constructor - constructs object from an amount in a long integer value
//   Returns: None
//    Throws: Nothing.
// Arguments: iAmount - integer object containing amount
//            denom   - denominator of the object
//
////////////////////////////////////////////////////////////////////////////////
MyMoneyMoney::MyMoneyMoney(const long int iAmount, const unsigned int denom)
{
    if (denom == 0)
        throw MYMONEYEXCEPTION_CSTRING("Denominator 0 not allowed!");
    *this = AlkValue(QString::fromLatin1("%1/%2").arg(iAmount).arg(denom), eMyMoney::Money::_decimalSeparator);
}


MyMoneyMoney::~MyMoneyMoney()
{
}

MyMoneyMoney MyMoneyMoney::abs() const
{
    return static_cast<const MyMoneyMoney>(AlkValue::abs());
}

QString MyMoneyMoney::formatMoney(int denom, bool showThousandSeparator) const
{
    return formatMoney(QString(), denomToPrec(denom), showThousandSeparator);
}

QString MyMoneyMoney::formatMoney(const QString& currency, const int prec, bool showThousandSeparator) const
{
    QString res;
    QString tmpCurrency = currency;
    int tmpPrec = prec;
    mpz_class denom = 1;
    mpz_class value;
    const int maxPrecision = 20;

    // if prec == -1 we want the maximum possible but w/o trailing zeroes
    if (tmpPrec == -1) {
        tmpPrec = maxPrecision;
    }
    mpz_ui_pow_ui(denom.get_mpz_t(), 10, tmpPrec);

    value = static_cast<const MyMoneyMoney>(convertDenominator(denom)).valueRef().get_num();

    bool bNegative = false;
    mpz_class left = value / static_cast<MyMoneyMoney>(convertDenominator(denom)).valueRef().get_den();
    mpz_class right = mpz_class((valueRef() - mpq_class(left)) * denom);

    if (right < 0) {
        right = -right;
        bNegative = true;
    }
    if (left < 0) {
        left = -left;
        bNegative = true;
    }

    // convert the integer (left) part to a string
    res.append(left.get_str().c_str());

    // if requested, insert thousand separators every three digits
    if (showThousandSeparator) {
        int pos = res.length();
        while ((0 < (pos -= 3)) && thousandSeparator() != 0)
            res.insert(pos, thousandSeparator());
    }

    // take care of the fractional part
    if (prec > 0 || (prec == -1 && right != 0)) {
        if (decimalSeparator() != 0)
            res += decimalSeparator();

        auto rs  = QString::fromLatin1("%1").arg(right.get_str().c_str());
        if (prec != -1)
            rs = rs.rightJustified(prec, QLatin1Char('0'), true);
        else {
            rs = rs.rightJustified(maxPrecision, QLatin1Char('0'), true);
            // no trailing zeroes or decimal separators
            while (rs.endsWith(QLatin1Char('0')))
                rs.truncate(rs.length() - 1);
            while (rs.endsWith(decimalSeparator()))
                rs.truncate(rs.length() - 1);
        }
        res += rs;
    }

    eMyMoney::Money::signPosition signpos = bNegative ? eMyMoney::Money::_negativeMonetarySignPosition : eMyMoney::Money::_positiveMonetarySignPosition;
    auto sign = bNegative ? QString::fromLatin1("-") : QString();

    switch (signpos) {
    case eMyMoney::Money::ParensAround:
        // do nothing here
        break;
    case eMyMoney::Money::PreceedQuantityAndSymbol:
        res.prepend(sign);
        break;
    case eMyMoney::Money::SucceedQuantityAndSymbol:
        res.append(sign);
        break;
    case eMyMoney::Money::PreceedSymbol:
        tmpCurrency.prepend(sign);
        break;
    case eMyMoney::Money::SucceedSymbol:
        tmpCurrency.append(sign);
        break;
    }
    if (!tmpCurrency.isEmpty()) {
        if (bNegative ? eMyMoney::Money::_negativePrefixCurrencySymbol : eMyMoney::Money::_positivePrefixCurrencySymbol) {
            if (bNegative ? eMyMoney::Money::_negativeSpaceSeparatesSymbol : eMyMoney::Money::_positiveSpaceSeparatesSymbol) {
                res.prepend(QLatin1Char(' '));
            }
            res.prepend(tmpCurrency);
        } else {
            if (bNegative ? eMyMoney::Money::_negativeSpaceSeparatesSymbol : eMyMoney::Money::_positiveSpaceSeparatesSymbol) {
                res.append(QLatin1Char(' '));
            }
            res.append(tmpCurrency);
        }
    }

    if (signpos == eMyMoney::Money::ParensAround) {
        res.prepend(QLatin1Char('('));
        res.append(QLatin1Char(')'));
    }

    return res;
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator+
//   Purpose: Addition operator - adds the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be added
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator+(const MyMoneyMoney& _b) const
{
    return static_cast<const MyMoneyMoney>(AlkValue::operator+(_b));
}


////////////////////////////////////////////////////////////////////////////////
//      Name: operator-
//   Purpose: Addition operator - subtracts the input amount from the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: AmountInPence - MyMoneyMoney object to be subtracted
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator-(const MyMoneyMoney& _b) const
{
    return static_cast<const MyMoneyMoney>(AlkValue::operator-(_b));
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator*
//   Purpose: Multiplication operator - multiplies the input amount to the object
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be multiplied
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator*(const MyMoneyMoney& _b) const
{
    return static_cast<const MyMoneyMoney>(AlkValue::operator*(_b));
}

////////////////////////////////////////////////////////////////////////////////
//      Name: operator/
//   Purpose: Division operator - divides the object by the input amount
//   Returns: The current object
//    Throws: Nothing.
// Arguments: b - MyMoneyMoney object to be used as dividend
//
////////////////////////////////////////////////////////////////////////////////
const MyMoneyMoney MyMoneyMoney::operator/(const MyMoneyMoney& _b) const
{
    return static_cast<const MyMoneyMoney>(AlkValue::operator/(_b));
}

bool MyMoneyMoney::isNegative() const
{
    return (valueRef() < 0) ? true : false;
}

bool MyMoneyMoney::isPositive() const
{
    return (valueRef() > 0) ? true : false;
}

bool MyMoneyMoney::isZero() const
{
    return valueRef() == 0;
}

bool MyMoneyMoney::isAutoCalc() const
{
    return (*this == autoCalc);
}

MyMoneyMoney MyMoneyMoney::convert(const signed64 _denom, const AlkValue::RoundingMethod how) const
{
    return static_cast<const MyMoneyMoney>(AlkValue::convertDenominator(_denom, how));
}

MyMoneyMoney MyMoneyMoney::reduce() const
{
    MyMoneyMoney out(*this);
    out.canonicalize();
    return out;
}

signed64 MyMoneyMoney::precToDenom(int prec)
{
    signed64 denom = 1;

    while (prec--)
        denom *= 10;

    return denom;
}

double MyMoneyMoney::toDouble() const
{
    return valueRef().get_d();
}

int MyMoneyMoney::denomToPrec(signed64 fract)
{
    int rc = 0;
    while (fract > 1) {
        rc++;
        fract /= 10;
    }
    return rc;
}

MyMoneyMoney MyMoneyMoney::convertDenominator(mpz_class denom, const AlkValue::RoundingMethod how) const
{
#if ALK_VERSION >= ALK_VERSION_CHECK(8, 1, 71)
    return static_cast<const MyMoneyMoney>(AlkValue::convertDenominator(denom, how));
#else
    MyMoneyMoney in(*this);
    mpz_class in_num(mpq_numref(in.valueRef().get_mpq_t()));

    MyMoneyMoney out; // initialize to zero

    int sign = sgn(in_num);
    if (sign != 0) {
        // sign is either -1 for negative numbers or +1 in all other cases

        MyMoneyMoney temp;
        // only process in case the denominators are different
        if (mpz_cmpabs(denom.get_mpz_t(), mpq_denref(valueRef().get_mpq_t())) != 0) {
            mpz_class in_denom(mpq_denref(in.valueRef().get_mpq_t()));
            mpz_class out_num, out_denom;

            if (sgn(in_denom) == -1) { // my denom is negative
                in_num = in_num * (-in_denom);
                in_num = 1;
            }

            mpz_class remainder;
            int denom_neg = 0;

            // if the denominator is less than zero, we are to interpret it as
            // the reciprocal of its magnitude.
            if (sgn(denom) < 0) {
                mpz_class temp_a;
                mpz_class temp_bc;
                denom = -denom;
                denom_neg = 1;
                temp_a = ::abs(in_num);
                temp_bc = in_denom * denom;
                remainder = temp_a % temp_bc;
                out_num = temp_a / temp_bc;
                out_denom = denom;
            } else {
                temp = AlkValue(denom, in_denom);
                // the canonicalization required here is part of the ctor
                // temp.d->m_val.canonicalize();

                out_num = ::abs(in_num * temp.valueRef().get_num());
                remainder = out_num % temp.valueRef().get_den();
                out_num = out_num / temp.valueRef().get_den();
                out_denom = denom;
            }

            if (remainder != 0) {
                switch (how) {
                case RoundFloor:
                    if (sign < 0) {
                        out_num = out_num + 1;
                    }
                    break;

                case RoundCeil:
                    if (sign > 0) {
                        out_num = out_num + 1;
                    }
                    break;

                case RoundTruncate:
                    break;

                case RoundPromote:
                    out_num = out_num + 1;
                    break;

                case RoundHalfDown:
                    if (denom_neg) {
                        if ((2 * remainder) > (in_denom * denom)) {
                            out_num = out_num + 1;
                        }
                    } else if ((2 * remainder) > (temp.valueRef().get_den())) {
                        out_num = out_num + 1;
                    }
                    break;

                case RoundHalfUp:
                    if (denom_neg) {
                        if ((2 * remainder) >= (in_denom * denom)) {
                            out_num = out_num + 1;
                        }
                    } else if ((2 * remainder) >= temp.valueRef().get_den()) {
                        out_num = out_num + 1;
                    }
                    break;

                case RoundRound:
                    if (denom_neg) {
                        if ((remainder * 2) > (in_denom * denom)) {
                            out_num = out_num + 1;
                        } else if ((2 * remainder) == (in_denom * denom)) {
                            if ((out_num % 2) != 0) {
                                out_num = out_num + 1;
                            }
                        }
                    } else {
                        if ((remainder * 2) > temp.valueRef().get_den()) {
                            out_num = out_num + 1;
                        } else if ((2 * remainder) == temp.valueRef().get_den()) {
                            if ((out_num % 2) != 0) {
                                out_num = out_num + 1;
                            }
                        }
                    }
                    break;

                case RoundNever:
                    qWarning() << "AlkValue: have remainder" << toString() << "->convertDenominator()";
                    break;
                }
            }

            // construct the new output value
            out = AlkValue(out_num * sign, out_denom);
        } else {
            out = *this;
        }
    }
    return out;
#endif
}
