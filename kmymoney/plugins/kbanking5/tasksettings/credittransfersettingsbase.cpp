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

#include <QDebug>

validators::lengthStatus creditTransferSettingsBase::checkNameLength(const QString& name) const
{
  if (name.length() > _payeeNameLength)
    return validators::tooLong;
  else if (name.length() < _payeeNameMinLength)
    return validators::tooShort;
  return validators::ok;
}

bool creditTransferSettingsBase::checkPurposeMaxLines(const QString& purpose) const
{
  return (purpose.split('\n').count() <= _purposeMaxLines);
}

validators::lengthStatus creditTransferSettingsBase::checkPurposeLength(const QString& purpose) const
{
  const int length = purpose.length();
  if (length > (_purposeMaxLines*_purposeLineLength))
    return validators::tooLong;
  else if (length < _purposeMinLength)
    return validators::tooShort;
  return validators::ok;
}

bool creditTransferSettingsBase::checkPurposeLineLength(const QString& purpose) const
{
  return validators::checkLineLength(purpose, _purposeLineLength);
}

bool creditTransferSettingsBase::checkPurposeCharset(const QString& string) const
{
  const QString chars = _allowedChars + QChar('\n');
  return validators::checkCharset(string, chars);
}

validators::lengthStatus creditTransferSettingsBase::checkRecipientLength(const QString& name) const
{
  const int length = name.length();
  if (length > _recipientNameLength)
    return validators::tooLong;
  else if (length == 0 || length < _recipientNameMinLength)
    return validators::tooShort;
  return validators::ok;
}

bool creditTransferSettingsBase::checkNameCharset(const QString& name) const
{
  return validators::checkCharset(name, _allowedChars);
}

bool creditTransferSettingsBase::checkRecipientCharset(const QString& name) const
{
  return validators::checkCharset(name, _allowedChars);
}

bool creditTransferSettingsBase::isBicMandatory(const QString& payeeIban, const QString& beneficaryIban) const
{
  const QString payeeContryCode = payeeIban.trimmed().left(2);
  const QString beneficaryCountryCode = beneficaryIban.trimmed().left(2);

  /**
   * Data source for sepa participants:
   * @url http://www.europeanpaymentscouncil.eu/index.cfm/knowledge-bank/epc-documents/epc-list-of-sepa-scheme-countries/
   * EPC409-09
   * Version 2.0
   * Date issued: 20 January 2014
   */
  QStringList sepaParticipants;
  sepaParticipants << "FI" << "AT" << "PT" << "BE" << "BG" << "ES" << "HR" << "CY" << "CZ" << "DK" << "EE" << "FI"
  << "FR" << "DE" << "GI" << "GR" << "HU" << "IS" << "IE" << "IT" << "LV" << "LI" << "LT" << "LU" << "PT" << "MT" << "MC" << "NL"
  << "NO" << "PL" << "RO" << "SM" << "SK" << "SI" << "ES" << "SE" << "CH" << "GB";

  // Starting form 1st Febuary 2016 no bic is needed between sepa countries
  if (QDate::currentDate() >= QDate(2016, 2, 1))
    return (!sepaParticipants.contains(payeeContryCode, Qt::CaseInsensitive) || !sepaParticipants.contains(beneficaryCountryCode, Qt::CaseInsensitive));

  // Before that date the bic is needed except for transfers within a single sepa country
  return (payeeContryCode.compare(beneficaryCountryCode, Qt::CaseInsensitive) != 0 || !sepaParticipants.contains(payeeContryCode, Qt::CaseInsensitive));
}

bool creditTransferSettingsBase::checkRecipientBic(const QString& bic) const
{
  const int length = bic.length();
  for (int i = 0; i < std::min(length, 6); ++i) {
    if (!bic.at(i).isLetter())
      return false;
  }
  for (int i = 6; i < length; ++i) {
    if (!bic.at(i).isLetterOrNumber())
      return false;
  }

  if (length == 11 || length == 8)
    return true;
  return false;
}

validators::lengthStatus creditTransferSettingsBase::checkEndToEndReferenceLength(const QString& reference) const
{
  if (reference.length() > m_endToEndReferenceLength)
    return validators::tooLong;
  return validators::ok;
}

validators::lengthStatus creditTransferSettingsBase::checkRecipientAccountNumber(const QString& accountNumber) const
{
  const int length = accountNumber.length();
  if (length == 0)
    return validators::tooShort;
  else if (length > 10)
    return validators::tooLong;
  return validators::ok;
}

validators::lengthStatus creditTransferSettingsBase::checkRecipientBankCode(const QString& bankCode) const
{
  const int length = bankCode.length();
  if (length < 8)
    return validators::tooShort;
  else if (length > 8)
    return validators::tooLong;
  return validators::ok;
}
