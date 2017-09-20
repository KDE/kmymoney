/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QtCore/QString>

namespace validators
{
enum lengthStatus {
  ok = 0,
  tooShort  = -1,
  tooLong = 1
};

/** @brief checks if all lines in text are shorter than length */
bool checkLineLength(const QString& text, const int& length);

/** @brief checks if text uses only charactes in allowedChars */
bool checkCharset(const QString& text, const QString& allowedChars);

};

#endif // VALIDATORS_H
