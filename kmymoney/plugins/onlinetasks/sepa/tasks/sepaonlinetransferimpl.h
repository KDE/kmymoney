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

  QString responsibleAccount() const {
    return _originAccount;
  }
  void setOriginAccount(const QString& accountId);

  MyMoneyMoney value() const {
    return _value;
  }
  virtual void setValue(MyMoneyMoney value) {
    _value = value;
  }

  virtual void setBeneficiary(const payeeIdentifiers::ibanBic& accountIdentifier) {
    _beneficiaryAccount = accountIdentifier;
  };
  virtual payeeIdentifier beneficiary() const {
    return payeeIdentifier(_beneficiaryAccount.clone());
  }
  virtual payeeIdentifiers::ibanBic beneficiaryTyped() const {
    return _beneficiaryAccount;
  }

  virtual void setPurpose(const QString purpose) {
    _purpose = purpose;
  }
  QString purpose() const {
    return _purpose;
  }

  virtual void setEndToEndReference(const QString& reference) {
    _endToEndReference = reference;
  }
  QString endToEndReference() const {
    return _endToEndReference;
  }

  payeeIdentifier originAccountIdentifier() const;

  MyMoneySecurity currency() const;

  bool isValid() const;

  QString jobTypeName() const;
  virtual QString storagePluginIid() const {
    return sepaStoragePlugin::iid;
  }
  virtual bool sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual bool sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual bool sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const;

  unsigned short int textKey() const {
    return _textKey;
  }
  unsigned short int subTextKey() const {
    return _subTextKey;
  }

  virtual bool hasReferenceTo(const QString& id) const;

  QSharedPointer<const sepaOnlineTransfer::settings> getSettings() const;

protected:
  sepaOnlineTransfer* clone() const;

  virtual sepaOnlineTransfer* createFromXml(const QDomElement &element) const;
  virtual onlineTask* createFromSqlDatabase(QSqlDatabase connection, const QString& onlineJobId) const;
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;

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
