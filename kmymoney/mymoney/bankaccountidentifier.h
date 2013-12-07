#ifndef BANKACCOUNTIDENTIFIER_H
#define BANKACCOUNTIDENTIFIER_H

#include <QtCore/QObject>
#include "accountidentifier.h"

/**
 * @brief Bank Account Number and Bank Code
 *
 * This identifies an acoount at a bank. They exist of three components
 * 1) Owner name
 * 2) An account number
 * 3) a bank identifier (even if this is optional)
 *
 * National account numbers and bank codes are usually numbers with a country depending length.
 *
 * @todo Add checkAccountNumber()
 */
class bankAccountIdentifier : public accountIdentifier
{
public:
  bankAccountIdentifier()
      : accountIdentifier(),
        _accountNumber(QString()),
        _bankCode(QString())
    {}

  bankAccountIdentifier( const bankAccountIdentifier& identifier )
      :  accountIdentifier(identifier),
        _accountNumber(identifier._accountNumber),
        _bankCode(identifier._bankCode)
    {}

  /** @brief Sets National Account Number */
  void setAccountNumber( const QString& accountNumber ) { _accountNumber = accountNumber; }
  /**
   * @brief National Code identifing the bank
   * e.g. the Bankleitzahl (BLZ) in Germany or Bankclearing-Number (BC-Number) in Swiss
   */
  void setBankCode ( const QString& bankCode ) { _bankCode = bankCode; }

  QString accountNumber() const { return _accountNumber; }
  QString bankCode() const { return _bankCode; }

  virtual QString getBankName() const { return QString(); }
  virtual bool isValid() const { return (!_ownerName.isEmpty() && !_accountNumber.isEmpty() && !_bankCode.isEmpty()); }

protected:
  QString _accountNumber;
  QString _bankCode;
};

#endif // BANKACCOUNTIDENTIFIER_H
