/***************************************************************************
                          mymoneyenums.h
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

#include <QHash>

#ifndef MYMONEYENUMS_H
#define MYMONEYENUMS_H

namespace eMyMoney {
      enum class Security {
      Stock,
      MutualFund,
      Bond,
      Currency,
      None
    };

    inline uint qHash(const Security key, uint seed) { return ::qHash(static_cast<uint>(key), seed); }
}
#endif
