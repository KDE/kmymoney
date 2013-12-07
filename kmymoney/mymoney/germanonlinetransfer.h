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


#ifndef GERMANONLINETRANSFER_H
#define GERMANONLINETRANSFER_H

#include "onlinetransfer.h"

#include <QtCore/QSharedPointer>

#include "germanaccountidentifier.h"

#include "kmm_mymoney_export.h"

class QRegExpValidator;

/**
 * @brief Online Banking national transfer
 *
 * Right now this is a mix of national transfer and national german transfer
 */
class KMM_MYMONEY_EXPORT germanOnlineTransfer : public onlineTransfer
{
public:
  ONLINETASK_META(germanOnlineTransfer, "org.kmymoney.creditTransfer.germany");

  germanOnlineTransfer();
  germanOnlineTransfer(const germanOnlineTransfer &other );

  QString responsibleAccount() const { return _originAccount; };
  void setOriginAccount( const QString& accountId );
  
  MyMoneyMoney value() const { return _value; }
  virtual void setValue(MyMoneyMoney value) { _value = value; }

  void setRecipient ( const germanAccountIdentifier& accountIdentifier ) { _remoteAccount = accountIdentifier; }
  const germanAccountIdentifier& getRecipient() const { return _remoteAccount; }

  virtual void setPurpose( const QString purpose ) { _purpose = purpose; }
  QString purpose() const { return _purpose; }

  /**
   * @brief Returns the origin account identifier
   * @return you are owner of the object
   */
  germanAccountIdentifier* originAccountIdentifier() const;

  /**
   * National account can handle the currency of the related account only.
   */
  MyMoneySecurity currency() const;

  bool isValid() const;

  germanOnlineTransfer* clone() const;

  unsigned short int textKey() const { return _textKey; }
  unsigned short int subTextKey() const { return _subTextKey; }

  class settings : public onlineTask::settings
  {
  public:
    settings()
      : _purposeMaxLines(0),
        _purposeLineLength(0),
        _recipientNameMaxLines(0),
        _recipientNameLength(0),
        _payeeNameMaxLines(0),
        _payeeNameLength(0)
    {}

    // Limits getter
    int purposeMaxLines() const { return _purposeMaxLines; }
    int purposeLineLength() const { return _purposeLineLength; }
    int recipientNameLineLength() const { return _recipientNameLength; }
    int payeeNameLineLength() const { return _payeeNameLength; }

    QValidator* purposeValidator( QObject *parent = 0 ) const;
    QValidator* recipientNameValidator( QObject *parent = 0 ) const;
    QValidator* payeeNameValidator( QObject *parent = 0 ) const;

    // Limits setter
    void setPurposeLimits( const int& lines, const int& lineLength )
    {
      _purposeMaxLines = lines;
      _purposeLineLength = lineLength;
    }

    void setRecipientNameLimits( const int& lines, const int& lineLength )
    {
      _recipientNameMaxLines = lines;
      _recipientNameLength = lineLength;
    }

    void setPayeeNameLimits( const int& lines, const int& lineLength )
    {
      _payeeNameMaxLines = lines;
      _payeeNameLength = lineLength;
    }

  private:

    /** @brief number of lines allowed in purpose */
    int _purposeMaxLines;
    /** @brief number of chars allowed in each purpose line */
    int _purposeLineLength;
    /** @brief number of lines allowed for recipient name */
    int _recipientNameMaxLines;
    /** @brief number of chars allowed in each recipient line */
    int _recipientNameLength;
    /** @brief number of lines allowed for payee name */
    int _payeeNameMaxLines;
    /** @brief number of chars allowed in each payee line */
    int _payeeNameLength;
  };

  QSharedPointer<const settings> getSettings() const;

protected:
  virtual convertType canConvertInto( const QString& onlineTaskName ) const;
  virtual convertType canConvert( const QString& onlineTaskName ) const;
  virtual onlineTask* convertInto( const QString& onlineTaskName, QString& messageString, bool& payeeChanged ) const;
  virtual void convert( const onlineTask&, QString& messageString, bool& payeeChanged );
  
private:
  mutable QSharedPointer<const settings> _settings;
  MyMoneyMoney _value;
  QString _purpose;

  QString _originAccount;
  germanAccountIdentifier _remoteAccount;
  
  unsigned short int _textKey;
  unsigned short int _subTextKey;

};

#endif // GERMANONLINETRANSFER_H
