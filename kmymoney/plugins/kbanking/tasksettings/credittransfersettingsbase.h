/*
 * This file is part of KMyMoney, A Personal Finance Manager by KDE
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

#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"

/**
 * @brief Base class for sepaCreditTransfer and germanCreditTransfer settings
 *
 * @internal Both credit transfers have similar fields
 */
class creditTransferSettingsBase : public sepaOnlineTransfer::settings
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
      m_endToEndReferenceLength(0),
      _allowedChars(QString("")) {}

  // Limits getter
  int purposeMaxLines() const {
    return _purposeMaxLines;
  }
  int purposeLineLength() const {
    return _purposeLineLength;
  }
  int purposeMinLength() const {
    return _purposeMinLength;
  }

  int recipientNameLineLength() const {
    return _recipientNameLength;
  }
  int recipientNameMinLength() const {
    return _recipientNameMinLength;
  }

  int payeeNameLineLength() const {
    return _payeeNameLength;
  }
  int payeeNameMinLength() const {
    return _payeeNameMinLength;
  }

  QString allowedChars() const {
    return _allowedChars;
  }

  virtual int endToEndReferenceLength() const {
    return m_endToEndReferenceLength;
  }

  // Checker
  bool checkPurposeCharset(const QString& string) const;
  bool checkPurposeLineLength(const QString& purpose) const;
  validators::lengthStatus checkPurposeLength(const QString& purpose) const;
  bool checkPurposeMaxLines(const QString& purpose) const;

  validators::lengthStatus checkNameLength(const QString& name) const;
  bool checkNameCharset(const QString& name) const;

  validators::lengthStatus checkRecipientLength(const QString& name) const;
  bool checkRecipientCharset(const QString& name) const;

  virtual validators::lengthStatus checkEndToEndReferenceLength(const QString& reference) const;

  virtual bool checkRecipientBic(const QString& bic) const;

  /**
   * @brief Checks if the bic is mandatory for the given iban
   *
   * For the check usually only the first two chars are needed. So you do not
   * need to validate the IBAN.
   *
   * There is no need to format fromIban or toIban in any way (it is trimmed automatically).
   */
  virtual bool isBicMandatory(const QString& fromIban, const QString& toIban) const;

  validators::lengthStatus checkRecipientAccountNumber(const QString& accountNumber) const;

  validators::lengthStatus checkRecipientBankCode(const QString& bankCode) const;

  // Limits setter
  void setEndToEndReferenceLength(const int& length) {
    m_endToEndReferenceLength = length;
  }

  void setPurposeLimits(const int& lines, const int& lineLength, const int& minLength) {
    _purposeMaxLines = lines;
    _purposeLineLength = lineLength;
    _purposeMinLength = minLength;
  }

  void setRecipientNameLimits(const int& lines, const int& lineLength, const int& minLength) {
    _recipientNameMaxLines = lines;
    _recipientNameLength = lineLength;
    _recipientNameMinLength = minLength;
  }

  void setPayeeNameLimits(const int& lines, const int& lineLength, const int& minLength) {
    _payeeNameMaxLines = lines;
    _payeeNameLength = lineLength;
    _payeeNameMinLength = minLength;
  }

  void setAllowedChars(QString characters) {
    _allowedChars = characters;
  }

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

  /** @brief Number of chars allowed for sepa reference */
  int m_endToEndReferenceLength;
};

#endif // CREDITTRANSFERSETTINGSBASE_H
