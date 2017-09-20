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

#ifndef SEPACREDITTRANSFERSETTINGSMOCKUP_H
#define SEPACREDITTRANSFERSETTINGSMOCKUP_H

#include "onlinetasks/sepa/tasks/sepaonlinetransfer.h"

class sepaCreditTransferSettingsMockup : public sepaOnlineTransfer::settings
{
public:
  virtual bool checkRecipientCharset(const QString&) const override {
    return true;
  }

  virtual validators::lengthStatus checkRecipientLength(const QString&) const override {
    return validators::ok;
  }

  virtual bool checkNameCharset(const QString&) const override {
    return true;
  }

  virtual validators::lengthStatus checkNameLength(const QString&) const override {
    return validators::ok;
  }

  virtual bool checkPurposeMaxLines(const QString&) const override {
    return true;
  }

  virtual validators::lengthStatus checkPurposeLength(const QString&) const override {
    return validators::ok;
  }

  virtual bool checkPurposeLineLength(const QString&) const override {
    return true;
  }

  virtual bool checkPurposeCharset(const QString&) const override {
    return true;
  }

  virtual QString allowedChars() const override {
    return QLatin1String("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\"'_-{}()[]~\\/^!?+<>=&:@., ");
  }

  virtual int payeeNameMinLength() const override {
    return 0;
  }

  virtual int payeeNameLineLength() const override {
    return 27;
  }

  virtual int recipientNameMinLength() const override {
    return 0;
  }

  virtual int recipientNameLineLength() const override {
    return 27;
  }

  virtual int purposeMinLength() const override {
    return 0;
  }

  virtual int purposeLineLength() const override {
    return 27;
  }

  virtual int purposeMaxLines() const override {
    return 50;
  }

  virtual validators::lengthStatus checkEndToEndReferenceLength(const QString&) const override {
    return validators::lengthStatus::ok;
  }

  virtual bool checkRecipientBic(const QString&) const override {
    return true;
  }

  virtual int endToEndReferenceLength() const override {
    return 27;
  }

  virtual bool isBicMandatory(const QString&, const QString&) const override {
    return false;
  }
};

#endif // SEPACREDITTRANSFERSETTINGSMOCKUP_H
