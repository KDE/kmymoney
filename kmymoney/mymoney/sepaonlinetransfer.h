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

#ifndef SEPAONLINETRANSFER_H
#define SEPAONLINETRANSFER_H

#include "onlinetransfer.h"

#include <algorithm>

#include <klocalizedstring.h>

#include "swiftaccountidentifier.h"
#include "credittransfersettingsbase.h"

/**
 * @brief SEPA Credit Transfer
 */
class KMM_MYMONEY_EXPORT sepaOnlineTransfer : public onlineTransfer
{
public:
  ONLINETASK_META(sepaOnlineTransfer, "org.kmymoney.creditTransfer.sepa");
  sepaOnlineTransfer();
  sepaOnlineTransfer(const sepaOnlineTransfer &other );

  QString responsibleAccount() const { return _originAccount; }
  void setOriginAccount( const QString& accountId );
  
  MyMoneyMoney value() const { return _value; }
  virtual void setValue(MyMoneyMoney value) { _value = value; }

  void setRecipient ( const swiftAccountIdentifier& accountIdentifier ) { _remoteAccount = accountIdentifier; }
  const sepaAccountIdentifier& getRecipient() const { return _remoteAccount; }

  virtual void setPurpose( const QString purpose ) { _purpose = purpose; }
  QString purpose() const { return _purpose; }

  virtual void setEndToEndReference( const QString& reference ) { _endToEndReference = reference; }
  QString endToEndReference() const { return _endToEndReference; }

  /**
   * @brief Returns the origin account identifier
   * @return you are owner of the object
   */
  sepaAccountIdentifier* originAccountIdentifier() const;

  /**
   * National account can handle the currency of the related account only.
   */
  MyMoneySecurity currency() const;

  bool isValid() const;

  QString jobTypeName() const { return i18n("SEPA Credit Transfer"); }

  unsigned short int textKey() const { return _textKey; }
  unsigned short int subTextKey() const { return _subTextKey; }
  
  virtual bool hasReferenceTo(const QString& id) const;

  class settings : public creditTransferSettingsBase
  {
  public:
    int endToEndReferenceLength() const { return m_endToEndReferenceLength; }
    void setEndToEndReferenceLength(const int& length) { m_endToEndReferenceLength = length; }
    lengthStatus checkEndToEndReferenceLength(const QString& reference) const;
    static bool isIbanValid( const QString& iban );
    
    bool checkRecipientBic( const QString& bic ) const
    {
      const int length = bic.length();
      for (int i = 0; i < std::min(length, 6); ++i) {
        if ( !bic.at(i).isLetter() )
          return false;
      }
      for (int i = 6; i < length; ++i) {
        if ( !bic.at(i).isLetterOrNumber() )
          return false;
      }
   
      if (length == 11 || length == 8)
        return true;
      return false;
    }
    
    /**
     * @brief Checks if the bic is mandatory for the given iban
     * 
     * For the check usually only the first two chars are needed. So you do not
     * need to validate the IBAN.
     * 
     * @todo LOW: Implement, should be simple to test: if the country code in iban is the same as in origin iban and
     * the iban belongs to a sepa country a bic is not necessary. Will change 1. Feb 2016.
     */
    virtual bool isBicMandatory( const QString& iban ) const { Q_UNUSED(iban); return true; }
    
  private:
    /** @brief Number of chars allowed for sepa reference */
    int m_endToEndReferenceLength;
  };

  QSharedPointer<const settings> getSettings() const;
  
protected:
  sepaOnlineTransfer* clone() const;

  virtual convertType canConvertInto( const QString& onlineTaskName ) const;
  virtual convertType canConvert( const QString& onlineTaskName ) const;
  virtual onlineTask* convertInto( const QString& onlineTaskName, QString& messageString, bool& payeeChanged ) const;
  virtual void convert( const onlineTask&, QString& messageString, bool& payeeChanged );
  
  virtual sepaOnlineTransfer* createFromXml(const QDomElement &element) const;
  virtual void writeXML(QDomDocument& document, QDomElement& parent) const;

private:
  mutable QSharedPointer<const settings> _settings;
  
  QString _originAccount;
  MyMoneyMoney _value;
  QString _purpose;
  QString _endToEndReference;

  sepaAccountIdentifier _remoteAccount;

  unsigned short int _textKey;
  unsigned short int _subTextKey;
};

#endif // SEPAONLINETRANSFER_H
