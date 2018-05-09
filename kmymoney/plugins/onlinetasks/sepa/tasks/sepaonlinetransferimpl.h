/*
  This file is part of KMyMoney, A Personal Finance Manager by KDE
  Copyright (C) 2013 Christian Dávid <christian-david@web.de>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SEPAONLINETRANSFERIMPL_H
#define SEPAONLINETRANSFERIMPL_H

#include "sepaonlinetransfer.h"
#include "../sepastorageplugin.h"

/**
 * @brief SEPA Credit Transfer
 */
class sepaOnlineTransferImpl : public sepaOnlineTransfer
{
  Q_INTERFACES(sepaOnlineTransfer)

public:
  ONLINETASK_META(sepaOnlineTransfer, "org.kmymoney.creditTransfer.sepa");
  sepaOnlineTransferImpl();
  sepaOnlineTransferImpl(const sepaOnlineTransferImpl &other);

  QString responsibleAccount() const final override {
    return _originAccount;
  }
  void setOriginAccount(const QString& accountId) final override;

  MyMoneyMoney value() const final override{
    return _value;
  }
  void setValue(MyMoneyMoney value) final override {
    _value = value;
  }

  void setBeneficiary(const payeeIdentifiers::ibanBic& accountIdentifier) final override {
    _beneficiaryAccount = accountIdentifier;
  }
  payeeIdentifier beneficiary() const final override {
    return payeeIdentifier(_beneficiaryAccount.clone());
  }
  payeeIdentifiers::ibanBic beneficiaryTyped() const final override {
    return _beneficiaryAccount;
  }

  void setPurpose(const QString purpose) final override {
    _purpose = purpose;
  }
  QString purpose() const final override {
    return _purpose;
  }

  void setEndToEndReference(const QString& reference) final override {
    _endToEndReference = reference;
  }
  QString endToEndReference() const final override {
    return _endToEndReference;
  }

  payeeIdentifier originAccountIdentifier() const final override;

  MyMoneySecurity currency() const final override;

  bool isValid() const final override;

  QString jobTypeName() const final override;
  QString storagePluginIid() const final override {
    return sepaStoragePlugin::iid;
  }
  bool sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const final override;
  bool sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const final override;
  bool sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const final override;

  unsigned short int textKey() const final override {
    return _textKey;
  }
  unsigned short int subTextKey() const final override {
    return _subTextKey;
  }

  bool hasReferenceTo(const QString& id) const final override;

  QSharedPointer<const sepaOnlineTransfer::settings> getSettings() const final override;

protected:
  sepaOnlineTransfer* clone() const final override;

  sepaOnlineTransfer* createFromXml(const QDomElement &element) const final override;
  onlineTask* createFromSqlDatabase(QSqlDatabase connection, const QString& onlineJobId) const final override;
  void writeXML(QDomDocument& document, QDomElement& parent) const final override;

private:
  void bindValuesToQuery(QSqlQuery& query, const QString& id) const;

  mutable QSharedPointer<const settings> _settings;

  QString _originAccount;
  MyMoneyMoney _value;
  QString _purpose;
  QString _endToEndReference;

  payeeIdentifiers::ibanBic _beneficiaryAccount;

  unsigned short int _textKey;
  unsigned short int _subTextKey;
};

#endif // SEPAONLINETRANSFERIMPL_H
