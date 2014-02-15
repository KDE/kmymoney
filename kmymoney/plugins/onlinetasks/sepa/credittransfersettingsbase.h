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

#ifndef CREDITTRANSFERSETTINGSBASE_H
#define CREDITTRANSFERSETTINGSBASE_H

#include "onlinetasks/interfaces/tasks/onlinetask.h"

class QValidator;

/**
 * @brief Base class for sepaCreditTransfer and germanCreditTransfer settings
 * 
 * @internal Both credit transfers have similar fields
 */
class KMM_MYMONEY_EXPORT creditTransferSettingsBase : public onlineTask::settings
{
public:
  creditTransferSettingsBase()
  : _purposeMaxLines(0),
  _purposeLineLength(0),
  _purposeMinLength(0),
  _recipientNameMaxLines(0),
  _recipientNameLength(0),
  _recipientNameMinLength(0),
  _payeeNameMaxLines(0),
  _payeeNameLength(0),
  _payeeNameMinLength(0),
  _allowedChars( QString("") )
  {}

  // Limits getter
  int purposeMaxLines() const { return _purposeMaxLines; }
  int purposeLineLength() const { return _purposeLineLength; }
  int purposeMinLength() const { return _purposeMinLength; }

  int recipientNameLineLength() const { return _recipientNameLength; }
  int recipientNameMinLength() const { return _recipientNameMinLength; }

  int payeeNameLineLength() const { return _payeeNameLength; }
  int payeeNameMinLength() const { return _payeeNameMinLength; }

  QString allowedChars() const { return _allowedChars; }

  // Checker
  enum lengthStatus {
    ok = 0,
    tooShort  = -1,
    tooLong = 1
  };

  bool checkPurposeCharset( const QString& string ) const;
  bool checkPurposeLineLength(const QString& purpose) const;
  lengthStatus checkPurposeLength(const QString& purpose) const;
  bool checkPurposeMaxLines(const QString& purpose) const;

  lengthStatus checkNameLength(const QString& name) const;
  bool checkNameCharset( const QString& name ) const;

  lengthStatus checkRecipientLength(const QString& name) const;
  bool checkRecipientCharset( const QString& name ) const;

  // Limits setter
  void setPurposeLimits( const int& lines, const int& lineLength, const int& minLength )
  {
    _purposeMaxLines = lines;
    _purposeLineLength = lineLength;
    _purposeMinLength = minLength;
  }

  void setRecipientNameLimits( const int& lines, const int& lineLength, const int& minLength )
  {
    _recipientNameMaxLines = lines;
    _recipientNameLength = lineLength;
    _recipientNameMinLength = minLength;
  }

  void setPayeeNameLimits( const int& lines, const int& lineLength, const int& minLength )
  {
    _payeeNameMaxLines = lines;
    _payeeNameLength = lineLength;
    _payeeNameMinLength = minLength;
  }

  void setAllowedChars(QString characters)
  {
    _allowedChars = characters;
  }

protected:
  /** @brief checks if all lines in text are shorter than length */
  static bool checkLineLength( const QString& text, const int& length );

  /** @brief checks if text uses only charactes in allowedChars */
  static bool checkCharset( const QString& text, const QString& allowedChars );

private:

  /** @brief number of lines allowed in purpose */
  int _purposeMaxLines;
  /** @brief number of chars allowed in each purpose line */
  int _purposeLineLength;
  /** @brief Minimal number of chars needed for purpose */
  int _purposeMinLength;

  /** @brief number of lines allowed for recipient name */
  int _recipientNameMaxLines;
  /** @brief number of chars allowed in each recipient line */
  int _recipientNameLength;
  /** @brief Minimal number of chars needed as recipient name */
  int _recipientNameMinLength;

  /** @brief number of lines allowed for payee name */
  int _payeeNameMaxLines;
  /** @brief number of chars allowed in each payee line */
  int _payeeNameLength;
  /** @brief Minibal number of chars for payee name */
  int _payeeNameMinLength;

  /** @brief characters allowd in purpose and recipient name */
  QString _allowedChars;
};

#endif // CREDITTRANSFERSETTINGSBASE_H
