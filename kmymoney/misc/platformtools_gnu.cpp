/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2017 Marc Hübner <mahueb55@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "platformtools.h"

#include <pwd.h>
#include <unistd.h>
#include <locale.h>

#include <QString>

QString platformTools::osUsername()
{
  QString name;
  struct passwd* pwd = getpwuid(geteuid());
  if( pwd != nullptr) {
    name = QString::fromLatin1(pwd->pw_name);
  }
  return name;
}

uint platformTools::processId()
{
  return getpid();
}

platformTools::currencySymbolPosition_t platformTools::currencySymbolPosition(bool negativeValues)
{
  platformTools::currencySymbolPosition_t  rc = platformTools::AfterQuantityMoneyWithSpace;
  struct lconv* lc = std::localeconv();
  if (lc) {
    const char precedes = negativeValues ? lc->n_cs_precedes : lc->p_cs_precedes;
    const char space = negativeValues ? lc->n_sep_by_space : lc->p_sep_by_space;
    if (precedes == 1) {
      rc = (space == 1) ? platformTools::BeforeQuantityMoneyWithSpace : platformTools::BeforeQuantityMoney;
    } else {
      rc = (space == 1) ? platformTools::AfterQuantityMoneyWithSpace : platformTools::AfterQuantityMoney;
    }
  }
  return rc;
}
