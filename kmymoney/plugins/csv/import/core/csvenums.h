/*
    SPDX-FileCopyrightText: 2017 Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>
    SPDX-FileCopyrightText: 2020 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
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
