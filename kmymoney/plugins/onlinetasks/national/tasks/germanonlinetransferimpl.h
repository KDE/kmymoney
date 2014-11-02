/*
  This file is part of KMyMoney, A Personal Finance Manager for KDE
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


#ifndef GERMANONLINETRANSFERIMPL_H
#define GERMANONLINETRANSFERIMPL_H

#include "onlinetransfer.h"

#include <QtCore/QSharedPointer>

#include "kmm_mymoney_export.h"
#include "germanonlinetransfer.h"
#include "../nationalstorageplugin.h"

/**
 * @brief Online Banking national transfer
 *
 * Right now this is a mix of national transfer and national german transfer
 */
class germanOnlineTransferImpl : public germanOnlineTransfer
{
  KMM_MYMONEY_UNIT_TESTABLE
  Q_INTERFACES(creditTransfer);

public:

  germanOnlineTransferImpl();
  germanOnlineTransferImpl(const germanOnlineTransferImpl &other );

  QString responsibleAccount() const { return _originAccount; };
  void setOriginAccount( const QString& accountId );

  MyMoneyMoney value() const { return _value; }
  virtual void setValue(MyMoneyMoney value) { _value = value; }

  void setBeneficiary ( const payeeIdentifiers::nationalAccount& accountIdentifier ) { _beneficiaryAccount = accountIdentifier; }
  payeeIdentifier beneficiary() const { return payeeIdentifier(_beneficiaryAccount.clone()); }
  virtual payeeIdentifiers::nationalAccount beneficiaryTyped() const { return _beneficiaryAccount; };

  virtual void setPurpose( const QString purpose ) { _purpose = purpose; }
  QString purpose() const { return _purpose; }

  virtual QString jobTypeName() const { return creditTransfer::jobTypeName(); }
  virtual QString storagePluginIid() const { return nationalStoragePlugin::iid; }

  virtual bool sqlSave(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual bool sqlModify(QSqlDatabase databaseConnection, const QString& onlineJobId) const;
  virtual bool sqlRemove(QSqlDatabase databaseConnection, const QString& onlineJobId) const;

  /**
   * @brief Returns the origin account identifier
   * @return you are owner of the object
   */
  payeeIdentifier originAccountIdentifier() const;

  /**
   * National account can handle the currency of the related account only.
   */
  MyMoneySecurity currency() const;

  bool isValid() const;

  germanOnlineTransfer* clone() const;

  unsigned short int textKey() const { return _textKey; }
  unsigned short int subTextKey() const { return _subTextKey; }

  QSharedPointer<const germanOnlineTransfer::settings> getSettings() const;

  virtual bool hasReferenceTo(const QString& id) const;

protected:
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;
  virtual germanOnlineTransferImpl* createFromXml(const QDomElement &element) const;

private:
  mutable QSharedPointer<const settings> _settings;
  MyMoneyMoney _value;
  QString _purpose;

  QString _originAccount;
  payeeIdentifiers::nationalAccount _beneficiaryAccount;

  unsigned short int _textKey;
  unsigned short int _subTextKey;

};

#endif // GERMANONLINETRANSFERIMPL_H
