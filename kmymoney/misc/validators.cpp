/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-laterrg/licenses/>.
*/

#include "validators.h"

#include <QStringList>

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
