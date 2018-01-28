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


// ----------------------------------------------------------------------------
// QT Includes

// ----------------------------------------------------------------------------
// KDE Includes

// ----------------------------------------------------------------------------
// Project Includes


public:

  static QColor schemeColor(const SchemeColor color);

  static QFont listCellFontEx();
  static QFont listHeaderFontEx();
  static QStringList listOfItems();

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
