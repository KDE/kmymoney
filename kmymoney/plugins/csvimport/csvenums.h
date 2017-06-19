/***************************************************************************
                               csvenums.h
                              -----------
begin                : Sun Jun 09 2010
copyright            : (C) 2017 by Łukasz Wojniłowicz
email                : lukasz.wojnilowicz@gmail.com
***************************************************************************/

/**************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QHash>

#ifndef CSVENUMS_H
#define CSVENUMS_H

enum class FieldDelimiter { Comma = 0, Semicolon, Colon, Tab, Auto };
enum class TextDelimiter { DoubleQuote = 0, SingleQuote };
enum class DecimalSymbol { Dot = 0, Comma, Auto };
enum class ThousandSeparator { Comma = 0, Dot };
enum class DateFormat { YearMonthDay = 0, MonthDayYear, DayMonthYear };
enum class Column { Date, Memo, Number, Payee, Amount, Credit, Debit, Category, Type, Price, Quantity, Fee, Symbol, Name, Empty = 0xFE, Invalid = 0xFF };
enum class Profile { Banking, Investment, CurrencyPrices, StockPrices };
enum class ProfileAction { Add, Remove, Rename, UpdateLastUsed };

inline uint qHash(const Column key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
inline uint qHash(const Profile key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

#endif
