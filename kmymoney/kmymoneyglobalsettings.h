/***************************************************************************
                          kmymoneyglobalsettings.h
                             -------------------
    copyright            : (C) 2006 by Thomas Baumgart
    email                : ipwizard@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMYMONEYGLOBALSETTINGS_H
#define KMYMONEYGLOBALSETTINGS_H

// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes

#include "kmymoneysettings.h"

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

class KMyMoneyGlobalSettings : public KMyMoneySettings
{
public:
  static QColor schemeColor(const SchemeColor color);

  static QFont listCellFont();
  static QFont listHeaderFont();
  static QStringList itemList();

  /**
    * returns the number of the first month in the fiscal year
    */
  static int firstFiscalMonth();

  /**
    * returns the number of the first day of the fiscal year
    */
  static int firstFiscalDay();

  /**
    * returns the date of the first day in the current fiscal year
    */
  static QDate firstFiscalDate();

  /**
    * Construct a MyMoneyForecast object setup with all KMyMoneySettings
    */
  static MyMoneyForecast forecast();
};
#endif
