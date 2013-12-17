/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
 * Copyright (C) 2013 Christian Dávid <christian-david@web.de>
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

#include "credittransfersettingsbase.h"

creditTransferSettingsBase::lengthStatus creditTransferSettingsBase::checkNameLength(const QString& name) const
{
    if (name.length() > _payeeNameLength)
        return tooLong;
    else if ( name.length() < _payeeNameMinLength )
        return tooShort;
    return ok;
}

bool creditTransferSettingsBase::checkPurposeMaxLines(const QString& purpose) const
{
    return (purpose.split('\n').count() <= _purposeMaxLines);
}

creditTransferSettingsBase::lengthStatus creditTransferSettingsBase::checkPurposeLength(const QString& purpose) const
{
    const int length = purpose.length();
    if ( length > (_purposeMaxLines*_purposeLineLength) )
      return tooLong;
    else if ( length < _purposeMinLength )
      return tooShort;
    return ok;
}

bool creditTransferSettingsBase::checkPurposeLineLength(const QString& purpose) const
{
  return checkLineLength(purpose, _purposeLineLength);
}

bool creditTransferSettingsBase::checkLineLength( const QString& text, const int& length )
{
  const QStringList lines = text.split('\n');
  foreach (QString line, lines) {
    if (line.length() > length)
      return false;
  }
  return true;
}

bool creditTransferSettingsBase::checkCharset(const QString& text, const QString& allowedChars)
{
  const int length = text.length();
  for (int i = 0; i < length; ++i) {
    if (!allowedChars.contains(text.at(i)))
      return false;
  }
  return true;
}

bool creditTransferSettingsBase::checkPurposeCharset(const QString& string) const
{
  const QString chars = _allowedChars + QChar('\n');
  return checkCharset( string, chars );
}

creditTransferSettingsBase::lengthStatus creditTransferSettingsBase::checkRecipientLength(const QString& name) const
{
    const int length = name.length();
    if (length > _recipientNameLength)
        return tooLong;
    else if ( length == 0 || length < _recipientNameMinLength)
        return tooShort;
    return ok;
}

bool creditTransferSettingsBase::checkNameCharset(const QString& name) const
{
  return checkCharset(name, _allowedChars);
}

bool creditTransferSettingsBase::checkRecipientCharset(const QString& name) const
{
  return checkCharset(name, _allowedChars);
}
