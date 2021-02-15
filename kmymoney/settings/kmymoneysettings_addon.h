/*
    SPDX-FileCopyrightText: 2018 Thomas Baumgart <tbaumgart@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMYMONEYSETTINGS_ADDON_H
#define KMYMONEYSETTINGS_ADDON_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

class MyMoneyForecast;

enum class SchemeColor {
  ListBackground1,
  ListBackground2,
  ListGrid,
  ListHighlightText,
  ListHighlight,
  WindowText,
  WindowBackground,
  Positive,
  Negative,
  TransactionImported,
  TransactionMatched,
  TransactionErroneous,
  FieldRequired,
  GroupMarker,
  MissingConversionRate
};

#endif
