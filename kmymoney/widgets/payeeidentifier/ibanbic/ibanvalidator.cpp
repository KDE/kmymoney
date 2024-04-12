/*
    SPDX-FileCopyrightText: 2013-2015 Christian Dávid <christian-david@web.de>
    SPDX-FileCopyrightText: 2024 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ibanvalidator.h"

#include "payeeidentifier/ibanbic/ibanbic.h"
#include <KLocalizedString>

#include "widgetenums.h"

ibanValidator::ibanValidator(QObject* parent)
    : QValidator(parent)
{
}

QValidator::State ibanValidator::validate(QString& input, int& pos) const
{
    // Check country code and convert it to uppercase (see ibanBic::ibanToPaperformat)
    if (input.length() >= 1) {
        fixup(input);
        // if fixup took away characters, we might need to adjust the cursor pos
        if (pos > input.length()) {
            pos = input.length();
        }
        // check if fixup inserted a blank and bump the cursor pos if so
        if ((pos > 1) && (input.at(pos - 1) == QLatin1Char(' '))) {
            ++pos;
        }
        if (!input.at(0).isLetter())
            return Invalid;
    }

    if (input.length() >= 2) {
        if (!input.at(1).isLetter())
            return Invalid;
    }

    // Check rest of the iban
    int characterCount = qMin(input.length(), 2);
    for (int i = 2; i < input.length(); ++i) {
        if (input.at(i).isLetterOrNumber()) {
            ++characterCount;
        } else if (!input.at(i).isSpace()) {
            return Invalid;
        }
    }

    if (characterCount > 32)
        return Invalid;

    if (characterCount > 5) {
        return Acceptable;
    }

    return Intermediate;
}

QPair< eWidgets::ValidationFeedback::MessageType, QString > ibanValidator::validateWithMessage(const QString& string)
{
    // string.length() > 32 should not happen because all line edits should have this validator installed
    if (string.length() < 5)
        return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::Error, i18n("This IBAN is too short."));

    if (!payeeIdentifiers::ibanBic::validateIbanChecksum(payeeIdentifiers::ibanBic::ibanToElectronic(string)))
        return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::Warning, i18n("This IBAN is invalid."));

    return QPair< eWidgets::ValidationFeedback::MessageType, QString >(eWidgets::ValidationFeedback::MessageType::None, QString());
}

void ibanValidator::fixup(QString& input) const
{
    input = payeeIdentifiers::ibanBic::ibanToPaperformat(input);
}
