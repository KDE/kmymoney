/*
    SPDX-FileCopyrightText: 2014 Christian Dávid <christian-david@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEPACREDITTRANSFERSETTINGSMOCKUP_H
#define SEPACREDITTRANSFERSETTINGSMOCKUP_H

#include "onlinetasks/sepa/sepaonlinetransfer.h"

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
