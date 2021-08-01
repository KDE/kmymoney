/*
    SPDX-FileCopyrightText: 2019 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "amountvalidator.h"
#include <cmath>

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "platformtools.h"

AmountValidator::AmountValidator(QObject * parent) :
    AmountValidator(-HUGE_VAL, HUGE_VAL, 1000, parent)
{
}

AmountValidator::AmountValidator(double bottom, double top, int decimals,
                                 QObject * parent) :
    QDoubleValidator(bottom, top, decimals, parent)
{
    setNotation(StandardNotation);
}

QValidator::State AmountValidator::validate(QString& input, int& pos) const
{
    const auto openParen = QStringLiteral("(");
    const auto closeParen = QStringLiteral(")");

    if ((platformTools::currencySignPosition(true) == platformTools::ParensAround)
        || (platformTools::currencySignPosition(false) == platformTools::ParensAround)) {
        const auto openCount = input.count(openParen);
        const auto closeCount = input.count(closeParen);

        // if we don't have any, use the normal validator
        if ((openCount == 0) && (closeCount == 0)) {
            return QDoubleValidator::validate(input, pos);
        }
        // more than one open and close paren is invalid
        if ((openCount > 1) || (closeCount > 1)) {
            return Invalid;
        }
        // close paren w/o open paren is invalid
        if (openCount < closeCount) {
            return Invalid;
        }
        // make sure open paren is first char
        if (openCount == 1 && !input.startsWith(openParen)) {
            return Invalid;
        }
        // make sure close paren is last char
        if (closeCount == 1 && !input.endsWith(closeParen)) {
            return Invalid;
        }
        // open and close paren count differs
        if (openCount != closeCount) {
            return Intermediate;
        }

        // remove the open and close paren and check the remainder
        auto modifiedInput(input);
        modifiedInput.remove(openParen);
        modifiedInput.remove(closeParen);
        return QDoubleValidator::validate(modifiedInput, pos);
    }
    return QDoubleValidator::validate(input, pos);
}
