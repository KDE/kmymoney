/*
 * Copyright 2017       Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CSVENUMS_H
#define CSVENUMS_H

#include <QHash>

enum class FieldDelimiter { Comma = 0, Semicolon, Colon, Tab, Auto };
enum class TextDelimiter { DoubleQuote = 0, SingleQuote };
enum class DecimalSymbol { Dot = 0, Comma, Auto };
enum class ThousandSeparator { Comma = 0, Dot };
enum class DateFormat { YearMonthDay = 0, MonthDayYear, DayMonthYear };
enum class Column { Date, Memo, Number, Payee, Amount, Credit, Debit, Category, Type, Price, Quantity, Fee, Symbol, Name, CreditDebitIndicator, Balance, Empty = 0xFE, Invalid = 0xFF };
enum class Profile { Banking, Investment, CurrencyPrices, StockPrices };
enum class ProfileAction { Add, Remove, Rename, UpdateLastUsed };

inline uint qHash(const Column key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
inline uint qHash(const Profile key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

#endif
