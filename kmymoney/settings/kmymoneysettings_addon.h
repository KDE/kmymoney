/***************************************************************************
                          kmymoneysettings_addon.h
                             -------------------
    copyright            : (C) 2018 by Thomas Baumgart
    email                : tbaumgart@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
