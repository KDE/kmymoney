/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QString>

namespace validators
{
enum lengthStatus {
    ok = 0,
    tooShort  = -1,
    tooLong = 1,
};

/** @brief checks if all lines in text are shorter than length */
bool checkLineLength(const QString& text, const int& length);

/** @brief checks if text uses only charactes in allowedChars */
bool checkCharset(const QString& text, const QString& allowedChars);

};

#endif // VALIDATORS_H
