/***************************************************************************
                          widgetenums.h
                             -------------------
    copyright            : (C) 2017 by Łukasz Wojniłowicz <lukasz.wojnilowicz@gmail.com>

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WIDGETENUMS_H
#define WIDGETENUMS_H

namespace eWidgets {

  enum class SortField {
  Unknown = 0,      ///< unknown sort criteria
  PostDate = 1,     ///< sort by post date
  EntryDate,        ///< sort by entry date
  Payee,            ///< sort by payee name
  Value,            ///< sort by value
  NoSort,               ///< sort by number field
  EntryOrder,       ///< sort by entry order
  Type,             ///< sort by CashFlowDirection
  Category,         ///< sort by Category
  ReconcileState,   ///< sort by reconciliation state
  Security,         ///< sort by security (only useful for investment accounts)
  // insert new values in front of this line
  MaxFields
  };

}

#endif
