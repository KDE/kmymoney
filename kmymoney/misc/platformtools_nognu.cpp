/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright 2017       Marc Hübner <mahueb55@gmail.com>
 * Copyright 2020       Thomas Baumgart <tbaumgart@kde.org>
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

#include <windows.h>
#include <lmcons.h>
#include <process.h>

#include <QString>

QString platformTools::osUsername()
{
  QString name;
  DWORD size = UNLEN+1;
  wchar_t  wcname[UNLEN+1];
  if(GetUserNameW((LPWSTR) wcname, &size)) {
    name = QString::fromWCharArray(wcname);
  }
  return name;
}

uint platformTools::processId()
{
    return _getpid();
}

platformTools::currencySymbolPosition_t platformTools::currencySymbolPosition(bool negativeValues)
{
  Q_UNUSED(negativeValues)
  return platformTools::AfterQuantityMoneyWithSpace;
}

platformTools::currencySignPosition_t platformTools::currencySignPosition(bool negativeValues)
{
  Q_UNUSED(negativeValues)
  return platformTools::PreceedQuantityAndSymbol;
}
