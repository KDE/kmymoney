/*
    SPDX-FileCopyrightText: 2015 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
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
