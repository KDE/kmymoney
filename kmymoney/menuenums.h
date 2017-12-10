/***************************************************************************
                          menuenums.h
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

#ifndef MENUENUMS_H
#define MENUENUMS_H

#include <QHashFunctions>

namespace eMenu {
  enum Action {
    // *************
    // The budget menu
    // *************
    NewBudget, RenameBudget, DeleteBudget,
    CopyBudget, ChangeBudgetYear, BudgetForecast
  };

  inline uint qHash(const Action key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}

#endif
