/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
 * Copyright (C) 2015 Christian Dávid <christian-david@web.de>
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

#include "misc/charvalidator.h"

charValidator::charValidator(QObject* parent, const QString& characters)
    : QValidator(parent),
    m_allowedCharacters(characters)
{
}

QValidator::State charValidator::validate(QString& string, int& pos) const
{
  Q_UNUSED(pos);
  const int length = string.length();
  for (int i = 0; i < length; ++i) {
    if (!m_allowedCharacters.contains(string.at(i)))
      return QValidator::Invalid;
  }
  return QValidator::Acceptable;
}

void charValidator::setAllowedCharacters(const QString& chars)
{
  m_allowedCharacters = chars;
}
