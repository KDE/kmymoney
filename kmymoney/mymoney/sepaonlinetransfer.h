#ifndef SEPAONLINETRANSFER_H
#define SEPAONLINETRANSFER_H

#include "onlinetransfer.h"

#include <klocalizedstring.h>

#include "swiftaccountidentifier.h"

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

  virtual void setReference( const QString& reference ) { _reference = reference; }
  QString reference() const { return _reference; }

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

  class settings : public onlineTask::settings
  {
  public:
    /** @brief number of lines allowed in purpose */
    int purposeMaxLines;
    /** @brief number of chars allowed in each purpose line */
    int purposeLineLength;
    /** @brief number of lines allowed for recipient name */
    int recipientNameMaxLines;
    /** @brief number of chars allowed in each recipient line */
    int recipientNameLength;
    /** @brief number of lines allowed for payee name */
    int payeeNameMaxLines;
    /** @brief number of chars allowed in each payee line */
    int payeeNameLength;
    /** @brief Number of chars allowed for sepa reference */
    int referenceLength;
  };

  QSharedPointer<const settings> getSettings() const;
  
protected:
  sepaOnlineTransfer* clone() const;

  virtual convertType canConvertInto( const QString& onlineTaskName ) const;
  virtual convertType canConvert( const QString& onlineTaskName ) const;
  virtual onlineTask* convertInto( const QString& onlineTaskName, QString& messageString, bool& payeeChanged ) const;
  virtual void convert( const onlineTask&, QString& messageString, bool& payeeChanged );

private:
  mutable QSharedPointer<const settings> _settings;
  
  QString _originAccount;
  MyMoneyMoney _value;
  QString _purpose;
  QString _reference;

  sepaAccountIdentifier _remoteAccount;

  unsigned short int _textKey;
  unsigned short int _subTextKey;
};

#endif // SEPAONLINETRANSFER_H
