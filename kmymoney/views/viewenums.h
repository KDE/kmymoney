/***************************************************************************
                          viewenums.h
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

#include <QHashFunctions>

#ifndef VIEWENUMS_H
#define VIEWENUMS_H

enum class View { Home = 0, Institutions, Accounts, Schedules, Categories, Tags,
                  Payees, Ledgers, Investments, Reports, Budget, Forecast, OnlineJobOutbox, None };

inline uint qHash(const View key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }

namespace eView {
  enum class Tag { All = 0,
                   Referenced, // used tags
                   Unused,     // unused tags
                   Opened,     // not closed tags
                   Closed      // closed tags
                 };
}

#endif
