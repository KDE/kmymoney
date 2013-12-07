#include "germanaccountidentifier.h"

#include <ktoblzcheck.h>
#include <string>

#include <QDebug>



/**
 * Uses ktoblzcheck to determine the bank name
 * @todo speed up by caching AccountNumberCheck
 *
 * @see http://ktoblzcheck.sourceforge.net/
 *
 * @internal Using std::string(QString().toLatin1()) because QT STL is probably deactivated
 */
QString germanAccountIdentifier::getBankName() const
{
  if( _bankCode.isEmpty() )
    return QString();

  AccountNumberCheck ktoblzcheck;
  AccountNumberCheck::Record bank;
  try {
    bank = ktoblzcheck.findBank(std::string(_bankCode.toLatin1()));
  } catch (int) {
    // Exception sent if bank not found
    return QString();
  }
  QString bankName = QString("%1, %2").arg(QString::fromLatin1(bank.bankName.c_str()))
  .arg(QString::fromLatin1(bank.location.c_str()));
  return bankName;
}

/**
 * Hard rules for german account identifiers:
 *
 * German bank codes are exactly 8 digits long,
 * and account numbers are up to 10 digits long.
 *
 * @todo Check charset of _ownerName
 * @todo It should be safe to assume ktoblzcheck knows all valid bank codes
 */
bool germanAccountIdentifier::isValid() const
{
  bool accountNumberIsNumber = false;
  bool bankCodeIsNumber = false;
  _accountNumber.toInt(&accountNumberIsNumber);
  _bankCode.toInt(&bankCodeIsNumber);

  if ( bankAccountIdentifier::isValid() && accountNumberIsNumber && bankCodeIsNumber
    && _bankCode.length() == 8 && _accountNumber.length() > 1 && _accountNumber.length() <= 10
  ) {
    return true;
  }
  return false;
}

/** @todo Use three way logic? Invalid (show error to user) - probably invalid (warn user) - valid (ok) */
bool germanAccountIdentifier::checkAccountNumber() const
{
  // must validate to hard limits first (should be very fast)
  if (!isValid())
    return false;

  AccountNumberCheck ktoblzcheck;
  AccountNumberCheck::Result result = ktoblzcheck.check(std::string(_bankCode.toLatin1()),
                                                        std::string(_accountNumber.toLatin1()));
  if (result == AccountNumberCheck::OK || result == AccountNumberCheck::UNKNOWN)
    return true;

  return false;
}

QValidator::State germanAccountNumberValidator::validate(QString& input, int& pos) const
{
  Q_UNUSED(pos);

  if (input.isEmpty())
    return Intermediate;

  if (input.length() > 10)
    return Invalid;

  for(int i = input.size()-1; i >= 0; i--) {
    if (!input.at(i).isDigit())
      return Invalid;
  }

  if (input.length() >= 1 && input.length() <= 10)
    return Acceptable;

  return Intermediate;
}

QValidator::State germanBankCodeValidator::validate(QString& input, int& pos) const
{
  Q_UNUSED(pos);

  if (input.isEmpty())
    return Intermediate;

  bool bankCodeNumberIsNumber = false;
  input.toInt(&bankCodeNumberIsNumber);

  if (!bankCodeNumberIsNumber || input.length() > 8)
    return Invalid;

  if (input.length() == 8)
    return Acceptable;

  return Intermediate;
}
