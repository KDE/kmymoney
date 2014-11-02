/*
 * This file is part of KMyMoney, A Personal Finance Manager for KDE
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

#ifndef GERMANCREDITTRANSFERSETTINGSMOCKUP_H
#define GERMANCREDITTRANSFERSETTINGSMOCKUP_H

#include "onlinetasks/national/tasks/germanonlinetransfer.h"

class germanCreditTransferSettingsMockup : public germanOnlineTransfer::settings
{
public:
  virtual validators::lengthStatus checkRecipientBankCode(const QString& bankCode) const
  {
    return validators::ok;
  }

  virtual validators::lengthStatus checkRecipientAccountNumber(const QString& accountNumber) const
  {
    return validators::ok;
  }

  virtual bool checkRecipientCharset(const QString& name) const
  {
    return true;
  }

  virtual validators::lengthStatus checkRecipientLength(const QString& name) const
  {
    return validators::ok;
  }

  virtual bool checkNameCharset(const QString& name) const
  {
    return true;
  }

  virtual validators::lengthStatus checkNameLength(const QString& name) const
  {
    return validators::ok;
  }

  virtual bool checkPurposeMaxLines(const QString& purpose) const
  {
    return true;
  }

  virtual validators::lengthStatus checkPurposeLength(const QString& purpose) const
  {
    return validators::ok;
  }

  virtual bool checkPurposeLineLength(const QString& purpose) const
  {
    return true;
  }
  virtual bool checkPurposeCharset(const QString& string) const
  {
    return true;
  }

  virtual QString allowedChars() const
  {
    return QLatin1String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\"'_-{}()[]~\\/[^!?+<>=&:@");
  }

  virtual int payeeNameMinLength() const
  {
    return 0;
  }

  virtual int payeeNameLineLength() const
  {
    return 27;
  }

  virtual int recipientNameMinLength() const
  {
    return 0;
  }

  virtual int recipientNameLineLength() const
  {
    return 27;
  }

  virtual int purposeMinLength() const
  {
    return 0;
  }

  virtual int purposeLineLength() const
  {
    return 27;
  }

  virtual int purposeMaxLines() const
  {
    return 50;
  }
};

#endif // GERMANCREDITTRANSFERSETTINGSMOCKUP_H
