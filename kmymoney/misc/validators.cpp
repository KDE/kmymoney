/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2014 Christian Dávid <christian-david@web.de>
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

#include "validators.h"

#include <QtCore/QStringList>

bool validators::checkLineLength(const QString& text, const int& length)
{
  const QStringList lines = text.split('\n');
  foreach (QString line, lines) {
    if (line.length() > length)
      return false;
  }
  return true;
}

bool validators::checkCharset(const QString& text, const QString& allowedChars)
{
  const int length = text.length();
  for (int i = 0; i < length; ++i) {
    if (!allowedChars.contains(text.at(i)))
      return false;
  }
  return true;
}
